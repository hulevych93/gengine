#pragma once

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <vector>

namespace Gengine {
class FileDetails {
 public:
  typedef std::shared_ptr<FileDetails> Ptr;

 public:
  FileDetails(boost::filesystem::wpath const& modulePath,
              std::time_t const& lastWriteTime,
              int level)
      : m_FilePath(modulePath), m_LastWriteTime(lastWriteTime), m_Level(level) {
    assert(boost::filesystem::exists(m_FilePath));
    assert(boost::filesystem::is_regular_file(m_FilePath));
    assert(!boost::filesystem::is_symlink(m_FilePath));
    assert(!boost::filesystem::is_directory(m_FilePath));
  }

  int Level() const { return m_Level; }

  std::time_t const& LastWriteTime() const { return m_LastWriteTime; }

  boost::filesystem::wpath const& FilePath() const { return m_FilePath; }

  std::wstring FileName() const { return m_FilePath.filename().wstring(); }

  std::wstring FileNameWithoutExtension() const {
    return m_FilePath.filename().replace_extension().wstring();
  }

  std::wstring RelativePath() const {
    return make_relative(m_FilePath, m_Level).wstring();
  }

 private:
  static boost::filesystem::wpath make_relative(
      boost::filesystem::wpath const& path,
      int level) {
    try {
      boost::filesystem::wpath result;
      boost::filesystem::wpath absolutePath = boost::filesystem::absolute(path);

      int endPath(-level);
      for (auto item : absolutePath)
        ++endPath;

      int itemIndex(0);
      for (auto item : absolutePath) {
        ++itemIndex;
        if (itemIndex >= endPath)
          result /= item;
      }

      return result;
    } catch (boost::filesystem::filesystem_error const&) {
      throw;
    }
  }

 private:
  int m_Level;
  std::time_t m_LastWriteTime;
  boost::filesystem::wpath m_FilePath;
};

typedef FileDetails::Ptr FileDetailsPtr_t;
typedef std::vector<FileDetails::Ptr> SearchResult_t;
}  // namespace Gengine