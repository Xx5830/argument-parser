#include "lib/argparser.hpp"
#include <cstring>
#include <gtest/gtest.h>
#include <string>
#include <vector>

class ArgParserTest : public ::testing::Test {
  protected:
    void SetUp() override { parser = std::make_unique<nargparse::ArgumentParser>("test_program"); }
    void TearDown() override { parser.reset(); }

    std::unique_ptr<nargparse::ArgumentParser> parser;

    bool ParseArgs(std::initializer_list<const char*> args) {
        std::vector<const char*> argv(args);
        return parser->Parse(static_cast<int>(argv.size()), argv.data());
    }
};

bool IsEven(const int& value) { return value % 2 == 0; }
bool IsPositive(const int& value) { return value > 0; }
bool IsValidPort(const int& value) { return value > 0 && value <= 65535; }
bool IsPositiveFloat(const float& value) { return value > 0.0f; }
bool IsValidTemperature(const float& value) { return value >= -273.15f && value <= 1000.0f; }
bool IsNormalizedFloat(const float& value) { return value >= 0.0f && value <= 1.0f; }
bool IsNotEmpty(const std::string& value) { return !value.empty(); }
bool IsValidEmail(const std::string& value) { return value.find('@') != std::string::npos && value.length() > 3; }
bool IsAlphaOnly(const std::string& value) {
    if (value.empty())
        return false;
    for (char c : value)
        if (!std::isalpha(c))
            return false;
    return true;
}

TEST_F(ArgParserTest, EmptyParser) { EXPECT_TRUE(ParseArgs({"program"})); }

TEST_F(ArgParserTest, FlagArguments) {
    bool flag1 = false, flag2 = false;
    parser->AddFlag("-f", "--flag1", &flag1, "First flag", false);
    parser->AddFlag("-g", "--flag2", &flag2, "Second flag", true);
    EXPECT_FALSE(flag1);
    EXPECT_TRUE(flag2);
    EXPECT_TRUE(ParseArgs({"program", "-f", "--flag2"}));
    EXPECT_TRUE(flag1);
    EXPECT_TRUE(flag2);
}

TEST_F(ArgParserTest, PositionalArguments) {
    int value = 0;
    parser->AddPositional("Number", &value);
    EXPECT_TRUE(ParseArgs({"program", "42"}));
    EXPECT_EQ(value, 42);
}

TEST_F(ArgParserTest, RepeatedArguments) {
    int first_value = 0;
    parser->AddPositional("Numbers", &first_value, nargparse::Nargs::OneOrMore);
    EXPECT_TRUE(ParseArgs({"program", "1", "2", "3", "4", "5"}));
    EXPECT_EQ(first_value, 1);
    EXPECT_EQ(parser->GetRepeatedCount("Numbers"), 5);
    for (int i = 0; i < 5; ++i) {
        auto val = parser->GetRepeated<int>("Numbers", i);
        ASSERT_TRUE(val.has_value());
        EXPECT_EQ(val.value(), i + 1);
    }
}

TEST_F(ArgParserTest, ValidationFunctionSuccess) {
    int first_value = 0;
    parser->AddPositional("Even numbers", &first_value, nargparse::Nargs::ZeroOrMore, IsEven);
    EXPECT_TRUE(ParseArgs({"program", "2", "4", "6"}));
    EXPECT_EQ(first_value, 2);
    EXPECT_EQ(parser->GetRepeatedCount("Even numbers"), 3);
}

TEST_F(ArgParserTest, ValidationFunctionFailure) {
    int first_value = 0;
    parser->AddPositional("Even numbers", &first_value, nargparse::Nargs::ZeroOrMore, IsEven);
    EXPECT_FALSE(ParseArgs({"program", "2", "3", "4"}));
}

TEST_F(ArgParserTest, HelpFlag) {
    parser->AddHelp();
    EXPECT_TRUE(ParseArgs({"program", "--help"}));
}

TEST_F(ArgParserTest, MixedArguments) {
    bool verbose = false, quiet = false;
    int first_number = 0;
    parser->AddFlag("-v", "--verbose", &verbose, "Verbose output");
    parser->AddFlag("-q", "--quiet", &quiet, "Quiet output");
    parser->AddPositional("Numbers", &first_number, nargparse::Nargs::ZeroOrMore);
    EXPECT_TRUE(ParseArgs({"program", "-v", "10", "20", "30"}));
    EXPECT_TRUE(verbose);
    EXPECT_FALSE(quiet);
    EXPECT_EQ(first_number, 10);
    EXPECT_EQ(parser->GetRepeatedCount("Numbers"), 3);
    for (int i = 0; i < 3; ++i) {
        auto val = parser->GetRepeated<int>("Numbers", i);
        ASSERT_TRUE(val.has_value());
        EXPECT_EQ(val.value(), (i + 1) * 10);
    }
}

