#pragma once

#include <string>

using string_t = std::string;

inline bool
string_startsWith(const string_t& str, const string_t& sub) {
  return 0 == str.compare(0, sub.size(), sub);
}


/// \return true if @path@ is contained in @containerPath@
/// \note false if @path@ == @containerPath@
///
/// containerPath should not end with a slash!
inline bool
isPathContained(const string_t& path, const string_t& containerPath) {
  if (path.length() > containerPath.length()) {
    return path[containerPath.length()] == '/' && string_startsWith(path, containerPath);
  }
  return false;
}

/// \return true if @path@ is equal or is contained in @containerPath@
///
/// containerPath should not end with a slash!
inline bool
isPathEqualOrContained(const string_t& path, const string_t& containerPath) {
  if (path.length() > containerPath.length()) {
    return path[containerPath.length()] == '/' && string_startsWith(path, containerPath);
  }
  return path == containerPath;
}

/// \return returns the remounted @path@ that is equal or is contained in @containerPath@ to the
/// @containerMountPath@
/// \note if @path@ is not equal or contained in @containerPath@ an empty string is returned
///
/// \example
///  @path@="/a/b/c"
///  @containerPath@="/a"
///  @containerMountPath@="/mnt"
///  return "/mnt/b/c"
///
inline string_t
getMountPath(const string_t& path, const string_t& containerPath,
             const string_t& containerMountPath) {
  if (path.length() > containerPath.length()) {
    if (path[containerPath.length()] == '/' && string_startsWith(path, containerPath)) {
      return containerMountPath + path.substr(containerPath.length());
    }
  }
  if (path == containerPath) return containerMountPath;
  return {};
}
