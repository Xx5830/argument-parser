#include "argparser.h"
#include <cstdlib>

bool nargparse::EqualString(const char *left, const char *right) {
    if (!left || !right) {
        return false;
    }

    for (uint32_t i = 0; left[i] != '/0'; i++) {
        if (left[i] != right[i]) {
            return false;
        }
    }

    return true;
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

void nargparse::ExpandParserList(Parser &parser, ParserNode *element) {
    if (parser.begin) {
        parser.prev_end = parser.prev_end->next = element;
    } else {
        parser.begin = parser.prev_end = element;
    }
}

void nargparse::ExpandPositionParserList(Parser &parser, PositionParserNode *element) {
    if (parser.begin) {
        parser.prev_end_pos = parser.prev_end_pos->next = element;
    } else {
        parser.begin_pos = parser.prev_end_pos = element;
    }
}

void nargparse::WritePositionArgument(PositionParserNode *node, const char *new_arg) {
    switch (node->place_save_first.type) {
    case VariantBase::BaseEnum::kLongInt: {
        int64_t current = atoll(new_arg);
        if (node->validation_int(current)) {
            VariantBase new_variant;
            new_variant.element.t1 = new int64_t{current};
            new_variant.type = VariantBase::BaseEnum::kLongInt;
            node->size++;

            if (node->begin_result == nullptr) {
                *node->place_save_first.element.t1 = current;
                node->begin_result = node->prev_result = new BaseNode{nullptr, new_variant};
            } else {
                node->prev_result = new BaseNode{nullptr, new_variant};
            }
        }
        break;
    }
    case VariantBase::BaseEnum::kDouble: {
        double current = atof(new_arg);
        if (node->validation_double(current)) {
            VariantBase new_variant;
            new_variant.element.t3 = new double{current};
            new_variant.type = VariantBase::BaseEnum::kLongInt;
            node->size++;

            if (node->begin_result == nullptr) {
                *node->place_save_first.element.t3 = current;
                node->begin_result = node->prev_result = new BaseNode{nullptr, new_variant};
            } else {
                node->prev_result = new BaseNode{nullptr, new_variant};
            }
        }
        break;
    }
    case VariantBase::BaseEnum::kString: {
        char *current;
        {
            uint32_t size = 0;
            while (new_arg[size] != '/0') {
                ++size;
            }

            current = new char[size + 1];
            for (uint32_t index = 0; index < size + 1; index++) {
                current[index] = new_arg[index];
            }
        }

        if (node->validation_string(current)) {
            VariantBase new_variant;
            new_variant.element.t4 = current;
            new_variant.type = VariantBase::BaseEnum::kString;
            node->size++;

            if (node->begin_result == nullptr) {
                node->place_save_first.element.t4 = current;
                node->begin_result = node->prev_result = new BaseNode{nullptr, new_variant};
            } else {
                node->prev_result = new BaseNode{nullptr, new_variant};
            }
        }
        break;
    }
    case VariantBase::BaseEnum::kBool: {
        bool current = 0;
        if (new_arg == "true" || !(new_arg == "0")) {
            current = 1;
        }

        if (node->validation_int(current)) {
            VariantBase new_variant;
            new_variant.element.t2 = new bool{current};
            new_variant.type = VariantBase::BaseEnum::kLongInt;
            node->size++;

            if (node->begin_result == nullptr) {
                *node->place_save_first.element.t2 = current;
                node->begin_result = node->prev_result = new BaseNode{nullptr, new_variant};
            } else {
                node->prev_result = new BaseNode{nullptr, new_variant};
            }
        }
        break;
    }
    }
}

nargparse::ParserNode *nargparse::GetParserNode(Parser &parser, const char *short_argument, const char *long_argument) {
    ParserNode *current = parser.begin;

    while (current && !(current->short_argument == short_argument) && !(current->long_argument == long_argument)) {
        current = current->next;
    }

    return current;
}

nargparse::PositionParserNode *nargparse::GetPositionParserNode(Parser &parser, const char *name) {
    PositionParserNode *current = parser.begin_pos;

    while (current && !(current->name == name)) {
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

void nargparse::AddFlag(Parser &parser, const char *short_argument, const char *long_argument, bool &flag,
                        const char *help_info, bool default_argument = false) {
    flag = default_argument;

    ParserNode *node = GetParserNode(parser, short_argument, long_argument);

    if (node != nullptr) {
        ExpandFlagList(node, &flag);
    } else {
        ExpandParserList(parser, MakeParserNode(short_argument, long_argument, help_info));
        ExpandFlagList(parser.prev_end, &flag);
    }
}

nargparse::ParserNode *nargparse::AddArgument(Parser &parser, const char *short_argument, const char *long_argument,
                                              VariantBase value, const char *help_info) {
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

void nargparse::AddArgument(Parser &parser, const char *short_argument, const char *long_argument, int64_t &value,
                            bool (*validation)(const int64_t &value), const char *help_info) {
    VariantBase current;
    current.type = VariantBase::BaseEnum::kLongInt;
    current.element.t1 = &value;
    ParserNode *node = AddArgument(parser, short_argument, long_argument, current, help_info);
    node->validation_int = validation;
}

void nargparse::AddArgument(Parser &parser, const char *short_argument, const char *long_argument, bool &value,
                            bool (*validation)(const double &value), const char *help_info) {
    VariantBase current;
    current.type = VariantBase::BaseEnum::kBool;
    current.element.t2 = &value;
    ParserNode *node = AddArgument(parser, short_argument, long_argument, current, help_info);
    node->validation_double = validation;
}

void nargparse::AddArgument(Parser &parser, const char *short_argument, const char *long_argument, double &value,
                            bool (*validation)(const bool &value), const char *help_info) {
    VariantBase current;
    current.type = VariantBase::BaseEnum::kDouble;
    current.element.t3 = &value;
    ParserNode *node = AddArgument(parser, short_argument, long_argument, current, help_info);
    node->validation_bool = validation;
}

void nargparse::AddArgument(Parser &parser, const char *short_argument, const char *long_argument, char *value,
                            bool (*validation)(const char *const &value), const char *help_info) {
    VariantBase current;
    current.type = VariantBase::BaseEnum::kString;
    current.element.t4 = value;
    ParserNode *node = AddArgument(parser, short_argument, long_argument, current, help_info);
    node->validation_string = validation;
}

//------------free

nargparse::PositionParserNode *nargparse::AddFreeArgument(Parser &parser, VariantBase value, const char *name,
                                                          CountArgument count_argument, const char *help_info) {
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

void nargparse::AddArgument(Parser &parser, int64_t &value, const char *name, CountArgument count_argument,
                            bool (*validation)(const int64_t &value), const char *help_info) {
    VariantBase current;
    current.type = VariantBase::BaseEnum::kLongInt;
    current.element.t1 = &value;
    PositionParserNode *node = AddFreeArgument(parser, current, name, count_argument, help_info);
    node->validation_int = validation;
}

void nargparse::AddArgument(Parser &parser, bool &value, const char *name, CountArgument count_argument,
                            bool (*validation)(const bool &value), const char *help_info) {
    VariantBase current;
    current.type = VariantBase::BaseEnum::kBool;
    current.element.t2 = &value;
    PositionParserNode *node = AddFreeArgument(parser, current, name, count_argument, help_info);
    node->validation_bool = validation;
}

void nargparse::AddArgument(Parser &parser, double &value, const char *name, CountArgument count_argument,
                            bool (*validation)(const double &value), const char *help_info) {
    VariantBase current;
    current.type = VariantBase::BaseEnum::kDouble;
    current.element.t3 = &value;
    PositionParserNode *node = AddFreeArgument(parser, current, name, count_argument, help_info);
    node->validation_double = validation;
}

void nargparse::AddArgument(Parser &parser, char *value, const char *name, CountArgument count_argument,
                            bool (*validation)(const char *const &value), const char *help_info) {
    VariantBase current;
    current.type = VariantBase::BaseEnum::kString;
    current.element.t4 = value;
    PositionParserNode *node = AddFreeArgument(parser, current, name, count_argument, help_info);
    node->validation_string = validation;
}
