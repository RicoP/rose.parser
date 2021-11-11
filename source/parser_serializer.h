#pragma once

#include <rose/hash.h>
#include <serializer/serializer.h>

///////////////////////////////////////////////////////////////////
//  AUTOGEN                                                      //
//  command:
//    rose.parser --include parser.h -O parser_serializer.h
///////////////////////////////////////////////////////////////////

enum class                   value_type_t;
const char * to_string(const value_type_t &);
namespace rose {
  namespace ecs {
    void      deserialize(value_type_t &o, IDeserializer &s);
    void        serialize(value_type_t &o, ISerializer &s);
  }
  hash_value         hash(const value_type_t &o);
  void construct_defaults(      value_type_t &o); //TODO: implement me
}


struct                member_info;
namespace rose {
  namespace ecs {
    bool operator==(const member_info &lhs, const member_info &rhs);
    bool operator!=(const member_info &lhs, const member_info &rhs);
    void      deserialize(member_info &o, IDeserializer &s);
    void        serialize(member_info &o, ISerializer &s);
  }
  hash_value         hash(const member_info &o);
  void construct_defaults(      member_info &o); //TODO: implement me
}


struct                struct_info;
namespace rose {
  namespace ecs {
    bool operator==(const struct_info &lhs, const struct_info &rhs);
    bool operator!=(const struct_info &lhs, const struct_info &rhs);
    void      deserialize(struct_info &o, IDeserializer &s);
    void        serialize(struct_info &o, ISerializer &s);
  }
  hash_value         hash(const struct_info &o);
  void construct_defaults(      struct_info &o); //TODO: implement me
}


struct                enum_info;
namespace rose {
  namespace ecs {
    bool operator==(const enum_info &lhs, const enum_info &rhs);
    bool operator!=(const enum_info &lhs, const enum_info &rhs);
    void      deserialize(enum_info &o, IDeserializer &s);
    void        serialize(enum_info &o, ISerializer &s);
  }
  hash_value         hash(const enum_info &o);
  void construct_defaults(      enum_info &o); //TODO: implement me
}


struct                enum_class_info;
namespace rose {
  namespace ecs {
    bool operator==(const enum_class_info &lhs, const enum_class_info &rhs);
    bool operator!=(const enum_class_info &lhs, const enum_class_info &rhs);
    void      deserialize(enum_class_info &o, IDeserializer &s);
    void        serialize(enum_class_info &o, ISerializer &s);
  }
  hash_value         hash(const enum_class_info &o);
  void construct_defaults(      enum_class_info &o); //TODO: implement me
}


struct                function_parameter_info;
namespace rose {
  namespace ecs {
    bool operator==(const function_parameter_info &lhs, const function_parameter_info &rhs);
    bool operator!=(const function_parameter_info &lhs, const function_parameter_info &rhs);
    void      deserialize(function_parameter_info &o, IDeserializer &s);
    void        serialize(function_parameter_info &o, ISerializer &s);
  }
  hash_value         hash(const function_parameter_info &o);
  void construct_defaults(      function_parameter_info &o); //TODO: implement me
}


struct                function_info;
namespace rose {
  namespace ecs {
    bool operator==(const function_info &lhs, const function_info &rhs);
    bool operator!=(const function_info &lhs, const function_info &rhs);
    void      deserialize(function_info &o, IDeserializer &s);
    void        serialize(function_info &o, ISerializer &s);
  }
  hash_value         hash(const function_info &o);
  void construct_defaults(      function_info &o); //TODO: implement me
}


struct                ParseContext;
namespace rose {
  namespace ecs {
    bool operator==(const ParseContext &lhs, const ParseContext &rhs);
    bool operator!=(const ParseContext &lhs, const ParseContext &rhs);
    void      deserialize(ParseContext &o, IDeserializer &s);
    void        serialize(ParseContext &o, ISerializer &s);
  }
  hash_value         hash(const ParseContext &o);
  void construct_defaults(      ParseContext &o); //TODO: implement me
}


