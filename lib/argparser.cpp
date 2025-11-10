#include "argparser.h"
#include <cstdlib>
#include <iostream>

bool nargparse::EqualString(const char *left, const char *right) {
    if (!left || !right) {
        return false;
    }

    uint32_t i = 0;
    for (; left[i] != '\0'; i++) {
        if (left[i] != right[i]) {
            return false;
        }
    }

    return left[i] == right[i];
}

void nargparse::ExpandFlagList(ParserNode *node, bool *flag) {
    if (node->begin_flag) {
        node->prev_end_flag = node->prev_end_flag->next = new FlagNode{nullptr, flag};
    } else {
        node->begin_flag = node->prev_end_flag = new FlagNode{nullptr, flag};
    }
}

void nargparse::ExpandBaseList(ParserNode *node, VariantBase element) {
    if (node->begin_base) {
        node->prev_end_base = node->prev_end_base->next = new BaseNode{nullptr, element};
    } else {
        node->begin_base = node->prev_end_base = new BaseNode{nullptr, element};
    }
}

void nargparse::ExpandParserList(ArgumentParser &parser, ParserNode *element) {
    if (parser.begin) {
        parser.prev_end = parser.prev_end->next = element;
    } else {
        parser.begin = parser.prev_end = element;
    }
}

void nargparse::ExpandPositionParserList(ArgumentParser &parser, PositionParserNode *element) {
    if (parser.begin_pos) {
        parser.prev_end_pos = parser.prev_end_pos->next = element;
    } else {
        parser.begin_pos = parser.prev_end_pos = element;
    }
}

bool nargparse::WritePositionArgument(PositionParserNode *node, const char *new_arg) {
    bool result_write = true;
    switch (node->place_save_first.type) {
    case VariantBase::BaseEnum::kInt: {
        int32_t current = atoll(new_arg);
        if (node->validation_int(current)) {
            VariantBase new_variant;
            new_variant.element.t1 = new int32_t{current};
            new_variant.type = VariantBase::BaseEnum::kInt;
            node->size++;

            if (node->begin_result == nullptr) {
                *node->place_save_first.element.t1 = current;
                node->begin_result = node->prev_result = new BaseNode{nullptr, new_variant};
            } else {
                node->prev_result = node->prev_result->next = new BaseNode{nullptr, new_variant};
            }
        } else {
            result_write = false;
        }
        break;
    }
    case VariantBase::BaseEnum::kFloat: {
        float current = atof(new_arg);
        if (node->validation_float(current)) {
            VariantBase new_variant;
            new_variant.element.t3 = new float{current};
            new_variant.type = VariantBase::BaseEnum::kFloat;
            node->size++;

            if (node->begin_result == nullptr) {
                *node->place_save_first.element.t3 = current;
                node->begin_result = node->prev_result = new BaseNode{nullptr, new_variant};
            } else {
                node->prev_result = node->prev_result->next = new BaseNode{nullptr, new_variant};
            }
        } else {
            result_write = false;
        }
        break;
    }
    case VariantBase::BaseEnum::kString: {
        const char *current = new_arg;

        if (node->validation_string(current)) {
            VariantBase new_variant;
            new_variant.element.t4 = current;
            new_variant.type = VariantBase::BaseEnum::kString;
            node->size++;

            if (node->begin_result == nullptr) {
                node->place_save_first.element.t4 = current;
                node->begin_result = node->prev_result = new BaseNode{nullptr, new_variant};
            } else {
                node->prev_result = node->prev_result->next = new BaseNode{nullptr, new_variant};
            }
        } else {
            result_write = false;
        }
        break;
    }
    case VariantBase::BaseEnum::kBool: {
        bool current = 0;
        if (EqualString(new_arg, "true") && !EqualString(new_arg, "0")) {
            current = 1;
        }

        if (node->validation_bool(current)) {
            VariantBase new_variant;
            new_variant.element.t2 = new bool{current};
            new_variant.type = VariantBase::BaseEnum::kBool;
            node->size++;

            if (node->begin_result == nullptr) {
                *node->place_save_first.element.t2 = current;
                node->begin_result = node->prev_result = new BaseNode{nullptr, new_variant};
            } else {
                node->prev_result = node->prev_result->next = new BaseNode{nullptr, new_variant};
            }
        } else {
            result_write = false;
        }
        break;
    }
    }

    return result_write;
}

