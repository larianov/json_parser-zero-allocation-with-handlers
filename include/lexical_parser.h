#pragma once
#include <cstddef>
#pragma once
#include <cstddef>
#include <string_view>

enum TokenType {
    TOKEN_LEFT_BRACE,      
    TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACKET,    
    TOKEN_RIGHT_BRACKET,   
    TOKEN_COMMA,           
    TOKEN_COLON,
    TOKEN_STRING,          
    TOKEN_NUMBER,          
    TOKEN_TRUE,            
    TOKEN_FALSE,           
    TOKEN_NULL,            
    TOKEN_EOF,            
    TOKEN_ERROR
};

struct token{
    TokenType type_;
    std::string_view payload_;
    std::size_t offset_;
};

class Lexer {
    const char* start;    
    const char* current;  
    const char* end;    
public:
    Lexer(const char* buf, size_t len);
    token next_token();
};