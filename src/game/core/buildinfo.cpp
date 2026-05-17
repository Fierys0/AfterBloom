#include "buildinfo.hpp"

#include <cstdio>

#include "cmake/generated_buildinfo.hpp"

std::string VERSION = "0.1";
bool isIndev = true;
// __DATE__ format: "Mmm dd yyyy"
// __TIME__ format: "HH:MM:SS"

std::string BuildInfo::GetBuildString() {
  std::string indev = isIndev ? "InDev" : "PROD";

  return "Version " + VERSION + " " + indev + " build " + BUILD_TIME;
}
