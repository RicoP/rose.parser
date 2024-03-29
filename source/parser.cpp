#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <stdarg.h>
#include <stdio.h>
#include "parser.h"

#include <rose/hash.h>
#include <rose/unused.h>
#include <rose/streambuffer.h>
#include <serializer/serializer.h>
#include <serializer/jsonserializer.h>

#include <windows.h>

#define IMPL_SERIALIZER
#include "parser_serializer.h"

#define ENDL "\n"
#define WHITESPACE " \n\r\t"

inline bool is_empty(const char * c) { return *c == 0; }
inline bool is_equal(const char * a, const char * b) {
  for(;;) {
    if (*a != *b) return false;
    if (*a == 0) return true;
    ++a;
    ++b;
  }
}

bool is_valid_operator(const char * op) {
  //https://www.ibm.com/docs/en/zos/2.1.0?topic=only-overloading-operators
  switch (rose::hash(op)) {
  case rose::hash("operator!"):
  case rose::hash("operator!="):
  case rose::hash("operator%"):
  case rose::hash("operator%="):
  case rose::hash("operator&"):
  case rose::hash("operator&&"):
  case rose::hash("operator&="):
  case rose::hash("operator*"):
  case rose::hash("operator*="):
  case rose::hash("operator+"):
  case rose::hash("operator++"):
  case rose::hash("operator+="):
  case rose::hash("operator,"):
  case rose::hash("operator-"):
  case rose::hash("operator--"):
  case rose::hash("operator-="):
  case rose::hash("operator->"):
  case rose::hash("operator->*"):
  case rose::hash("operator<"):
  case rose::hash("operator<<"):
  case rose::hash("operator<<="):
  case rose::hash("operator<="):
  case rose::hash("operator="):
  case rose::hash("operator=="):
  case rose::hash("operator>"):
  case rose::hash("operator>="):
  case rose::hash("operator>>"):
  case rose::hash("operator>>="):
  case rose::hash("operator|"):
  case rose::hash("operator|="):
  case rose::hash("operator||"):
  case rose::hash("operator~"):
  case rose::hash("operator/"):
  case rose::hash("operator/="):
    return true;
  default:
    return false;
  }
}

template<size_t N>
void copy(char(&dst)[N], const char * src) {
  for (size_t i = 0; i != N; ++i) {
    dst[i] = src[i];
    if (dst[i] == 0) break;
  }
}

std::vector<const char *> input_files;

void error(const char * msg, rose::StreamBuffer & buffer) {
  char tmp[20] = "";
  buffer.sws_read_till(tmp, WHITESPACE);
  fprintf(stderr, "%s: %s(%i) [found '%s']" ENDL, msg, buffer.path, buffer.cursor_line, tmp);
  exit(1);
}

//str -> "str"
void quotify(char * str, size_t len, rose::StreamBuffer & buffer) {
  size_t l = std::strlen(str);
  if (l >= len - 2) error("string to long", buffer);
  str[l + 2] = 0;
  str[l + 1] = '\"';
  for (size_t i = l; i != 0; --i) {
    str[i] = str[i - 1];
  }
  str[0] = '\"';
}

template<size_t N>
void quotify(char(&str)[N], rose::StreamBuffer & buffer) {
  quotify(str, N, buffer);
}

int read_in_namespaces(char * buffer, int N, const std::vector<namespace_path> & namespaces, rose::StreamBuffer & sb) {
  char * p = buffer;
  int size = N;
  int len = 0;
  for (auto & ns : namespaces) {
    int s = std::snprintf(p, size, "%s::", ns.path);
    size -= s;
    len += s;
    if (size < 0) error("out of space.", sb);
    p += s;
  }
  return len;
}

template<size_t N>
int read_in_namespaces(char (&buffer)[N], const std::vector<namespace_path> & namespaces, rose::StreamBuffer & sb) {
  return read_in_namespaces(buffer, N, namespaces, sb);
}

