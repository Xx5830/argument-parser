#pragma once

#include <cstdint>
#include <utility>

namespace nargparse {

typedef enum class CountArgument {
    kNargsZeroOrMore,
    kNargsOneOrMore,
    kNargsRequired,
    kNargsOptional
};

union BaseTypePointer {
    int64_t *t1;
    bool *t2;
    double *t3;
    char *t4;
};

struct VariantBase {
    enum class BaseEnum { kLongInt, kBool, kDouble, kString };
    BaseTypePointer element;
    BaseEnum type;
};

struct FlagNode {
    FlagNode *next = nullptr;
    bool *element;
};

struct BaseNode {
    BaseNode *next = nullptr;
    VariantBase element;
};

struct ParserNode {
    ParserNode *next = nullptr;

    const char *short_argument = nullptr;
    const char *long_argument = nullptr;
    const char *help = nullptr;
    FlagNode *begin_flag = nullptr;
    FlagNode *prev_end_flag = nullptr;
    BaseNode *begin_base = nullptr;
    BaseNode *prev_end_base = nullptr;
    bool (*validation_int)(const int64_t &value) = nullptr;
    bool (*validation_double)(const double &value) = nullptr;
    bool (*validation_bool)(const bool &value) = nullptr;
    bool (*validation_string)(const char *const &value) = nullptr;
};

struct PositionParserNode {
    PositionParserNode *next = nullptr;

    const char *name = nullptr;
    const char *help = nullptr;
    CountArgument count_argument;
    VariantBase place_save_first;
    uint32_t size = 0;
    BaseNode *begin_result = nullptr;
    BaseNode *prev_result = nullptr;
    bool (*validation_int)(const int64_t &value) = nullptr;
    bool (*validation_double)(const double &value) = nullptr;
    bool (*validation_bool)(const bool &value) = nullptr;
    bool (*validation_string)(const char *const &value) = nullptr;
};

struct Parser {
    ParserNode *begin;
    ParserNode *prev_end;
    PositionParserNode *begin_pos;
    PositionParserNode *prev_end_pos;
};

bool FTrueInt(const int64_t &value) { return true; }
bool FTrueDouble(const double &value) { return true; }
bool FTrueBool(const bool &value) { return true; }
bool FTrueString(const char *const &value) { return true; }

char *non_info = "empty info";
CountArgument default_count_argument = CountArgument::kNargsRequired;

bool EqualString(const char *left, const char *right);

void ExpandFlagList(ParserNode *node, bool *flag);
void ExpandBaseList(ParserNode *node, VariantBase element);
void ExpandParserList(Parser &parser, ParserNode *element);

void ExpandPositionParserList(Parser &parser, PositionParserNode *element);
void WritePositionArgument(PositionParserNode *node, const char *new_arg);

ParserNode *GetParserNode(Parser &parser, const char *short_argument, const char *long_argument);

nargparse::PositionParserNode *GetPositionParserNode(Parser &parser, const char *name);

ParserNode *MakeParserNode(const char *short_argument, const char *long_argument, const char *help_info);

PositionParserNode *MakePositionParserNode(const char* name, const char *help_info);

void AddFlag(Parser &parser, const char *short_argument, const char *long_argument, bool &flag, const char *help_info,
             bool default_argument = false);

ParserNode *AddArgument(Parser &parser, const char *short_argument, const char *long_argument, VariantBase value,
                        const char *help_info = non_info);
void AddArgument(Parser &parser, const char *short_argument, const char *long_argument, int64_t &value,
                 bool (*validation)(const int64_t &value) = FTrueInt, const char *help_info = non_info);
void AddArgument(Parser &parser, const char *short_argument, const char *long_argument, bool &value,
                 bool (*validation)(const double &value) = FTrueDouble, const char *help_info = non_info);
void AddArgument(Parser &parser, const char *short_argument, const char *long_argument, double &value,
                 bool (*validation)(const bool &value) = FTrueBool, const char *help_info = non_info);
void AddArgument(Parser &parser, const char *short_argument, const char *long_argument, char *value,
                 bool (*validation)(const char *const &value) = FTrueString, const char *help_info = non_info);

//---------------free

PositionParserNode* AddFreeArgument(Parser &parser, VariantBase value, const char* name, CountArgument count_argument = CountArgument::kNargsZeroOrMore, const char* help_info = non_info);

void AddArgument(Parser &parser, int64_t &value, const char *name, CountArgument count_argument,
                 bool (*validation)(const int64_t &value) = FTrueInt, const char *help_info = non_info);

void AddArgument(Parser &parser, bool &value, const char *name, CountArgument count_argument,
                 bool (*validation)(const bool &value) = FTrueBool, const char *help_info = non_info);

void AddArgument(Parser &parser, double &value, const char *name, CountArgument count_argument,
                 bool (*validation)(const double &value) = FTrueDouble, const char *help_info = non_info);

void AddArgument(Parser &parser, char *value, const char *name, CountArgument count_argument,
                 bool (*validation)(const char *const &value) = FTrueString, const char *help_info = non_info);

bool Parse(Parser &parser, uint32_t argc, char** argv){

}

} // namespace nargparse
