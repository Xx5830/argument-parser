Это современная C++ библиотека для разбора аргументов командной строки. Поддерживает флаги, опциональные и обязательные аргументы, позиционные параметры, повторяющиеся значения и пользовательские валидаторы. Вся мощь шаблонов и type-safe API без макросов и скрытого состояния.

## Особенности

-  **Header-only** - просто подключи `argparser.hpp` и используй.
-  **Типобезопасность** - аргументы принимаются по указателю на конкретный тип (`int`, `float`, `std::string`, `bool`).
-  **Автоматическая справка** - встроенная поддержка `--help`.
-  **Повторяющиеся аргументы** - `ZeroOrMore`, `OneOrMore` с доступом по индексу.
-  **Валидация** - задавай свои функции проверки для каждого аргумента.
-  **Позиционные и именованные** - гибкое смешивание в любом порядке.
-  **Кроссплатформенность** - сборка под Windows, Linux, macOS.

## Быстрый пример

```cpp
#include "argparser.hpp"
#include <iostream>

int main(int argc, const char** argv) {
    using namespace nargparse;

    std::string host;
    int port = 8080;
    bool verbose = false;

    ArgumentParser parser("My Server");
    parser.AddHelp();
    parser.AddPositional("host", &host, Nargs::Required);
    parser.AddArgument<int>("-p", "--port", &port, "Port number", Nargs::Optional,
                            [](const int& p) { return p > 0 && p <= 65535; });
    parser.AddFlag("-v", "--verbose", &verbose, "Enable verbose output");

    if (!parser.Parse(argc, argv)) {
        parser.PrintHelp();
        return 1;
    }

    std::cout << "Starting " << host << ":" << port << " verbose=" << std::boolalpha << verbose << "\n";
    return 0;
}
