#include "argparser.hpp"
#include <iostream>

namespace nargparse {

void ArgumentParser::PrintHelp() const {
    std::cout << "Usage: " << name_ << " [options]\n";
    for (const auto& arg : args_) {
        if (arg->is_positional) {
            std::cout << "  [" << arg->pos_name << "]\t" << arg->help << "\n";
        } else if (arg->is_flag) {
            std::cout << "  " << arg->short_name << ", " << arg->long_name << "\t" << arg->help << "\n";
        } else {
            std::cout << "  " << arg->short_name << ", " << arg->long_name << " <val>\t" << arg->help << "\n";
        }
    }
}

ArgumentParser::ArgConcept* ArgumentParser::FindByName(std::string_view name) const {
    for (const auto& arg : args_) {
        if (arg->is_positional && arg->pos_name == name)
            return arg.get();
        if (arg->long_name.size() > 2 && arg->long_name.substr(2) == name)
            return arg.get();
        if (arg->help == name)
            return arg.get();
    }
    return nullptr;
}

size_t ArgumentParser::GetRepeatedCount(std::string_view name) const {
    auto* arg = FindByName(name);
    return arg ? arg->ParsedCount() : 0;
}

bool ArgumentParser::Parse(int argc, const char** argv) {
    for (auto& arg : args_) {
        if (auto* model = dynamic_cast<ArgModel<int>*>(arg.get()))
            model->values.clear();
        else if (auto* model = dynamic_cast<ArgModel<float>*>(arg.get()))
            model->values.clear();
        else if (auto* model = dynamic_cast<ArgModel<std::string>*>(arg.get()))
            model->values.clear();
        else if (auto* model = dynamic_cast<ArgModel<bool>*>(arg.get()))
            model->values.clear();
    }

    auto pos_it = args_.begin();
    auto advance_pos = [&]() {
        while (pos_it != args_.end() && !(*pos_it)->is_positional)
            ++pos_it;
    };
    advance_pos();

    for (int i = 1; i < argc; ++i) {
        std::string_view token(argv[i]);

        if (has_help_ && (token == "--help" || token == "-h")) {
            PrintHelp();
            return true;
        }

        if (token.starts_with("-") && token.size() > 1) {
            std::string_view key = token;
            std::string_view val;

            auto eq_idx = token.find('=');
            if (eq_idx != std::string_view::npos) {
                key = token.substr(0, eq_idx);
                val = token.substr(eq_idx + 1);
            }

            ArgConcept* matched = nullptr;
            for (const auto& arg : args_) {
                if (!arg->is_positional && (arg->short_name == key || arg->long_name == key)) {
                    matched = arg.get();
                    break;
                }
            }

            if (!matched)
                return false;

            if (matched->is_flag) {
                matched->MarkFlag();
                continue;
            }

            if (matched->count_rule == Nargs::Required || matched->count_rule == Nargs::Optional) {
                if (matched->ParsedCount() > 0)
                    return false;
            }

            if (eq_idx == std::string_view::npos) {
                if (i + 1 >= argc)
                    return false;
                val = argv[++i];
            }

            if (!matched->ParseValue(val))
                return false;
        } else {
            if (pos_it == args_.end())
                return false;

            if (!(*pos_it)->ParseValue(token))
                return false;

            if ((*pos_it)->count_rule == Nargs::Required || (*pos_it)->count_rule == Nargs::Optional) {
                ++pos_it;
                advance_pos();
            }
        }
    }

    for (const auto& arg : args_) {
        if (arg->count_rule == Nargs::Required && arg->ParsedCount() == 0)
            return false;
        if (arg->count_rule == Nargs::OneOrMore && arg->ParsedCount() == 0)
            return false;
    }

    return true;
}

} // namespace nargparse