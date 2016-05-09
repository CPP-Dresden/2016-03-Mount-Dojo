#include "mounts.h"

#include <cassert>

Mounts::Mounts(const string_t& virtualBasePath) {
  mount(string_t(), virtualBasePath);
}

string_t
Mounts::absolutePath(const string_t& virtualPath) const {
  /*
   * we search for the deepest mount point of virtualPath
   *
   * a
   * a/b -> up is a
   * a/c -> up is b
   * b
   *
   * upperbound finds the first entry > given
   *
   * upperbound("b/") -> it to end() // --it gives "b", direct match
   * upperbound("a/b/c/") -> it to "a/c" // --it gives "a/b" - the closest mount
   * upperbound("a/ba/") -> it to "a/c" // --it gives "a/b", up gives "a"
   *
   */
  auto it = m_virtualPathToMountPoint.upper_bound(virtualPath + '/');

  // as we always have root mounted we should match
  assert(it != m_virtualPathToMountPoint.begin());

  if (it != m_virtualPathToMountPoint.begin()) {
    --it;
    while (!isPathEqualOrContained(virtualPath, it->first)) {
      it = it->second.up;
    }
  }
  return getMountPath(virtualPath, it->first, it->second.absolute->first);
}

bool
Mounts::mount(const string_t& virtualPath, const string_t& absolutePath) {
  // check for nested mounts
  auto absoluteUpIt = m_absolutePathToVirtualPathes.lower_bound(absolutePath);
  if (absoluteUpIt != m_absolutePathToVirtualPathes.end() &&
      isPathContained(absoluteUpIt->first, absolutePath)) {
    return false; // nested mount - rejected
  }
  absoluteUpIt = m_absolutePathToVirtualPathes.upper_bound(absolutePath + '/');
  if (absoluteUpIt != m_absolutePathToVirtualPathes.begin()) {
    --absoluteUpIt;
    if (isPathContained(absolutePath, absoluteUpIt->first)) {
      return false; // nested mount - rejected
    }
  }
  // create mount
  auto absoluteIt = m_absolutePathToVirtualPathes.insert(std::make_pair(absolutePath, virtualPath));

  auto virtualUpIt = m_virtualPathToMountPoint.upper_bound(virtualPath + '/');
  if (virtualUpIt != m_virtualPathToMountPoint.begin()) {
    --virtualUpIt;
    while (!isPathEqualOrContained(virtualPath, virtualUpIt->first)) {
      virtualUpIt = virtualUpIt->second.up;
    }
  }
  AbsoluteMount_t mountPoint = {absoluteIt, virtualUpIt};
  m_virtualPathToMountPoint[virtualPath] = mountPoint;
  return true;
}
