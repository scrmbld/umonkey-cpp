#include <cassert>
#include <cctype>
#include <fstream>
#include <iostream>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <vector>
using namespace std;

// read file into string
string readFile(string filename) {
  const size_t BUFSIZE = 1024;
  string result;
  ifstream ins(filename);
  char buf[BUFSIZE];
  while (ins.read(buf, BUFSIZE)) {
    result.append(buf, 0, ins.gcount());
  }
  result.append(buf, 0, ins.gcount());
  return result;
}

// token types
// TODO: figure out which ones of these we don't need (not strictly necessary)
enum class TokenType {
  Illegal,
  Eof,

  // literals
  Ident,
  Int,
  String,

  // operators
  Assign, // =
  Exclam,
  Equal, // ==
  NotEqual,
  Less,
  Greater,
  Plus,
  Minus,
  Star,
  Slash,

  // delimiters
  Comma,
  Semicolon,
  LParen,
  RParen,
  LCurly,
  RCurly,

  // keywords
  Function,
  If,
  Else,
  For,
  Let,
  True,
  False,
  Return
};

// we need this to print out the result of our lexer
string tokenTypeToStr(TokenType tt) {
  // lookup table of ttype -> string
  const vector<string> tokenTypeStrings = {
    "Illegal",
    "Eof",

    // literals
    "Ident",
    "Int",
    "String",

    // operators
    "Assign",
    "Exclam",
    "Equal",
    "NotEqual",
    "Less",
    "Greater",
    "Plus",
    "Minus",
    "Star",
    "Slash",

    // delimiters
    "Comma",
    "Semicolon",
    "LParen",
    "RParen",
    "LCurly",
    "RCurly",

    // keywords
    "Function",
    "If",
    "Else",
    "For",
    "Let",
    "True",
    "False",
    "Return",
  };

  return tokenTypeStrings.at(static_cast<int>(tt));
}

// Token type
// we need the string literal for getting the values of  int and string literals
struct Token {
  TokenType type;
  string literal;
};

// TODO: test cases
class Lexer {
private:
  string sourceText;
  size_t position;
  size_t nextPosition;
  char ch;

  // consumes a character
  // increments our position values and sets ch
  void readChar() {
    if (nextPosition > sourceText.size() - 1) {
      ch = '\0';
      position = nextPosition;
    } else {
      ch = sourceText.at(nextPosition);
      position = nextPosition;
      nextPosition++;
    }
  }

  char peekChar() {
    assert(nextPosition < sourceText.size());
    return sourceText.at(nextPosition);
  }

  void consumeWhitespace() {
    while (isspace(ch) && ch != '\0') {
      readChar();
    }
  }

  Token lexInt() {
    string intLit;
    while (isdigit(ch)) {
      intLit.push_back(ch);
      readChar();
    }

    return {TokenType::Int, intLit};
  }

  Token lexString() {
    string stringLit;
    // TODO: allow escape sequence for '"' characters
    while (ch != '\"') {
      stringLit.push_back(ch);
    }

    return {TokenType::String, stringLit};
  }

  string readIdentifier() {
    string result;
    // first character must be alphabetic
    if (!isalpha(ch)) {
      return "";
    }
    // subsequent characters chan be numbers or underscores
    while (isalnum(ch) || ch == '_') {
      result.push_back(ch);
      readChar();
    }
    return result;
  }

public:
  Lexer(string sourceText) {
    this->sourceText = sourceText;
    this->position = 0;
    this->nextPosition = 0;
    // read the first character to initialize everything
    readChar();
  }

  // lex the next token
  Token nextToken() {
    consumeWhitespace();

    optional<Token> tok;

    switch (ch) {
      case '\0':
        tok = {TokenType::Eof, ""};
        break;
      case '=':
        switch(peekChar()) {
          case '=':
            readChar(); // advance, we are reading 2 chars instead of 1
            tok = {TokenType::Equal, "=="};
            break;
          default:
            tok = {TokenType::Assign, "="};
            break;
        }
        break;
      case '!':
        switch (peekChar()) {
          case '=':
            readChar();
            tok = {TokenType::NotEqual, "!="};
            break;
          default:
            tok = {TokenType::Exclam, "!"};
            break;
        }
        break;
      case '<':
        tok = {TokenType::Less, "<"};
        break;
      case '>':
        tok = {TokenType::Greater, ">"};
        break;
      case '+':
        tok = {TokenType::Plus, "+"};
        break;
      case '-':
        tok = {TokenType::Minus, "-"};
        break;
      case '*':
        tok = {TokenType::Star, "*"};
        break;
      case '/':
        tok = {TokenType::Slash, "*"};
        break;
      case ',':
        tok = {TokenType::Comma, ","};
        break;
      case ';':
        tok = {TokenType::Semicolon, ";"};
        break;
      case '(':
        tok = {TokenType::LParen, "("};
        break;
      case ')':
        tok = {TokenType::RParen, ")"};
        break;
      case '{':
        tok = {TokenType::LCurly, "{"};
        break;
      case '}':
        tok = {TokenType::RCurly, "}"};
        break;
      default:
         if (isalpha(ch)) {
          string literal = readIdentifier();
          cout << '\"' << literal << '\"' << endl;
          if (literal == "if") {
            return {TokenType::If, literal};
          } else if (literal == "else") {
            return {TokenType::Else, literal};
          } else if (literal == "fn") {
            return {TokenType::Function, literal};
          } else if (literal == "let") {
            return {TokenType::Let, literal};
          } else if (literal == "true") {
            return {TokenType::True, literal};
          } else if (literal == "false") {
            return {TokenType::False, literal};
          } else if (literal == "return") {
            return {TokenType::Return, literal};
          } else {
            return {TokenType::Ident, literal};
          }
        } else if (isdigit(ch)) {
          return lexInt();
        } else if (ch == '\"') {
          return lexString();
        } else {
          readChar(); // avoid infinite loop on illegal
          return {TokenType::Illegal, ""};
        }
    }

    if (tok.has_value()) {
      readChar();
      return tok.value();
    } else { // this fixes the warning and should be the final "else" anyways
      return {TokenType::Illegal, ""};
    }
  }

  // lex the whole string
  vector<Token> lexSource() {
    vector<Token> result;
    Token currentToken = nextToken();
    result.push_back(currentToken);
    while (currentToken.type != TokenType::Eof) {
      currentToken = nextToken();
      result.push_back(currentToken);
    }

    return result;
  }
};

int main() {
  string programText = readFile("infile");
  Lexer lex(programText);
  vector<Token> programTokens = lex.lexSource();
  for (Token t : programTokens) {
    cout << tokenTypeToStr(t.type) << ", ";
  }
  return 0;
}
