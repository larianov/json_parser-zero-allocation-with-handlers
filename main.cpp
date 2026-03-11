#include "parser.h"
#include "searchhandler.h"
#include <fstream>
#include <ios>
#include <vector>
#include <iostream>
int main(void){
    std::ifstream file{"data.json", std::ios::binary};
    if(!file){
        std::cerr << "File wasn't opened" << std::endl;
        return 1;
    }
    file.seekg(0, std::ios::end);
    std::vector<char>buf(file.tellg());
    file.seekg(0, std::ios::beg);
    if (!file.read(buf.data(), buf.size())) {
        std::cerr << "Error reading file\n";
        return 1;
    }
    Searchhandler handler{"store.location.zip"};
    Parser parse{buf.data(), buf.size(), handler};
    auto res = parse.parse();
    if(!res){
        std::cout << res.error().error_descrip << std::endl;
        std::cout << res.error().offset_of_error << std::endl;
    }
    if (handler.found()) {
        std::cout << handler.result() << std::endl;
    }
    else return 1;
}