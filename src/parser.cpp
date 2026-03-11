#include "parser.h"
#include <cstddef>
#include <expected>
#include "handler.h"
#include "lexical_parser.h"
// #include <iostream>

void Parser::push(bool type){
    if(type){
        cont_mask|=(1ull << depth);
        key_mask|=(1ull<<depth);
    } 
    opened_mask|=(1ull<<depth);
    after_colon = false;
    depth++;
}

void Parser::pop(){
    depth--;
    opened_mask &= ~(1ULL << depth);    
    cont_mask &= ~(1ULL << depth);
    key_mask &= ~(1ULL << depth);
}


Parser::Parser(const char *buf, const size_t len, Handler &handler) : lexer(buf, len), handler(handler){
}

void Parser::reset(){
    cont_mask = 0;
    key_mask = 0;
    opened_mask = 0;
    flag_after_value = false;
    depth = 0;
    after_colon = false;
}



std::expected<void, Parse_Error> Parser::parse(){
    reset();
    while (true) {
        auto token = lexer.next_token();
        // std::cout << "token: " << token.type_ 
        //           << " depth: " << depth
        //           << " after_value: " << flag_after_value
        //           << " waiting_key: " << waiting_key()
        //           << "\n";
        switch (token.type_) {
            case TOKEN_LEFT_BRACE:
                if(depth >= 63){
                    return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_TOO_DEEP, "On 63 level of deep recursion with { you came too deep", token.offset_});
                }
                if(depth > 0 && in_object() && !waiting_key() && !after_colon) return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_UNEXPECTED_TOKEN, "Expected colon after key", token.offset_});                
                push(true);
                flag_after_value = false;
                handler.on_object_start();
                break;
            case TOKEN_RIGHT_BRACE:
                if(!in_object()) return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_UNEXPECTED_TOKEN, "Meet right closes brace without object", token.offset_});
                if(!flag_after_value && !just_opened()){
                    return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_INVALID_VALUE, "Trailing comma", token.offset_});
                }
                pop();
                flag_after_value = true;
                handler.on_object_end();
                break;
            case TOKEN_LEFT_BRACKET:
                if(depth>=63) return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_TOO_DEEP, "On 63 level of deep recursion with [ you came too deep", token.offset_});
                if(depth > 0 && in_object() && !waiting_key() && !after_colon) return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_UNEXPECTED_TOKEN, "Expected colon after key", token.offset_});
                push(false);
                flag_after_value = false;
                handler.on_array_start();
                break;
            case TOKEN_RIGHT_BRACKET:
                if(in_object()) return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_UNEXPECTED_TOKEN, "Expected ] but found }", token.offset_});
                if(!flag_after_value && !just_opened()){
                    return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_INVALID_VALUE, "Trailing comma", token.offset_});
                }
                pop();
                flag_after_value = true;
                handler.on_array_end();
                break;
            case TOKEN_STRING:
                if(depth > 0 && in_object() && !waiting_key() && !after_colon) return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_UNEXPECTED_TOKEN, "Expected colon after key", token.offset_});
                if(depth == 0 && flag_after_value == true) return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_UNEXPECTED_TOKEN, "Two values without comma", token.offset_});
                if(waiting_key()){
                    flag_after_value = false;
                    opened_mask &= ~(1ULL << (depth-1));
                    key_mask &= ~(1ULL << (depth-1));
                    handler.on_key(token.payload_);
                }
                else {
                    flag_after_value = true;
                    opened_mask &= ~(1ULL << (depth-1));
                    handler.on_string(token.payload_);
                }
                after_colon = false;
                break;
            case TOKEN_NUMBER:
                if(depth > 0 && in_object() && !waiting_key() && !after_colon) return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_UNEXPECTED_TOKEN, "Expected colon after key", token.offset_});
                if(depth == 0 && flag_after_value == true) return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_UNEXPECTED_TOKEN, "Two values without comma", token.offset_});
                flag_after_value = true;
                opened_mask &= ~(1ULL << (depth-1));
                handler.on_number(token.payload_);
                after_colon = false;
                break;
            case TOKEN_TRUE:
                if(depth > 0 && in_object() && !waiting_key() && !after_colon) return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_UNEXPECTED_TOKEN, "Expected colon after key", token.offset_});
                if(depth == 0 && flag_after_value == true) return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_UNEXPECTED_TOKEN, "Two values without comma", token.offset_});
                flag_after_value = true;
                opened_mask &= ~(1ULL << (depth-1));
                handler.on_bool(true);
                after_colon = false;
                break;
            case TOKEN_FALSE:
                if(depth > 0 && in_object() && !waiting_key() && !after_colon) return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_UNEXPECTED_TOKEN, "Expected colon after key", token.offset_});
                if(depth == 0 && flag_after_value == true) return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_UNEXPECTED_TOKEN, "Two values without comma", token.offset_});
                flag_after_value = true;
                opened_mask &= ~(1ULL << (depth-1));
                handler.on_bool(false);
                after_colon = false;
                break;
            case TOKEN_NULL:
                if(depth > 0 && in_object() && !waiting_key() && !after_colon) return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_UNEXPECTED_TOKEN, "Expected colon after key", token.offset_});
                if(depth == 0 && flag_after_value == true) return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_UNEXPECTED_TOKEN, "Two values without comma", token.offset_});
                flag_after_value = true;
                opened_mask &= ~(1ULL << (depth-1));
                handler.on_null();
                after_colon = false;
                break;
            case TOKEN_COLON:
                if(!in_object()) return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_UNEXPECTED_TOKEN, "Meet colon not being in object", token.offset_});
                if(waiting_key()) return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_UNEXPECTED_TOKEN, "Meet colon without key", token.offset_});
                if(flag_after_value)return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_UNEXPECTED_TOKEN, "Meet colon after value", token.offset_});
                if(after_colon) return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_UNEXPECTED_TOKEN, "Double colon", token.offset_});
                after_colon = true;
                break;
            case TOKEN_COMMA:
                if(depth == 0) return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_UNEXPECTED_TOKEN, "Comma at top level", token.offset_});
                if(!flag_after_value) return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_UNEXPECTED_TOKEN, "Meet comma not after value", token.offset_});
                flag_after_value = false;
                opened_mask &= ~(1ULL << (depth-1));
                if(in_object())key_mask|=(1ull<<(depth-1));
                break;
            case TOKEN_EOF:
                if (depth != 0)  return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_UNEXPECTED_EOF, "Meet EOF not on \"zero depth\" so you have unclosed ", token.offset_});
                if(!flag_after_value) return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_UNEXPECTED_TOKEN, "Meet colon not being in object", token.offset_});
                return {};
            case TOKEN_ERROR:
                return std::unexpected<Parse_Error>(Parse_Error{PARSE_ERROR_INVALID_VALUE, "Not existing token appeared", token.offset_});
        }
    }
}