#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "cfg.h"
#include "lib/array.h"

#define IND "    "
#define IND2 IND IND
#define IND3 IND IND IND
#define IND4 IND IND IND IND
#define IND5 IND IND IND IND IND

void print_field_value(FieldValue *value) {
    switch (value->type) {
        case FT_uint:
            printf("%ld", value->value.uint_value);
            break;
        case FT_int:
            printf("%ld", value->value.int_value);
            break;
        case FT_float:
            printf("%f", value->value.float_value);
            break;
        case FT_enum:
        case FT_string:
            printf("%s", value->value.string_value);
            break;
        case FT_bool:
            printf("%s", value->value.bool_value ? "true" : "false");
            break;
        case FT_list:
            for (size_t i = 0; i < array_size(value->value.array_value); i++) {
                print_field_value(array_get(value->value.array_value, i));
            }
            break;
    }
}

void print_strings(char *inds, array *arr) {
    for (int i = 0; i < array_size(arr); i++) {
        printf("%s %s\n", inds, (char *)array_get(arr, i));
    }
}

void print_setter(Setter *setter) {
    if (array_size(setter->ids) > 0) {
        printf("%s", (char *)array_get(setter->ids, 0));
    }
    for (size_t i = 1; i < array_size(setter->ids); i++) {
        printf(".%s", (char *)array_get(setter->ids, i));
    }
    printf(" = ");
    print_field_value(setter->value);
}

void print_setters(char *inds, array *arr) {
    for (int i = 0; i < array_size(arr); i++) {
        printf("%s", inds);
        print_setter(array_get(arr, i));
        printf(",\n");
    }
}

void print_enum(Enum *enum_struct) {
    printf("enum %s {\n", enum_struct->id);
    printf(IND "prefix: %s,\n", enum_struct->prefix ? enum_struct->prefix : "<none>");
    printf(IND "values = {\n");
    print_strings(IND2, enum_struct->values);
    printf(IND "}\n}\n\n");
}

void print_multioption(MultiOption *multioption) {
    printf("multioption {\n");
    printf(IND "options = {\n");
    print_strings(IND2, multioption->options);
    printf(IND "},\n");
    printf(IND "fields = {\n");
    print_setters(IND2, multioption->fields);
    printf(IND "}\n}\n\n");
}

void print_optionset(OptionSet *optionset) {

}

void print_targetoptions(array *targetoptions) {

}

void print_config(Config *config) {

}

void print_configuration(Configuration *configuration) {
    for (int i = 0; i < array_size(configuration->enums); i++) {
        print_enum(array_get(configuration->enums, i));
    }

    print_config(configuration->config);

    for (int i = 0; i < array_size(configuration->multioptions); i++) {
        print_multioption(array_get(configuration->multioptions, i));
    }

    for (int i = 0; i < array_size(configuration->optionsets); i++) {
        print_optionset(array_get(configuration->optionsets, i));
    }

    print_targetoptions(configuration->targetoptions);
}