TEST_F(ArgParserTest, InvalidInteger) {
    int value = 0;
    parser->AddPositional("Number", &value);
    EXPECT_FALSE(ParseArgs({"program", "not_a_number"}));
}

TEST_F(ArgParserTest, EmptyArguments) {
    bool flag = false;
    parser->AddFlag("-f", "--flag", &flag, "A flag");
    EXPECT_TRUE(ParseArgs({"program"}));
    EXPECT_FALSE(flag);
}

TEST_F(ArgParserTest, ShortFlag) {
    bool flag = false;
    parser->AddFlag("-f", "--flag", &flag, "A flag");
    EXPECT_TRUE(ParseArgs({"program", "-f"}));
    EXPECT_TRUE(flag);
}

TEST_F(ArgParserTest, LongFlag) {
    bool flag = false;
    parser->AddFlag("-f", "--flag", &flag, "A flag");
    EXPECT_TRUE(ParseArgs({"program", "--flag"}));
    EXPECT_TRUE(flag);
}

TEST_F(ArgParserTest, MultipleFlags) {
    bool flag1 = false, flag2 = false, flag3 = false;
    parser->AddFlag("-a", "--alpha", &flag1, "Alpha flag");
    parser->AddFlag("-b", "--beta", &flag2, "Beta flag");
    parser->AddFlag("-c", "--gamma", &flag3, "Gamma flag");
    EXPECT_TRUE(ParseArgs({"program", "-a", "--beta", "-c"}));
    EXPECT_TRUE(flag1);
    EXPECT_TRUE(flag2);
    EXPECT_TRUE(flag3);
}

TEST_F(ArgParserTest, CustomValidationSuccess) {
    int first_value = 0;
    parser->AddPositional("Positive numbers", &first_value, nargparse::Nargs::ZeroOrMore, IsPositive);
    EXPECT_TRUE(ParseArgs({"program", "1", "2", "3"}));
    EXPECT_EQ(first_value, 1);
    EXPECT_EQ(parser->GetRepeatedCount("Positive numbers"), 3);
}

TEST_F(ArgParserTest, CustomValidationFailure) {
    int first_value = 0;
    parser->AddPositional("Positive numbers", &first_value, nargparse::Nargs::ZeroOrMore, IsPositive);
    EXPECT_FALSE(ParseArgs({"program", "1", "-2", "3"}));
}

TEST_F(ArgParserTest, EdgeCases) {
    EXPECT_TRUE(ParseArgs({"program"}));
    bool flag = false;
    parser->AddFlag("-f", "--flag", &flag, "A flag");
    EXPECT_TRUE(ParseArgs({"program", "-f"}));
    EXPECT_TRUE(flag);
}

TEST_F(ArgParserTest, NamedArguments) {
    int count = 0;
    std::string name;
    parser->AddArgument<int>("-n", "--count", &count, "Count value");
    parser->AddArgument<std::string>("-s", "--name", &name, "Name value");
    EXPECT_TRUE(ParseArgs({"program", "-n", "42", "--name", "test"}));
    EXPECT_EQ(count, 42);
    EXPECT_EQ(name, "test");
}

TEST_F(ArgParserTest, RequiredNamedArgumentsProvided) {
    int port = 0;
    parser->AddArgument<int>("-p", "--port", &port, "Port number", nargparse::Nargs::Required);
    EXPECT_TRUE(ParseArgs({"program", "--port", "8080"}));
    EXPECT_EQ(port, 8080);
}

TEST_F(ArgParserTest, RequiredNamedArgumentsMissing) {
    int port = 0;
    parser->AddArgument<int>("-p", "--port", &port, "Port number", nargparse::Nargs::Required);
    EXPECT_FALSE(ParseArgs({"program"}));
}

TEST_F(ArgParserTest, MixedAllTypes) {
    bool verbose = false;
    int port = 0;
    std::string filename;
    parser->AddFlag("-v", "--verbose", &verbose, "Verbose output");
    parser->AddArgument<int>("-p", "--port", &port, "Port number");
    parser->AddPositional("Input file", &filename, nargparse::Nargs::Required);
    EXPECT_TRUE(ParseArgs({"program", "-v", "input.txt", "-p", "3000"}));
    EXPECT_TRUE(verbose);
    EXPECT_EQ(port, 3000);
    EXPECT_EQ(filename, "input.txt");
}

