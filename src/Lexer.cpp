#include <iostream>
#include <string>
#include "io.h"
#include "Token.h"
#include "Lexer.h"

Lexer::Lexer(const std::string& src)
    : src(src), pos(0), c(src[pos]), loc({1,1})
{
    // std::cout << "========== START DEBUG READ ==========\n";
    // debugRead(true);
    // std::cout << "=========== END DEBUG READ ===========\n";
}

void Lexer::debugRead(bool pretty)
{
    int debugLineNo = 1;
    while (c != '\0')
    {
        if (pretty)
        {
            std::cout << debugLineNo << ": ";
            while (c != '\n' && c != '\0')
            {
                std::cout << c;
                pos++;
                c = src[pos];
            }
            std::cout << '\n';
            debugLineNo++;
        }
        else
        {
            std::cout << (int) c << ' ';
        }
        pos++;
        c = src[pos];
    }
}

Token Lexer::peekToken(int offset)
{
    // TODO: handle overflowing past EOF

    // store previous lexer position
    int prevPos = pos;
    char prevChar = c;
    SourceLocation prevLoc = loc;

    // get next token
    Token tok = nextToken();
    if (offset > 1)
    {
        // subtract 1 from offset because first peek is already done above
        for (int i = 0; i < offset-1; i++)
        {
            tok = nextToken();
        }
    }

    // reset lexer position
    pos = prevPos;
    c = prevChar;
    loc = prevLoc;

    return tok;
}

Token Lexer::nextToken()
{
    if (isspace(c))
    {
        // remove whitespace
        skipWhitespace();
    }
    if (c == '/' && peek() == '/')
    {
        // remove comments
        skipComment();
    }
    if (isalpha(c) || c == '_')
    {
        return lexAlpha();
    }
    else if (isnumber(c))
    {
        return lexNum();
    }
    else
    {
        return lexOther();
    }
}

Token Lexer::lexAlpha()
{
    // TODO: refactor below while loop into separate function
    std::string s;
    // want loc of token to start at beginning of token's value
    SourceLocation tmpLoc(loc.x, loc.y);
    while (isalnum(c) || c == '_')
    {
        s += c;
        next();
    }

    Token::token_type type = Token::TOK_ID;
    if (isKwd(s))
        type = Token::TOK_KWD;
    else if (isType(s))
        type = Token::TOK_TYPE;
    return Token(type, s, tmpLoc);
}

Token Lexer::lexNum()
{
    // TODO: refactor below while loop into separate function
    std::string s;
    SourceLocation tmpLoc(loc.x, loc.y);
    while (isalnum(c))
    {
        s += c;
        next();
    }
    return Token(Token::TOK_NUM, s, tmpLoc);
}

Token Lexer::lexOther()
{
    Token::token_type type = lexTokenType();
    // have to subtract length to get correct start position
    SourceLocation tokLoc(loc.x-(tokenValues.at(type).size()-1), loc.y);
    Token tok = Token(type, tokLoc);
    next();
    return tok;
}

Token::token_type Lexer::lexTokenType()
{
    skipWhitespace();
    switch(c)
    {
        case '(':
            return Token::TOK_OPAREN;
        case ')':
            return Token::TOK_CPAREN;
        case '{':
            return Token::TOK_OBRACK;
        case '}':
            return Token::TOK_CBRACK;
        case ';':
            return Token::TOK_SCOLON;
        case ':':
            return Token::TOK_COLON;
        case ',':
            return Token::TOK_COMMA;
        case '+':
            return Token::TOK_PLUS;
        case '-':
        {
            if (peek() == '>')
            {
                next();
                return Token::TOK_RARROW;
            }
            return Token::TOK_MINUS;
        }
        case '*':
            return Token::TOK_STAR;
        case '/':
        {
            return Token::TOK_FSLASH;
        }
        case '!':
        {
            if (peek() == '=')
            {
                next();
                return Token::TOK_NEQUALS;
            }
        }
        case '=':
        {
            if (peek() == '=')
            {
                next();
                return Token::TOK_DEQUALS;
            }
            return Token::TOK_EQUALS;
        }
        case '<':
        {
            if (peek() == '=')
            {
                next();
                return Token::TOK_LTE;
            }
            return Token::TOK_LT;
        }
        case '>':
        {
            if (peek() == '=')
            {
                next();
                return Token::TOK_GTE;
            }
            return Token::TOK_GT;
        }
        case '&':
        {
            if (peek() == '&')
            {
                next();
                return Token::TOK_DAMPSND;
            }
        }
        case '|':
        {
            if (peek() == '|')
            {
                next();
                return Token::TOK_DBAR;
            }
        }
        case '\0':
            return Token::TOK_EOF;
        default:
            std::cout << "[Dev Warning] Lexer returning TOK_UND with value of <" << c << "> and (ASCII: " << int(c) << ") (Line " << loc.y << " at position " << loc.x << ")\n";
            next();
            return lexTokenType();
    }
}

bool Lexer::isKwd(const std::string& s)
{
    // reserved keywords
    return (s == "return"
            || s == "func"
            || s == "if"
            || s == "else"
            || s == "for"
            || s == "while"
            || s == "in");
}

bool Lexer::isType(const std::string& s)
{
    return (s == "int"
            || s == "array");
}

void Lexer::next()
{
    pos++;
    loc.x++;
    c = src[pos];
}

char Lexer::peek(int offset)
{
    return src[pos+1];
}

void Lexer::skipWhitespace()
{
    while (isspace(c))
    {
        if (c == '\n')
        {
            loc.y++;
            loc.x = 0;
        }
        next();
    }
}

void Lexer::skipComment()
{
    while (c != '\n' && c != '\0')
    {
        next();
    }
    loc.y++;
    loc.x = 0;
    if (isspace(c))
    {
        skipWhitespace();
    }
}

std::string Lexer::getLine(int lineNo)
{
    char srcChar = src[0];
    int srcPos = 0;
    for (int i = 0; i < lineNo-1; i++)
    {
        while (srcChar != '\n' && srcChar != '\0')
        {
            srcPos++;
            srcChar = src[srcPos];
        }
        srcPos++;
        srcChar = src[srcPos];
    }

    std::string line;
    while (srcChar != '\n' && srcChar != '\0')
    {
        line += srcChar;
        srcPos++;
        srcChar = src[srcPos];
    }

    return line;
}
