#include "argparser.h"
#include <iostream>

using namespace nargparse;

int main(){
    ArgumentParser parser = CreateParser("BigParser");

    const char* argv[] = {"program"};
    bool result = Parse(parser, 1, argv);
    std::cout << result << std::endl;
}