TEST_F(ArgParserTest, NamedArgumentValidationSuccess) {
    int port = 0;
    parser->AddArgument<int>("-p", "--port", &port, "Port number", nargparse::Nargs::Optional, IsValidPort);
    EXPECT_TRUE(ParseArgs({"program", "-p", "8080"}));
    EXPECT_EQ(port, 8080);
}

TEST_F(ArgParserTest, NamedArgumentValidationFailure) {
    int port = 0;
    parser->AddArgument<int>("-p", "--port", &port, "Port number", nargparse::Nargs::Optional, IsValidPort);
    EXPECT_FALSE(ParseArgs({"program", "-p", "99999"}));
}

TEST_F(ArgParserTest, MixedFlagsNamedAndPositional) {
    bool verbose = false;
    std::string name;
    int number = 0;
    parser->AddFlag("-v", "--verbose", &verbose, "Verbose");
    parser->AddArgument<std::string>("-n", "--name", &name, "Name");
    parser->AddPositional("number", &number);
    EXPECT_TRUE(ParseArgs({"program", "-v", "--name", "itmo", "44"}));
    EXPECT_TRUE(verbose);
    EXPECT_EQ(name, "itmo");
    EXPECT_EQ(number, 44);
}

TEST_F(ArgParserTest, MultipleNamedArgumentCalls) {
    bool mult = false;
    int first_number = 0;
    parser->AddFlag("-m", "--mult", &mult, "Multiply");
    parser->AddArgument<int>("-n", "--number", &first_number, "Numbers", nargparse::Nargs::ZeroOrMore);
    EXPECT_TRUE(ParseArgs({"program", "--mult", "-n", "1", "-n", "2", "-n", "3", "-n", "4", "-n", "5"}));
    EXPECT_TRUE(mult);
    EXPECT_EQ(first_number, 1);
    EXPECT_EQ(parser->GetRepeatedCount("Numbers"), 5);
    for (int i = 0; i < 5; ++i) {
        auto val = parser->GetRepeated<int>("Numbers", i);
        ASSERT_TRUE(val.has_value());
        EXPECT_EQ(val.value(), i + 1);
    }
}

TEST_F(ArgParserTest, EqualsSignSyntax) {
    bool verbose = false;
    std::string name;
    int number = 0;
    parser->AddFlag("-v", "--verbose", &verbose, "Verbose");
    parser->AddArgument<std::string>("-n", "--name", &name, "Name");
    parser->AddPositional("number", &number);
    EXPECT_TRUE(ParseArgs({"program", "-v", "--name=itmo", "44"}));
    EXPECT_TRUE(verbose);
    EXPECT_EQ(name, "itmo");
    EXPECT_EQ(number, 44);
}

TEST_F(ArgParserTest, DuplicateSingleArgument) {
    bool verbose = false;
    std::string name;
    int number = 0;
    parser->AddFlag("-v", "--verbose", &verbose, "Verbose");
    parser->AddArgument<std::string>("-n", "--name", &name, "Name");
    parser->AddPositional("number", &number);
    EXPECT_FALSE(ParseArgs({"program", "-v", "--name", "itmo", "44", "--name=itmo2"}));
}

TEST_F(ArgParserTest, FloatValidationSuccess) {
    float temperature = 0.0f;
    parser->AddArgument<float>("-t", "--temp", &temperature, "Temperature", nargparse::Nargs::Optional,
                               IsValidTemperature);
    EXPECT_TRUE(ParseArgs({"program", "-t", "25.5"}));
    EXPECT_NEAR(temperature, 25.5f, 0.01f);
}

TEST_F(ArgParserTest, FloatValidationFailure) {
    float temperature = 0.0f;
    parser->AddArgument<float>("-t", "--temp", &temperature, "Temperature", nargparse::Nargs::Optional,
                               IsValidTemperature);
    EXPECT_FALSE(ParseArgs({"program", "-t", "2000.0"}));
}