nargparse::ParserNode *nargparse::GetParserNode(ArgumentParser &parser, const char *short_argument,
                                                const char *long_argument) {
    ParserNode *current = parser.begin;

    while (current && !EqualString(current->short_argument, short_argument) &&
           !EqualString(current->long_argument, long_argument)) {
        current = current->next;
    }

    return current;
}

nargparse::ParserNode *nargparse::GetParserNode(ArgumentParser &parser, const char *name) {
    ParserNode *current = parser.begin;

    while (current && !EqualString(current->short_argument, name) && !EqualString(current->long_argument, name)) {
        current = current->next;
    }

    return current;
}

nargparse::PositionParserNode *nargparse::GetPositionParserNode(ArgumentParser &parser, const char *name) {
    PositionParserNode *current = parser.begin_pos;

    while (current && !EqualString(current->name, name)) {
        current = current->next;
    }

    return current;
}

nargparse::ParserNode *nargparse::MakeParserNode(const char *short_argument, const char *long_argument,
                                                 const char *help_info) {
    ParserNode *parser_node = new ParserNode;
    parser_node->short_argument = short_argument;
    parser_node->long_argument = long_argument;
    parser_node->help = help_info;

    return parser_node;
}

nargparse::PositionParserNode *nargparse::MakePositionParserNode(const char *name, const char *help_info) {
    PositionParserNode *pos_node = new PositionParserNode;
    pos_node->name = name;
    pos_node->help = help_info;

    return pos_node;
}

void nargparse::AddFlag(ArgumentParser &parser, const char *short_argument, const char *long_argument, bool &flag,
                        const char *help_info, bool default_argument) {
    flag = default_argument;

    ParserNode *node = GetParserNode(parser, short_argument, long_argument);

    if (node != nullptr) {
        ExpandFlagList(node, &flag);
    } else {
        ExpandParserList(parser, MakeParserNode(short_argument, long_argument, help_info));
        ExpandFlagList(parser.prev_end, &flag);
    }
}

void nargparse::AddFlag(ArgumentParser &parser, const char *short_argument, const char *long_argument, bool *flag,
                        const char *help_info, bool default_argument) {
    AddFlag(parser, short_argument, long_argument, *flag, help_info, default_argument);
}

nargparse::ParserNode *nargparse::AddArgument(ArgumentParser &parser, const char *short_argument,
                                              const char *long_argument, VariantBase value, const char *help_info) {
    ParserNode *node = GetParserNode(parser, short_argument, long_argument);

    if (node != nullptr) {
        ExpandBaseList(node, value);
        return node;
    } else {
        ExpandParserList(parser, MakeParserNode(short_argument, long_argument, help_info));
        ExpandBaseList(parser.prev_end, value);
        return parser.prev_end;
    }
}

// default AddArgument

void nargparse::AddArgument(ArgumentParser &parser, const char *short_argument, const char *long_argument,
                            int32_t *value, bool (*validation)(const int32_t &value), const char *help_info,
                            CountArgument count_argument, const char *name) {
    VariantBase current;
    current.type = VariantBase::BaseEnum::kInt;
    current.element.t1 = value;
    ParserNode *node = AddArgument(parser, short_argument, long_argument, current, help_info);
    node->validation_int = validation;
    node->count_argument = count_argument;
    node->name = name;
}

void nargparse::AddArgument(ArgumentParser &parser, const char *short_argument, const char *long_argument, bool *value,
                            bool (*validation)(const bool &value), const char *help_info,
                            CountArgument count_argument, const char *name) {
    VariantBase current;
    current.type = VariantBase::BaseEnum::kBool;
    current.element.t2 = value;
    ParserNode *node = AddArgument(parser, short_argument, long_argument, current, help_info);
    node->validation_bool = validation;
    node->count_argument = count_argument;
    node->name = name;
}

void nargparse::AddArgument(ArgumentParser &parser, const char *short_argument, const char *long_argument,
                            float *value, bool (*validation)(const float &value), const char *help_info,
                            CountArgument count_argument, const char *name) {
    VariantBase current;
    current.type = VariantBase::BaseEnum::kFloat;
    current.element.t3 = value;
    ParserNode *node = AddArgument(parser, short_argument, long_argument, current, help_info);
    node->validation_float = validation;
    node->count_argument = count_argument;
    node->name = name;
}

