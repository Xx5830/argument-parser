#include "argparser.h"
#include <iostream>

using namespace nargparse;

int main(){
    ArgumentParser parser = CreateParser("BigParser");

    bool verbose = false;
    bool quiet = false;
    int first_number = 0;

    nargparse::AddFlag(parser, "-v", "--verbose", &verbose, "Verbose output");
    nargparse::AddFlag(parser, "-q", "--quiet", &quiet, "Quiet output");

    nargparse::AddArgument(parser, &first_number, "Numbers", nargparse::kNargsZeroOrMore);

    const char* argv[] = {"program", "-v", "10", "20", "30"};
    EXPECT_TRUE(nargparse::Parse(parser, 5, argv));

    EXPECT_TRUE(verbose);
    EXPECT_FALSE(quiet);
    EXPECT_EQ(first_number, 10);

    int count = nargparse::GetRepeatedCount(parser, "Numbers");
    EXPECT_EQ(count, 3);

    int val;
    EXPECT_TRUE(nargparse::GetRepeated(parser, "Numbers", 0, &val));
    EXPECT_EQ(val, 10);
    EXPECT_TRUE(nargparse::GetRepeated(parser, "Numbers", 1, &val));
    EXPECT_EQ(val, 20);
    EXPECT_TRUE(nargparse::GetRepeated(parser, "Numbers", 2, &val));
    EXPECT_EQ(val, 30);
}