TEST_F(ArgParserTest, FloatVectorValidationSuccess) {
    float first_score = 0.0f;
    parser->AddArgument<float>("-s", "--score", &first_score, "Scores", nargparse::Nargs::ZeroOrMore,
                               IsNormalizedFloat);
    EXPECT_TRUE(ParseArgs({"program", "-s", "0.95", "-s", "0.87", "-s", "1.0"}));
    EXPECT_NEAR(first_score, 0.95f, 0.01f);
    EXPECT_EQ(parser->GetRepeatedCount("Scores"), 3);
    auto opt = parser->GetRepeated<float>("Scores", 0);
    ASSERT_TRUE(opt);
    EXPECT_NEAR(*opt, 0.95f, 0.01f);
    opt = parser->GetRepeated<float>("Scores", 1);
    ASSERT_TRUE(opt);
    EXPECT_NEAR(*opt, 0.87f, 0.01f);
    opt = parser->GetRepeated<float>("Scores", 2);
    ASSERT_TRUE(opt);
    EXPECT_NEAR(*opt, 1.0f, 0.01f);
}

TEST_F(ArgParserTest, FloatVectorValidationFailure) {
    float first_score = 0.0f;
    parser->AddArgument<float>("-s", "--score", &first_score, "Scores", nargparse::Nargs::ZeroOrMore,
                               IsNormalizedFloat);
    EXPECT_FALSE(ParseArgs({"program", "-s", "0.5", "-s", "1.5"}));
}

TEST_F(ArgParserTest, StringValidationSuccess) {
    std::string email;
    parser->AddArgument<std::string>("-e", "--email", &email, "Email", nargparse::Nargs::Optional, IsValidEmail);
    EXPECT_TRUE(ParseArgs({"program", "-e", "user@example.com"}));
    EXPECT_EQ(email, "user@example.com");
}

TEST_F(ArgParserTest, StringValidationFailure) {
    std::string email;
    parser->AddArgument<std::string>("-e", "--email", &email, "Email", nargparse::Nargs::Optional, IsValidEmail);
    EXPECT_FALSE(ParseArgs({"program", "-e", "invalid"}));
}

TEST_F(ArgParserTest, StringVectorValidationSuccess) {
    std::string first_name;
    parser->AddPositional("names", &first_name, nargparse::Nargs::OneOrMore, IsAlphaOnly);
    EXPECT_TRUE(ParseArgs({"program", "Alice", "Bob", "Charlie"}));
    EXPECT_EQ(first_name, "Alice");
    EXPECT_EQ(parser->GetRepeatedCount("names"), 3);
    auto opt = parser->GetRepeated<std::string>("names", 0);
    ASSERT_TRUE(opt);
    EXPECT_EQ(*opt, "Alice");
    opt = parser->GetRepeated<std::string>("names", 1);
    ASSERT_TRUE(opt);
    EXPECT_EQ(*opt, "Bob");
    opt = parser->GetRepeated<std::string>("names", 2);
    ASSERT_TRUE(opt);
    EXPECT_EQ(*opt, "Charlie");
}

TEST_F(ArgParserTest, StringVectorValidationFailure) {
    std::string first_name;
    parser->AddPositional("names", &first_name, nargparse::Nargs::OneOrMore, IsAlphaOnly);
    EXPECT_FALSE(ParseArgs({"program", "Alice", "Bob123"}));
}

TEST_F(ArgParserTest, StringVectorLifetimes) {
    std::string first_name;
    {
        parser->AddPositional("names", &first_name, nargparse::Nargs::OneOrMore);
        char a[] = "program", b[] = "Alice", c[] = "Bob", d[] = "Charlie";
        const char* argv[] = {a, b, c, d};
        EXPECT_TRUE(parser->Parse(4, argv));
        EXPECT_EQ(first_name, "Alice");
    }
    EXPECT_EQ(parser->GetRepeatedCount("names"), 3);
    auto opt = parser->GetRepeated<std::string>("names", 0);
    ASSERT_TRUE(opt);
    EXPECT_EQ(*opt, "Alice");
    opt = parser->GetRepeated<std::string>("names", 1);
    ASSERT_TRUE(opt);
    EXPECT_EQ(*opt, "Bob");
    opt = parser->GetRepeated<std::string>("names", 2);
    ASSERT_TRUE(opt);
    EXPECT_EQ(*opt, "Charlie");
}

TEST_F(ArgParserTest, MixedTypesWithValidation) {
    int count = 0;
    float ratio = 0.0f;
    std::string name;
    parser->AddArgument<int>("-c", "--count", &count, "Count", nargparse::Nargs::Optional, IsPositive);
    parser->AddArgument<float>("-r", "--ratio", &ratio, "Ratio", nargparse::Nargs::Optional, IsNormalizedFloat);
    parser->AddArgument<std::string>("-n", "--name", &name, "Name", nargparse::Nargs::Optional, IsNotEmpty);
    EXPECT_TRUE(ParseArgs({"program", "-c", "42", "-r", "0.75", "-n", "TestApp"}));
    EXPECT_EQ(count, 42);
    EXPECT_NEAR(ratio, 0.75f, 0.01f);
    EXPECT_EQ(name, "TestApp");
}