void parse(ParseContext & ctx, rose::StreamBuffer & buffer) {
  auto skip_function_body = [&buffer]() {
    int depth = 0;
    do {
      //TODO: deal with {} in comments
      buffer.skip_till_any("{}");
      char c = buffer.get();
      depth += c == '{' ? 1 : -1;
      if (c == 0) break;
      assert(depth >= 0);
    } while (depth);
  };

  char tmp[64] = "";
  global_annotations_t global_annotation = global_annotations_t::NONE;
  bool is_in_imposter_comment = false;
  std::vector<namespace_path> namespaces;
  while (!buffer.eof) {
    ////////////////////////////////////////////////////////
    // COMMENTS                                           //
    ////////////////////////////////////////////////////////
    while (buffer.skip_comment()) {
      // TODO: we need a more flexible way to check for comments
    }

    ////////////////////////////////////////////////////////
    // GLOBAL ANNOTATION                                  //
    ////////////////////////////////////////////////////////
    char annotation_s[64];
    bool has_annotation = buffer.test_annotation(annotation_s);
    bool has_second_annotation = buffer.test_annotation(annotation_s);

    if (has_second_annotation) error("No support for more than one global annotations yet.", buffer);

    if (has_annotation) {
      quotify(annotation_s, buffer);
      JsonDeserializer jsond(annotation_s);
      rose::deserialize(global_annotation, jsond);

      if (global_annotation == global_annotations_t::Imposter) {
        bool ok = buffer.test_and_skip("/*");
        if (!ok) error("expects '/*' with Imposter annotation.", buffer);
        is_in_imposter_comment = true;
      }
    }

    if (is_in_imposter_comment) {
      if (buffer.test_and_skip("*/")) {
        global_annotation = global_annotations_t::NONE;
        is_in_imposter_comment = false;
        continue;
      }
    }

    ////////////////////////////////////////////////////////
    // MACROS                                             //
    ////////////////////////////////////////////////////////
    if (buffer.test_and_skip("#")) {
      //Macro
      buffer.read_till(tmp, WHITESPACE);
      switch (rose::hash(tmp)) { 
      case rose::hash("include"):
      case rose::hash("pragma"):
      case rose::hash("define"):
        buffer.skip_line();
        break;
      case rose::hash("ifdef"):
      case rose::hash("ifndef"):
      case rose::hash("if"):
        //skip all ifXXX macros.
        buffer.skip_line();
        for (;;) {
          if (buffer.sws_peek() == '#') {
            if (buffer.test("#endif")) {
              buffer.skip_line();
              break;
            }
          }
          buffer.skip_line();
        }

        break;
      default:
        error("unknown PP macro.", buffer);
        break;
      }
      continue;
    }

    ////////////////////////////////////////////////////////
    // NAMESPACE                                          //
    ////////////////////////////////////////////////////////
    if (buffer.test_and_skip("namespace ")) {
      namespace_path path;
      buffer.sws_read_till(path.path, "{" WHITESPACE);
      namespaces.push_back(path);
      buffer.test_and_skip("{");
      continue;
    }

    if (buffer.test_and_skip("}")) {
      if (namespaces.size() == 0)
        error("unexpected '}'", buffer);

      namespaces.pop_back();
      continue;
    }

    ////////////////////////////////////////////////////////
    // ENUMS                                              //
    ////////////////////////////////////////////////////////
    if (buffer.test_and_skip("enum ")) {
      if (buffer.test_and_skip("class ") || buffer.test_and_skip("struct ")) {
        if (global_annotation != global_annotations_t::NONE && global_annotation != global_annotations_t::Flag) {
          error("enum class annotation can't be anything other than 'Flag'", buffer);
        }
        enum_class_info & enumci = ctx.enum_classes.emplace_back();
        enumci.enum_annotations = global_annotation;
        global_annotation = global_annotations_t::NONE;

        enumci.namespaces = namespaces;
        int s = read_in_namespaces(enumci.name_withns, namespaces, buffer);
        buffer.sws_read_till(enumci.name_withoutns, "{:" WHITESPACE);
        std::strcpy(enumci.name_withns + s, enumci.name_withoutns); //append name to namespaces part
        char c = buffer.sws_get();

        if (c == ':') {
          enumci.custom_type = true;
          buffer.sws_read_till(enumci.type, "{");
          c = buffer.sws_get();
        }

        if (c != '{') error("Expected '{'", buffer);

        for (;;) {
          while (buffer.skip_comment())
          {
            //Skip the comments
          }

          c = buffer.sws_peek();
          if (c == '}') {
            buffer.skip(1);
            c = buffer.sws_get();
            if (c != ';') error("expect ';'", buffer);
            break;
          }

          enum_info & enumi = enumci.enums.emplace_back();
          buffer.sws_read_till(enumi.name, ",=}" WHITESPACE);
          c = buffer.sws_peek();
          if (c == '=') {
            buffer.skip(1);
            enumi.value_type = value_type_t::Set;
            buffer.sws_read_till(enumi.value, ",}");
            c = buffer.sws_peek();
          }
          if (c == ',') buffer.skip(1);
        }

        if (enumci.enums.size() > 0) {
          enumci.enums[0].value_type = value_type_t::Set;
        }

        assert(enumci.enums.size() != 0);
        enumci.default_value = enumci.enums[0];
      }
      else {
        error("expected 'class' after 'enum'.", buffer);
      }
      continue;
    }

    ////////////////////////////////////////////////////////
    // STRUCTS                                            //
    ////////////////////////////////////////////////////////
    if (buffer.test_and_skip("struct ")) {
      struct_info & structi = ctx.structs.emplace_back();

      structi.global_annotations = global_annotation;
      global_annotation = global_annotations_t::NONE;

      structi.namespaces = namespaces;
      int s = read_in_namespaces(structi.name_withns, namespaces, buffer);
      buffer.sws_read_till(structi.name_withoutns, ";{" WHITESPACE);
      std::strcpy(structi.name_withns + s, structi.name_withoutns);

      buffer.skip_ws();

      char c = buffer.get();
      if (c == '{') {
        for (;;) {
          member_annotations_t annotation = member_annotations_t::NONE;
          member_info ignored_member; //in case we want to ignore the member

          char annotation_s[64];
          if (buffer.test_annotation(annotation_s)) {
            quotify(annotation_s, buffer);
            JsonDeserializer jsond(annotation_s);
            rose::deserialize(annotation, jsond);
          }
          if (buffer.test_annotation(annotation_s)) {
            error("No support for double annotations yet.", buffer);
          }
          if (buffer.skip_comment()) {
            ; // we skiped the comment
          }

          member_info memberi;
          buffer.sws_read_till(memberi.type, "(" WHITESPACE);

          for (;;) {
              memberi.count = 1;
              memberi.kind = Member_info_kind::Field;
              memberi.annotations = annotation;
              buffer.sws_read_till(memberi.name, "([;," WHITESPACE);
              c = buffer.sws_get();

              bool member_chain = false;

              if (c == '(') {

                if (is_empty(memberi.name)) {
                    char * constrcutor_name = memberi.type;
                    // Empty name means we have a constrcutor / destructor
                    memberi.kind = Member_info_kind::Constructor;
                    if (constrcutor_name[0] == '~') {
                        memberi.kind = Member_info_kind::Destructor;
                        constrcutor_name++;
                    }
                    if (!is_equal(constrcutor_name, structi.name_withoutns)) {
                        // Sanity check
                        error("Constructor / Destructor must have same name", buffer);
                    }
                } else {
                    // function
                    memberi.kind = Member_info_kind::Function;
                }

                c = buffer.skip_till_any(";{(=");
                if (c == '=') {
                    // We don't care about assigned values for functions
                    // can be 0 or default or delete
                    c = buffer.skip_till_any(";");
                    buffer.skip(1);
                } else {
                    skip_function_body();
                }
              } else {
                  if (c == '[') {
                    buffer.sws_read_till(tmp, "]");
                    memberi.count = atoi(tmp);
                    buffer.skip(1);
                    c = buffer.sws_get();
                  }

                  member_chain = c == ','; //new member with same type and annotations
                  if (c == '=') {
                    buffer.sws_read_till(memberi.default_value, ";");
                    c = buffer.sws_get();
                  }
              }

              if (annotation != member_annotations_t::Ignore) {
                structi.members.push_back(memberi);
              }
              
              if (member_chain) continue;
              else break;
          }

          if (buffer.sws_peek() == '}') {
            buffer.skip(1);
            if (buffer.sws_get() != ';') error("Expected ';'", buffer);
            break;
          }
        }
      }
      c = buffer.peek();
      if (c == '\r' || c == '\n') buffer.skip_line();
      continue;
    }

    ////////////////////////////////////////////////////////
    // Assume: GLOBAL FUNCTIONS                           //
    ////////////////////////////////////////////////////////
    buffer.skip_ws();
    if (!buffer.eof) {
      buffer.test_and_skip("inline ");

      char type[64];
      buffer.sws_read_c_identifier(type);

      char name[128];
      buffer.sws_read_c_identifier(name);

      if (rose::hash(name) == rose::hash("operator")) {
        size_t len = strlen("operator");
        size_t size = sizeof(name) - len;
        char * p = name + len;

        buffer.read_till(p, size, "(" WHITESPACE);

        if (!is_valid_operator(name)) {
          error("Unknown operator", buffer);
        }
      }

      if (buffer.test_and_skip("(")) {
        function_info & funci = ctx.functions.emplace_back();
        copy(funci.type, type);
        copy(funci.name, name);

        while (buffer.sws_peek() != ')')
        {
          buffer.sws_read_till(tmp, ",)");
          if (!is_empty(tmp)) {
            function_parameter_info & para = funci.parameters.emplace_back();

            rose::StreamBuffer para_buffer;
            para_buffer.load_mem(tmp);

            para.is_const = para_buffer.test_and_skip("const ");

            para_buffer.sws_read_till(para.type, "*&" WHITESPACE);
            char c = para_buffer.sws_peek();
            if (c == '&' || c == '*')
            {
              para.modifier = para_buffer.get();
            }
            para_buffer.sws_read_till(para.name, WHITESPACE);
          }
          buffer.test_and_skip(",");
        }
        assert(buffer.peek() == ')');
        buffer.skip(1);
        if (buffer.test_and_skip(";")) { /* empty function body */ }
        //else if (buffer.test_and_skip("{")) {
        else if (buffer.peek() == '{') {
          skip_function_body();
        }
        else error("expected either ';' or '{'.", buffer);
      }
      else error("Expected '('", buffer);
    }
  }
}