#ifdef IMPL_SERIALIZER

    //internal helper methods
    template<class T>
    bool rose_parser_equals(const T& lhs, const T& rhs) {
      return lhs == rhs;
    }

    template<class T, size_t N>
    bool rose_parser_equals(const T(&lhs)[N], const T(&rhs)[N]) {
      for (size_t i = 0; i != N; ++i) {
        if (lhs[i] != rhs[i]) return false;
      }
      return true;
    }

    template<size_t N>
    bool rose_parser_equals(const char(&lhs)[N], const char(&rhs)[N]) {
      for (size_t i = 0; i != N; ++i) {
        if (lhs[i] != rhs[i]) return false;
        if (lhs[i] == 0) return false;
      }
      return true;
    }

    template<class T>
    bool rose_parser_equals(const std::vector<T> &lhs, const std::vector<T> &rhs) {
      if (lhs.size() != rhs.size()) return false;
      for (size_t i = 0; i != lhs.size(); ++i) {
        if (lhs[i] != rhs[i]) return false;
      }
      return true;
    }

    template<class TL, class TR>
    void assign(TL& lhs, TR&& rhs) {
      lhs = rhs;
    }

    template<class T>
    void construct_default(std::vector<T> & v) {
      c.clear();
    }
  
const char * to_string(const value_type_t & e) {
    switch(e) {
        case value_type_t::Increment: return "Increment";
        case value_type_t::Set: return "Set";
        default: return "<UNKNOWN>";
    }
}
void rose::ecs::serialize(value_type_t& o, ISerializer& s) {
  switch (o) {
    case value_type_t::Increment: {
      char str[] = "Increment";
      serialize(str, s);
      break;
    }
    case value_type_t::Set: {
      char str[] = "Set";
      serialize(str, s);
      break;
    }
    default: /* unknown */ break;
  }
}
void rose::ecs::deserialize(value_type_t& o, IDeserializer& s) {
  char str[64];
  deserialize(str, s);
  rose::hash_value h = rose::hash(str);
  switch (h) {
  case rose::hash("Increment"): o = value_type_t::Increment; break;
  case rose::hash("Set"): o = value_type_t::Set; break;
  default: /*unknown value*/ break;
  }
}
rose::hash_value       rose::hash(const value_type_t& o) {
  return static_cast<rose::hash_value>(o);
}

///////////////////////////////////////////////////////////////////
//  struct member_info
///////////////////////////////////////////////////////////////////
bool operator==(const member_info &lhs, const member_info &rhs) {
  return
    rose_parser_equals(lhs.type, rhs.type) &&
    rose_parser_equals(lhs.name, rhs.name) &&
    rose_parser_equals(lhs.default_value, rhs.default_value) &&
    rose_parser_equals(lhs.count, rhs.count) ;
}

bool operator!=(const member_info &lhs, const member_info &rhs) {
  return
    !rose_parser_equals(lhs.type, rhs.type) ||
    !rose_parser_equals(lhs.name, rhs.name) ||
    !rose_parser_equals(lhs.default_value, rhs.default_value) ||
    !rose_parser_equals(lhs.count, rhs.count) ;
}

void rose::ecs::serialize(member_info &o, ISerializer &s) {
  if(s.node_begin("member_info", rose::hash("member_info"), &o)) {
    s.key("type");
    serialize(o.type, s);
    s.key("name");
    serialize(o.name, s);
    s.key("default_value");
    serialize(o.default_value, s);
    s.key("count");
    serialize(o.count, s);
    s.node_end();
  }
  s.end();
}

void rose::ecs::deserialize(member_info &o, IDeserializer &s) {
  //TODO: implement me
  //construct_defaults(o);

  while (s.next_key()) {
    switch (s.hash_key()) {
      case rose::hash("type"):
        deserialize(o.type, s);
        break;
      case rose::hash("name"):
        deserialize(o.name, s);
        break;
      case rose::hash("default_value"):
        deserialize(o.default_value, s);
        break;
      case rose::hash("count"):
        deserialize(o.count, s);
        break;
      default: s.skip_key(); break;
    }
  }
}

