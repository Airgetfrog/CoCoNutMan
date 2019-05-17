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
        case F_prefix:
            return "prefix";
        case F_values:
            return "values";
        default:
            return "unnamed attribute";
    }
}
