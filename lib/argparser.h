#pragma once

#include <cstdint>
#include <utility>

namespace nargparse {

enum class CountArgument { kNargsZeroOrMore, kNargsOneOrMore, kNargsRequired, kNargsOptional };
const CountArgument kNargsZeroOrMore = CountArgument::kNargsZeroOrMore; // 0-inf
const CountArgument kNargsOneOrMore = CountArgument::kNargsOneOrMore;   // 1-inf
const CountArgument kNargsRequired = CountArgument::kNargsRequired;     // 1-1
const CountArgument kNargsOptional = CountArgument::kNargsOptional;     // 0-1
const CountArgument kDefaultCountArgument = CountArgument::kNargsOptional;

union BaseTypePointer {
    int32_t *t1;
    bool *t2;
    float *t3;
    const char *t4;
};

struct VariantBase {
    enum class BaseEnum { kInt, kBool, kFloat, kString };
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
    bool (*validation_int)(const int32_t &value) = nullptr;
    bool (*validation_float)(const float &value) = nullptr;
    bool (*validation_bool)(const bool &value) = nullptr;
    bool (*validation_string)(const char *const &value) = nullptr;
    CountArgument count_argument = kDefaultCountArgument;
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
    bool (*validation_int)(const int32_t &value) = nullptr;
    bool (*validation_float)(const float &value) = nullptr;
    bool (*validation_bool)(const bool &value) = nullptr;
    bool (*validation_string)(const char *const &value) = nullptr;
};

const int32_t kBuffSize = 128;
struct ArgumentParser {
    const char *name;
    uint32_t buff_size;

