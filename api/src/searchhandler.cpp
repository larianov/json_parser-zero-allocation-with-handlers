#include "searchhandler.h"

Searchhandler::Searchhandler(std::string_view str) {
    while (!str.empty()) {
        const auto dot = str.find('.');
        const auto part = str.substr(0, dot);
        if (part.empty() || elems == path.size()) return;
        path[elems++] = part;
        if (dot == std::string_view::npos) {
            valid_path_ = true;
            return;
        }
        str.remove_prefix(dot + 1);
    }
}

bool Searchhandler::container_start(bool object) {
    if (found_ || !valid_path_) return true;
    if (overflow_depth > 0 || depth == frames.size()) {
        ++overflow_depth;
        return true;
    }

    Frame next{};
    next.object = object;
    if (depth == 0) {
        next.matching = object;
    } else {
        auto& parent = frames[depth - 1];
        next.matched = parent.matched + 1;
        next.matching = object && parent.object && parent.matching &&
                        parent.key_matches && next.matched < elems;
        parent.key_matches = false;
    }
    frames[depth++] = next;
    return true;
}

bool Searchhandler::container_end() {
    if (found_ || !valid_path_) return true;
    if (overflow_depth > 0) --overflow_depth;
    else if (depth > 0) --depth;
    return true;
}

bool Searchhandler::on_key(std::string_view str) {
    if (found_ || !valid_path_ || overflow_depth > 0 || depth == 0) return true;
    auto& frame = frames[depth - 1];
    frame.key_matches = frame.object && frame.matching &&
                        frame.matched < elems && str == path[frame.matched];
    return true;
}

bool Searchhandler::scalar(std::string_view value) {
    if (found_ || !valid_path_ || overflow_depth > 0 || depth == 0) return true;
    auto& frame = frames[depth - 1];
    if (frame.object && frame.matching && frame.key_matches &&
        frame.matched + 1 == elems) {
        result_ = value;
        found_ = true;
    }
    frame.key_matches = false;
    return true;
}

bool Searchhandler::on_array_start() { return container_start(false); }
bool Searchhandler::on_array_end() { return container_end(); }
bool Searchhandler::on_object_start() { return container_start(true); }
bool Searchhandler::on_object_end() { return container_end(); }
bool Searchhandler::on_number(std::string_view str) { return scalar(str); }
bool Searchhandler::on_string(std::string_view str) { return scalar(str); }
bool Searchhandler::on_null() { return scalar("null"); }
bool Searchhandler::on_bool(bool val) { return scalar(val ? "true" : "false"); }
bool Searchhandler::valid_path() const { return valid_path_; }
bool Searchhandler::found() { return found_; }
std::string_view Searchhandler::result() { return result_; }
