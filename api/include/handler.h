#pragma once
#include <string_view>
class Handler{
public:
    virtual bool on_object_start() = 0;
    virtual bool on_object_end() = 0;
    virtual bool on_array_start() = 0;
    virtual bool on_array_end() = 0;
    virtual bool on_key(std::string_view) = 0;
    virtual bool on_string(std::string_view) = 0;
    virtual bool on_number(std::string_view) = 0;
    virtual bool on_bool(bool) = 0;
    virtual bool on_null() = 0;
    virtual ~Handler() = default;
};