    ParserNode *begin;
    ParserNode *prev_end;
    PositionParserNode *begin_pos;
    PositionParserNode *prev_end_pos;
};

bool FTrueInt(const int32_t &value) { return true; }
bool FTrueFloat(const float &value) { return true; }
bool FTrueBool(const bool &value) { return true; }
bool FTrueString(const char *const &value) { return true; }

const char *non_info = "empty info";
CountArgument default_count_argument = CountArgument::kNargsRequired;

bool EqualString(const char *left, const char *right);

void ExpandFlagList(ParserNode *node, bool *flag);
void ExpandBaseList(ParserNode *node, VariantBase element);
void ExpandParserList(ArgumentParser &parser, ParserNode *element);

void ExpandPositionParserList(ArgumentParser &parser, PositionParserNode *element);
bool WritePositionArgument(PositionParserNode *node, const char *new_arg);

ParserNode *GetParserNode(ArgumentParser &parser, const char *short_argument, const char *long_argument);
ParserNode *GetParserNode(ArgumentParser &parser, const char *name);

nargparse::PositionParserNode *GetPositionParserNode(ArgumentParser &parser, const char *name);

ParserNode *MakeParserNode(const char *short_argument, const char *long_argument, const char *help_info);

PositionParserNode *MakePositionParserNode(const char *name, const char *help_info);

void AddFlag(ArgumentParser &parser, const char *short_argument, const char *long_argument, bool &flag,
             const char *help_info = non_info, bool default_argument = false);

void AddFlag(ArgumentParser &parser, const char *short_argument, const char *long_argument, bool *flag,
             const char *help_info = non_info, bool default_argument = false);

ParserNode *AddArgument(ArgumentParser &parser, const char *short_argument, const char *long_argument,
                        VariantBase value, const char *help_info = non_info);

void AddArgument(ArgumentParser &parser, const char *short_argument, const char *long_argument, int32_t *value,
                 bool (*validation)(const int32_t &value) = FTrueInt, const char *help_info = non_info,
                 CountArgument count_argument = kDefaultCountArgument);
void AddArgument(ArgumentParser &parser, const char *short_argument, const char *long_argument, bool *value,
                 bool (*validation)(const bool &value) = FTrueBool, const char *help_info = non_info,
                 CountArgument count_argument = kDefaultCountArgument);
void AddArgument(ArgumentParser &parser, const char *short_argument, const char *long_argument, float *value,
                 bool (*validation)(const float &value) = FTrueFloat, const char *help_info = non_info,
                 CountArgument count_argument = kDefaultCountArgument);
void AddArgument(ArgumentParser &parser, const char *short_argument, const char *long_argument, const char *value,
                 bool (*validation)(const char *const &value) = FTrueString, const char *help_info = non_info,
                 CountArgument count_argument = kDefaultCountArgument);
void AddArgument(ArgumentParser &parser, const char *short_argument, const char *long_argument,
                 char (*value)[kBuffSize], bool (*validation)(const char *const &value) = FTrueString,
                 const char *help_info = non_info, CountArgument count_argument = kDefaultCountArgument);

void AddArgument(ArgumentParser &parser, const char *short_argument, const char *long_argument, int32_t *value,
                 const char *help_info = non_info, CountArgument count_argument = kDefaultCountArgument,
                 bool (*validation)(const int32_t &value) = FTrueInt);
void AddArgument(ArgumentParser &parser, const char *short_argument, const char *long_argument, bool *value,
                 const char *help_info = non_info, CountArgument count_argument = kDefaultCountArgument,
                 bool (*validation)(const bool &value) = FTrueBool);
void AddArgument(ArgumentParser &parser, const char *short_argument, const char *long_argument, float *value,
                 const char *help_info = non_info, CountArgument count_argument = kDefaultCountArgument,
                 bool (*validation)(const float &value) = FTrueFloat);
void AddArgument(ArgumentParser &parser, const char *short_argument, const char *long_argument, const char *value,
                 const char *help_info = non_info, CountArgument count_argument = kDefaultCountArgument,
                 bool (*validation)(const char *const &value) = FTrueString);
void AddArgument(ArgumentParser &parser, const char *short_argument, const char *long_argument,
                 char (*value)[kBuffSize], const char *help_info = non_info,
                 CountArgument count_argument = kDefaultCountArgument,
                 bool (*validation)(const char *const &value) = FTrueString);

PositionParserNode *AddPositionArgument(ArgumentParser &parser, VariantBase value, const char *name,
                                        CountArgument count_argument = kDefaultCountArgument,
                                        const char *help_info = non_info);

void AddArgument(ArgumentParser &parser, int32_t *value, const char *name,
                 CountArgument count_argument = kDefaultCountArgument,
                 bool (*validation)(const int32_t &value) = FTrueInt, const char *help_info = non_info);
void AddArgument(ArgumentParser &parser, bool *value, const char *name,
                 CountArgument count_argument = kDefaultCountArgument,
                 bool (*validation)(const bool &value) = FTrueBool, const char *help_info = non_info);
void AddArgument(ArgumentParser &parser, float *value, const char *name,
                 CountArgument count_argument = kDefaultCountArgument,
                 bool (*validation)(const float &value) = FTrueFloat, const char *help_info = non_info);
void AddArgument(ArgumentParser &parser, const char *value, const char *name,
                 CountArgument count_argument = kDefaultCountArgument,
                 bool (*validation)(const char *const &value) = FTrueString, const char *help_info = non_info);
void AddArgument(ArgumentParser &parser, char (*value)[kBuffSize], const char *name,
                 CountArgument count_argument = kDefaultCountArgument,
                 bool (*validation)(const char *const &value) = FTrueString, const char *help_info = non_info);

void MarkFlags(ParserNode *node);

bool SetValues(ParserNode *node, const char *value);

bool Parse(ArgumentParser &parser, uint32_t argc, const char **argv);

void FreeBaseList(BaseNode *node);

void FreeFlagList(FlagNode *node);

void FreeUsuallyArguments(ArgumentParser &parser);

void FreePositionArguments(ArgumentParser &parser);

void FreeParser(ArgumentParser &parser);

uint32_t GetRepeatedCount(ArgumentParser &parser, const char *name);

bool GetRepeated(ArgumentParser &parser, const char *name, uint32_t index, const char *value);

bool GetRepeated(ArgumentParser &parser, const char *name, uint32_t index, int32_t *value);

bool GetRepeated(ArgumentParser &parser, const char *name, uint32_t index, bool *value);

bool GetRepeated(ArgumentParser &parser, const char *name, uint32_t index, float *value);

bool GetRepeated(ArgumentParser &parser, const char *name, uint32_t index, const char **value);

ArgumentParser CreateParser(const char *name, uint32_t buff_size = 1024);

void AddHelp(ArgumentParser &parser);

void PrintHelp(ArgumentParser &parser);
    
} // namespace nargparse
