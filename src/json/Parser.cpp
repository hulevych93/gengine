#include <json/Parser.h>
#include <limits>

#include <json/JSON.h>

namespace Gengine {
namespace JSON {
using namespace JSON::details;

JSON::Parser::Parser() {}

JSON::Parser::~Parser() {}

void JSON::Parser::Parse(const string_t& document, JSON::Value& root) {
  m_document = document;
  auto start = m_document.begin();
  auto end = m_document.end();

  ParseValue(start, end, root);
}

bool JSON::Parser::NextCharacter(JSON::char_t& character) {
  bool result = false;
  if (m_position != m_end) {
    character = *m_position;
    ++m_position;
    result = true;
  } else {
    character = '\0';
  }

  return result;
}

JSON::char_t JSON::Parser::PeekCharacter() const {
  return *m_position;
}

void JSON::Parser::ParseValue(Location& start,
                              Location& end,
                              JSON::Value& root) {
  m_position = start;
  m_start = start;
  m_end = end;

  Token token;
  GetNextToken(token);
  ParseValue(token, root);
}

void JSON::Parser::ParseValue(Token& token, JSON::Value& root) {
  switch (token.kind) {
    case Token::Kind::OpenBrace:
      ParseObject(token, root);
      break;
    case Token::Kind::OpenBracket:
      ParseArray(token, root);
      break;
    case Token::Kind::StringLiteral:
      try {
        if (!token.value.IsNull()) {
          root = token.value.ToString();
        } else {
          throw std::logic_error("Unable to parse string");
        }
      } catch (const std::exception&) {
        throw std::logic_error("Unable to parse string");
      }
      break;
    case Token::Kind::NumberLiteral:
      try {
        if (!token.value.IsNull()) {
          root = token.value.ToNumber();
        } else {
          throw std::logic_error("Unable to parse number");
        }
      } catch (const std::exception&) {
        throw std::logic_error("Unable to parse number");
      }
      break;
    case Token::Kind::BooleanLiteral:
      try {
        if (!token.value.IsNull()) {
          root = token.value.ToBool();
        } else {
          throw std::logic_error("Unable to parse bool");
        }
      } catch (const std::exception&) {
        throw std::logic_error("Unable to parse bool");
      }
      break;
    case Token::Kind::NullLiteral:
      root = JSON::Value();

      break;
    default:
      throw std::logic_error("Incorrect token type");
      break;
  }

  GetNextToken(token);
}

void JSON::Parser::ParseObject(Token& token, JSON::Value& root) {
  JSON::Object object;
  try {
    GetNextToken(token);

    while (token.kind != Parser::Token::Kind::CloseBrace) {
      // State 1: New field or end of object, looking for field name or closing
      // brace
      string_t fieldName;
      switch (token.kind) {
        case Parser::Token::Kind::StringLiteral:
          if (!token.value.IsNull() &&
              token.value.Type() == JSON::type_t::TypeString) {
            fieldName = token.value.ToString();
          } else {
            throw std::logic_error("Incorrect token type");
          }
          GetNextToken(token);
          break;
        default:
          throw std::logic_error("Field name must be a string value");
          break;
      }

      // State 2: Looking for a colon.
      switch (token.kind) {
        case Parser::Token::Kind::Colon:
          GetNextToken(token);
          break;
        default:
          throw std::logic_error("Colon hasn't found");
          break;
      }

      // State 3: Looking for an expression.
      JSON::Value value;
      ParseValue(token, value);
      object.insert(std::make_pair(fieldName, value));

      // State 4: Looking for a comma or a closing brace
      switch (token.kind) {
        case Parser::Token::Kind::Comma:
          GetNextToken(token);
          break;
        case Parser::Token::Kind::CloseBrace:
          break;
        default:
          throw std::logic_error("Comma hasn't found");
          break;
      }
    }
    root = object;
  } catch (const std::exception& ex) {
    throw std::logic_error("Unable to parse object: " + std::string(ex.what()));
  }
}

void JSON::Parser::ParseArray(Token& token, JSON::Value& root) {
  JSON::Array _array;
  try {
    GetNextToken(token);

    while (token.kind != Parser::Token::Kind::CloseBracket) {
      // State 1: Looking for an expression.
      JSON::Value value;
      ParseValue(token, value);
      _array.push_back(value);

      // State 4: Looking for a comma or a closing bracket
      switch (token.kind) {
        case Parser::Token::Kind::Comma:
          GetNextToken(token);
          break;
        case Parser::Token::Kind::CloseBracket:
          break;
        default:
          throw std::logic_error("Incorrect token type");
      }
    }
    root = _array;
  } catch (std::exception& ex) {
    throw std::logic_error("Unable to parse array: " + std::string(ex.what()));
  }
}

void JSON::Parser::GetNextToken(Token& token) {
try_again:
  char_t ch = '\0';
  EatWhitespace(ch);

  CreateToken(token, Token::Kind::EndOfFile);

  switch (ch) {
    case '{':
      CreateToken(token, Token::Kind::OpenBrace, token.start);
      break;
    case '[':
      CreateToken(token, Token::Kind::OpenBracket, token.start);
      break;
    case '}':
      CreateToken(token, Token::Kind::CloseBrace, token.start);
      break;
    case ']':
      CreateToken(token, Token::Kind::CloseBracket, token.start);
      break;
    case ',':
      CreateToken(token, Token::Kind::Comma, token.start);
      break;
    case ':':
      CreateToken(token, Token::Kind::Colon, token.start);
      break;

    case 't':
      if (!CompleteKeywordTrue(token)) {
        throw std::logic_error("Unable to complete true literal");
      }
      break;
    case 'f':
      if (!CompleteKeywordFalse(token)) {
        throw std::logic_error("Unable to complete false literal");
      }
      break;
    case 'n':
      if (!CompleteKeywordNull(token)) {
        throw std::logic_error("Unable to complete null literal");
      }
      break;
    case '/':
      if (!CompleteComment(token)) {
        throw std::logic_error("Unable to complete comment");
      }
      // For now, we're ignoring comments.
      goto try_again;
    case '"':
      if (!CompleteStringLiteral(token)) {
        throw std::logic_error("Unable to complete string literal");
      }
      break;

    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      if (!CompleteNumberLiteral(ch, token)) {
        throw std::logic_error("Unable to complete number literal");
      }
      break;
    case '\0':
      // EndOfFile. Ignore it!
      break;
    default:
      throw std::logic_error("Incorrect token");
      break;
  }
}

void JSON::Parser::CreateToken(Token& token, Token::Kind kind) {
  token.kind = kind;
  token.start = m_position;
}

bool JSON::Parser::HandleUnescapeChar(string_t& value) {
  char_t ch = '\0';
  if (NextCharacter(ch)) {
    switch (ch) {
      case '\"':
        value.push_back('\"');
        return true;
      case '\\':
        value.push_back('\\');
        return true;
      case '/':
        value.push_back('/');
        return true;
      case 'b':
        value.push_back('\b');
        return true;
      case 'f':
        value.push_back('\f');
        return true;
      case 'r':
        value.push_back('\r');
        return true;
      case 'n':
        value.push_back('\n');
        return true;
      case 't':
        value.push_back('\t');
        return true;
      default:
        return false;
    }
  }

  return false;
}

void JSON::Parser::CreateToken(Token& token,
                               Token::Kind kind,
                               Location& start) {
  token.kind = kind;
  token.start = start;
}

bool JSON::Parser::CompleteComment(Token& token) {
  // We already found a '/' character as the first of a token -- what kind of
  // comment is it?
  bool result = false;
  char_t ch = '\0';

  if (NextCharacter(ch) && (ch == '/' || ch == '*')) {
    if (ch == '/') {
      // Line comment -- look for a newline or EOF to terminate.
      while (NextCharacter(ch) && ch != '\n') {
      }
      result = true;
    } else {
      // Block comment -- look for a terminating "*/" sequence.

      ch = NextCharacter(ch);

      while (true) {
        if (NextCharacter(ch) && ch == '*') {
          if (ch == '*') {
            auto currChar = PeekCharacter();

            if (currChar == '/') {
              // Consume the character
              NextCharacter(ch);
              result = true;
              break;
            }

            ch = currChar;
          }
        } else {
          result = false;
          break;
        }
      }
    }

    token.kind = Token::Kind::Comment;
  }

  return result;
}

bool JSON::Parser::CompleteStringLiteral(Token& token) {
  bool result = false;

  char_t ch = '\0';
  string_t value;
  while (NextCharacter(ch) && ch != '"') {
    if (ch == '\\') {
      HandleUnescapeChar(value);
    } else if ((ch >= 0x00) && (ch < char_t(0x20))) {
      break;
    } else {
      value.push_back(ch);
    }
  }

  if (ch == '"') {
    token.kind = Token::Kind::StringLiteral;
    token.value = value;
    result = true;
  }

  return result;
}

bool JSON::Parser::CompleteNumberLiteral(char_t first, Token& token) {
  bool result = false;
  char_t decimal_separator = '.';

  bool minus_sign;

  if (first == '-') {
    minus_sign = true;
    NextCharacter(first);
  } else {
    minus_sign = false;
  }

  try {
    if ((first < '0') || (first > '9')) {
      throw std::exception();
    }

    char_t ch = PeekCharacter();
    // Check for two (or more) zeros at the beginning
    if (first == '0' && ch == '0') {
      throw std::exception();
    }

    string_t value;
    value.push_back(first);
    bool isReal = false;
    while (!IsEndOfValue(ch)) {
      if ((ch >= '0') && (ch <= '9')) {
        value.push_back(ch);
        NextCharacter(ch);
        ch = PeekCharacter();
      } else if (ch == decimal_separator) {
        if (isReal) {
          throw std::exception();
        }
        isReal = true;
        value.push_back(ch);
        NextCharacter(ch);
        ch = PeekCharacter();

        // Check that the following char is a digit
        if ((ch >= '0') && (ch <= '9')) {
          value.push_back(ch);
          NextCharacter(ch);
          ch = PeekCharacter();
        } else {
          throw std::exception();
        }
      } else if ((ch == 'e') || (ch == 'E')) {
        // Exponent
        if (isReal) {
          throw std::exception();
        }

        isReal = true;
        value.push_back(ch);
        NextCharacter(ch);
        ch = PeekCharacter();

        // Check for the exponent sign
        if ((ch == '+') || (ch == '-')) {
          value.push_back(ch);
          NextCharacter(ch);
          ch = PeekCharacter();
        }

        // First number of the exponent
        if ((ch >= '0') && (ch <= '9')) {
          value.push_back(ch);
          NextCharacter(ch);
          ch = PeekCharacter();
        } else {
          throw std::exception();
        }
      } else {
        throw std::exception();
      }
    }

    if (isReal) {
      double realValue = std::stod(value);
      token.value = Number{realValue};
    } else {
      JSON::uint64_t uintValue = std::stoull(value);
      if (minus_sign) {
        JSON::int64_t intValue = 0;
        if (uintValue < static_cast<JSON::uint64_t>(
                            std::numeric_limits<JSON::int64_t>::max())) {
          intValue -= uintValue;
          token.value = Number{intValue};
        } else {
          throw std::exception();
        }
      } else {
        token.value = Number{uintValue};
      }
    }

    token.kind = Token::Kind::NumberLiteral;
    result = true;
  } catch (const std::exception&) {
  }

  return result;
}

bool JSON::Parser::IsEndOfValue(const char_t& character) {
  return (character == ',') || (character == '\0') || (character == ']') ||
         (character == '}') || (character == '\r') || (character == ' ');
}

bool JSON::Parser::CompleteKeywordTrue(Token& token) {
  char_t ch;
  if (NextCharacter(ch) && ch != 'r')
    return false;
  if (NextCharacter(ch) && ch != 'u')
    return false;
  if (NextCharacter(ch) && ch != 'e')
    return false;
  token.kind = Token::Kind::BooleanLiteral;
  token.value = true;
  return true;
}

bool JSON::Parser::CompleteKeywordFalse(Token& token) {
  char_t ch;
  if (NextCharacter(ch) && ch != 'a')
    return false;
  if (NextCharacter(ch) && ch != 'l')
    return false;
  if (NextCharacter(ch) && ch != 's')
    return false;
  if (NextCharacter(ch) && ch != 'e')
    return false;
  token.kind = Token::Kind::BooleanLiteral;
  token.value = false;
  return true;
}

bool JSON::Parser::CompleteKeywordNull(Token& token) {
  char_t ch;
  if (NextCharacter(ch) && ch != 'u')
    return false;
  if (NextCharacter(ch) && ch != 'l')
    return false;
  if (NextCharacter(ch) && ch != 'l')
    return false;
  token.kind = Token::Kind::NullLiteral;
  token.value = Value{};
  return true;
}

void JSON::Parser::EatWhitespace(char_t& character) {
  while (NextCharacter(character) && IsWhitespace(character)) {
  }
}

bool JSON::Parser::IsWhitespace(const char_t& character) {
  return ((character == ' ') || (character == '\t') || (character == '\r') ||
          (character == '\n'));
}
}  // namespace JSON
}  // namespace Gengine