TEST_F(ArgParserTest, PositionalFloatArguments) {
    float temperature = 0.0f, first_reading = 0.0f;
    parser->AddPositional("temperature", &temperature);
    parser->AddPositional("readings", &first_reading, nargparse::Nargs::ZeroOrMore);
    EXPECT_TRUE(ParseArgs({"program", "36.6", "98.6", "99.1", "97.8"}));
    EXPECT_NEAR(temperature, 36.6f, 0.01f);
    EXPECT_NEAR(first_reading, 98.6f, 0.01f);
    EXPECT_EQ(parser->GetRepeatedCount("readings"), 3);
    auto opt = parser->GetRepeated<float>("readings", 0);
    ASSERT_TRUE(opt);
    EXPECT_NEAR(*opt, 98.6f, 0.01f);
    opt = parser->GetRepeated<float>("readings", 1);
    ASSERT_TRUE(opt);
    EXPECT_NEAR(*opt, 99.1f, 0.01f);
    opt = parser->GetRepeated<float>("readings", 2);
    ASSERT_TRUE(opt);
    EXPECT_NEAR(*opt, 97.8f, 0.01f);
}

TEST_F(ArgParserTest, StructureBasedOptions) {
    struct ServerConfig {
        std::string host;
        int port = 0;
        bool verbose = false;
        bool debug = false;
        float timeout = 0.0f;
    };
    ServerConfig config;
    parser->AddArgument<std::string>("", "--host", &config.host, "Server host", nargparse::Nargs::Optional, IsNotEmpty);
    parser->AddArgument<int>("-p", "--port", &config.port, "Server port", nargparse::Nargs::Optional, IsValidPort);
    parser->AddFlag("-v", "--verbose", &config.verbose, "Verbose output");
    parser->AddFlag("-d", "--debug", &config.debug, "Debug mode");
    parser->AddArgument<float>("-t", "--timeout", &config.timeout, "Timeout", nargparse::Nargs::Optional,
                               IsPositiveFloat);
    EXPECT_TRUE(ParseArgs({"program", "--host", "localhost", "-p", "8080", "-v", "-t", "30.5"}));
    EXPECT_EQ(config.host, "localhost");
    EXPECT_EQ(config.port, 8080);
    EXPECT_TRUE(config.verbose);
    EXPECT_FALSE(config.debug);
    EXPECT_NEAR(config.timeout, 30.5f, 0.01f);
}

TEST_F(ArgParserTest, NestedStructureOptions) {
    struct DatabaseConfig {
        std::string host;
        int port = 0;
        std::string name;
    };
    struct LogConfig {
        bool verbose = false;
        std::string logfile;
        int level = 0;
    };
    struct ApplicationConfig {
        DatabaseConfig database;
        LogConfig logging;
        std::string app_name;
        std::string first_tag;
    };
    ApplicationConfig app_config;
    parser->AddArgument<std::string>("", "--db-host", &app_config.database.host, "Database host");
    parser->AddArgument<int>("", "--db-port", &app_config.database.port, "Database port", nargparse::Nargs::Optional,
                             IsValidPort);
    parser->AddArgument<std::string>("", "--db-name", &app_config.database.name, "Database name");
    parser->AddFlag("-v", "--verbose", &app_config.logging.verbose, "Verbose logging");
    parser->AddArgument<std::string>("", "--logfile", &app_config.logging.logfile, "Log file path");
    parser->AddArgument<int>("", "--log-level", &app_config.logging.level, "Log level", nargparse::Nargs::Optional,
                             IsPositive);
    parser->AddArgument<std::string>("-n", "--name", &app_config.app_name, "Application name");
    parser->AddArgument<std::string>("-t", "--tag", &app_config.first_tag, "Tags", nargparse::Nargs::ZeroOrMore);

    const char* argv[] = {"program",
                          "--db-host",
                          "postgres.example.com",
                          "--db-port",
                          "5432",
                          "--db-name",
                          "myapp",
                          "-v",
                          "--logfile",
                          "/var/log/app.log",
                          "--log-level",
                          "3",
                          "-n",
                          "MyApplication",
                          "-t",
                          "production",
                          "-t",
                          "backend",
                          "-t",
                          "api"};
    EXPECT_TRUE(parser->Parse(20, argv));
    EXPECT_EQ(app_config.database.host, "postgres.example.com");
    EXPECT_EQ(app_config.database.port, 5432);
    EXPECT_EQ(app_config.database.name, "myapp");
    EXPECT_TRUE(app_config.logging.verbose);
    EXPECT_EQ(app_config.logging.logfile, "/var/log/app.log");
    EXPECT_EQ(app_config.logging.level, 3);
    EXPECT_EQ(app_config.app_name, "MyApplication");
    EXPECT_EQ(app_config.first_tag, "production");
    EXPECT_EQ(parser->GetRepeatedCount("Tags"), 3);
    auto t = parser->GetRepeated<std::string>("Tags", 0);
    ASSERT_TRUE(t);
    EXPECT_EQ(*t, "production");
    t = parser->GetRepeated<std::string>("Tags", 1);
    ASSERT_TRUE(t);
    EXPECT_EQ(*t, "backend");
    t = parser->GetRepeated<std::string>("Tags", 2);
    ASSERT_TRUE(t);
    EXPECT_EQ(*t, "api");
}

