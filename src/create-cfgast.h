#ifndef _CREATE_AST_H
#define _CREATE_AST_H

#include "lib/array.h"
#include <stdbool.h>
#include <stdint.h>

ConfigSpec *create_config_spec(array *enums, array *groupflags, array *optionsets, array *targetoptions, Config *config);

Enum *create_enum(char *id, array *attributes);

MultiOption *create_multioption(char *info, array *attributes);

Setter *create_setter(array *ids, FieldValue *value);

OptionSet *create_option_set(char *info, array *attributes);

Token *create_token(char *token, array *setters);

Config *create_config(char *id, char *info, array *attributes);

Field *create_field(char *id, char *info, enum FieldType type, bool is_list, FieldValue *default_value, array *attributes);

Field *create_field_config(Config *config);

Range *create_range(bool left_open, bool left_inf, bool right_open, bool right_inf, FieldValue *left_bound, FieldValue *right_bound);

FieldValue *create_field_value_uint(uint64_t value);

FieldValue *create_field_value_int(int64_t value);

FieldValue *create_field_value_float(float value);

FieldValue *create_field_value_string(char *value);

FieldValue *create_field_value_bool(bool value);

FieldValue *create_field_value_array(array *value);

Attribute *create_attribute_bool(enum Attributes attribute, bool value);

Attribute *create_attribute_string(enum Attributes attribute, char *value);

Attribute *create_attribute_array(enum Attributes attribute, array *value);

Attribute *create_attribute_range(enum Attributes attribute, struct Range *value);

#endif