//printf trim trailing whitespaces
template<typename... Args>
void printf_ttws(const char * f, Args... args) {
  char buffer[1024];

  sprintf(buffer, f, args...);

  char * pto = buffer;
  char * pfrom = buffer;

  size_t spaces = 0;

  RHash state = rose::hash("COPY");

  for (size_t i = 0; i != sizeof(buffer); ++i) {
    char c = *pfrom;
    if (c == 0) {
      *pto = 0;
      break;
    }

    if (c != ' ' && state == rose::hash("COPY")) {
      *pto = *pfrom;
      ++pto;
      ++pfrom;
      continue;
    }
    if (c == ' ' && state == rose::hash("COPY")) {
      spaces = 1;
      ++pfrom;
      state = rose::hash("SPACE");
      continue;
    }
    if (c == '\n' && state == rose::hash("SPACE")) {
      *pto = '\n';
      ++pto;
      ++pfrom;
      state = rose::hash("COPY");
      continue;
    }
    if (c == ' ' && state == rose::hash("SPACE")) {
      ++pfrom;
      ++spaces;
      continue;
    }
    if (state == rose::hash("SPACE")) {
      for (size_t s = 0; s != spaces; ++s) {
        *(pto++) = ' ';
      }
      *(pto++) = c;

      ++pfrom;
      state = rose::hash("COPY");
      continue;
    }
  }
  fputs(buffer, stdout);
}

