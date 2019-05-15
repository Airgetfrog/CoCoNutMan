#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "lib/array.h"

typedef struct ConfigSpec {
    array *enums;
    array *multioptions;
    array *optionsets;
    array *targetoptions;
    struct Config *config;
} ConfigSpec;

enum FieldType {
    FT_uint,
    FT_int,
    FT_float,
    FT_string,
    FT_bool,
    FT_enum,
    FT_list
};

typedef struct FieldValue {
    enum FieldType type;
    union {
        uint64_t uint_value;
        int64_t int_value;
        float float_value;
        char *string_value;
        bool bool_value;
        array *array_value;
    } value;
} FieldValue;

enum Attributes {
    E_prefix,
    E_values,
    GF_options,
    GF_fields,
    OS_options,
    OS_tokens,
    OS_separator,
    C_configfile,
    C_fields,
    F_configfile,
    F_options,
    F_argument,
    F_separator,
    F_range,
    F_prefix,
    F_values
};

typedef struct Attribute {
    enum Attributes attribute;
    union {
        bool bool_value;
        char *string_value;
        array *array_value;
        struct Range *range_value;
    } value;
} Attribute;

typedef struct Enum {
    char *id;
    array *attributes;
    char *prefix;
    array *values;
} Enum;

typedef struct MultiOption {
    char *info;
    array *attributes;
    array *options;
    array *setters;
} MultiOption;

typedef struct Setter {
    array *ids;
    struct FieldValue *value;
} Setter;

typedef struct OptionSet {
    char *info;
    array *attributes;
    array *options;
    array *tokens;
} OptionSet;

typedef struct Token {
    char *token;
    array *setters;
} Token;

typedef struct Config {
    char *id;
    char *info;
    array *attributes;
    bool configfile;
    array *fields;
} Config;

typedef struct Field {
    char *id;
    char *info;
    enum FieldType type;
    bool is_list;
    struct Config *config;
    struct FieldValue *default_value;
    array *attributes;
    bool configfile;
    array *options;
    bool is_argument;
    char *separator;
    struct Range *range;
    char *prefix;
    array *values;
} Field;

typedef struct Range {
    bool left_open;
    bool left_inf;
    bool right_open;
    bool right_inf;
    struct FieldValue *left_bound;
    struct FieldValue *right_bound;
} Range;
