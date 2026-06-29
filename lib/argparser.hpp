#pragma once

#include <charconv>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace nargparse {

enum class Nargs { ZeroOrMore, OneOrMore, Required, Optional };

class ArgumentParser {
  public:
    explicit ArgumentParser(std::string name) : name_(std::move(name)) {}

    template <typename T, typename Validator>
    std::enable_if_t<std::is_invocable_r_v<bool, Validator, const T&>, void>
    AddArgument(std::string short_name, std::string long_name, T* store_to, std::string help_info, Nargs count,
                Validator validator) {
        auto arg = std::make_unique<ArgModel<T>>();
        arg->short_name = std::move(short_name);
        arg->long_name = std::move(long_name);
        arg->help = std::move(help_info);
        arg->store_to = store_to;
        arg->count_rule = count;
        arg->validator = validator;
        args_.push_back(std::move(arg));
    }

    template <typename T>
    void AddArgument(std::string short_name, std::string long_name, T* store_to, std::string help_info = "",
                     Nargs count = Nargs::Optional) {
        AddArgument(std::move(short_name), std::move(long_name), store_to, std::move(help_info), count,
                    std::function<bool(const T&)>{});
    }

    template <typename T, typename Validator>
    std::enable_if_t<std::is_invocable_r_v<bool, Validator, const T&>, void>
    AddPositional(std::string name, T* store_to, Nargs count, Validator validator) {
        auto arg = std::make_unique<ArgModel<T>>();
        arg->pos_name = std::move(name);
        arg->is_positional = true;
        arg->store_to = store_to;
        arg->count_rule = count;
        arg->validator = validator;
        args_.push_back(std::move(arg));
    }

    template <typename T>
    void AddPositional(std::string name, T* store_to, Nargs count = Nargs::Optional) {
        AddPositional(std::move(name), store_to, count, std::function<bool(const T&)>{});
    }

    void AddFlag(std::string short_name, std::string long_name, bool* store_to, std::string help_info = "",
                 bool default_val = false) {
        auto arg = std::make_unique<ArgModel<bool>>();
        arg->short_name = std::move(short_name);
        arg->long_name = std::move(long_name);
        arg->help = std::move(help_info);
        arg->store_to = store_to;
        arg->is_flag = true;
        arg->count_rule = Nargs::Optional;
        if (store_to)
            *store_to = default_val;
        args_.push_back(std::move(arg));
    }

    void AddHelp() { has_help_ = true; }
    void PrintHelp() const;

    bool Parse(int argc, const char** argv);

    size_t GetRepeatedCount(std::string_view name) const;

    template <typename T>
    std::optional<T> GetRepeated(std::string_view name, size_t index) const {
        if (auto* base = FindByName(name)) {
            if (auto* model = dynamic_cast<const ArgModel<T>*>(base)) {
                if (index < model->values.size()) {
                    return model->values[index];
                }
            }
        }
        return std::nullopt;
    }

  private:
    struct ArgConcept {
        virtual ~ArgConcept() = default;
        virtual bool ParseValue(std::string_view str) = 0;
        virtual void MarkFlag() = 0;
        virtual size_t ParsedCount() const = 0;

        std::string short_name;
        std::string long_name;
        std::string pos_name;
        std::string help;
        Nargs count_rule = Nargs::Optional;
        bool is_positional = false;
        bool is_flag = false;
    };

    template <typename T>
    struct ArgModel final : ArgConcept {
        T* store_to = nullptr;
        std::function<bool(const T&)> validator;
        std::vector<T> values;

        bool ParseValue(std::string_view str) override;
        void MarkFlag() override {
            if constexpr (std::is_same_v<T, bool>) {
                if (store_to)
                    *store_to = true;
                values.push_back(true);
            }
        }
        size_t ParsedCount() const override { return values.size(); }
    };

    std::string name_;
    bool has_help_ = false;
    std::vector<std::unique_ptr<ArgConcept>> args_;

    ArgConcept* FindByName(std::string_view name) const;
};

template <typename T>
std::optional<T> ConvertString(std::string_view str);

template <>
inline std::optional<int> ConvertString<int>(std::string_view str) {
    int result;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
    if (ec == std::errc() && ptr == str.data() + str.size())
        return result;
    return std::nullopt;
}

template <>
inline std::optional<float> ConvertString<float>(std::string_view str) {
    try {
        std::string s(str);
        size_t pos = 0;
        float f = std::stof(s, &pos);
        if (pos == s.length())
            return f;
    } catch (...) {
    }
    return std::nullopt;
}

template <>
inline std::optional<std::string> ConvertString<std::string>(std::string_view str) {
    return std::string(str);
}

template <>
inline std::optional<bool> ConvertString<bool>(std::string_view str) {
    if (str == "true" || str == "1")
        return true;
    if (str == "false" || str == "0")
        return false;
    return std::nullopt;
}

template <typename T>
bool ArgumentParser::ArgModel<T>::ParseValue(std::string_view str) {
    auto val_opt = ConvertString<T>(str);
    if (!val_opt)
        return false;
    if (validator && !validator(*val_opt))
        return false;
    if (values.empty() && store_to)
        *store_to = *val_opt;
    values.push_back(std::move(*val_opt));
    return true;
}

} // namespace nargparse