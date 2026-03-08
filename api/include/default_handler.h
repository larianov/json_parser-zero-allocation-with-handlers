#include "handler.h"

class Default_Handler : public Handler{
    bool on_object_start() override { return true; }
    bool on_object_end()   override { return true; }
    bool on_array_start()  override { return true; }
    bool on_array_end()    override { return true; }
    bool on_key(std::string_view) override { return true; }
    bool on_string(std::string_view) override { return true; }
    bool on_number(std::string_view) override { return true; }
    bool on_bool(bool) override { return true; }
    bool on_null() override { return true; }
};