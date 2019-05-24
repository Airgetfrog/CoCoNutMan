#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cfg.h"
#include "cfg-names.h"
#include "lib/array.h"

#define out(...) fprintf(fp, __VA_ARGS__)
#define IND "    "

void generate_enum(Enum *enum_struct, FILE *fp) {
    out("typedef enum %s {\n", enum_struct->id);
    for (size_t i = 0; i < array_size(enum_struct->values) - 1 ; i++) {
        char *value = (char *)array_get(enum_struct->values, i);
        out(IND "%s_%s,\n", enum_struct->prefix, value);
    }
    if (array_size(enum_struct->values) > 0) {
        char *value = (char *)array_get(enum_struct->values, array_size(enum_struct->values) - 1);
        out(IND "%s_%s\n", enum_struct->prefix, value);
    }
    out("} %s;\n\n", enum_struct->id);
}

void generate_field(Field *field, FILE *fp, int inds) {
    for (size_t i = 0; i < inds; i++) {
        out(IND);
    }
    out("%s %s;\n", field->enum_id ? field->enum_id : FieldType_name(field->type), field->id);
}

void generate_config(Config *config, FILE *fp, int inds) {
    for (size_t i = 0; i < inds; i++) {
        out(IND);
    }
    out("struct %s {\n", config->id);

    for (size_t i = 0; i < array_size(config->fields); i++) {
        Field *field = (Field *)array_get(config->fields, i);
        if (field->config) {
            generate_config(field->config, fp, inds + 1);
        } else {
            generate_field(field, fp, inds + 1);
        }
    }

    for (size_t i = 0; i < inds; i++) {
        out(IND);
    }
    out("} %s;\n", config->id);
}

void generate_header(Configuration *configuration, FILE *fp) {
    out("#ifndef __CFG_CCNM_H__\n");
    out("#define __CFG_CCNM_H__\n\n");

    out("#include <stdio.h>\n\n");

    for (size_t i = 0; i < array_size(configuration->enums); i++) {
        generate_enum(array_get(configuration->enums, i), fp);
    }

    if (configuration->config) {
        generate_config(configuration->config, fp, 0);
        out("\n");
    }

    out("void ccnm_parse(int argc, char *argv[], FILE *fp);\n\n");

    out("#endif\n");
}
