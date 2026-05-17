#pragma once
#include <string>

extern std::string VERSION;
extern bool isIndev;

class BuildInfo {
public:
  static std::string GetBuildString();
};