void has_compare_ops(bool & has_eqop, bool & has_neqop, bool & has_serialize, bool & has_deserialize, ParseContext & c, const char * sname) {
  RHash shash = rose::hash(sname);
  has_eqop = false;
  has_neqop = false;
  has_serialize = false;
  has_deserialize = false;

  for (auto & inf : c.functions) {
    if (inf.parameters.size() == 2 &&
      rose::hash(inf.name) == rose::hash("operator==") &&
      rose::hash(inf.parameters[0].type) == shash &&
      rose::hash(inf.parameters[1].type) == shash)
      has_eqop = true;

    if (inf.parameters.size() == 2 &&
      rose::hash(inf.name) == rose::hash("operator!=") &&
      rose::hash(inf.parameters[0].type) == shash &&
      rose::hash(inf.parameters[1].type) == shash)
      has_neqop = true;

    if (inf.parameters.size() == 2 &&
      rose::hash(inf.name) == rose::hash("serialize") &&
      rose::hash(inf.parameters[0].type) == shash)
      has_serialize = true;

    if (inf.parameters.size() == 2 &&
      rose::hash(inf.name) == rose::hash("deserialize") &&
      rose::hash(inf.parameters[0].type) == shash)
      has_deserialize = true;
  }

  
  int number_of_eq_ops = 0;
  if (has_eqop)
    ++number_of_eq_ops;
  if (has_neqop)
    ++number_of_eq_ops;

  if (number_of_eq_ops == 1) {
    fprintf(stderr, "%s must overload either all or non of the '==' and '!=' operators" ENDL, sname);
    exit(1);
  }
}

RHash filtered_struct_hash(struct_info structi) {
  auto struct_no_functions = structi;
  auto new_end = std::stable_partition(
    struct_no_functions.members.begin(), struct_no_functions.members.end(),
    [](const auto & member) {
        return member.kind == Member_info_kind::Field;
    });
  struct_no_functions.members.erase(new_end, struct_no_functions.members.end());
  return rose::hash(struct_no_functions);
}

