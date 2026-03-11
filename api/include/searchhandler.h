#include "default_handler.h"
#include <array>
#include <cstddef>
#include <string_view>

class Searchhandler : public Default_Handler{
private:
    std::array<std::string_view, 64>path;
    size_t elems{};
    size_t skip_depth{};
    bool found_{false};
    size_t depth_now{};
    std::string_view result_;
public:
    bool found();
    std::string_view result();
    Searchhandler(std::string_view path_for_json);
    bool on_array_start() override;
    bool on_array_end() override;
    bool on_object_start() override;
    bool on_object_end() override;
    bool on_key(std::string_view str) override;
    bool on_number(std::string_view str)override;
    bool on_string(std::string_view str)override;
    bool on_null()override;
    bool on_bool(bool val)override;
};