rose::hash_value rose::hash(const member_info &o) {
  rose::hash_value h = 0;
  h ^= rose::hash(o.type);
  h = rose::xor64(h);
  h ^= rose::hash(o.name);
  h = rose::xor64(h);
  h ^= rose::hash(o.default_value);
  h = rose::xor64(h);
  h ^= rose::hash(o.count);
  h = rose::xor64(h);
  return h;
}
///////////////////////////////////////////////////////////////////
//  struct struct_info
///////////////////////////////////////////////////////////////////
bool operator==(const struct_info &lhs, const struct_info &rhs) {
  return
    rose_parser_equals(lhs.name, rhs.name) &&
    rose_parser_equals(lhs.members, rhs.members) ;
}

bool operator!=(const struct_info &lhs, const struct_info &rhs) {
  return
    !rose_parser_equals(lhs.name, rhs.name) ||
    !rose_parser_equals(lhs.members, rhs.members) ;
}

void rose::ecs::serialize(struct_info &o, ISerializer &s) {
  if(s.node_begin("struct_info", rose::hash("struct_info"), &o)) {
    s.key("name");
    serialize(o.name, s);
    s.key("members");
    serialize(o.members, s);
    s.node_end();
  }
  s.end();
}

void rose::ecs::deserialize(struct_info &o, IDeserializer &s) {
  //TODO: implement me
  //construct_defaults(o);

  while (s.next_key()) {
    switch (s.hash_key()) {
      case rose::hash("name"):
        deserialize(o.name, s);
        break;
      case rose::hash("members"):
        deserialize(o.members, s);
        break;
      default: s.skip_key(); break;
    }
  }
}

rose::hash_value rose::hash(const struct_info &o) {
  rose::hash_value h = 0;
  h ^= rose::hash(o.name);
  h = rose::xor64(h);
  h ^= rose::hash(o.members);
  h = rose::xor64(h);
  return h;
}
///////////////////////////////////////////////////////////////////
//  struct enum_info
///////////////////////////////////////////////////////////////////
bool operator==(const enum_info &lhs, const enum_info &rhs) {
  return
    rose_parser_equals(lhs.name, rhs.name) &&
    rose_parser_equals(lhs.value, rhs.value) &&
    rose_parser_equals(lhs.value_type, rhs.value_type) ;
}

bool operator!=(const enum_info &lhs, const enum_info &rhs) {
  return
    !rose_parser_equals(lhs.name, rhs.name) ||
    !rose_parser_equals(lhs.value, rhs.value) ||
    !rose_parser_equals(lhs.value_type, rhs.value_type) ;
}

void rose::ecs::serialize(enum_info &o, ISerializer &s) {
  if(s.node_begin("enum_info", rose::hash("enum_info"), &o)) {
    s.key("name");
    serialize(o.name, s);
    s.key("value");
    serialize(o.value, s);
    s.key("value_type");
    serialize(o.value_type, s);
    s.node_end();
  }
  s.end();
}

void rose::ecs::deserialize(enum_info &o, IDeserializer &s) {
  //TODO: implement me
  //construct_defaults(o);

  while (s.next_key()) {
    switch (s.hash_key()) {
      case rose::hash("name"):
        deserialize(o.name, s);
        break;
      case rose::hash("value"):
        deserialize(o.value, s);
        break;
      case rose::hash("value_type"):
        deserialize(o.value_type, s);
        break;
      default: s.skip_key(); break;
    }
  }
}

rose::hash_value rose::hash(const enum_info &o) {
  rose::hash_value h = 0;
  h ^= rose::hash(o.name);
  h = rose::xor64(h);
  h ^= rose::hash(o.value);
  h = rose::xor64(h);
  h ^= rose::hash(o.value_type);
  h = rose::xor64(h);
  return h;
}
///////////////////////////////////////////////////////////////////
//  struct enum_class_info
///////////////////////////////////////////////////////////////////
bool operator==(const enum_class_info &lhs, const enum_class_info &rhs) {
  return
    rose_parser_equals(lhs.name, rhs.name) &&
    rose_parser_equals(lhs.type, rhs.type) &&
    rose_parser_equals(lhs.custom_type, rhs.custom_type) &&
    rose_parser_equals(lhs.enums, rhs.enums) &&
    rose_parser_equals(lhs.default_value, rhs.default_value) ;
}