void nargparse::AddArgument(ArgumentParser &parser, const char *short_argument, const char *long_argument,
                            const char *value, bool (*validation)(const char *const &value), const char *help_info,
                            CountArgument count_argument, const char *name) {
    VariantBase current;
    current.type = VariantBase::BaseEnum::kString;
    current.element.t4 = value;
    ParserNode *node = AddArgument(parser, short_argument, long_argument, current, help_info);
    node->validation_string = validation;
    node->count_argument = count_argument;
    node->name = name;
}

void nargparse::AddArgument(ArgumentParser &parser, const char *short_argument, const char *long_argument,
                            char (*value)[kBuffSize], bool (*validation)(const char *const &value),
                            const char *help_info, CountArgument count_argument, const char *name) {
    VariantBase current;
    current.type = VariantBase::BaseEnum::kString;
    current.element.t4 = *value;
    ParserNode *node = AddArgument(parser, short_argument, long_argument, current, help_info);
    node->validation_string = validation;
    node->count_argument = count_argument;
    node->name = name;
}
// ------2

void nargparse::AddArgument(ArgumentParser &parser, const char *short_argument, const char *long_argument,
                            int32_t *value, const char *name, CountArgument count_argument,
                            bool (*validation)(const int32_t &value), const char *help_info) {
    AddArgument(parser, short_argument, long_argument, value, validation, help_info);
}

void nargparse::AddArgument(ArgumentParser &parser, const char *short_argument, const char *long_argument, bool *value,
                            const char *name, CountArgument count_argument,
                            bool (*validation)(const bool &value), const char *help_info) {
    AddArgument(parser, short_argument, long_argument, value, validation, help_info);
}

void nargparse::AddArgument(ArgumentParser &parser, const char *short_argument, const char *long_argument,
                            float *value, const char *name, CountArgument count_argument,
                            bool (*validation)(const float &value), const char *help_info) {
    AddArgument(parser, short_argument, long_argument, value, validation, help_info);
}

void nargparse::AddArgument(ArgumentParser &parser, const char *short_argument, const char *long_argument,
                            const char *value, const char *name, CountArgument count_argument,
                            bool (*validation)(const char *const &value), const char *help_info) {
    AddArgument(parser, short_argument, long_argument, value, validation, help_info);
}

void nargparse::AddArgument(ArgumentParser &parser, const char *short_argument, const char *long_argument,
                            char (*value)[kBuffSize], const char *name, CountArgument count_argument,
                            bool (*validation)(const char *const &value), const char *help_info) {
    AddArgument(parser, short_argument, long_argument, value, validation, help_info);
}

nargparse::PositionParserNode *nargparse::AddPositionArgument(ArgumentParser &parser, VariantBase value,
                                                              const char *name, CountArgument count_argument,
                                                              const char *help_info) {
    PositionParserNode *node = GetPositionParserNode(parser, name);

    if (node != nullptr) {
        node->place_save_first = value;
        return node;
    } else {
        ExpandPositionParserList(parser, MakePositionParserNode(name, help_info));
        node = parser.prev_end_pos;
        node->place_save_first = value;
        node->count_argument = count_argument;
        return node;
    }
}

void nargparse::AddArgument(ArgumentParser &parser, int32_t *value, const char *name, CountArgument count_argument,
                            bool (*validation)(const int32_t &value), const char *help_info) {
    VariantBase current;
    current.type = VariantBase::BaseEnum::kInt;
    current.element.t1 = value;
    PositionParserNode *node = AddPositionArgument(parser, current, name, count_argument, help_info);
    node->validation_int = validation;
}

void nargparse::AddArgument(ArgumentParser &parser, bool *value, const char *name, CountArgument count_argument,
                            bool (*validation)(const bool &value), const char *help_info) {
    VariantBase current;
    current.type = VariantBase::BaseEnum::kBool;
    current.element.t2 = value;
    PositionParserNode *node = AddPositionArgument(parser, current, name, count_argument, help_info);
    node->validation_bool = validation;
}

void nargparse::AddArgument(ArgumentParser &parser, float *value, const char *name, CountArgument count_argument,
                            bool (*validation)(const float &value), const char *help_info) {
    VariantBase current;
    current.type = VariantBase::BaseEnum::kFloat;
    current.element.t3 = value;
    PositionParserNode *node = AddPositionArgument(parser, current, name, count_argument, help_info);
    node->validation_float = validation;
}

