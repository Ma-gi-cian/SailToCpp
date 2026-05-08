#pragma once
#include <string>
#include <vector>

namespace sail
{
    using namespace std;

    enum class TokenType
    {
        // Literals
        INTEGER_LITERAL, // 7, 18, 3
        BIT_LITERAL,     // 0b0101
        HEX_LITERAL,     // 0x1F

        // Identifiers & Keywords
        IDENTIFIER,      // rs1, rd, result, inb
        FUNCTION_CLAUSE, // function clause
        EXECUTE,         // execute
        LET,             // let
        IN,              // in
        IF,              // if
        THEN,            // then
        ELSE,            // else
        MATCH,           // match
        RETURN,          // return
        FOREACH,         // foreach

        // Types
        BITS,   // bits(N)
        BOOL,   // bool
        INT,    // int
        NAT,    // nat
        UNIT,   // unit
        VECTOR, // vector

        // RISC-V Specific
        RETIRE_SUCCESS, // RETIRE_SUCCESS
        RETIRE_FAIL,    // RETIRE_FAIL
        EXTS,           // EXTS - sign extend
        EXTZ,           // EXTZ - zero extend
        X_REG,          // X(rd), X(rs1)
        F_REG,          // F(rd), F(rs1) - float regs
        CSR_REG,        // CSR access

        // Operators - Arithmetic
        PLUS,   // +
        MINUS,  // -
        STAR,   // *
        SLASH,  // /
        MODULO, // %

        // Operators - Bitwise
        XOR,          // ^
        AND,          // &
        OR,           // |
        NOT,          // ~
        LSHIFT,       // <<
        RSHIFT,       // >>
        ARITH_RSHIFT, // >>> arithmetic right shift

        // Operators - Comparison
        EQUAL_EQUAL,   // ==
        NOT_EQUAL,     // !=
        LESS,          // <
        LESS_EQUAL,    // <=
        GREATER,       // >
        GREATER_EQUAL, // >=

        // Operators - Logical
        AND_AND, // &&
        OR_OR,   // ||
        BANG,    // !

        // Assignment
        EQUAL,    // =
        AT_EQUAL, // @= concatenation assign

        // Bit Manipulation
        AT,       // @ concatenation
        DOTDOT,   // .. bit slice range
        LBRACKET, // [
        RBRACKET, // ]

        // Sail Builtins - Bit Operations
        ROR,            // ror32, ror64
        ROL,            // rol32, rol64
        SAIL_ZEROS,     // sail_zeros
        SAIL_ONES,      // sail_ones
        REPLICATE_BITS, // replicate_bits
        TRUNCATE,       // truncate
        ZERO_EXTEND,    // zero_extend
        SIGN_EXTEND,    // sign_extend
        UNSIGNED,       // unsigned()
        SIGNED,         // signed()

        // Delimiters
        LPAREN,     // (
        RPAREN,     // )
        LBRACE,     // {
        RBRACE,     // }
        COMMA,      // ,
        SEMICOLON,  // ;
        COLON,      // :
        ARROW,      // ->
        FAT_ARROW,  // =>
        UNDERSCORE, // _

        // Comments
        COMMENT_LINE,  // //
        COMMENT_BLOCK, // /* */

        // Special
        EOF_TYPE, // Changed from EOF (EOF is a macro in cstdio)
        UNKNOWN
    };

    class Token
    {
      public:
        enum TokenType sType;
        string sText;
        size_t startoff;
        size_t endoff;
        size_t linenumber;
    };

    class Tokenizer
    {
      public:
        vector<Token> parse(const string & inprogram);
    };
} // namespace sail