bool operator!=(const enum_class_info &lhs, const enum_class_info &rhs) {
  return
    !rose_parser_equals(lhs.name, rhs.name) ||
    !rose_parser_equals(lhs.type, rhs.type) ||
    !rose_parser_equals(lhs.custom_type, rhs.custom_type) ||
    !rose_parser_equals(lhs.enums, rhs.enums) ||
    !rose_parser_equals(lhs.default_value, rhs.default_value) ;
}

void rose::ecs::serialize(enum_class_info &o, ISerializer &s) {
  if(s.node_begin("enum_class_info", rose::hash("enum_class_info"), &o)) {
    s.key("name");
    serialize(o.name, s);
    s.key("type");
    serialize(o.type, s);
    s.key("custom_type");
    serialize(o.custom_type, s);
    s.key("enums");
    serialize(o.enums, s);
    s.key("default_value");
    serialize(o.default_value, s);
    s.node_end();
  }
  s.end();
}

void rose::ecs::deserialize(enum_class_info &o, IDeserializer &s) {
  //TODO: implement me
  //construct_defaults(o);

  while (s.next_key()) {
    switch (s.hash_key()) {
      case rose::hash("name"):
        deserialize(o.name, s);
        break;
      case rose::hash("type"):
        deserialize(o.type, s);
        break;
      case rose::hash("custom_type"):
        deserialize(o.custom_type, s);
        break;
      case rose::hash("enums"):
        deserialize(o.enums, s);
        break;
      case rose::hash("default_value"):
        deserialize(o.default_value, s);
        break;
      default: s.skip_key(); break;
    }
  }
}

rose::hash_value rose::hash(const enum_class_info &o) {
  rose::hash_value h = 0;
  h ^= rose::hash(o.name);
  h = rose::xor64(h);
  h ^= rose::hash(o.type);
  h = rose::xor64(h);
  h ^= rose::hash(o.custom_type);
  h = rose::xor64(h);
  h ^= rose::hash(o.enums);
  h = rose::xor64(h);
  h ^= rose::hash(o.default_value);
  h = rose::xor64(h);
  return h;
}
///////////////////////////////////////////////////////////////////
//  struct function_parameter_info
///////////////////////////////////////////////////////////////////
bool operator==(const function_parameter_info &lhs, const function_parameter_info &rhs) {
  return
    rose_parser_equals(lhs.name, rhs.name) &&
    rose_parser_equals(lhs.type, rhs.type) &&
    rose_parser_equals(lhs.modifier, rhs.modifier) &&
    rose_parser_equals(lhs.is_const, rhs.is_const) ;
}

bool operator!=(const function_parameter_info &lhs, const function_parameter_info &rhs) {
  return
    !rose_parser_equals(lhs.name, rhs.name) ||
    !rose_parser_equals(lhs.type, rhs.type) ||
    !rose_parser_equals(lhs.modifier, rhs.modifier) ||
    !rose_parser_equals(lhs.is_const, rhs.is_const) ;
}

void rose::ecs::serialize(function_parameter_info &o, ISerializer &s) {
  if(s.node_begin("function_parameter_info", rose::hash("function_parameter_info"), &o)) {
    s.key("name");
    serialize(o.name, s);
    s.key("type");
    serialize(o.type, s);
    s.key("modifier");
    serialize(o.modifier, s);
    s.key("is_const");
    serialize(o.is_const, s);
    s.node_end();
  }
  s.end();
}

void rose::ecs::deserialize(function_parameter_info &o, IDeserializer &s) {
  //TODO: implement me
  //construct_defaults(o);

  while (s.next_key()) {
    switch (s.hash_key()) {
      case rose::hash("name"):
        deserialize(o.name, s);
        break;
      case rose::hash("type"):
        deserialize(o.type, s);
        break;
      case rose::hash("modifier"):
        deserialize(o.modifier, s);
        break;
      case rose::hash("is_const"):
        deserialize(o.is_const, s);
        break;
      default: s.skip_key(); break;
    }
  }
}

