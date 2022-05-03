#pragma once

#include <boost/filesystem.hpp>
#include <boost/iterator/iterator_adaptor.hpp>

namespace Gengine {

template <typename T>
class DirectoryIterator
    : public boost::iterator_adaptor<DirectoryIterator<T>, T> {
  typedef boost::iterator_adaptor<DirectoryIterator<T>, T> super_t;
  friend class boost::iterator_core_access;

 public:
  DirectoryIterator() = default;

  explicit DirectoryIterator(boost::filesystem::wpath const& path)
      : super_t(BOOST_DEDUCED_TYPENAME super_t::base_type(path)) {}

  int level() { return level(super_t::base()); }

 private:
  void increment() {
    try {
      BOOST_DEDUCED_TYPENAME super_t::base_type& iter =
          super_t::base_reference();
      if (boost::filesystem::is_symlink(*iter))
        no_push(iter);

      ++iter;
    } catch (...) {
      no_push(super_t::base_reference());
      try {
        ++super_t::base_reference();
      } catch (...) {
      }
    }
  }

  int level(boost::filesystem::recursive_directory_iterator const& iter) {
    return iter.level();
  }
  int level(boost::filesystem::directory_iterator const& iter) { return 0; }

  void no_push(boost::filesystem::directory_iterator& iter) {}
  void no_push(boost::filesystem::recursive_directory_iterator& iter) {
    iter.no_push();
  }
};

typedef DirectoryIterator<boost::filesystem::directory_iterator>
    DirectoryIterator_t;
typedef DirectoryIterator<boost::filesystem::recursive_directory_iterator>
    RecursiveDirectoryIterator_t;
}  // namespace Gengine