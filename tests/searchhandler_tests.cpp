#include "parser.h"
#include "searchhandler.h"
#include <iostream>
#include <string>
#include <string_view>

namespace {
int failures = 0;

void check(std::string_view name, std::string_view json,
           std::string_view path, bool expected_found,
           std::string_view expected_value = {}) {
    Searchhandler handler{path};
    Parser parser{json.data(), json.size(), handler};
    const auto result = parser.parse();
    if (!result || handler.found() != expected_found ||
        (expected_found && handler.result() != expected_value)) {
        std::cerr << "FAIL: " << name << '\n';
        ++failures;
    }
}
}

int main() {
    check("nested string", R"({"board":{"station":"Enschede"}})",
          "board.station", true, "Enschede");
    check("skip object before target",
          R"({"ignored":{"a":1,"b":2},"target":"Dortmund Hbf"})",
          "target", true, "Dortmund Hbf");
    check("skip nested arrays and objects",
          R"({"ignored":[1,{"target":"wrong"},[2,3]],"target":"right"})",
          "target", true, "right");
    check("skip sibling inside matching parent",
          R"({"board":{"other":{"a":1,"b":2},"train":{"destination":"Dortmund Hbf"}}})",
          "board.train.destination", true, "Dortmund Hbf");
    check("same key at wrong depth",
          R"({"other":{"target":"wrong"},"target":"right"})",
          "target", true, "right");
    check("incomplete prefix is not a match", R"({"board":1,"station":"wrong"})",
          "board.station", false);
    check("containers are not scalar results", R"({"target":{"value":42}})",
          "target", false);
    check("arrays are not searched", R"({"target":[{"value":42}]})",
          "target.value", false);
    check("root arrays are not searched", R"([{"target":42}])", "target", false);
    check("empty sibling containers", R"({"a":{},"b":[],"target":42})",
          "target", true, "42");
    check("number", R"({"target":-1.25e2})", "target", true, "-1.25e2");
    check("true", R"({"target":true})", "target", true, "true");
    check("false", R"({"target":false})", "target", true, "false");
    check("null", R"({"target":null})", "target", true, "null");
    check("missing key", R"({"other":42})", "target", false);
    check("first duplicate wins", R"({"target":1,"target":2})", "target", true, "1");
    check("raw escapes", R"({"target":"Dortmund\nHbf"})", "target", true,
          R"(Dortmund\nHbf)");

    for (const auto path : {"", ".target", "target.", "board..station"}) {
        Searchhandler handler{path};
        if (handler.valid_path()) {
            std::cerr << "FAIL: invalid path accepted: " << path << '\n';
            ++failures;
        }
    }
    std::string long_path = "key";
    for (int i = 1; i < 64; ++i) long_path += ".key";
    Searchhandler max_path{long_path};
    if (!max_path.valid_path()) ++failures;
    long_path += ".key";
    Searchhandler too_long{long_path};
    if (too_long.valid_path()) ++failures;

    std::string deep_json, deep_path;
    for (int i = 0; i < 63; ++i) {
        deep_json += R"({"key":)";
        if (i > 0) deep_path += '.';
        deep_path += "key";
    }
    deep_json += "42";
    deep_json += std::string(63, '}');
    check("parser depth limit", deep_json, deep_path, true, "42");

    if (failures == 0) std::cout << "All Searchhandler tests passed\n";
    return failures == 0 ? 0 : 1;
}
