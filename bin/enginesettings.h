///////////////////////////////////////////////////////////////////
// AUTOGENERATED do not edit by hand                             //
///////////////////////////////////////////////////////////////////
#pragma once
#include <serializer/serializer.h>
#include <rose/ecs.h>
#include <cstring>

struct EngineSettings {
  bool raytracer;

};

 void construct_defaults(EngineSettings & o) {
  std::memset(&o, 0, sizeof(EngineSettings));
  o.raytracer = true;
}