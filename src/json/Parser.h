#pragma once

#include <json/Common.h>
#include <memory>

namespace Gengine {
namespace JSON {
class Value;

namespace details {
class NullValue;
}

class Parser {
 public:
  typedef string_t::iterator Location;

  struct Token {
    enum class Kind : std::uint8_t {
      EndOfFile,
      OpenBrace,
      CloseBrace,
      OpenBracket,
      CloseBracket,
      Comma,
      Colon,
      StringLiteral,
      NumberLiteral,
      BooleanLiteral,
      NullLiteral,
      Comment
    };

    Token() : kind(Kind::EndOfFile) {}

    Kind kind;
    std::unique_ptr<details::NullValue> value;
    Location start;
  };

 public:
  Parser();
  ~Parser();

  void Parse(const string_t& document, Value& root);

 private:
  bool NextCharacter(JSON::char_t& character);
  char_t PeekCharacter() const;

  void ParseValue(Location& start, Location& end, JSON::Value& root);
  void ParseValue(Token& token, JSON::Value& root);
  void ParseObject(Token& token, JSON::Value& root);
  void ParseArray(Token& token, JSON::Value& root);

  void GetNextToken(Token& token);
  void CreateToken(Token& token, Token::Kind kind);
  void CreateToken(Token& token, Token::Kind kind, Location& start);

  bool CompleteComment(Token& token);
  bool CompleteStringLiteral(Token& token);
  bool CompleteNumberLiteral(char_t first, Token& token);
  bool CompleteKeywordTrue(Token& token);
  bool CompleteKeywordFalse(Token& token);
  bool CompleteKeywordNull(Token& token);

  void EatWhitespace(char_t& character);
  static bool IsWhitespace(const char_t& character);
  static bool IsEndOfValue(const char_t& character);
  bool HandleUnescapeChar(string_t& value);

 private:
  Location m_position;
  Location m_start;
  Location m_end;

  string_t m_document;
};
}  // namespace JSON
}  // namespace Gengine