TEST_F(ArgParserTest, ComplexStructureWithMixedTypes) {
    struct NetworkSettings {
        std::string interface;
        int mtu = 0;
        float bandwidth = 0.0f;
    };
    struct SecuritySettings {
        bool enable_ssl = false;
        bool enable_auth = false;
        std::string first_allowed_host;
    };
    struct ServiceConfig {
        NetworkSettings network;
        SecuritySettings security;
        int first_worker_port = 0;
        std::string service_name;
    };
    ServiceConfig service;
    parser->AddArgument<std::string>("", "--interface", &service.network.interface, "Network interface");
    parser->AddArgument<int>("", "--mtu", &service.network.mtu, "MTU size", nargparse::Nargs::Optional, IsPositive);
    parser->AddArgument<float>("", "--bandwidth", &service.network.bandwidth, "Bandwidth", nargparse::Nargs::Optional,
                               IsPositiveFloat);
    parser->AddFlag("", "--ssl", &service.security.enable_ssl, "Enable SSL");
    parser->AddFlag("", "--auth", &service.security.enable_auth, "Enable auth");
    parser->AddArgument<std::string>("", "--allow", &service.security.first_allowed_host, "Allowed hosts",
                                     nargparse::Nargs::ZeroOrMore);
    parser->AddArgument<int>("-w", "--worker-port", &service.first_worker_port, "Worker ports",
                             nargparse::Nargs::ZeroOrMore, IsValidPort);
    parser->AddArgument<std::string>("-s", "--service", &service.service_name, "Service name",
                                     nargparse::Nargs::Optional, IsAlphaOnly);

    const char* argv[] = {"program", "--interface", "eth0",    "--mtu",       "1500",    "--bandwidth", "100.5",
                          "--ssl",   "--auth",      "--allow", "192.168.1.1", "--allow", "10.0.0.1",    "-w",
                          "8080",    "-w",          "8081",    "-w",          "8082",    "-s",          "WebService"};
    EXPECT_TRUE(parser->Parse(21, argv));
    EXPECT_EQ(service.network.interface, "eth0");
    EXPECT_EQ(service.network.mtu, 1500);
    EXPECT_NEAR(service.network.bandwidth, 100.5f, 0.01f);
    EXPECT_TRUE(service.security.enable_ssl);
    EXPECT_TRUE(service.security.enable_auth);
    EXPECT_EQ(service.security.first_allowed_host, "192.168.1.1");
    EXPECT_EQ(parser->GetRepeatedCount("Allowed hosts"), 2);
    auto h = parser->GetRepeated<std::string>("Allowed hosts", 0);
    ASSERT_TRUE(h);
    EXPECT_EQ(*h, "192.168.1.1");
    h = parser->GetRepeated<std::string>("Allowed hosts", 1);
    ASSERT_TRUE(h);
    EXPECT_EQ(*h, "10.0.0.1");
    EXPECT_EQ(service.first_worker_port, 8080);
    EXPECT_EQ(parser->GetRepeatedCount("Worker ports"), 3);
    auto p = parser->GetRepeated<int>("Worker ports", 0);
    ASSERT_TRUE(p);
    EXPECT_EQ(*p, 8080);
    p = parser->GetRepeated<int>("Worker ports", 1);
    ASSERT_TRUE(p);
    EXPECT_EQ(*p, 8081);
    p = parser->GetRepeated<int>("Worker ports", 2);
    ASSERT_TRUE(p);
    EXPECT_EQ(*p, 8082);
    EXPECT_EQ(service.service_name, "WebService");
}

