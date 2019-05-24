/* Does the converting of attribute list to actual struct fields.
 * Does not output errors unless warnings are errors.
 * Also does propagation of configfile bool, as there is no way to check
 *  whether the attribute is user-defined or not from the struct.
 * Warnings occur when an attribute is duplicate (final instance used) or
 *  when a field has an attribute not of its type.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/print.h"
#include "cfg.h"
#include "cfg-names.h"
#include "create-cfg.h"
#include "lib/array.h"
#include "lib/memory.h"
#include "lib/smap.h"

#define BQS "\033[1m‘%s’\033[0m"

void convert_attributes(void *node, array *attributes) {
    Field *field;
    for (size_t i = 0; i < array_size(attributes); i++) {
        Attribute *attribute = (Attribute *)array_get(attributes, i);
        switch (attribute->attribute) {
            case E_prefix:
                ((Enum *)node)->prefix = attribute->value.string_value;
                break;
            case E_values:
                ((Enum *)node)->values = attribute->value.array_value;
                break;
            case MO_options:
                ((MultiOption *)node)->options = attribute->value.array_value;
                break;
            case MO_fields:
                ((MultiOption *)node)->fields = attribute->value.array_value;
                break;
            case OS_options:
                ((OptionSet *)node)->options = attribute->value.array_value;
                break;
            case OS_tokens:
                ((OptionSet *)node)->tokens = attribute->value.array_value;
                break;
            case OS_separator:
                ((OptionSet *)node)->separator = attribute->value.string_value;
                break;
            case C_configfile:
                ((Config *)node)->configfile = attribute->value.bool_value;
                break;
            case C_fields:
                ((Config *)node)->fields = attribute->value.array_value;
                break;
            case F_configfile:
                ((Field *)node)->configfile = attribute->value.bool_value;
                break;
            case F_options:
                ((Field *)node)->options = attribute->value.array_value;
                break;
            case F_argument:
                ((Field *)node)->is_argument = attribute->value.bool_value;
                break;
            case F_separator:
                field = (Field *)node;
                if (!field->is_list) {
                    print_warning(attribute, "field " BQS " has separator but is not a list", field->id);
                } else {
                    field->separator = attribute->value.string_value;
                }
                break;
            case F_range:
                field = (Field *)node;
                if (field->type != FT_uint && field->type != FT_int && field->type != FT_float) {
                    print_warning(attribute, "field " BQS " has range but is of type " BQS, field->id, FieldType_name(field->type));
                } else {
                    field->range = attribute->value.range_value;
                }
                break;
            case F_prefix:
                field = (Field *)node;
                if (field->type != FT_enum || field->enum_id) {
                    print_warning(attribute, "field " BQS " has prefix but is not an enum", field->id);
                } else {
                    field->prefix = attribute->value.string_value;
                }
                break;
            case F_values:
                field = (Field *)node;
                if (field->type != FT_enum || field->enum_id) {
                    print_warning(attribute, "field " BQS " has values but is not an enum", field->id);
                } else {
                    field->values = attribute->value.array_value;
                }
                break;
        }
    }
}

int check_attributes(array *attributes) {
    int errors = 0;

    smap_t *attr_map = smap_init(32);
    for (size_t i = 0; i < array_size(attributes); i++) {
        Attribute *attribute = (Attribute *)array_get(attributes, i);
        enum Attributes attribute_type = attribute->attribute;
        char *aname = Attribute_name(attribute->attribute);
        if (smap_retrieve(attr_map, aname)) {
            print_warning(attribute, "duplicate attribute " BQS, aname);
            print_note(smap_retrieve(attr_map, aname), "previous use of " BQS " was here", aname);
            print_note_no_loc("final instance of duplicate attribute used\n");
        }
        smap_insert(attr_map, aname, attribute);
    }
    smap_free(attr_map);

    return errors;
}

int check_and_convert_config(Config *config, bool configfile) {
    int errors = 0;

    errors += check_attributes(config->attributes);
    config->configfile = configfile;
    if (!errors) {
        convert_attributes(config, config->attributes);
    }

    for (size_t i = 0; i < array_size(config->fields); i++) {
        Field *field = (Field *)array_get(config->fields, i);
        if (field->config) {
            field->id = field->config->id;
            errors += check_and_convert_config(field->config, config->configfile);
        } else {
            int local_errors = check_attributes(field->attributes);
            if (!local_errors) {
                field->configfile = config->configfile;
                convert_attributes(field, field->attributes);
            }
            errors += local_errors;
        }
    }

    return errors;
}

int check_and_convert_attributes(Configuration *configuration) {
    int errors = 0;

    for (size_t i = 0; i < array_size(configuration->enums); i++) {
        Enum *enum_struct = (Enum *)array_get(configuration->enums, i);
        int local_errors = check_attributes(enum_struct->attributes);
        if (!local_errors) {
            convert_attributes(enum_struct, enum_struct->attributes);
        }
        errors += local_errors;
    }

    for (size_t i = 0; i < array_size(configuration->multioptions); i++) {
        MultiOption *multioption = (MultiOption *)array_get(configuration->multioptions, i);
        int local_errors = check_attributes(multioption->attributes);
        if (!local_errors) {
            convert_attributes(multioption, multioption->attributes);
        }
        errors += local_errors;
    }

    for (size_t i = 0; i < array_size(configuration->optionsets); i++) {
        OptionSet *optionset = (OptionSet *)array_get(configuration->optionsets, i);
        int local_errors = check_attributes(optionset->attributes);
        if (!local_errors) {
            convert_attributes(optionset, optionset->attributes);
        }
        errors += local_errors;
    }

    if (configuration->config) {
        errors += check_and_convert_config(configuration->config, false);
    }

    return errors;
}
