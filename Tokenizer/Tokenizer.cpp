#include <cctype>
#include <iostream>
#include <unordered_map>
#include "Tokenizer.hpp"

namespace sail
{

    using namespace std;

    static const unordered_map<string, TokenType> KEYWORDS = {
        {"function", TokenType::FUNCTION_CLAUSE},
        {"clause", TokenType::FUNCTION_CLAUSE},
        {"execute", TokenType::EXECUTE},
        {"let", TokenType::LET},
        {"in", TokenType::IN},
        {"if", TokenType::IF},
        {"then", TokenType::THEN},
        {"else", TokenType::ELSE},
        {"match", TokenType::MATCH},
        {"return", TokenType::RETURN},
        {"foreach", TokenType::FOREACH},
        {"bits", TokenType::BITS},
        {"bool", TokenType::BOOL},
        {"int", TokenType::INT},
        {"nat", TokenType::NAT},
        {"unit", TokenType::UNIT},
        {"vector", TokenType::VECTOR},
        {"RETIRE_SUCCESS", TokenType::RETIRE_SUCCESS},
        {"RETIRE_FAIL", TokenType::RETIRE_FAIL},
        {"EXTS", TokenType::EXTS},
        {"EXTZ", TokenType::EXTZ},
        {"X", TokenType::X_REG},
        {"F", TokenType::F_REG},
        {"CSR", TokenType::CSR_REG},
        {"ror32", TokenType::ROR},
        {"ror64", TokenType::ROR},
        {"rol32", TokenType::ROL},
        {"rol64", TokenType::ROL},
        {"sail_zeros", TokenType::SAIL_ZEROS},
        {"sail_ones", TokenType::SAIL_ONES},
        {"replicate_bits", TokenType::REPLICATE_BITS},
        {"truncate", TokenType::TRUNCATE},
        {"zero_extend", TokenType::ZERO_EXTEND},
        {"sign_extend", TokenType::SIGN_EXTEND},
        {"unsigned", TokenType::UNSIGNED},
        {"signed", TokenType::SIGNED},
    };

    /*
    RETIRE_SUCCESS - and similar requires a check if alnum or '_'
    */
    bool ident(char c) { return isalnum(c) || c == '_'; }

