#include "mounts.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <vector>

namespace {

template <typename V>
bool
vector_contains(const V& vec, const typename V::value_type& value) {
  auto it = std::find(vec.begin(), vec.end(), value);
  return it != vec.end();
}

void
testTools() {
  assert(true == isPathEqualOrContained("abc/def", "abc"));
  assert(true == isPathEqualOrContained("abc", "abc"));
  assert(false == isPathEqualOrContained("abc", "abc/def"));
  assert(false == isPathEqualOrContained("abc", "ab"));
  assert(false == isPathEqualOrContained("ab", "abc"));

  assert("mnt/def" == getMountPath("abc/def", "abc", "mnt"));
}

Mounts
testSetup() {
  /**
   * filesystem:
   *
   * /documents
   *   document
   * /customers
   *   kunde1
   *     documents -> /documents
   *   kunde2
   *     documents -> /documents
   * /virtual
   *   work
   *     kunde1 -> /customers/kunde1
   *	   kunde2 -> /customers/kunde2
   * /workspace
   *   work -> /virtual/work
   *
   * virtual pathes:
   *
   * "" (mounted to: "/workspace")
   *   /work (mounted to: "/virtual/work")
   *     kunde1 (mounted to: "/customers/kunde1")
   *       documents (mounted to: "/documents")
   *         document
   *     kunde2 (mounted to: "/customers/kunde2")
   *       documents (mounted to: "/documents")
   *         document
   *   /something
   */
  Mounts result("/workspace");
  result.mount("/work", "/virtual/work");
  result.mount("/work/kunde1", "/customers/kunde1");
  result.mount("/work/kunde2", "/customers/kunde2");
  result.mount("/work/kunde1/documents", "/documents");
  result.mount("/work/kunde2/documents", "/documents");
  return result;
}

void
testGetter() {
  Mounts mounts = testSetup();

  std::vector<string_t> virtualPathes;
  auto collectVirtualPathes = [&](const string_t& virtualPath) {
    virtualPathes.push_back(virtualPath);
  };

  virtualPathes.clear();
  mounts.forEachMount("/home/user1", collectVirtualPathes);
  assert(virtualPathes.size() == 0); // path is not mounted to virtual

  virtualPathes.clear();
  mounts.forEachMount("/documents/document", collectVirtualPathes);
  assert(virtualPathes.size() == 2);
  assert(vector_contains(virtualPathes, "/work/kunde1/documents/document"));
  assert(vector_contains(virtualPathes, "/work/kunde2/documents/document"));

  virtualPathes.clear();
  mounts.forEachMount("/customers/kunde1/invoices/invoice1", collectVirtualPathes);
  assert(virtualPathes.size() == 1); // path is not mounted to virtual
  assert(vector_contains(virtualPathes, "/work/kunde1/invoices/invoice1"));

  // mount points
  assert("/workspace" == mounts.absolutePath(""));
  assert("/documents/document" == mounts.absolutePath("/work/kunde1/documents/document"));
  assert("/documents/document" == mounts.absolutePath("/work/kunde2/documents/document"));

  // non mount points
  assert("/workspace/a" == mounts.absolutePath("/a"));
  assert("/workspace/z" == mounts.absolutePath("/z"));
  assert("/virtual/work/kunde0a" == mounts.absolutePath("/work/kunde0a"));
  assert("/virtual/work/kunde1a" == mounts.absolutePath("/work/kunde1a"));
  assert("/virtual/work/kunde2a" == mounts.absolutePath("/work/kunde2a"));

  auto documentsVirtualPath = mounts.GetVirtualPath("/documents");
  assert("/work/kunde1/documents" == documentsVirtualPath ||
         "/work/kunde2/documents" == documentsVirtualPath);

  std::map<string_t, string_t> subMounts;
  auto collectSubMounts = [&](const string_t& virtualPath, const string_t& absolutePath) {
    subMounts[virtualPath] = absolutePath;
  };
  mounts.forEachMountBelow("/work", collectSubMounts);
  assert(subMounts.size() == 4);
  assert(subMounts["/work/kunde1"] == "/customers/kunde1");
  assert(subMounts["/work/kunde1/documents"] == "/documents");
}

void
testUnmountsIn() {
  Mounts mounts = testSetup();
  std::vector<string_t> removedAbsolutePathes;
  auto logRemovedAbsolutePathes = [&](const string_t& absolutePath) {
    removedAbsolutePathes.push_back(absolutePath);
  };
  mounts.unmountIn("/work", logRemovedAbsolutePathes);
  assert(4 == removedAbsolutePathes.size());
  assert(vector_contains(removedAbsolutePathes, "/virtual/work"));
  assert(vector_contains(removedAbsolutePathes, "/customers/kunde1"));
  assert(vector_contains(removedAbsolutePathes, "/customers/kunde2"));
  assert(vector_contains(removedAbsolutePathes, "/documents"));
  assert("/workspace/work/kunde1/documents/document" ==
         mounts.absolutePath("/work/kunde1/documents/document"));

  mounts = testSetup();
  removedAbsolutePathes.clear();
  mounts.unmountIn("/work/kunde1", logRemovedAbsolutePathes);
  assert(1 == removedAbsolutePathes.size());
  assert(vector_contains(removedAbsolutePathes, "/customers/kunde1"));
  assert("/virtual/work/kunde1/documents/document" ==
         mounts.absolutePath("/work/kunde1/documents/document"));
  assert("/documents/document" == mounts.absolutePath("/work/kunde2/documents/document"));
}

void
testUnmountAbsolute() {
  Mounts mounts = testSetup();
  std::vector<string_t> removedAbsolutePathes;
  auto logRemovedAbsolutePathes = [&](const string_t& absolutePath) {
    removedAbsolutePathes.push_back(absolutePath);
  };
  mounts.unmountAbsolute("/documents", logRemovedAbsolutePathes);
  assert(1 == removedAbsolutePathes.size());
  assert(vector_contains(removedAbsolutePathes, "/documents"));
  assert("/customers/kunde1/documents/document" ==
         mounts.absolutePath("/work/kunde1/documents/document"));
  assert("/customers/kunde2/documents/document" ==
         mounts.absolutePath("/work/kunde2/documents/document"));
}

void
testNestedMount() {
  // nested mounts are not allowed test that it's rejected
  Mounts mounts = testSetup();

  auto success =
      mounts.mount("/test", "/documents/document"); // "/documents" is already mounted to customers
  assert(!success);

  success = mounts.mount("/customers",
                         "/customers"); // "/customers/kunde1" is already mounted to work/kunde1
  assert(!success);
}

void
testExample() {
  Mounts mounts("C:/Users/admin/documents");
  mounts.mount("/work", "W:/documents");
  mounts.mount("/nas", "N:/documents");
  mounts.mount("/work/kunde1", "W:/kunde1");
  mounts.mount("/work/kunde2", "W:/kunde2");
  mounts.mount("/work/kunde1/shared", "W:/shared");
  mounts.mount("/work/kunde2/shared", "W:/shared");

  assert(mounts.absolutePath("/work/kunde1/Vertrag.doc") == "W:/kunde1/Vertrag.doc");

  auto mountPathes = mounts.allMountPathes("W:/shared/Styleguide.md");
  assert(mountPathes.size() == 2);
  assert(vector_contains(mountPathes, "/work/kunde1/shared/Styleguide.md"));
  assert(vector_contains(mountPathes, "/work/kunde2/shared/Styleguide.md"));

  auto mountPoints = mounts.getMountPointsIn("/work/kunde1");
  assert(mountPoints.size() == 2);
  assert(vector_contains(mountPoints, std::make_pair("/work/kunde1", "W:/kunde1")));
  assert(vector_contains(mountPoints, std::make_pair("/work/kunde1/shared", "W:/shared")));
}

} // namespace

int
main() {
  testTools();
  testGetter();
  testUnmountsIn();
  testUnmountAbsolute();
  testNestedMount();
  testExample();
  std::cout << "success!" << std::endl;
}
