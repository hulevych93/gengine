#pragma once

#include <string>

namespace Gengine {

/**
 * @brief toUtf8
 *
 * Convert given wstring to the utf8 encoding according to ISO-8859-8.
 * @param[in] input wstring.
 * @return converted utf8 string buffer.
 */
std::string toUtf8(const std::wstring& in);

/**
 * @brief utf8toWchar
 *
 * Convert given utf8 string to wstring representation.
 * @param[in] input string.
 * @return converted wstring buffer.
 */
std::wstring utf8toWchar(const std::string& in);

}  // namespace Gengine
