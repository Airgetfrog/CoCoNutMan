#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cfg.h"
#include "cfg-names.h"
#include "lib/array.h"

#include "cfg.ccnm.h"

#define out(...) fprintf(fp, __VA_ARGS__)
#define IND "    "

void generate_enum(Enum *enum_struct, FILE *fp) {
    out("typedef enum {\n");
    char *value = (char *)array_get(enum_struct->values, 0);
    out(IND "%s_%s = 0,\n", enum_struct->prefix, value);
    for (size_t i = 1; i < array_size(enum_struct->values) - 1 ; i++) {
        char *value = (char *)array_get(enum_struct->values, i);
        out(IND "%s_%s,\n", enum_struct->prefix, value);
    }
    if (array_size(enum_struct->values) > 0) {
        char *value = (char *)array_get(enum_struct->values, array_size(enum_struct->values) - 1);
        out(IND "%s_%s\n", enum_struct->prefix, value);
    }
    out("} CFG_%s;\n\n", enum_struct->id);
}

void generate_field(Field *field, FILE *fp, int inds) {
    for (size_t i = 0; i < inds; i++) {
        out(IND);
    }
    if (field->enum_id) {
        out("CFG_%s ", field->enum_id);
    } else if (field->type == FT_uint) {
        out("unsigned int ");
    } else if (field->type == FT_string) {
        out("char *");
    } else {
        out("%s ", FieldType_name(field->type));
    }
    if (field->is_list) {
        out("*");
    }
    out("%s;\n", field->id);
    if (field->is_list) {
        for (size_t i = 0; i < inds; i++) {
            out(IND);
        }
        out("size_t %s_length;\n", field->id);
    }
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

    out("#include <stdio.h>\n");
    out("#include <stdbool.h>\n\n");

    out("typedef enum CCNM_RESULT {\n"
        IND "CCNM_SUCCESS = 0,\n"
        IND "CCNM_IO_ERROR,\n"
        IND "CCNM_FILE_ERROR,\n"
        IND "CCNM_CLI_ERROR\n"
        "} CCNM_RESULT;\n\n");

    for (size_t i = 0; i < array_size(configuration->enums); i++) {
        generate_enum(array_get(configuration->enums, i), fp);
    }

    if (configuration->config) {
        generate_config(configuration->config, fp, 0);
        out("\n");
    }

    if (globals.generate_configfile) {
        out("CCNM_RESULT ccnm_parse(int argc, char *argv[], FILE *fp);\n");
    } else {
        out("CCNM_RESULT ccnm_parse(int argc, char *argv[]);\n");
    }
    out("void ccnm_print_usage(void);\n");
    out("void ccnm_free(void);\n\n");

    out("#endif\n");
}