    vector<Token> Tokenizer::parse(const std::string & contents)
    {
        vector<Token> tokens;
        size_t line = 1;
        size_t i = 0;

        // New(Ma-gi-cian) : we can have functions inside function in cpp using lambda or else we
        // get error on implementing the function.
        auto create_token = [&](TokenType type, const string & text, size_t start, size_t end)
        {
            Token t;
            t.sType = type;
            t.sText = text;
            t.linenumber = line;
            t.startoff = start;
            t.endoff = end;

            tokens.push_back(t);
        };

        while (i < contents.size())
        {
            char c = contents[i];
            if (c == '\n')
            {
                ++line;
                ++i;
                continue;
            }

            if (isspace(c))
            {
                ++i;
                continue;
            }

            // handle the comments
            if (c == '/' && contents[i + 1] == '/')
            {
                i += 2;
                while (i < contents.size() && contents[i] != '\n')
                {
                    ++i;
                }
                continue;
            }

            if (isdigit(c))
            {

                size_t start = i;
                // just created a binary token - BIT_LITERAL
                if (c == '0' && contents[i + 1] == 'b')
                {
                    i += 2; // consuming that '0b'
                    while (i < contents.size() && (contents[i] == '0' || contents[i] == '1'))
                    {
                        ++i;
                    }

                    // we have reached the end of the binary number
                    create_token(TokenType::BIT_LITERAL, contents.substr(start, i - start), start,
                                 i);
                    continue;
                }

                if (c == '0' && contents[i + 1] == 'x')
                {
                    i += 2;
                    while (i < contents.size() && isxdigit(contents[i]))
                    {
                        ++i;
                    }
                    create_token(TokenType::HEX_LITERAL, contents.substr(start, i - start), start,
                                 i);
                    continue;
                }

                while (i < contents.size() && isdigit(contents[i]))
                {
                    ++i;
                }
                create_token(TokenType::INTEGER_LITERAL, contents.substr(start, i - start), start,
                             i);
                continue;
            }

            if (isalnum(c))
            {
                size_t start = i;
                while (i < contents.size() && ident(contents[i]))
                {
                    ++i;
                }
                // Go until a whitespace is not found

                string word = contents.substr(start, i - start);

                if (word == "function")
                {
                    size_t saved = i;
                    while (i < contents.size() && isspace(contents[i]))
                    {
                        ++i;
                    }
                    if (i + 6 <= contents.size() && contents.substr(i, 6) == "clause"
                        && (i + 6 >= contents.size() || !ident(contents[i + 6])))
                    {
                        i += 6;
                        create_token(TokenType::FUNCTION_CLAUSE, "function clause", start, i);
                        continue;
                    }

                    i = saved;
                    create_token(TokenType::FUNCTION_CLAUSE, word, start, i);
                    continue;
                }

                auto it = KEYWORDS.find(word);
                TokenType type = (it != KEYWORDS.end()) ? it->second : TokenType::IDENTIFIER;
                create_token(type, word, start, i);
                continue;
            }

            size_t tok_start = i;

            switch (c)
            {
                case '+':
                    create_token(TokenType::PLUS, "+", tok_start, ++i);
                    break;

                case '-': // we are getting an arrow maybe
                    if (contents[i + 1] == '>')
                    {
                        i += 2;
                        create_token(TokenType::ARROW, "->", tok_start, i);
                    }
                    else
                    {
                        create_token(TokenType::MINUS, "-", tok_start, i);
                    }
                    break;

                case '*':
                    create_token(TokenType::STAR, "*", tok_start, ++i);
                    break;

                case '/':
                    create_token(TokenType::SLASH, "/", tok_start, ++i);
                    break;

                case '%':
                    create_token(TokenType::MODULO, "%", tok_start, ++i);
                    break;

                case '^':
                    create_token(TokenType::XOR, "^", tok_start, ++i);
                    break;

                case '~':
                    create_token(TokenType::NOT, "~", tok_start, ++i);
                    break;

                case '|':
                    if (contents[i + 1] == '|')
                    {
                        // we got a or
                        i += 2;
                        create_token(TokenType::OR_OR, "||", tok_start, i);
                    }
                    else
                    {
                        create_token(TokenType::OR, "|", tok_start, ++i);
                    }
                    break;

                case '<': // < , <= , <<
                    if (contents[i + 1] == '=')
                    {
                        i += 2;
                        create_token(TokenType::LESS_EQUAL, "<=", tok_start, i);
                    }
                    else if (contents[i + 1] == '<')
                    {
                        i += 2;
                        create_token(TokenType::LSHIFT, "<<", tok_start, i);
                    }
                    else
                    {
                        create_token(TokenType::LESS, "<", tok_start, ++i);
                    }
                    break;

                case '>':
                    if (contents[i + 1] == '=')
                    {
                        i += 2;
                        create_token(TokenType::GREATER_EQUAL, ">=", tok_start, i);
                    }
                    else if (contents[i + 1] == '>')
                    {
                        if (contents[i + 2] == '>')
                        {
                            i += 3;
                            create_token(TokenType::ARITH_RSHIFT, ">>>", tok_start, i);
                        }
                        else
                        {
                            i += 2;
                            create_token(TokenType::RSHIFT, ">>", tok_start, i);
                        }
                    }
                    else
                    {
                        create_token(TokenType::GREATER, ">", tok_start, ++i);
                    }
                    break;

                case '=':
                    if (contents[i + 1] == '=')
                    {
                        i += 2;
                        create_token(TokenType::EQUAL_EQUAL, "==", tok_start, i);
                    }
                    else if (contents[i + 1] == '>')
                    {
                        i += 2;
                        create_token(TokenType::FAT_ARROW, "=>", tok_start, i);
                    }
                    else
                    {
                        create_token(TokenType::EQUAL, "=", tok_start, ++i);
                    }
                    break;

                case '!':
                    if (contents[i + 1] == '=')
                    {
                        i += 2;
                        create_token(TokenType::NOT_EQUAL, "!=", tok_start, i);
                    }
                    else
                    {
                        create_token(TokenType::BANG, "!", tok_start, ++i);
                    }
                    break;

                case '@':
                    if (contents[i + 1] == '=')
                    {
                        i += 2;
                        create_token(TokenType::AT_EQUAL, "@=", tok_start, i);
                    }
                    else
                    {
                        create_token(TokenType::AT, "@", tok_start, ++i);
                    }
                    break;

                case '.':
                    if (contents[i + 1] == '.')
                    {
                        i += 2;
                        create_token(TokenType::DOTDOT, "..", tok_start, i);
                    }
                    else
                    {
                        create_token(TokenType::UNKNOWN, ".", tok_start, ++i);
                    }
                    break;

                case '(':
                    create_token(TokenType::LPAREN, "(", tok_start, ++i);
                    break;
                case ')':
                    create_token(TokenType::RPAREN, ")", tok_start, ++i);
                    break;
                case '{':
                    create_token(TokenType::LBRACE, "{", tok_start, ++i);
                    break;
                case '}':
                    create_token(TokenType::RBRACE, "}", tok_start, ++i);
                    break;
                case '[':
                    create_token(TokenType::LBRACKET, "[", tok_start, ++i);
                    break;
                case ']':
                    create_token(TokenType::RBRACKET, "]", tok_start, ++i);
                    break;
                case ',':
                    create_token(TokenType::COMMA, ",", tok_start, ++i);
                    break;
                case ';':
                    create_token(TokenType::SEMICOLON, ";", tok_start, ++i);
                    break;
                case ':':
                    create_token(TokenType::COLON, ":", tok_start, ++i);
                    break;
                case '_':
                    create_token(TokenType::UNDERSCORE, "_", tok_start, ++i);
                    break;

                default:
                    create_token(TokenType::UNKNOWN, string(1, c), tok_start, ++i);
                    break;
            }
        }

        // Will write an End of file token at the end

        Token eof;
        eof.sType = TokenType::EOF_TYPE;
        eof.sText = "eof";
        eof.endoff = i;
        eof.startoff = i;
        eof.linenumber = line;
        eof.filepath = "";

        tokens.push_back(eof);

        return tokens;
    }
} // namespace sail
