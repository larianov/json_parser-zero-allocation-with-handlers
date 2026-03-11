#include "searchhandler.h"
#include <iterator>
#include <ranges>
#include <string_view>
#include <iostream>

Searchhandler::Searchhandler(std::string_view str){
    for(const auto &&x:std::ranges::views::split(str, '.')){
        path[elems++] = std::string_view(&*x.begin(), std::ranges::distance(x));
    }
}

bool Searchhandler::on_key(std::string_view str) {
    std::cout << "key: " << str 
              << " depth_now: " << depth_now 
              << " skip: " << skip_depth << "\n";
    if(found_)return true;
    if(skip_depth > 0)return true;
    if(depth_now >= elems){
        skip_depth = 1;
        return true;;
    }
    if(str == path[depth_now]){
        depth_now++;
        return true;
    }else {
        skip_depth = 1;  
    }
    return true;
}

bool Searchhandler::on_array_start(){
    if(found_)return true;
    if(skip_depth>0)skip_depth++;
    return true;
}

bool Searchhandler::on_object_start(){
    if(found_)return true;
    if(skip_depth>0)skip_depth++;
    return true;
}

bool Searchhandler::on_array_end(){
    if(found_)return true;
    if(skip_depth>0)skip_depth--;
    else depth_now--;
    return true;
}

bool Searchhandler::on_object_end(){
    if(found_)return true;
    if(skip_depth > 0)skip_depth--;
    else if(depth_now > 0) depth_now--;
    return true;
}

bool Searchhandler::on_string(std::string_view str){
    if(found_)return true;
    if(skip_depth > 0){
        skip_depth--;  
        return true;
    }    
    if(depth_now == elems){
        result_ = str;
        found_ = true;
        return true;
    }
    return true;
}

bool Searchhandler::on_number(std::string_view str){
    if(found_)return true;
    if(skip_depth > 0){
        skip_depth--;  
        return true;
    }    
    if(depth_now == elems){
        result_ = str;
        found_ = true;
        return true;
    }
    return true;
}

bool Searchhandler::on_null(){
    if(found_)return true;
    if(skip_depth > 0){
        skip_depth--;  
        return true;
    }    
    if(depth_now == elems){
        result_ = "null";
        found_ = true;
        return true;
    }
    return true;
}

bool Searchhandler::on_bool(bool val){
    if(found_)return true;
    if(skip_depth > 0){
        skip_depth--;  
        return true;
    }    
    if(depth_now == elems){
        result_ = val ? "true" : "false";
        found_ = true;
        return true;
    }
    return true;
}

bool Searchhandler::found(){
    return found_;
}

std::string_view Searchhandler::result(){
    return result_;
}