rose::hash_value rose::hash(const function_parameter_info &o) {
  rose::hash_value h = 0;
  h ^= rose::hash(o.name);
  h = rose::xor64(h);
  h ^= rose::hash(o.type);
  h = rose::xor64(h);
  h ^= rose::hash(o.modifier);
  h = rose::xor64(h);
  h ^= rose::hash(o.is_const);
  h = rose::xor64(h);
  return h;
}
///////////////////////////////////////////////////////////////////
//  struct function_info
///////////////////////////////////////////////////////////////////
bool operator==(const function_info &lhs, const function_info &rhs) {
  return
    rose_parser_equals(lhs.name, rhs.name) &&
    rose_parser_equals(lhs.type, rhs.type) &&
    rose_parser_equals(lhs.parameters, rhs.parameters) ;
}

bool operator!=(const function_info &lhs, const function_info &rhs) {
  return
    !rose_parser_equals(lhs.name, rhs.name) ||
    !rose_parser_equals(lhs.type, rhs.type) ||
    !rose_parser_equals(lhs.parameters, rhs.parameters) ;
}

void rose::ecs::serialize(function_info &o, ISerializer &s) {
  if(s.node_begin("function_info", rose::hash("function_info"), &o)) {
    s.key("name");
    serialize(o.name, s);
    s.key("type");
    serialize(o.type, s);
    s.key("parameters");
    serialize(o.parameters, s);
    s.node_end();
  }
  s.end();
}

void rose::ecs::deserialize(function_info &o, IDeserializer &s) {
  //TODO: implement me
  //construct_defaults(o);

  while (s.next_key()) {
    switch (s.hash_key()) {
      case rose::hash("name"):
        deserialize(o.name, s);
        break;
      case rose::hash("type"):
        deserialize(o.type, s);
        break;
      case rose::hash("parameters"):
        deserialize(o.parameters, s);
        break;
      default: s.skip_key(); break;
    }
  }
}

rose::hash_value rose::hash(const function_info &o) {
  rose::hash_value h = 0;
  h ^= rose::hash(o.name);
  h = rose::xor64(h);
  h ^= rose::hash(o.type);
  h = rose::xor64(h);
  h ^= rose::hash(o.parameters);
  h = rose::xor64(h);
  return h;
}
///////////////////////////////////////////////////////////////////
//  struct ParseContext
///////////////////////////////////////////////////////////////////
bool operator==(const ParseContext &lhs, const ParseContext &rhs) {
  return
    rose_parser_equals(lhs.enum_classes, rhs.enum_classes) &&
    rose_parser_equals(lhs.functions, rhs.functions) &&
    rose_parser_equals(lhs.structs, rhs.structs) ;
}

bool operator!=(const ParseContext &lhs, const ParseContext &rhs) {
  return
    !rose_parser_equals(lhs.enum_classes, rhs.enum_classes) ||
    !rose_parser_equals(lhs.functions, rhs.functions) ||
    !rose_parser_equals(lhs.structs, rhs.structs) ;
}

void rose::ecs::serialize(ParseContext &o, ISerializer &s) {
  if(s.node_begin("ParseContext", rose::hash("ParseContext"), &o)) {
    s.key("enum_classes");
    serialize(o.enum_classes, s);
    s.key("functions");
    serialize(o.functions, s);
    s.key("structs");
    serialize(o.structs, s);
    s.node_end();
  }
  s.end();
}

void rose::ecs::deserialize(ParseContext &o, IDeserializer &s) {
  //TODO: implement me
  //construct_defaults(o);

  while (s.next_key()) {
    switch (s.hash_key()) {
      case rose::hash("enum_classes"):
        deserialize(o.enum_classes, s);
        break;
      case rose::hash("functions"):
        deserialize(o.functions, s);
        break;
      case rose::hash("structs"):
        deserialize(o.structs, s);
        break;
      default: s.skip_key(); break;
    }
  }
}

rose::hash_value rose::hash(const ParseContext &o) {
  rose::hash_value h = 0;
  h ^= rose::hash(o.enum_classes);
  h = rose::xor64(h);
  h ^= rose::hash(o.functions);
  h = rose::xor64(h);
  h ^= rose::hash(o.structs);
  h = rose::xor64(h);
  return h;
}

#endif
