#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "cfgast.h"
#include "lib/array.h"

#define IND "    "
#define IND2 IND IND
#define IND3 IND IND IND
#define IND4 IND IND IND IND
#define IND5 IND IND IND IND IND

void print_enum(Enum *enum_struct) {

}

void print_multioption(MultiOption *multioption) {

}

void print_optionset(OptionSet *optionset) {

}

void print_targetoptions(array *targetoptions) {

}

void print_config(Config *config) {

}

void print_configspec(ConfigSpec *configspec) {
    for (int i = 0; i < array_size(configspec->enums); i++) {
        print_enum(array_get(configspec->enums, i));
    }

    for (int i = 0; i < array_size(configspec->multioptions); i++) {
        print_multioption(array_get(configspec->multioptions, i));
    }

    for (int i = 0; i < array_size(configspec->optionsets); i++) {
        print_optionset(array_get(configspec->optionsets, i));
    }

    print_targetoptions(configspec->targetoptions);

    print_config(configspec->config);
}