TEST_F(ArgParserTest, NargsOneOrMoreWithNoValues) {
    int first_value = 0;
    parser->AddPositional("numbers", &first_value, nargparse::Nargs::OneOrMore);
    EXPECT_FALSE(ParseArgs({"program"}));
}

TEST_F(ArgParserTest, NargsOneOrMoreWithSingleValue) {
    int first_value = 0;
    parser->AddPositional("numbers", &first_value, nargparse::Nargs::OneOrMore);
    EXPECT_TRUE(ParseArgs({"program", "42"}));
    EXPECT_EQ(parser->GetRepeatedCount("numbers"), 1);
    EXPECT_EQ(first_value, 42);
}

TEST_F(ArgParserTest, NargsOneOrMoreWithMultipleValues) {
    int first_value = 0;
    parser->AddPositional("numbers", &first_value, nargparse::Nargs::OneOrMore);
    EXPECT_TRUE(ParseArgs({"program", "1", "2", "3", "4", "5"}));
    EXPECT_EQ(parser->GetRepeatedCount("numbers"), 5);
    EXPECT_EQ(first_value, 1);
    auto v = parser->GetRepeated<int>("numbers", 4);
    ASSERT_TRUE(v);
    EXPECT_EQ(*v, 5);
}

TEST_F(ArgParserTest, NamedNargsOneOrMoreWithNoValues) {
    std::string first_tag;
    parser->AddArgument<std::string>("-t", "--tag", &first_tag, "Tags", nargparse::Nargs::OneOrMore);
    EXPECT_FALSE(ParseArgs({"program"}));
}

TEST_F(ArgParserTest, NamedNargsOneOrMoreWithValues) {
    std::string first_tag;
    parser->AddArgument<std::string>("-t", "--tag", &first_tag, "Tags", nargparse::Nargs::OneOrMore);
    EXPECT_TRUE(ParseArgs({"program", "-t", "alpha", "-t", "beta", "-t", "gamma"}));
    EXPECT_EQ(first_tag, "alpha");
    EXPECT_EQ(parser->GetRepeatedCount("Tags"), 3);
    auto t = parser->GetRepeated<std::string>("Tags", 0);
    ASSERT_TRUE(t);
    EXPECT_EQ(*t, "alpha");
    t = parser->GetRepeated<std::string>("Tags", 1);
    ASSERT_TRUE(t);
    EXPECT_EQ(*t, "beta");
    t = parser->GetRepeated<std::string>("Tags", 2);
    ASSERT_TRUE(t);
    EXPECT_EQ(*t, "gamma");
}

TEST_F(ArgParserTest, FunnyArgumentValue) {
    std::string pos, arg, fun;
    parser->AddPositional("pos", &pos, nargparse::Nargs::ZeroOrMore);
    parser->AddArgument<std::string>("", "--arg", &arg, "Serious business only.", nargparse::Nargs::Optional);
    parser->AddArgument<std::string>("", "--funny", &fun, "Nothing funny here.", nargparse::Nargs::Optional);
    EXPECT_TRUE(ParseArgs({"program", "--arg", "--funny", "positional"}));
    EXPECT_EQ(pos, "positional");
    EXPECT_EQ(arg, "--funny");
    EXPECT_EQ(fun, "");
}

TEST_F(ArgParserTest, CStyleIterationNamedInt) {
    int first_number = 0;
    parser->AddArgument<int>("-n", "--number", &first_number, "Numbers", nargparse::Nargs::ZeroOrMore);
    EXPECT_TRUE(ParseArgs({"program", "-n", "10", "-n", "20", "-n", "30"}));
    EXPECT_EQ(first_number, 10);
    EXPECT_EQ(parser->GetRepeatedCount("Numbers"), 3);
    EXPECT_EQ(*parser->GetRepeated<int>("Numbers", 0), 10);
    EXPECT_EQ(*parser->GetRepeated<int>("Numbers", 1), 20);
    EXPECT_EQ(*parser->GetRepeated<int>("Numbers", 2), 30);
    EXPECT_FALSE(parser->GetRepeated<int>("Numbers", 3).has_value());
}

