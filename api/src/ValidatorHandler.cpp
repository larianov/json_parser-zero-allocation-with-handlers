#include "ValidatorHandler.h"
#include <algorithm>
#include <charconv>
#include <cstddef>
#include <string_view>
#include <system_error>

bool Validator_handler::on_number(std::string_view str){
    if(!valid_) return false;
    if(std::any_of(str.begin(), str.end(), [](char x){return x == '.' || x == 'e' || x == 'E';})) {
        double x;
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), x, std::chars_format::general);
        if(ec == std::errc{} && ptr == str.data() + str.size()) return true;
        reason_ = "Converting of double didn't work, overflowing";
    } else {
        int x;
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), x);
        if(ec == std::errc{} && ptr == str.data() + str.size()) return true;
        reason_ = "Converting of int didn't work, overflowing";
    }
    valid_ = false;
    return false;
}

bool Validator_handler::on_string(std::string_view str){
    if(!valid_) return false;
    std::string_view list_of_escapings = "\"\\/bfnrt";
    std::string_view list_of_unicode = "1234567890abcdefABCDEF";
    for (size_t i = 0; i < str.size(); i++) {
        if(str[i] != '\\') continue;
        if(i + 1 >= str.size())return false;
        i++;
        if(str[i] == 'u'){
            if(i + 4 >= str.size()){
                reason_ = "Too small unicode";
                valid_ = false;
                return false;
            }
            i++;
            for(size_t j = i; j < i+4; j++){
                if(list_of_unicode.find(str[j]) == std::string::npos){
                    reason_ = "Not existing unicode";
                    valid_ = false;
                    return false;
                }
            }
            i+=4;
            continue;
        }
        if(list_of_escapings.find(str[i]) == std::string::npos){
            reason_ = "Not existing escaping";
            valid_ = false;
            return false;
        }
    }
    return true;
}

bool Validator_handler::valid()const{return valid_;};
std::string_view Validator_handler::reason()const{return reason_;};