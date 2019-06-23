#include "cfg.h"

char *Attribute_name(enum Attributes attribute) {
    switch (attribute) {
        case E_prefix:
            return "prefix";
        case E_values:
            return "values";
        case MO_options:
            return "options";
        case MO_fields:
            return "fields";
        case OS_options:
            return "options";
        case OS_tokens:
            return "tokens";
        case OS_separator:
            return "separator";
        case C_configfile:
            return "configfile";
        case C_fields:
            return "fields";
        case F_configfile:
            return "configfile";
        case F_options:
            return "options";
        case F_argument:
            return "argument";
        case F_separator:
            return "separator";
        case F_range:
            return "range";
        default:
            return "unnamed attribute";
    }
}

char *FieldType_name(enum FieldType type) {
    switch (type) {
        case FT_uint:
            return "uint";
        case FT_int:
            return "int";
        case FT_float:
            return "float";
        case FT_string:
            return "string";
        case FT_bool:
            return "bool";
        case FT_enum:
            return "enum";
        case FT_list:
            return "list";
        default:
            return "unknown type";
    }
}
