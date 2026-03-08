#include "parser.h"
#include "ValidatorHandler.h"
#include <cstring>
#include <iostream>

int main(void){
    const char json[] = R"({"emoji": 999999999999999999999999999999999999999999})";
    Validator_handler handler;
    Parser parer{json, strlen(json), handler};
    auto res = parer.parse();
    if(!res){
        std::cout << res.error().error_descrip << std::endl;
        std::cout << res.error().offset_of_error << std::endl;
    }
    if(!handler.valid()) std::cout<<handler.reason() << std::endl;
    
}