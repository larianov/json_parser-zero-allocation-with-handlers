#pragma once
#include "default_handler.h"
#include <array>
#include <cstddef>
#include <string_view>

// Searches object keys separated by dots. Arrays are skipped, not indexed.
// The path and JSON input must outlive the handler and its returned views.
class Searchhandler : public Default_Handler {
    struct Frame {
        std::size_t matched{};
        bool object{};
        bool matching{};
        bool key_matches{};
    };

    std::array<std::string_view, 64> path{};
    std::array<Frame, 64> frames{};
    std::size_t elems{};
    std::size_t depth{};
    std::size_t overflow_depth{};
    bool valid_path_{false};
    bool found_{false};
    std::string_view result_{};

    bool container_start(bool object);
    bool container_end();
    bool scalar(std::string_view value);

public:
    explicit Searchhandler(std::string_view path_for_json);
    bool valid_path() const;
    bool found();
    std::string_view result();
    bool on_array_start() override;
    bool on_array_end() override;
    bool on_object_start() override;
    bool on_object_end() override;
    bool on_key(std::string_view str) override;
    bool on_number(std::string_view str) override;
    bool on_string(std::string_view str) override;
    bool on_null() override;
    bool on_bool(bool val) override;
};
