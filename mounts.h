#pragma once

#include "path_utils.h"

#include <map>
#include <string>
#include <vector>

using string_t = std::string;

/// \brief manage mounts from filesystem to the virtual path system
class Mounts {
public:
  Mounts(const string_t& virtualBasePath);

  /// \brief invokes callback for each virtualPath where @absolutePath@ is
  /// mounted to
  template <typename Callback>
  void
  forEachMount(const string_t& absolutePath, Callback callback) const {
    /*
     * we search all mounts that contain absolutePath
     *
     * this does allows to have multiple mounts (two virtual pathes that refer
     * to the same
     * absolute path)
     * NO Nested mounts are allowed (two virtual pathes refer to nested folders)
     */
    auto it = m_absolutePathToVirtualPathes.upper_bound(absolutePath);
    if (it == m_absolutePathToVirtualPathes.begin()) {
      return; // out of range
    }
    --it; // we got upper_bound so we go back at least once
    if (!isPathEqualOrContained(absolutePath, it->first)) {
      return; // absolutePath not in path
    }
    const auto& absoluteTargetPath = it->first;
    do {
      auto virtualPath = getMountPath(absolutePath, absoluteTargetPath, it->second);
      if (it == m_absolutePathToVirtualPathes.begin()) {
        callback(virtualPath);
        break; // finished
      }
      auto nextIt = it;
      --nextIt; // get next iterator now, so callback can rename it
      callback(virtualPath);
      it = nextIt;
    } while (it->first == absoluteTargetPath);
  }

  inline std::vector<string_t>
  allMountPathes(const string_t& absolutePath) const {
    std::vector<string_t> result;
    forEachMount(absolutePath, [&](const string_t& virtualPath) { result.push_back(virtualPath); });
    return result;
  }

  template <typename Callback>
  void
  forEachMountBelow(const string_t& virtualPath, Callback callback) const {
    forEachMountBelowIt(virtualPath, [&](const VirtualPathToAbsoluteMount_t::const_iterator& it) {
      callback(it->first, it->second.absolute->first);
    });
  }

  template <typename Callback>
  void
  forEachMountIn(const string_t& virtualPath, Callback callback) const {
    forEachMountInIt(virtualPath, [&](const VirtualPathToAbsoluteMount_t::const_iterator& it) {
      callback(it->first, it->second.absolute->first);
    });
  }

  inline std::vector<std::pair<string_t, string_t>>
  getMountPointsIn(const string_t& virtualPath) const {
    std::vector<std::pair<string_t, string_t>> result;
    forEachMountIn(virtualPath, [&](const string_t& virtualPath, const string_t& absolutePath) {
      result.push_back(std::make_pair(virtualPath, absolutePath));
    });
    return result;
  }

  /// \return one of the virtual pathes where @absolutePath@ is mounted to
  /// (empty if none exists or mounted to root)
  inline string_t
  GetVirtualPath(const string_t& absolutePath) const {
    auto it = m_absolutePathToVirtualPathes.find(absolutePath);
    if (it == m_absolutePathToVirtualPathes.end()) return {};
    return it->second;
  }

  /// \return the absolute path that corresponds to the given @virtualPath@
  string_t absolutePath(const string_t& virtualPath) const;

  /// \brief add a new mount point
  /// \param virtualPath where the mount point occurs
  /// \param absolutePath where the mount point leads to
  bool mount(const string_t& virtualPath, const string_t& absolutePath);

  inline void
  unmountIn(const string_t& virtualPath) {
    unmountIn(virtualPath, [](const string_t&) {});
  }

  /// \brief removes all mount points inside the virtual path
  template <typename Callback>
  void
  unmountIn(const string_t& virtualPath, Callback callback) {
    forEachMountInIt(virtualPath, [&](VirtualIterator_t it) -> VirtualIterator_t {
      auto absoluteIt = it->second.absolute;
      auto absolutePath = absoluteIt->first;
      absoluteIt = m_absolutePathToVirtualPathes.erase(absoluteIt);
      auto previousIt = absoluteIt;
      if ((absoluteIt == m_absolutePathToVirtualPathes.end() ||
           absoluteIt->first != absolutePath) &&
          (previousIt == m_absolutePathToVirtualPathes.begin() ||
           (--previousIt)->first != absolutePath)) {
        callback(absolutePath); // absolutePath is no longer mounted
      }
      return m_virtualPathToMountPoint.erase(it);
    });
  }

  inline void
  unmountAbsolute(const string_t& absolutePath) {
    unmountAbsolute(absolutePath, [](const string_t&) {});
  }

  /// \brief removes the mount points that target absolutePath
  template <typename Callback>
  void
  unmountAbsolute(const string_t& absolutePath, Callback callback) {
    // remove for all pathes == absolutePath
    auto it = m_absolutePathToVirtualPathes.find(absolutePath);
    while (it != m_absolutePathToVirtualPathes.end()) {
      auto virtualPath = it->second; // make a copy will get invalid
      unmountIn(virtualPath, callback);
      it = m_absolutePathToVirtualPathes.find(absolutePath);
    }
  }

private:
  template <typename Callback>
  void
  forEachMountBelowIt(const string_t& virtualPath, Callback callback) const {
    auto it = m_virtualPathToMountPoint.upper_bound(virtualPath + '/');
    while (it != m_virtualPathToMountPoint.end() &&
           isPathEqualOrContained(it->first, virtualPath)) {
      callback(it);
      ++it;
    }
  }

  template <typename Callback>
  void
  forEachMountBelowIt(const string_t& virtualPath, Callback callback) {
    auto it = m_virtualPathToMountPoint.upper_bound(virtualPath + '/');
    while (it != m_virtualPathToMountPoint.end() &&
           isPathEqualOrContained(it->first, virtualPath)) {
      it = callback(it);
    }
  }

  template <typename Callback>
  void
  forEachMountInIt(const string_t& virtualPath, Callback callback) const {
    auto it = m_virtualPathToMountPoint.find(virtualPath);
    if (it != m_virtualPathToMountPoint.end()) {
      callback(it);
    }
    forEachMountBelowIt(virtualPath, callback);
  }

  template <typename Callback>
  void
  forEachMountInIt(const string_t& virtualPath, Callback callback) {
    auto it = m_virtualPathToMountPoint.find(virtualPath);
    if (it != m_virtualPathToMountPoint.end()) {
      callback(it);
    }
    forEachMountBelowIt(virtualPath, callback);
  }

private:
  using AbsolutePathToVirtualPath_t = std::multimap<string_t, string_t>;
  using AbsoluteIterator_t = AbsolutePathToVirtualPath_t::iterator;
  struct AbsoluteMount_t;
  using VirtualPathToAbsoluteMount_t = std::map<string_t, AbsoluteMount_t>;
  using VirtualIterator_t = VirtualPathToAbsoluteMount_t::iterator;
  struct AbsoluteMount_t {
    AbsoluteIterator_t absolute;
    VirtualIterator_t up;
  };

  AbsolutePathToVirtualPath_t m_absolutePathToVirtualPathes;
  VirtualPathToAbsoluteMount_t m_virtualPathToMountPoint;
};
