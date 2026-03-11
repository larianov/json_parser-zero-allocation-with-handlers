#include "default_handler.h"
#include <string_view>

class Validator_handler : public Default_Handler{
    bool valid_{true};
    std::string_view reason_{};
    public:
    bool valid()const;
    std::string_view reason()const;
    bool on_number(std::string_view str) override;
    bool on_string(std::string_view str)override;
};