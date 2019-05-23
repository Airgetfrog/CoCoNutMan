#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "cfg.h"
#include "cfg-names.h"
#include "lib/array.h"

#define IND "    "
#define IND2 IND IND
#define IND3 IND IND IND
#define IND4 IND IND IND IND
#define IND5 IND IND IND IND IND

void print_inds(int inds) {
    for (size_t i = 0; i < inds; i++) {
        printf(IND);
    }
}

void print_list(array *arr, void (*printfun)(), int inds, char *sep) {
    for (int i = 0; i < array_size(arr) - 1; i++) {
        print_inds(inds);
        (*printfun)(array_get(arr, i), inds);
        printf("%s", sep);
    }
    if (array_size(arr) > 0) {
        print_inds(inds);
        (*printfun)(array_get(arr, array_size(arr) - 1), inds);
    }
}

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
            printf("[");
            print_list(value->value.array_value, &print_field_value, 0, "; ");
            printf("]");
            break;
    }
}

void print_string(char *string) {
    printf("%s", string);
}

void print_setter(Setter *setter) {
    print_list(setter->ids, &print_string, 0, ".");
    printf(" = ");
    if (setter->value) {
        print_field_value(setter->value);
    } else {
        printf("?");
    }
}

void print_enum(Enum *enum_struct) {
    printf("enum %s {\n", enum_struct->id);
    printf(IND "prefix = %s,\n", enum_struct->prefix ? enum_struct->prefix : "<none>");
    printf(IND "values = {\n");
    print_list(enum_struct->values, &print_string, 2, ",\n");
    printf("\n" IND "}\n}\n\n");
}

void print_multioption(MultiOption *multioption) {
    printf("multioption {\n");
    if (multioption->info) {
        printf(IND "info = \"%s\",\n", multioption->info);
    } else {
        printf(IND "info = <none>,\n");
    }
    printf(IND "options = {\n");
    print_list(multioption->options, &print_string, 2, ",\n");
    printf("\n" IND "},\n");
    printf(IND "fields = {\n");
    print_list(multioption->fields, &print_setter, 2, ",\n");
    printf("\n" IND "}\n}\n\n");
}

void print_token(Token *token) {
    printf("%s = {\n", token->token);
    print_list(token->setters, &print_setter, 3, ",\n");
    printf("\n" IND2 "}");
}

void print_optionset(OptionSet *optionset) {
    printf("optionset {\n");
    if (optionset->info) {
        printf(IND "info = \"%s\",\n", optionset->info);
    } else {
        printf(IND "info = <none>,\n");
    }
    printf(IND "options = {\n");
    print_list(optionset->options, &print_string, 2, ",\n");
    printf("\n" IND "},\n");
    printf(IND "separator = \"%s\",\n", optionset->separator ? optionset->separator : "<none>");
    printf(IND "tokens = {\n");
    print_list(optionset->tokens, &print_token, 2, ",\n");
    printf("\n" IND "}\n}\n\n");
}

void print_targetoptions(array *targetoptions) {
    printf("targetoptions = {\n");
    print_list(targetoptions, &print_string, 1, ",\n");
    printf("\n}\n");
}

void print_config(Config *config, int inds);

void print_field(Field *field, int inds) {
    if (field->config) {
        print_config(field->config, inds);
    } else {
        printf("%s%s %s = ", field->enum_id ? field->enum_id : FieldType_name(field->type), field->is_list ? " list" : "", field->id);
        print_field_value(field->default_value);
        printf(" {\n");
        print_inds(inds);
        if (field->info) {
            printf(IND "info = \"%s\"", field->info);
        } else {
            printf(IND "info = <none>");
        }
        if (field->options) {
            printf(",\n");
            print_inds(inds);
            printf(IND "options = {\n");
            print_list(field->options, &print_string, inds + 2, ",\n");
            printf("\n");
            print_inds(inds);
            printf(IND "}");
        }
        if (field->is_argument) {
            printf(",\n");
            print_inds(inds);
            printf(IND "argument");
        } else if (field->configfile) {
            printf(",\n");
            print_inds(inds);
            printf(IND "configfile = true");
        }
        printf("\n");
        print_inds(inds);
        printf("}");
    }
}

void print_config(Config *config, int inds) {
    printf("config %s {\n", config->id);
    print_inds(inds);
    if (config->info) {
        printf(IND "info = \"%s\",\n", config->info);
    } else {
        printf(IND "info = <none>,\n");
    }
    print_inds(inds);
    printf(IND "fields = {\n");
    print_list(config->fields, &print_field, inds + 2, ",\n");
    printf("\n");
    print_inds(inds);
    printf(IND "}\n");
    print_inds(inds);
    printf("}");
}

void print_configuration(Configuration *configuration) {
    for (int i = 0; i < array_size(configuration->enums); i++) {
        print_enum(array_get(configuration->enums, i));
    }

    print_config(configuration->config, 0);
    printf("\n\n");

    for (int i = 0; i < array_size(configuration->multioptions); i++) {
        print_multioption(array_get(configuration->multioptions, i));
    }

    for (int i = 0; i < array_size(configuration->optionsets); i++) {
        print_optionset(array_get(configuration->optionsets, i));
    }

    print_targetoptions(configuration->targetoptions);
}