void nargparse::AddArgument(ArgumentParser &parser, const char *value, const char *name, CountArgument count_argument,
                            bool (*validation)(const char *const &value), const char *help_info) {
    VariantBase current;
    current.type = VariantBase::BaseEnum::kString;
    current.element.t4 = value;
    PositionParserNode *node = AddPositionArgument(parser, current, name, count_argument, help_info);
    node->validation_string = validation;
}

void nargparse::AddArgument(ArgumentParser &parser, char (*value)[kBuffSize], const char *name,
                            CountArgument count_argument, bool (*validation)(const char *const &value),
                            const char *help_info) {
    VariantBase current;
    current.type = VariantBase::BaseEnum::kString;
    current.element.t4 = *value;
    PositionParserNode *node = AddPositionArgument(parser, current, name, count_argument, help_info);
    node->validation_string = validation;
}

void nargparse::MarkFlags(ParserNode *node) {
    FlagNode *current = node->begin_flag;
    while (current) {
        *current->element = true;
        current = current->next;
    }
}

bool nargparse::SetValues(ParserNode *node, const char *value) {
    BaseNode *current_node = node->begin_base;

    bool validation_result = true;
    while (current_node) {
        switch (current_node->element.type) {
        case VariantBase::BaseEnum::kInt: {
            int32_t current = atoll(value);
            if (node->validation_int(current)) {
                *current_node->element.element.t1 = current;
            } else {
                validation_result = false;
            }
            break;
        }
        case VariantBase::BaseEnum::kFloat: {
            float current = atof(value);
            if (node->validation_float(current)) {
                *current_node->element.element.t3 = current;
            } else {
                validation_result = false;
            }
            break;
        }
        case VariantBase::BaseEnum::kString: {
            const char *current = value;

            if (node->validation_string(current)) {
                current_node->element.element.t4 = current;
            } else {
                validation_result = false;
            }
            break;
        }
        case VariantBase::BaseEnum::kBool: {
            bool current = false;
            if (EqualString(value, "true") || !EqualString(value, "0")) {
                current = true;
            }

            if (node->validation_bool(current)) {
                *current_node->element.element.t2 = current;
            } else {
                validation_result = false;
            }
            break;
        }
        }

        current_node = current_node->next;
    }

    return validation_result;
}

bool nargparse::Parse(ArgumentParser &parser, uint32_t argc, const char **argv) {
    bool result_parsing = true;
    PositionParserNode *current_position_node = parser.begin_pos;

    for (uint32_t index_argv = 1; result_parsing && index_argv < argc; index_argv++) {
        ParserNode *node = GetParserNode(parser, argv[index_argv]);

        if (node) {
            if (EqualString(node->short_argument, "-h")){
                if (node->was_info == 0){
                    std::cout << "There help" << std::endl;
                }

                node->was_info = 1;
                continue;
            }

            node->was_info = 1;
            ++index_argv;
            const char *argument = argv[index_argv];

            MarkFlags(node);
            result_parsing &= SetValues(node, argv[index_argv]);

            if (current_position_node && current_position_node->size > 0 && current_position_node->next) {
                if (current_position_node->count_argument == CountArgument::kNargsZeroOrMore ||
                    current_position_node->count_argument == CountArgument::kNargsOneOrMore) {
                    current_position_node = current_position_node->next;
                }
            }
        } else {
            if (!current_position_node) {
                return false;
            }

            result_parsing &= WritePositionArgument(current_position_node, argv[index_argv]);

            if (current_position_node->count_argument == CountArgument::kNargsRequired ||
                current_position_node->count_argument == CountArgument::kNargsOptional) {
                if (current_position_node->size > 1) {
                    result_parsing = false;
                }
                current_position_node = current_position_node->next;
            }
        }
    }

    if (current_position_node) {
        if (current_position_node->count_argument == CountArgument::kNargsRequired ||
            current_position_node->count_argument == CountArgument::kNargsOneOrMore) {
            if (current_position_node->size < 1) {
                result_parsing = false;
            }
            current_position_node = current_position_node->next;
        }
    }

    ParserNode* node = parser.begin;
    while (node){
        if (node->count_argument == CountArgument::kNargsRequired && node->was_info == 0){
            result_parsing = false;
            break;
        }
        node->was_info = 0;
        node = node->next;
    }

    return result_parsing;
}

