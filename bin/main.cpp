#include "argparser.hpp"
#include <iostream>
#include <string>

int main(int argc, const char** argv) {
    using namespace nargparse;

    std::string host;
    int port = 8080;
    bool verbose = false;

    ArgumentParser parser("My Awesome Server");

    parser.AddHelp();
    parser.AddPositional("host", &host, Nargs::Required);
    parser.AddArgument<int>("-p", "--port", &port, "Port to bind", Nargs::Optional,
                            [](const int& p) { return p > 0 && p <= 65535; });
    parser.AddFlag("-v", "--verbose", &verbose, "Enable verbose output");

    if (!parser.Parse(argc, argv)) {
        std::cerr << "Error parsing arguments!\n";
        parser.PrintHelp();
        return 1;
    }

    std::cout << "Starting server on " << host << ":" << port << "\n";
    std::cout << "Verbose: " << (verbose ? "yes" : "no") << "\n";

    return 0;
}