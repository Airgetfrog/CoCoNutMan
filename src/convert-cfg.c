#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/print.h"
#include "cfg.h"
#include "cfg-names.h"

void convert_attributes(void *node, array *attributes) {
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
                ((Field *)node)->separator = attribute->value.string_value;
                break;
            case F_range:
                ((Field *)node)->range = attribute->value.range_value;
                break;
            case F_prefix:
                ((Field *)node)->prefix = attribute->value.string_value;
                break;
            case F_values:
                ((Field *)node)->values = attribute->value.array_value;
                break;
        }
    }
}

int check_attributes(array *attributes) {
    int errors = 0;

    size_t count = array_size(attributes);
    for (size_t i = 0; i < count; i++) {
        Attribute *attribute = (Attribute *)array_get(attributes, i);
        enum Attributes attribute_type = attribute->attribute;

        for (size_t j = i + 1; j < count; j++) {
            Attribute *attribute_j = (Attribute *)array_get(attributes, j);

            if (attribute_type == attribute_j->attribute) {
                print_warning(attribute_j, "Duplicate attribute %s.", Attribute_name(attribute_j->attribute));
                errors++;
                break;
            }
        }
    }

    return errors;
}

int check_and_convert_config(Config *config) {
    int errors = 0;

    errors += check_attributes(config->attributes);
    if (!errors) {
        convert_attributes(config, config->attributes);
    }

    for (size_t i = 0; i < array_size(config->fields); i++) {
        Field *field = (Field *)array_get(config->fields, i);
        if (field->config) {
            errors += check_and_convert_config(field->config);
        } else {
            int local_errors = check_attributes(field->attributes);
            if (!local_errors) {
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

    errors += check_and_convert_config(configuration->config);

    return errors;
}
