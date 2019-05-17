#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cfg.h"
#include "create-cfg.h"

#include "lib/array.h"
#include "lib/memory.h"
#include "lib/print.h"

Configuration *create_configuration(array *enums, array *multioptions, array *optionsets, array *targetoptions, Config *config) {
    Configuration *node = mem_alloc(sizeof(Configuration));
    node->enums = enums;
    node->multioptions = multioptions;
    node->optionsets = optionsets;
    node->targetoptions = targetoptions;
    node->config = config;
    return node;
}

Enum *create_enum(char *id, array *attributes) {
    Enum *node = mem_alloc(sizeof(Enum));
    node->id = id;
    node->attributes = attributes;
    node->prefix = NULL;
    node->values = NULL;
    return node;
}

MultiOption *create_multioption(char *info, array *attributes) {
    MultiOption *node = mem_alloc(sizeof(MultiOption));
    node->info = info;
    node->attributes = attributes;
    node->options = NULL;
    node->fields = NULL;
    return node;
}

Setter *create_setter(array *ids, FieldValue *value) {
    Setter *node = mem_alloc(sizeof(Setter));
    node->ids = ids;
    node->value = value;
    return node;
}

OptionSet *create_option_set(char *info, array *attributes) {
    OptionSet *node = mem_alloc(sizeof(OptionSet));
    node->info = info;
    node->attributes = attributes;
    node->options = NULL;
    node->tokens = NULL;
    node->separator = NULL;
    return node;
}

Token *create_token(char *token, array *setters) {
    Token *node = mem_alloc(sizeof(Token));
    node->token = token;
    node->setters = setters;
    return node;
}

Config *create_config(char *id, char *info, array *attributes) {
    Config *node = mem_alloc(sizeof(Config));
    node->id = id;
    node->info = info;
    node->attributes = attributes;
    node->fields = NULL;
    return node;
}

Field *create_field(char *id, char *info, enum FieldType type, bool is_list, FieldValue *default_value, array *attributes) {
    Field *node = mem_alloc(sizeof(Field));
    node->id = id;
    node->info = info;
    node->type = type;
    node->is_list = is_list;
    node->config = NULL;
    node->default_value = default_value;
    node->attributes = attributes;
    node->options = NULL;
    node->separator = NULL;
    node->range = NULL;
    node->prefix = NULL;
    node->values = NULL;
    return node;
}

Field *create_field_config(Config *config) {
    Field *node = mem_alloc(sizeof(Field));
    node->config = config;
    return node;
}

Range *create_range(bool left_open, bool left_inf, bool right_open, bool right_inf, FieldValue *left_bound, FieldValue *right_bound) {
    Range *node = mem_alloc(sizeof(Range));
    node->left_open = left_open;
    node->right_open = right_open;
    node->left_inf = left_inf;
    node->right_inf = right_inf;
    node->left_bound = left_bound;
    node->right_bound = right_bound;
    return node;
}

FieldValue *create_field_value_uint(uint64_t value) {
    FieldValue *node = mem_alloc(sizeof(FieldValue));
    node->type = FT_uint;
    node->value.uint_value = value;
    return node;
}

FieldValue *create_field_value_int(int64_t value) {
    FieldValue *node = mem_alloc(sizeof(FieldValue));
    node->type = FT_int;
    node->value.int_value = value;
    return node;
}

FieldValue *create_field_value_float(float value) {
    FieldValue *node = mem_alloc(sizeof(FieldValue));
    node->type = FT_float;
    node->value.float_value = value;
    return node;
}

FieldValue *create_field_value_string(char *value) {
    FieldValue *node = mem_alloc(sizeof(FieldValue));
    node->type = FT_string;
    node->value.string_value = value;
    return node;
}

FieldValue *create_field_value_bool(bool value) {
    FieldValue *node = mem_alloc(sizeof(FieldValue));
    node->type = FT_bool;
    node->value.bool_value = value;
    return node;
}

FieldValue *create_field_value_array(array *value) {
    FieldValue *node = mem_alloc(sizeof(FieldValue));
    node->type = FT_list;
    node->value.array_value = value;
    return node;
}

Attribute *create_attribute_bool(enum Attributes attribute, bool value) {
    Attribute *node = mem_alloc(sizeof(Attribute));
    node->attribute = attribute;
    node->value.bool_value = value;
    return node;
}

Attribute *create_attribute_string(enum Attributes attribute, char *value) {
    Attribute *node = mem_alloc(sizeof(Attribute));
    node->attribute = attribute;
    node->value.string_value = value;
    return node;
}

Attribute *create_attribute_array(enum Attributes attribute, array *value) {
    Attribute *node = mem_alloc(sizeof(Attribute));
    node->attribute = attribute;
    node->value.array_value = value;
    return node;
}

Attribute *create_attribute_range(enum Attributes attribute, struct Range *value) {
    Attribute *node = mem_alloc(sizeof(Attribute));
    node->attribute = attribute;
    node->value.range_value = value;
    return node;
}
