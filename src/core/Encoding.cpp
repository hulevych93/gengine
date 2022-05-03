#include <core/Encoding.h>

#include <boost/locale.hpp>

namespace Gengine {

std::string toUtf8(const std::wstring& strWcharString) {
  return boost::locale::conv::from_utf(strWcharString, "ISO-8859-8");
}

std::wstring utf8toWchar(const std::string& strUTF8String) {
  return boost::locale::conv::to_utf<wchar_t>(strUTF8String, "ISO-8859-8");
}

}  // namespace Gengine
