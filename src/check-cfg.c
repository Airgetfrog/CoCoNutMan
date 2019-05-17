#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cfg.h"

int check_enum(Enum *enum_struct) {
    int errors = 0;
    return errors;
}

int check_configuration(Configuration *configuration) {
    int errors = 0;

    for (size_t i = 0; i < array_size(configuration->enums); i++) {
        errors += check_enum(array_get(configuration->enums, i));
    }

    return errors;
}
