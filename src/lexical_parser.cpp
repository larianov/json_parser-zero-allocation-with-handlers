#include "lexical_parser.h"
#include <cctype>
#include <cstddef>
#include <string_view>

Lexer::Lexer(const char* buf, size_t len) : start(buf), current(start), end(buf+len)
{}

token Lexer::next_token(){
    token toret{};
    while (current != end && isspace(*current)) {
        current++;
    }
    if(current == end){
        toret.type_ = TOKEN_EOF;
        toret.offset_ = current - start;
        return toret;
    }
    while (current != end) {
        if(*current == '{'){
            toret.offset_ = current-start;
            current++;
            toret.type_ = TOKEN_LEFT_BRACE;
            return toret;
        }
        else if(*current == '}'){
            toret.offset_ = current-start;
            current++;
            toret.type_ = TOKEN_RIGHT_BRACE;
            return toret;
        }else if(*current == '['){
            toret.type_ = TOKEN_LEFT_BRACKET;
            toret.offset_ = current-start;
            current++;
            return toret;
        }else if(*current == ']'){
            toret.type_ = TOKEN_RIGHT_BRACKET;
            toret.offset_ = current-start;
            current++;
            return toret;
        }else if (*current == ',') {
            toret.type_ = TOKEN_COMMA;
            toret.offset_ = current-start;
            current++;
            return toret;
        }
        else if (*current == ':') {
            toret.type_ = TOKEN_COLON;
            toret.offset_ = current-start;
            current++;
            return toret;
        }
        else if (*current == '\"') {
            toret.offset_ = current-start;
            current++;
            const char *temp = current;
            while (current  != end && *current != '\"') {
                if(*current == '\\'){
                    if(current+2 >= end){
                        toret.type_ = TOKEN_ERROR;
                        toret.offset_ = current-start;
                        return toret;
                    }
                    current+=2;
                    continue;
                }
                current++;
            }
            if(current == end){
                toret.type_ = TOKEN_ERROR;
                return toret;
            }
            toret.type_ = TOKEN_STRING;
            toret.payload_ = std::string_view(temp, current-temp);
            current++;
            return toret;
        }
        else if(*current  == '-' || isdigit(*current)){
            toret.offset_ = current-start;
            const char *temp = current;
            bool flag_two_dots = false;
            current++;
            while (current != end) {
                if(isdigit(*current)){
                    current++;
                    continue;
                }
                else if (*current == 'e' || *current == 'E') {
                    if (current+1 < end && (*(current+1) == '+' || *(current+1) == '-')){
                        current+=2;
                        continue;
                    }
                    current++;
                    continue;
                }
                else if(*current == '.'){
                    if(!flag_two_dots){
                        flag_two_dots = true;
                        current++;
                        continue;
                    }
                    else {
                        toret.type_ = TOKEN_ERROR;
                        return toret;
                    }
                }else {
                    break;
                }
            }
            toret.payload_ = std::string_view(temp, current-temp);
            if(toret.payload_ == "-"){
                toret.type_ = TOKEN_ERROR;
                return toret;
            }
            std::string_view num = toret.payload_;
            if (num.size() > 1 && num[0] == '0' && isdigit(num[1])){
                toret.type_ = TOKEN_ERROR;
                return toret;
            }
            if (num.size() > 2 && num[0] == '-' && num[1] == '0' && isdigit(num[2])) {
                toret.type_ = TOKEN_ERROR;
                return toret;   
            }
            if(num.back() == '.'){
                toret.type_ = TOKEN_ERROR;
                return toret;   
            }
            if (num.back() == 'e' || num.back() == 'E'){
                toret.type_ = TOKEN_ERROR;
                return toret;   
            }
            if (num.back() == '+' || num.back() == '-'){
                toret.type_ = TOKEN_ERROR;
                return toret;   
            }
            toret.type_ = TOKEN_NUMBER;
            return toret;
        }
        //trux
        else if(*current == 't'){
            
            toret.offset_ = current-start;
            if(current+3>=end){
                toret.offset_ = current-start;
                toret.type_ = TOKEN_ERROR;
                return toret;
            }
            if(*(current+1) != 'r' || *(current+2) != 'u' || *(current+3) != 'e'){
                toret.offset_ = current-start;
                toret.type_ = TOKEN_ERROR;
                return toret;;
            }
            current+=3;
            toret.type_ = TOKEN_TRUE;
            current++;
            return toret;;
        }
        else if(*current == 'f'){
            toret.offset_ = current-start;
            if(current+4>=end){
                toret.offset_ = current-start;
                toret.type_ = TOKEN_ERROR;
                return toret;
            }
            if(*(current+1) != 'a' || *(current+2) != 'l' || *(current+3) != 's' || *(current+4) != 'e'){
                toret.offset_ = current-start;
                toret.type_ = TOKEN_ERROR;
                return toret;
            }
            current+=4;
            toret.type_ = TOKEN_FALSE;
            current++;
            return toret;
        }
        else if(*current == 'n'){
            toret.offset_=current-start;
            if(current+3>=end){
                toret.offset_ = current-start;
                toret.type_ = TOKEN_ERROR;
                return toret;
            }
            if(*(current+1) != 'u' || *(current+2) != 'l' || *(current+3) != 'l'){
                toret.offset_ = current-start;
                toret.type_ = TOKEN_ERROR;
                return toret;
            }
            current+=3;
            toret.type_ = TOKEN_NULL;
            current++;
            return toret;
        }
        else {
            toret.offset_ = current-start;
            toret.type_ = TOKEN_ERROR;
            return toret;
        }
    }
    toret.offset_ = current-start;
    toret.type_ = TOKEN_EOF;
    return toret;
}