#include "handler.h"
#include <iostream>
#include "parser.h"
class DebugHandler : public Handler {
    int indent = 0;
    
    void print(const std::string& msg) {
        for(int i = 0; i < indent; i++) std::cout << "  ";
        std::cout << msg << "\n";
    }
public:
    bool on_object_start()override { print("{"); indent++; return true; }
    bool on_object_end()override { indent--; print("}"); return true; }
    bool on_array_start()override { print("["); indent++; return true; }
    bool on_array_end()override { indent--; print("]"); return true; }
    bool on_key(std::string_view k) override { print("key: " + std::string(k)); return true; }
    bool on_string(std::string_view v)override { print("str: " + std::string(v)); return true; }
    bool on_number(std::string_view v)override { print("num: " + std::string(v)); return true; }
    bool on_bool(bool v)override { print(v ? "true" : "false"); return true; }
    bool on_null()override { print("null"); return true; }
    
    
    void run_test(const std::string& name, const std::string& input) {
        std::cout << "\n=== " << name << " ===\n";
        std::cout << "input: " << input << "\n";
        
        std::string buf = input;
        DebugHandler handler;
        Parser parser(buf.data(), buf.size(), handler);
        
        auto result = parser.parse();
        if(result) {
            std::cout << "OK\n";
        } else {
            auto& err = result.error();
            std::cout << "ERROR: " << err.error_descrip
                      << " at offset " << err.offset_of_error << "\n";
        }
    }
};