void dump_cpp(ParseContext & c, int argc = 0, char ** argv = nullptr) {
  printf_ttws("#pragma once" ENDL);
  printf_ttws("" ENDL);
  printf_ttws("#include <new>" ENDL);
  printf_ttws("#include <rose/hash.h>" ENDL);
  printf_ttws("#include <rose/typetraits.h>" ENDL);
  printf_ttws("#include <rose/serializer.h>" ENDL);
  printf_ttws("#include <rose/world.h>" ENDL);
  printf_ttws("" ENDL);
  printf_ttws("///////////////////////////////////////////////////////////////////" ENDL);
  printf_ttws("//  AUTOGEN                                                        " ENDL);
  if (argc && argv) {
    printf_ttws("//  command:" ENDL);
    printf_ttws("//    rose.parser");
    for (int i = 1; i < argc; ++i) {
      printf_ttws(" %s", argv[i]);
    }
    printf_ttws("" ENDL);
  }
  printf_ttws("///////////////////////////////////////////////////////////////////" ENDL);

  // deump definition
  
  for (auto & enumci : c.enum_classes) {
    const char * ename = enumci.name_withns;
    const char * etype = enumci.type;
    
    printf_ttws("///////////////////////////////////////////////////////////////////" ENDL);
    printf_ttws("//  predef enum %s" ENDL, ename);
    printf_ttws("///////////////////////////////////////////////////////////////////" ENDL);
    if (enumci.enum_annotations == global_annotations_t::Flag) {
        puts("");
        printf_ttws("inline rose::BoolConvertible<%s> operator|(const %s &lhs, const %s &rhs) { return { static_cast<%s>(static_cast<%s>(lhs) | static_cast<%s>(rhs)) }; }" ENDL, ename, ename, ename, ename, etype, etype);
        printf_ttws("inline rose::BoolConvertible<%s> operator&(const %s &lhs, const %s &rhs) { return { static_cast<%s>(static_cast<%s>(lhs) & static_cast<%s>(rhs)) }; }" ENDL, ename, ename, ename, ename, etype, etype);
        printf_ttws("inline rose::BoolConvertible<%s> operator^(const %s &lhs, const %s &rhs) { return { static_cast<%s>(static_cast<%s>(lhs) ^ static_cast<%s>(rhs)) }; }" ENDL, ename, ename, ename, ename, etype, etype);
        printf_ttws("inline %s operator|=(%s & lhs, %s rhs) { return lhs = lhs | rhs; }                                                    " ENDL, ename, ename, ename);
        printf_ttws("inline %s operator&=(%s & lhs, %s rhs) { return lhs = lhs & rhs; }                                                    " ENDL, ename, ename, ename);
        printf_ttws("inline %s operator^=(%s & lhs, %s rhs) { return lhs = lhs ^ rhs; }                                                    " ENDL, ename, ename, ename);
    }


    printf_ttws("namespace rose {" ENDL);
    printf_ttws("inline const char * to_string(const %s & e);" ENDL, ename);
    printf_ttws("inline void serialize(%s& o, ISerializer& s); " ENDL, ename);
    printf_ttws("inline void deserialize(%s& o, IDeserializer& s); " ENDL, ename);
    printf_ttws("inline RHash       hash(const %s& o); " ENDL, ename);
    printf_ttws("} //namespace rose \n" ENDL);
  }

  for (auto & structi : c.structs) {
    const char * sname = structi.name_withns;
    //const char * sname_nons = structi.name_withoutns;

    printf_ttws("///////////////////////////////////////////////////////////////////" ENDL);
    printf_ttws("//  predef struct %s" ENDL, sname);
    printf_ttws("///////////////////////////////////////////////////////////////////" ENDL);

    printf_ttws("namespace rose {" ENDL);

    bool has_eqop = false;
    bool has_neqop = false;
    bool has_serialize = false;
    bool has_deserialize = false;
    has_compare_ops(has_eqop, has_neqop, has_serialize, has_deserialize, c, sname);

    if (!has_eqop) {
      printf_ttws("inline bool equals(const %s &lhs, const %s &rhs);" ENDL, sname, sname);
      printf_ttws("inline bool operator==(const %s &lhs, const %s &rhs) { return equals(lhs, rhs); }" ENDL, sname, sname);
      printf_ttws("inline bool operator!=(const %s &lhs, const %s &rhs) { return !equals(lhs, rhs); }" ENDL, sname, sname);
    }

    if (!has_serialize) {
      printf_ttws("inline void serialize(%s &o, ISerializer &s);                   " ENDL, sname);
    }

    if (!has_deserialize) {
      printf_ttws("inline void deserialize(%s &o, IDeserializer &s);  " ENDL, sname);
    }
    printf_ttws("inline RHash hash(const %s &o);" ENDL, sname);
    puts("");

    ///////////////////////////////////////////////////////////////////
    // type info                                                     //
    ///////////////////////////////////////////////////////////////////

    printf_ttws("template <>                                                                                                                                    " ENDL);
    printf_ttws("struct type_id<%s>; " ENDL, sname);

    printf_ttws("template <>                                                                                                                                    " ENDL);
    printf_ttws("inline const reflection::TypeInfo & reflection::get_type_info<%s>(); " ENDL, sname);
    printf_ttws("} //namespace rose \n" ENDL);
    puts("");
  }

  // dump implementation

  puts(R"MLS(
#ifndef IMPL_SERIALIZER_UTIL
#define IMPL_SERIALIZER_UTIL

///////////////////////////////////////////////////////////////////
// internal helper methods
///////////////////////////////////////////////////////////////////

namespace rose {
template<class T>
bool rose_parser_equals(const T& lhs, const T& rhs) {
  return lhs == rhs;
}

template<class T, size_t N>
bool rose_parser_equals(const T(&lhs)[N], const T(&rhs)[N]) {
  for (size_t i = 0; i != N; ++i) {
    if (!rose_parser_equals(lhs, rhs)) return false;
  }
  return true;
}

template<size_t N>
bool rose_parser_equals(const char(&lhs)[N], const char(&rhs)[N]) {
  for (size_t i = 0; i != N; ++i) {
    if (lhs[i] != rhs[i]) return false;
    if (lhs[i] == 0) return true;
  }
  return true;
}

template<class T>
bool rose_parser_equals(const std::vector<T> &lhs, const std::vector<T> &rhs) {
  if (lhs.size() != rhs.size()) return false;
  for (size_t i = 0; i != lhs.size(); ++i) {
    if (!rose_parser_equals(lhs, rhs)) return false;
  }
  return true;
}

template<class T>
RHash rose_parser_hash(const T & value) { return hash(value); }

template<class T>
RHash rose_parser_hash(const std::vector<T>& v) {
  RHash h = 0;
  for (const auto& o : v) {
    h ^= rose_parser_hash(o);
    h = xor64(h);
  }
  return h;
}

}
#endif
  )MLS");

  for (auto & enumci : c.enum_classes) {
    const char * ename = enumci.name_withns;
    
    printf_ttws("///////////////////////////////////////////////////////////////////" ENDL);
    printf_ttws("//  impl enum %s" ENDL, ename);
    printf_ttws("///////////////////////////////////////////////////////////////////" ENDL);

    printf_ttws("inline const char * rose::to_string(const %s & e) {" ENDL, ename);
    printf_ttws("    switch(e) {" ENDL);
    for (auto & enumi : enumci.enums) {
      const char * eval = enumi.name;
      printf_ttws("        case %s::%s: return \"%s\";" ENDL, ename, eval, eval);
    }
    printf_ttws("        default: return \"<UNKNOWN>\";" ENDL);
    printf_ttws("    }" ENDL);
    printf_ttws("}" ENDL);


    printf_ttws("inline void rose::serialize(%s& o, ISerializer& s) {                  " ENDL, ename);
    printf_ttws("  switch (o) {                                                      " ENDL);

    for (auto & enumi : enumci.enums) {
      const char * eval = enumi.name;
      printf_ttws("    case %s::%s: {                                                " ENDL, ename, eval);
      printf_ttws("      char str[] = \"%s\";                                        " ENDL, eval);
      printf_ttws("      serialize(str, s);                                          " ENDL);
      printf_ttws("      break;                                                      " ENDL);
      printf_ttws("    }                                                             " ENDL);
    }

    printf_ttws("    default: /* unknown */ break;                                   " ENDL);
    printf_ttws("  }                                                                 " ENDL);
    printf_ttws("}                                                                   " ENDL);

    printf_ttws("inline void rose::deserialize(%s& o, IDeserializer& s) {            " ENDL, ename);
    printf_ttws("  char str[64];                                                     " ENDL);
    printf_ttws("  deserialize(str, s);                                              " ENDL);
    printf_ttws("  RHash h = rose::hash(str);                             " ENDL);
    printf_ttws("  switch (h) {                                                      " ENDL);
    for (auto & enumi : enumci.enums) {
      const char * eval = enumi.name;
      printf_ttws("  case rose::hash(\"%s\"): o = %s::%s; break;                     " ENDL, eval, ename, eval);
    }
    printf_ttws("  default: /*unknown value*/ break;                                 " ENDL);
    printf_ttws("  }                                                                 " ENDL);
    printf_ttws("}                                                                   " ENDL);

    printf_ttws("inline RHash rose::hash(const %s& o) {          " ENDL, ename);
    printf_ttws("  return static_cast<RHash>(o);                 " ENDL);
    printf_ttws("}                                                  \n" ENDL);
  }

  for (auto & structi : c.structs) {
    const char * sname = structi.name_withns;
    const char * sname_nons = structi.name_withoutns;

    printf_ttws("///////////////////////////////////////////////////////////////////" ENDL);
    printf_ttws("//  impl struct %s" ENDL, sname);
    printf_ttws("///////////////////////////////////////////////////////////////////" ENDL);

    ///////////////////////////////////////////////////////////////////
    // == and != operator                                            //
    ///////////////////////////////////////////////////////////////////

    bool has_eqop = false;
    bool has_neqop = false;
    bool has_serialize = false;
    bool has_deserialize = false;
    has_compare_ops(has_eqop, has_neqop, has_serialize, has_deserialize, c, sname);

    if (!has_eqop) {
      printf_ttws("inline bool rose::equals(const %s &lhs, const %s &rhs) {" ENDL, sname, sname);
      fputs("  return" ENDL, stdout);
      bool first = true;
      for (auto & member : structi.members) {
        if (member.kind != Member_info_kind::Field)
          continue;

        if (first) {
          first = false;
        } else  {
          printf_ttws(" &&" ENDL, stdout);
        }
        printf_ttws("    rose::rose_parser_equals(lhs.%s, rhs.%s)", member.name, member.name);
      }
      printf_ttws(";" ENDL "} " ENDL ENDL);
    }



    if (!has_serialize) {
      ///////////////////////////////////////////////////////////////////
      // serializer                                                    //
      ///////////////////////////////////////////////////////////////////
      printf_ttws("inline void rose::serialize(%s &o, ISerializer &s) {                     " ENDL, sname);
      printf_ttws("  if(s.node_begin(\"%s\", hash(\"%s\"), &o)) {               " ENDL, sname, sname);

      for (auto & member : structi.members) {
        if (member.kind != Member_info_kind::Field)
          continue;
        const char * mname = member.name;
        printf_ttws("    s.key(\"%s\");                                               " ENDL, mname);
        if (member.count > 1 && rose::hash(member.type) == rose::hash("char")) {
          //when type is char[n] then treat is as a string.
          int bit = 0;
          bit |= (member.annotations == member_annotations_t::Data) ? 1 << 0 : 0;
          bit |= (member.annotations == member_annotations_t::String) ? 1 << 1 : 0;
          switch (bit)
          {
          case 1 << 0: //DATA
            printf_ttws("    serialize(o.%s, s);                                        " ENDL, mname);
            break;
          case 1 << 1: //STRING
            printf_ttws("    serialize(o.%s, s, std::strlen(o.%s));                     " ENDL, mname, mname);
            break;
          case 0: //NONE
            fprintf(stderr, "Member '%s::%s' must have either annotations @String or @Data.", sname, member.name);
            exit(1);
            break;
          case 1 << 0 | 1 << 1: //BOTH
            fprintf(stderr, "Member '%s::%s' can't have both annotations @String and @Data.", sname, member.name);
            exit(1);
            break;
          default:
            //Shoyuld be unreachable
            assert(false);
            break;
          }
        }
        else {
          printf_ttws("    serialize(o.%s, s);                                        " ENDL, mname);
        }
      }
      printf_ttws("    s.node_end();                                                  " ENDL);
      printf_ttws("  }                                                                " ENDL);
      printf_ttws("  s.end();                                                         " ENDL);
      printf_ttws("}                                                                   \n" ENDL);
    }

    if (!has_deserialize) {
      ///////////////////////////////////////////////////////////////////
      // deserializer                                                  //
      ///////////////////////////////////////////////////////////////////
      printf_ttws("inline void rose::deserialize(%s &o, IDeserializer &s) {  " ENDL, sname);
      printf_ttws("  while (s.next_key()) {                                " ENDL);
      printf_ttws("    switch (s.hash_key()) {                             " ENDL);

      for (auto & member : structi.members) {
        if (member.kind != Member_info_kind::Field)
          continue;
        const char * mname = member.name;
        printf_ttws("      case rose::hash(\"%s\"):                        " ENDL, mname);
        printf_ttws("        deserialize(o.%s, s);                         " ENDL, mname);
        printf_ttws("        break;                                        " ENDL);
      }
      printf_ttws("      default: s.skip_key(); break;                     " ENDL);
      printf_ttws("    }                                                   " ENDL);
      printf_ttws("  }                                                     " ENDL);
      printf_ttws("}                                                        \n" ENDL);
    }

    ///////////////////////////////////////////////////////////////////
    // hashing                                                       //
    ///////////////////////////////////////////////////////////////////
    printf_ttws("inline RHash rose::hash(const %s &o) {             " ENDL, sname);
    printf_ttws("  RHash h = 0; " ENDL);
    bool first = true;
    for (std::size_t i = 0; i != structi.members.size(); ++i) {
      auto & member = structi.members[i];
      if (member.kind != Member_info_kind::Field)
        continue;
      if (!first) printf_ttws("  h = rose::xor64(h);                    " ENDL);
      printf_ttws("  h ^= rose::rose_parser_hash(o.%s);                 " ENDL, member.name);
      first = false;
    }
    printf_ttws("  return h;                          " ENDL);
    printf_ttws("}                                    " ENDL);
    puts("");

    ///////////////////////////////////////////////////////////////////
    // type info                                                     //
    ///////////////////////////////////////////////////////////////////

    
    printf_ttws("template <>                                           " ENDL);
    printf_ttws("struct rose::type_id<%s> {                            " ENDL, sname);
    printf_ttws("    inline static RHash VALUE = %lluULL;   " ENDL, (unsigned long long)filtered_struct_hash(structi));    
    printf_ttws("};                                                    " ENDL);
    printf_ttws(ENDL);

    printf_ttws("template <>                                                                                                                           " ENDL);
    printf_ttws("inline const rose::reflection::TypeInfo & rose::reflection::get_type_info<%s>() {                                                     " ENDL, sname);
    printf_ttws("  static rose::reflection::TypeInfo info = {                                                                                          " ENDL);
    printf_ttws("    /*             unique_id */ rose::hash(\"%s\"),                                                                                   " ENDL, sname);
    printf_ttws("    /*           member_hash */ %lluULL,                                                                                              " ENDL, (unsigned long long)filtered_struct_hash(structi));
    printf_ttws("    /*      memory_footprint */ sizeof(%s),                                                                                           " ENDL, sname);
    printf_ttws("    /*      memory_alignment */ 16,                                                                                                   " ENDL);
    printf_ttws("    /*                  name */ \"%s\",                                                                                               " ENDL, sname);
    printf_ttws("    /*  fp_default_construct */ +[](void * ptr) { new (ptr) %s(); },                                                                  " ENDL, sname);
    printf_ttws("    /*   fp_default_destruct */ +[](void * ptr) { std::launder(reinterpret_cast<%s*>(ptr))->~%s(); },                                 " ENDL, sname, sname_nons);
    printf_ttws("    /*          fp_serialize */ +[](void * ptr, ISerializer & s) { ::rose::serialize(*std::launder(reinterpret_cast<%s*>(ptr)), s); },    " ENDL, sname);
    printf_ttws("    /*        fp_deserialize */ +[](void * ptr, IDeserializer & d) { ::rose::deserialize(*std::launder(reinterpret_cast<%s*>(ptr)), d); } " ENDL, sname);
    printf_ttws("  };                                                                                                                                           " ENDL);
    printf_ttws("  return info;                                                                                                                                 " ENDL);
    printf_ttws("}                                                                                                                                              " ENDL);
    puts("");
  }

  //end
}

