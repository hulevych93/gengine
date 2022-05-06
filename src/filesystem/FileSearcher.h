#pragma once

#include <filesystem/DirectoryIterator.h>
#include <filesystem/FileDetails.h>
#include <filesystem/FileFilters.h>
#include <filesystem/FileSortRules.h>
#include <functional>

namespace Gengine {

class FileSeacher {
 public:
  FileSeacher() = delete;
  using SearchResult_t = SearchResult_t;
  using SearchCallback_t = std::function<bool(const FileDetails::Ptr&)>;

 public:
  static SearchResult_t Search(const boost::filesystem::wpath& path) {
    SearchResult_t result;
    SearchImpl<DirectoryIterator_t>(path,
                                    [&result](const FileDetails::Ptr& ptr) {
                                      result.emplace_back(ptr);
                                      return true;
                                    });
    return result;
  }

  static SearchResult_t RecursiveSearch(const boost::filesystem::wpath& path) {
    SearchResult_t result;
    SearchImpl<RecursiveDirectoryIterator_t>(
        path, [&result](const FileDetails::Ptr& ptr) {
          result.emplace_back(ptr);
          return true;
        });
    return result;
  }

  static void Search(const boost::filesystem::wpath& path,
                     SearchCallback_t callback) {
    SearchImpl<DirectoryIterator_t>(path, callback);
  }

  static void RecursiveSearch(const boost::filesystem::wpath& path,
                              SearchCallback_t callback) {
    SearchImpl<RecursiveDirectoryIterator_t>(path, callback);
  }

  static SearchResult_t Search(const boost::filesystem::wpath& path,
                               const std::wstring& filter,
                               std::uint32_t flag = 0) {
    SearchResult_t result;
    RegexFilter regexFilter(filter, flag);
    SearchImpl<DirectoryIterator_t>(path,
                                    [&result](const FileDetails::Ptr& ptr) {
                                      result.emplace_back(ptr);
                                      return true;
                                    },
                                    regexFilter);
    return result;
  }

  static SearchResult_t RecursiveSearch(const boost::filesystem::wpath& path,
                                        const std::wstring& filter,
                                        std::uint32_t flag = 0) {
    SearchResult_t result;
    RegexFilter regexFilter(filter, flag);
    SearchImpl<RecursiveDirectoryIterator_t>(
        path,
        [&result](const FileDetails::Ptr& ptr) {
          result.emplace_back(ptr);
          return true;
        },
        regexFilter);
    return result;
  }

  static SearchResult_t Filter(SearchResult_t const& container,
                               std::wstring const& filter) {
    RegexFilter regexFilter(filter);
    return FilterImpl(container, regexFilter);
  }

  static SearchResult_t Sort(SearchResult_t const& container,
                             ISortRule const& rule) {
    return SortImpl(container, rule);
  }

 private:
  template <typename IteratorType>
  static void SearchImpl(const boost::filesystem::wpath& path,
                         SearchCallback_t callback = SearchCallback_t(),
                         const IFilter& filter = DefaultFilter()) {
    if (!boost::filesystem::exists(path))
      throw std::runtime_error(
          boost::str(boost::format("Path %1% is not found") % path.string()));

    boost::filesystem::file_type type = boost::filesystem::status(path).type();
    if (!boost::filesystem::is_directory(path) &&
        type != boost::filesystem::reparse_file)
      throw std::runtime_error(boost::str(
          boost::format("Path %1% is not a directory") % path.string()));

    try {
      IteratorType begin(path), end;
      for (IteratorType iter = begin; iter != end; ++iter) {
        auto item = *iter;
        if (boost::filesystem::is_directory(item) ||
            boost::filesystem::is_symlink(item))
          continue;

        boost::filesystem::wpath path = item;
        if (!filter.Match(path.filename().wstring()))
          continue;

        std::time_t time = boost::filesystem::last_write_time(item);
        auto details = std::make_shared<FileDetails>(item, time, iter.level());
        if (!callback(details))
          break;
      }
    } catch (boost::filesystem::filesystem_error const&) {
      throw;
    } catch (const std::exception&) {
      throw;
    }
  }

  static SearchResult_t FilterImpl(SearchResult_t const& container,
                                   RegexFilter const& filter) {
    SearchResult_t result;

    for (auto item : container) {
      std::wstring const& file = item->FileName();
      if (filter.Match(file))
        result.push_back(item);
    }

    return result;
  }

  static SearchResult_t SortImpl(SearchResult_t const& container,
                                 ISortRule const& rule) {
    SearchResult_t result(container);

    auto predFunction = [&rule](SearchResult_t::value_type const& lhs,
                                SearchResult_t::value_type const& rhs) {
      return rule(lhs, rhs);
    };
    std::sort(result.begin(), result.end(), predFunction);

    return result;
  }
};

}  // namespace Gengine