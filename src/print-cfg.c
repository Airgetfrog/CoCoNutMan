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

void print_array(char *inds, array *arr) {
    for (int i = 0; i < array_size(arr); i++) {
        printf("%s %s,\n", inds, (char *)array_get(arr, i));
    }
}

void print_enum(Enum *enum_struct) {
    printf("enum %s {\n", enum_struct->id);
    printf(IND "prefix: %s\n", enum_struct->prefix ? enum_struct->prefix : "<none>");
    printf(IND "values = {\n");
    print_array(IND2, enum_struct->values);
    printf(IND "}\n");
    printf("}\n");
}

void print_multioption(MultiOption *multioption) {

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

    for (int i = 0; i < array_size(configuration->multioptions); i++) {
        print_multioption(array_get(configuration->multioptions, i));
    }

    for (int i = 0; i < array_size(configuration->optionsets); i++) {
        print_optionset(array_get(configuration->optionsets, i));
    }

    print_targetoptions(configuration->targetoptions);

    print_config(configuration->config);
}
