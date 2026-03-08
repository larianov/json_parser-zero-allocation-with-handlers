#pragma once
#include "lexical_parser.h"
#include <cstddef>
#include <cstdint>
#include <expected>
#include <string_view>
#include "handler.h"

enum ParseResult{
    PARSE_ERROR_TOO_DEEP,
    PARSE_ERROR_UNEXPECTED_TOKEN,
    PARSE_ERROR_UNEXPECTED_EOF,
    PARSE_ERROR_INVALID_VALUE
};

struct Parse_Error{
    ParseResult result; 
    std::string_view error_descrip;
    size_t offset_of_error;
};

class Parser{
private:
    Lexer lexer;
    uint64_t cont_mask{};
    uint64_t key_mask{};
    int depth{};
    Handler &handler;
    bool flag_after_value{false};
    uint64_t opened_mask{};
    void reset();
    
    bool just_opened() {
        return (opened_mask >> (depth - 1)) & 1;
    }
    
    bool in_object(){
        if(depth == 0)return false;
        return((cont_mask >> (depth-1)) & 1);
    }
    bool waiting_key(){
        return((key_mask >> (depth-1)) & 1);
    }
    
    void push(bool type);
    void pop();
    
    public:
    Parser(const char *buf, const size_t len, Handler &handler);
    std::expected<void, Parse_Error> parse();
};