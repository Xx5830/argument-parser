#include "argparser.h"
#include <iostream>

using namespace nargparse;

int main(){
    ArgumentParser parser = CreateParser("BigParser");

    int first_value = 0;

    nargparse::AddArgument(parser, &first_value, "Numbers", nargparse::kNargsOneOrMore);

    const char* argv[] = {"program", "1", "2", "3", "4", "5"};
    bool flag_true = nargparse::Parse(parser, 6, argv);

    int count = nargparse::GetRepeatedCount(parser, "Numbers");
    for (int i = 0; i < count; ++i) {
        int value;
        flag_true = (nargparse::GetRepeated(parser, "Numbers", i, &value));
    }
}