void nargparse::FreeBaseList(BaseNode *node) {
    while (node) {
        BaseNode *next = node->next;
        delete node;
        node = next;
    }
}

void nargparse::FreeFlagList(FlagNode *node) {
    while (node) {
        FlagNode *next = node->next;
        delete node;
        node = next;
    }
}

void nargparse::FreeUsuallyArguments(ArgumentParser &parser) {
    ParserNode *current_node = parser.begin;
    while (current_node) {
        ParserNode *next = current_node->next;
        FreeBaseList(current_node->begin_base);
        FreeFlagList(current_node->begin_flag);
        delete current_node;
        current_node = next;
    }
}

void nargparse::FreePositionArguments(ArgumentParser &parser) {
    PositionParserNode *current_node = parser.begin_pos;
    while (current_node) {
        PositionParserNode *next = current_node->next;
        FreeBaseList(current_node->begin_result);
        delete current_node;
        current_node = next;
    }
}

void nargparse::FreeParser(ArgumentParser &parser) {
    FreeUsuallyArguments(parser);
    FreePositionArguments(parser);
}

uint32_t nargparse::GetRepeatedCount(ArgumentParser &parser, const char *name) {
    PositionParserNode *node = GetPositionParserNode(parser, name);
    uint32_t result = node->size;

    return result;
}

bool nargparse::GetRepeated(ArgumentParser &parser, const char *name, uint32_t index, int32_t *value) {
    PositionParserNode *node = GetPositionParserNode(parser, name);
    if (!node) {
        return false;
    }
    BaseNode *current_value = node->begin_result;

    for (uint32_t k = 0; k < index; k++) {
        current_value = current_value->next;
        if (!current_value) {
            return false;
        }
    }

    *value = *current_value->element.element.t1;
    return true;
}

bool nargparse::GetRepeated(ArgumentParser &parser, const char *name, uint32_t index, bool *value) {
    PositionParserNode *node = GetPositionParserNode(parser, name);
    if (!node) {
        return false;
    }
    BaseNode *current_value = node->begin_result;

    for (uint32_t k = 0; k < index; k++) {
        current_value = current_value->next;
        if (!current_value) {
            return false;
        }
    }

    *value = *current_value->element.element.t2;
    return true;
}

bool nargparse::GetRepeated(ArgumentParser &parser, const char *name, uint32_t index, float *value) {
    PositionParserNode *node = GetPositionParserNode(parser, name);
    if (!node) {
        return false;
    }
    BaseNode *current_value = node->begin_result;

    for (uint32_t k = 0; k < index; k++) {
        current_value = current_value->next;
        if (!current_value) {
            return false;
        }
    }

    *value = *current_value->element.element.t3;
    return true;
}

bool nargparse::GetRepeated(ArgumentParser &parser, const char *name, uint32_t index, const char *value) {
    PositionParserNode *node = GetPositionParserNode(parser, name);
    if (!node) {
        return false;
    }
    BaseNode *current_value = node->begin_result;

    for (uint32_t k = 0; k < index; k++) {
        current_value = current_value->next;
        if (!current_value) {
            return false;
        }
    }

    value = current_value->element.element.t4;
    return true;
}

bool nargparse::GetRepeated(ArgumentParser &parser, const char *name, uint32_t index, const char **value) {
    PositionParserNode *node = GetPositionParserNode(parser, name);
    if (!node) {
        return false;
    }
    BaseNode *current_value = node->begin_result;

    for (uint32_t k = 0; k < index; k++) {
        current_value = current_value->next;
        if (!current_value) {
            return false;
        }
    }

    *value = current_value->element.element.t4;
    return true;
}

nargparse::ArgumentParser nargparse::CreateParser(const char *name, uint32_t buff_size) {
    return ArgumentParser{name, buff_size};
}

void nargparse::AddHelp(ArgumentParser &parser) {
    int a = 5;
    AddArgument(parser, "-h", "--help", &a, "help_info");
}

void nargparse::PrintHelp(ArgumentParser &parser) {
    /* const char* name;
    GetRepeated(parser, "names", 0, &name); */
}



//Вывод хелпы добавить не забудь