TEST_F(ArgParserTest, CStyleIterationNamedFloat) {
    float first_score = 0.0f;
    parser->AddArgument<float>("-s", "--score", &first_score, "Scores", nargparse::Nargs::ZeroOrMore);
    EXPECT_TRUE(ParseArgs({"program", "-s", "1.5", "-s", "2.7", "-s", "3.9"}));
    EXPECT_NEAR(first_score, 1.5f, 0.01f);
    EXPECT_EQ(parser->GetRepeatedCount("Scores"), 3);
    EXPECT_NEAR(*parser->GetRepeated<float>("Scores", 0), 1.5f, 0.01f);
    EXPECT_NEAR(*parser->GetRepeated<float>("Scores", 1), 2.7f, 0.01f);
    EXPECT_NEAR(*parser->GetRepeated<float>("Scores", 2), 3.9f, 0.01f);
}

TEST_F(ArgParserTest, CStyleIterationNamedString) {
    std::string first_tag;
    parser->AddArgument<std::string>("-t", "--tag", &first_tag, "Tags", nargparse::Nargs::ZeroOrMore);
    EXPECT_TRUE(ParseArgs({"program", "-t", "alpha", "-t", "beta", "-t", "gamma"}));
    EXPECT_EQ(first_tag, "alpha");
    EXPECT_EQ(parser->GetRepeatedCount("Tags"), 3);
    EXPECT_EQ(*parser->GetRepeated<std::string>("Tags", 0), "alpha");
    EXPECT_EQ(*parser->GetRepeated<std::string>("Tags", 1), "beta");
    EXPECT_EQ(*parser->GetRepeated<std::string>("Tags", 2), "gamma");
}

TEST_F(ArgParserTest, CStyleIterationPositionalInt) {
    int first_value = 0;
    parser->AddPositional("numbers", &first_value, nargparse::Nargs::ZeroOrMore);
    EXPECT_TRUE(ParseArgs({"program", "100", "200", "300", "400"}));
    EXPECT_EQ(first_value, 100);
    EXPECT_EQ(parser->GetRepeatedCount("numbers"), 4);
    for (int i = 0; i < 4; ++i) {
        EXPECT_EQ(*parser->GetRepeated<int>("numbers", i), (i + 1) * 100);
    }
}

TEST_F(ArgParserTest, CStyleIterationPositionalString) {
    std::string first_file;
    parser->AddPositional("files", &first_file, nargparse::Nargs::OneOrMore);
    EXPECT_TRUE(ParseArgs({"program", "file1.txt", "file2.txt", "file3.txt"}));
    EXPECT_EQ(first_file, "file1.txt");
    EXPECT_EQ(parser->GetRepeatedCount("files"), 3);
    EXPECT_EQ(*parser->GetRepeated<std::string>("files", 0), "file1.txt");
    EXPECT_EQ(*parser->GetRepeated<std::string>("files", 1), "file2.txt");
    EXPECT_EQ(*parser->GetRepeated<std::string>("files", 2), "file3.txt");
}

TEST_F(ArgParserTest, CStyleIterationEmpty) {
    int first_number = 0;
    parser->AddArgument<int>("-n", "--number", &first_number, "Numbers", nargparse::Nargs::ZeroOrMore);
    EXPECT_TRUE(ParseArgs({"program"}));
    EXPECT_EQ(parser->GetRepeatedCount("Numbers"), 0);
    EXPECT_FALSE(parser->GetRepeated<int>("Numbers", 0).has_value());
}

TEST_F(ArgParserTest, CStyleIterationMixed) {
    int first_port = 0;
    std::string first_server;
    parser->AddArgument<int>("-p", "--port", &first_port, "Ports", nargparse::Nargs::ZeroOrMore);
    parser->AddArgument<std::string>("-s", "--server", &first_server, "Servers", nargparse::Nargs::ZeroOrMore);
    EXPECT_TRUE(ParseArgs({"program", "-p", "8080", "-s", "localhost", "-p", "9090", "-s", "example.com"}));
    EXPECT_EQ(first_port, 8080);
    EXPECT_EQ(first_server, "localhost");
    EXPECT_EQ(parser->GetRepeatedCount("Ports"), 2);
    EXPECT_EQ(parser->GetRepeatedCount("Servers"), 2);
    EXPECT_EQ(*parser->GetRepeated<int>("Ports", 0), 8080);
    EXPECT_EQ(*parser->GetRepeated<int>("Ports", 1), 9090);
    EXPECT_EQ(*parser->GetRepeated<std::string>("Servers", 0), "localhost");
    EXPECT_EQ(*parser->GetRepeated<std::string>("Servers", 1), "example.com");
}