#ifndef _CFG_H
#define _CFG_H

#include <stdbool.h>
#include <stdint.h>

#include "lib/array.h"
#include "lib/smap.h"

typedef struct Configuration {
    array *enums;
    array *multioptions;
    array *optionsets;
    array *targetoptions;
    struct Config *config;
} Configuration;

typedef enum FieldType {
    FT_uint,
    FT_int,
    FT_float,
    FT_string,
    FT_bool,
    FT_enum,
    FT_list
} FieldType;

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

typedef enum Attributes {
    E_prefix,
    E_values,
    MO_options,
    MO_fields,
    OS_options,
    OS_tokens,
    OS_separator,
    C_configfile,
    C_fields,
    F_configfile,
    F_options,
    F_disable,
    F_argument,
    F_separator,
    F_range
} Attributes;

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
    array *fields;
    bool takes_argument;
} MultiOption;

typedef struct Setter {
    array *ids;
    struct FieldValue *value;
    struct Field *field;
} Setter;

typedef struct OptionSet {
    char *info;
    array *attributes;
    array *options;
    array *tokens;
    char *separator;
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

    smap_t *smap;
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
    array *disable_options;
    bool is_argument;
    char *separator;
    struct Range *range;
    char *enum_id;
    struct Enum *enum_struct;
} Field;

typedef struct Range {
    bool left_open;
    bool right_open;
    struct FieldValue *left_bound;
    struct FieldValue *right_bound;
} Range;

#endif