void printhelp() {
  puts(
    "NAME" ENDL
    "       rose.parser - generate serialization code for simple c headers." ENDL
    ENDL
    "SYNOPSIS" ENDL
    "       rose.parser [OPTION]" ENDL
    ENDL
    "BUILDDATE" ENDL
    "       " __DATE__ ", " __TIME__ ENDL
    ENDL

    "DESCRIPTION" ENDL
    "       -H, --help" ENDL
    "              show this help." ENDL
    ENDL
    "       -I, --includes" ENDL
    "              followed by a list of headers files, that should be parsed." ENDL
    ENDL
    "       -O, --output" ENDL
    "              The output file. [default: stdout]" ENDL
    ENDL
    "       -J, --json" ENDL
    "              A optional JSON file containing meta info of the header files." ENDL
    ENDL
    "       -V, --verbose" ENDL
    "              Verbose output." ENDL
    ENDL
    "       -E, --error" ENDL
    "              Force Error." ENDL
    ENDL
    "AUTHOR" ENDL
    "       Written by Rico Possienka." ENDL
  );
}

int main(int argc, char ** argv) {
  if (argc < 2) {
    printhelp();
    exit(1);
  }

  RHash state = rose::hash("NONE");

  char tmp_path[260];
  char dst_path[260];

  bool write_to_file = false; //in case we redirect stdout to something else
  bool verbose = false;

  const char * json_path = nullptr;

  for (int i = 1; i < argc; ++i) {
    const char * arg = argv[i];
    RHash h = rose::hash(arg);
    if (h == rose::hash("--datetime")) {
      fprintf(stderr, "Build Time: %s" ENDL, __DATE__ " " __TIME__);
      continue;
    }
    if (h == rose::hash("--help") || h == rose::hash("-H")) {
      state = rose::hash("NONE");
      printhelp();
      continue;
    }
    if (h == rose::hash("--error") || h == rose::hash("-E")) {
      exit(1);
      continue;
    }
    if (h == rose::hash("--include") || h == rose::hash("-I")) {
      state = rose::hash("INCLUDE");
      continue;
    }
    if (h == rose::hash("--verbose") || h == rose::hash("-V")) {
      verbose = true;
      continue;
    }
    if (h == rose::hash("--output") || h == rose::hash("-O")) {
      state = rose::hash("NONE");
      ++i;
      assert(i != argc);
      sprintf(dst_path, "%s", argv[i]);
      sprintf(tmp_path, "%s.bak", argv[i]);
      (void)freopen(tmp_path, "wb", stdout);
      write_to_file = true;
      continue;
    }
    if (h == rose::hash("--json") || h == rose::hash("-J")) {
      state = rose::hash("NONE");
      ++i;
      assert(i != argc);
      const char * path = argv[i];
      json_path = path;
      continue;
    }
    if (h == rose::hash("--watch") || h == rose::hash("-W")) {
      //TODO: use filewatch option https://github.com/ThomasMonkman/filewatch
      continue;
    }

    switch (state) {
    case rose::hash("INCLUDE"): input_files.push_back(arg); break;
    default: printf("Unknown argument %s." ENDL, arg); exit(1); break;
    }
  }

  ParseContext c;

  for (auto path : input_files) {
    rose::StreamBuffer buffer;
    if (verbose) fprintf(stderr, "Parsing File %s" ENDL, path);
    buffer.load(path);
    parse(c, buffer);
    buffer.unload();
  }

  dump_cpp(c, argc, argv);

  if (write_to_file) {
    fclose(stdout);
    auto ok = MoveFileExA(tmp_path, dst_path, MOVEFILE_REPLACE_EXISTING);
    rose::unused(ok);
  }

  if (json_path) {
    FILE * f = fopen(json_path, "w");
    assert(f);
    JsonSerializer jsons(f);
    rose::serialize(c, jsons);
    fclose(f);
  }

  return 0;
}
