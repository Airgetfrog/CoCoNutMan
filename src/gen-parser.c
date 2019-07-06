#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>

#include "cfg.h"
#include "cfg-names.h"
#include "lib/array.h"
#include "lib/str.h"

#include "cfg.ccnm.h"

#define out(...) fprintf(fp, __VA_ARGS__)
#define IND "    "
#define IND2 IND IND
#define IND3 IND IND IND
#define IND4 IND IND IND IND
#define IND5 IND IND IND IND IND
#define IND6 IND IND IND IND IND IND
#define NL "\n"

static const char *long_static_literal =
    "int parse_uint_helper(void *dest, array **arr, char *sep, char *string, char *source, const char *name, int argind, bool vl, bool lo, unsigned int l, bool vr, bool ro, unsigned int r) {" NL
    IND "if (sep) {" NL
    IND2 "int errors = 0;" NL
    IND2 "array *strings = ccn_str_split(string, sep[0]);" NL
    IND2 "for (size_t i = 0; i < array_size(strings); i++) {" NL
    IND3 "errors += parse_uint_helper(dest, arr, NULL, array_get(strings, i), source, name, argind, vl, lo, l, vr, ro, r);" NL
    IND2 "}" NL
    IND2 "array_cleanup(strings, &free);" NL
    IND2 "return errors;" NL
    IND "}" NL
    IND "unsigned long ul;" NL
    IND "errno = 0;" NL
    IND "if (errno == ERANGE || (ul = strtoul(string, NULL, 10)) > UINT_MAX) {" NL
    IND2 "printf(\"Value %s could not be converted.\\n\", string);" NL
    IND2 "return 1;" NL
    IND "}" NL
    IND "if ((vl && ((lo && ul < l) || ul <= l)) || (vr && ((ro && ul > r) || ul >= r))) {" NL
    IND2 "if (source) {" NL
    IND3 "printf(\"Invalid value %lu for %s %s.\\n\", ul, source, name);" NL
    IND2 "} else {" NL
    IND3 "printf(\"Invalid value %lu for argument %d.\\n\", ul, argind);" NL
    IND2 "}" NL
    IND2 "return 1;" NL
    IND "}" NL
    IND "if (arr) {" NL
    IND2 "dest = malloc(sizeof(unsigned int));" NL
    IND2 "array_append(*arr, dest);" NL
    IND "}" NL
    IND "*(unsigned int *)dest = ul;" NL
    IND "return 0;" NL
    "}" NL
    NL
    "int parse_int_helper(void *dest, array **arr, char *sep, char *string, char *source, const char *name, int argind, bool vl, bool lo, int l, bool vr, bool ro, int r) {" NL
    IND "if (sep) {" NL
    IND2 "int errors = 0;" NL
    IND2 "array *strings = ccn_str_split(string, sep[0]);" NL
    IND2 "for (size_t i = 0; i < array_size(strings); i++) {" NL
    IND3 "errors += parse_int_helper(dest, arr, NULL, array_get(strings, i), source, name, argind, vl, lo, l, vr, ro, r);" NL
    IND2 "}" NL
    IND2 "array_cleanup(strings, &free);" NL
    IND2 "return errors;" NL
    IND "}" NL
    IND "long sl;" NL
    IND "errno = 0;" NL
    IND "if (errno == ERANGE || (sl = strtol(string, NULL, 10)) > INT_MAX || sl < INT_MIN) {" NL
    IND2 "printf(\"Value %s could not be converted.\\n\", string);" NL
    IND2 "return 1;" NL
    IND "}" NL
    IND "if ((vl && ((lo && sl < l) || sl <= l)) || (vr && ((ro && sl > r) || sl >= r))) {" NL
    IND2 "if (source) {" NL
    IND3 "printf(\"Invalid value %ld for %s %s.\\n\", sl, source, name);" NL
    IND2 "} else {" NL
    IND3 "printf(\"Invalid value %ld for argument %d.\\n\", sl, argind);" NL
    IND2 "}" NL
    IND2 "return 1;" NL
    IND "}" NL
    IND "if (arr) {" NL
    IND2 "dest = malloc(sizeof(int));" NL
    IND2 "array_append(*arr, dest);" NL
    IND "}" NL
    IND "*(int *)dest = sl;" NL
    IND "return 0;" NL
    "}" NL
    NL
    "int parse_float_helper(void *dest, array **arr, char *sep, char *string, char *source, const char *name, int argind, bool vl, bool lo, double l, bool vr, bool ro, double r) {" NL
    IND "if (sep) {" NL
    IND2 "int errors = 0;" NL
    IND2 "array *strings = ccn_str_split(string, sep[0]);" NL
    IND2 "for (size_t i = 0; i < array_size(strings); i++) {" NL
    IND3 "errors += parse_float_helper(dest, arr, NULL, array_get(strings, i), source, name, argind, vl, lo, l, vr, ro, r);" NL
    IND2 "}" NL
    IND2 "array_cleanup(strings, &free);" NL
    IND2 "return errors;" NL
    IND "}" NL
    IND "double d = strtof(string, NULL);" NL
    IND "if ((vl && ((lo && d < l) || d <= l)) || (vr && ((ro && d > r) || d >= r))) {" NL
    IND2 "if (source) {" NL
    IND3 "printf(\"Invalid value %f for %s %s.\\n\", d, source, name);" NL
    IND2 "} else {" NL
    IND3 "printf(\"Invalid value %f for argument %d.\\n\", d, argind);" NL
    IND2 "}" NL
    IND2 "return 1;" NL
    IND "}" NL
    IND "if (arr) {" NL
    IND2 "dest = malloc(sizeof(double));" NL
    IND2 "array_append(*arr, dest);" NL
    IND "}" NL
    IND "*(double *)dest = d;" NL
    IND "return 0;" NL
    "}" NL
    NL
    "int parse_string_helper(void *dest, array **arr, char *sep, char *string) {" NL
    IND "if (sep) {" NL
    IND2 "int errors = 0;" NL
    IND2 "array *strings = ccn_str_split(string, sep[0]);" NL
    IND2 "for (size_t i = 0; i < array_size(strings); i++) {" NL
    IND3 "errors += parse_string_helper(dest, arr, NULL, array_get(strings, i));" NL
    IND2 "}" NL
    IND2 "array_cleanup(strings, &free);" NL
    IND2 "return errors;" NL
    IND "}" NL
    IND "if (arr) {" NL
    IND2 "dest = malloc(sizeof(char *));" NL
    IND2 "array_append(*arr, dest);" NL
    IND2 "*(char **)dest = strdup(string);" NL
    IND "} else {" NL
    IND2 "free(*(char **)dest);" NL
    IND2 "*(char **)dest = strdup(string);" NL
    IND "}" NL
    IND "return 0;" NL
    "}" NL
    NL
    "int parse_bool_helper(void *dest, array **arr, bool val) {" NL
    IND "if (arr) {" NL
    IND2 "dest = malloc(sizeof(bool));" NL
    IND2 "array_append(*arr, dest);" NL
    IND "}" NL
    IND "*(bool *)dest = val;" NL
    IND "return 0;" NL
    "}" NL
    NL
    "int parse_enum_helper(void *dest, array **arr, char *sep, char *string, char *source, const const char *name, int argind, const char *args[], int n) {" NL
    IND "if (sep) {" NL
    IND2 "int errors = 0;" NL
    IND2 "array *strings = ccn_str_split(string, sep[0]);" NL
    IND2 "for (size_t i = 0; i < array_size(strings); i++) {" NL
    IND3 "errors += parse_enum_helper(dest, arr, NULL, array_get(strings, i), source, name, argind, args, n);" NL
    IND2 "}" NL
    IND2 "array_cleanup(strings, &free);" NL
    IND2 "return errors;" NL
    IND "}" NL
    IND "if (arr) {" NL
    IND2 "dest = malloc(sizeof(int));" NL
    IND2 "array_append(*arr, dest);" NL
    IND "}" NL
    IND "for (size_t i = 0; i < n; i++) {" NL
    IND2 "if (ccn_str_equal(string, args[i])) {" NL
    IND3 "*(int *)dest = i;" NL
    IND3 "return 0;" NL
    IND2 "}" NL
    IND "}" NL
    IND "if (source) {" NL
    IND2 "printf(\"Invalid value %s for %s %s.\\n\", string, source, name);" NL
    IND "} else {" NL
    IND2 "printf(\"Invalid value %s for argument %d.\\n\", string, argind);" NL
    IND "}" NL
    IND "return 1;" NL
    "}" NL
    NL
    "unsigned int *new_uint(unsigned int val) {" NL
    IND "unsigned int *ui = malloc(sizeof(unsigned int));" NL
    IND "*ui = val;" NL
    IND "return ui;" NL
    "}" NL
    NL
    "int *new_int(int val) {" NL
    IND "int *i = malloc(sizeof(int));" NL
    IND "*i = val;" NL
    IND "return i;" NL
    "}" NL
    NL
    "double *new_float(double val) {" NL
    IND "double *d = malloc(sizeof(double));" NL
    IND "*d = val;" NL
    IND "return d;" NL
    "}" NL
    NL
    "char **new_string(char *val) {" NL
    IND "char **s = malloc(sizeof(char *));" NL
    IND "*s = val;" NL
    IND "return s;" NL
    "}" NL
    NL
    "bool *new_bool(bool val) {" NL
    IND "bool *b = malloc(sizeof(bool));" NL
    IND "*b = val;" NL
    IND "return b;" NL
    "}" NL
    NL
    ;

static const char *long_configfile_literal =
    "struct file_opt {" NL
    IND "char *str;" NL
    IND "uint8_t type;" NL
    IND "void *loc;" NL
    IND "bool list;" NL
    IND "array **arr;" NL
    IND "bool vl;" NL
    IND "bool lo;" NL
    IND "bool vr;" NL
    IND "bool ro;" NL
    IND "union {" NL
    IND2 "unsigned int ui;" NL
    IND2 "int i;" NL
    IND2 "double d;" NL
    IND "} l;" NL
    IND "union {" NL
    IND2 "unsigned int ui;" NL
    IND2 "int i;" NL
    IND2 "double d;" NL
    IND "} r;" NL
    IND "const char **enum_args;" NL
    IND "size_t enum_size;" NL
    "};" NL
    NL
    "int parse_quoted_string_helper(struct file_opt *fopt, char *tok, char *line, const char *name) {" NL
    IND "if (tok[0] != '\"') {" NL
    IND2 "printf(\"Invalid value %s for setting %s.\\n\", tok, name);" NL
    IND2 "return 1;" NL
    IND "}" NL
    IND "bool escaped = false;" NL
    IND "char *rest = strtok(NULL, \"\");" NL
    IND "if (rest) {" NL
    IND2 "line = ccn_str_cat(tok, rest);" NL
    IND "} else {" NL
    IND2 "line = strdup(tok);" NL
    IND "}" NL
    IND "char *dup = malloc(strlen(line) * sizeof(char));" NL
    IND "size_t put = 0;" NL
    IND "for (size_t i = 1; i < strlen(line); i++) {" NL
    IND2 "if (escaped) {" NL
    IND3 "dup[put++] = line[i];" NL
    IND3 "escaped = false;" NL
    IND3 "continue;" NL
    IND2 "}" NL
    IND2 "if (line[i] == '\\\\') {" NL
    IND3 "escaped = true;" NL
    IND3 "continue;" NL
    IND2 "}" NL
    IND2 "if (line[i] == '\"') {" NL
    IND3 "dup[put] = '\\0';" NL
    IND3 "parse_string_helper(fopt->loc, fopt->arr, NULL, dup);" NL
    IND3 "free(dup);" NL
    IND3 "free(line);" NL
    IND3 "return 0;" NL
    IND2 "}" NL
    IND2 "dup[put++] = line[i];" NL
    IND "}" NL
    IND "printf(\"Invalid value %s for setting %s.\\n\", tok, name);" NL
    IND "free(dup);" NL
    IND "free(line);" NL
    IND "return 1;" NL
    "}" NL
    NL
    "int parse_bool_string_helper(struct file_opt *fopt, char *tok, const char *name) {" NL
    IND "if (ccn_str_equal(tok, \"true\")) {" NL
    IND2 "parse_bool_helper(fopt->loc, fopt->arr, true);" NL
    IND "} else if (ccn_str_equal(tok, \"false\")) {" NL
    IND2 "parse_bool_helper(fopt->loc, fopt->arr, false);" NL
    IND "} else {" NL
    IND2 "printf(\"Invalid value %s for setting %s.\\n\", tok, name);" NL
    IND2 "return 1;" NL
    IND "}" NL
    IND "return 0;" NL
    "}" NL
    NL
    "struct setting {" NL
    IND "struct file_opt *fopt;" NL
    IND "char *tok;" NL
    IND "char *name;" NL
    IND "char *line;" NL
    IND "char *dup;" NL
    "};" NL
    NL
    "struct setting *make_setting(struct file_opt *fopt, char *tok, char *name, char *line, char *dup) {" NL
    IND "struct setting *s = malloc(sizeof(struct setting));" NL
    IND "s->fopt = fopt;" NL
    IND "s->tok = tok;" NL
    IND "s->name = name;" NL
    IND "s->line = line;" NL
    IND "s->dup = dup;" NL
    IND "return s;" NL
    "}" NL
    NL
    "void free_setting(void *v) {" NL
    IND "struct setting *s = (struct setting *)v;" NL
    IND "free(s->tok);" NL
    IND "free(s->name);" NL
    IND "free(s->line);" NL
    IND "free(s->dup);" NL
    IND "free(s);" NL
    "}" NL
    NL
    "struct target {" NL
    IND "array *settings;" NL
    IND "struct target *parent;" NL
    "};" NL
    NL
    "struct target *make_target(struct target *parent) {" NL
    IND "struct target *t = malloc(sizeof(struct target));" NL
    IND "t->settings = create_array();" NL
    IND "t->parent = parent;" NL
    IND "return t;" NL
    "}" NL
    NL
    "static smap_t *target_map;" NL
    NL
    "int apply_target(struct target *t) {" NL
    IND "int errors = 0;" NL
    IND "if (t->parent) {" NL
    IND2 "errors += apply_target(t->parent);" NL
    IND "}" NL
    IND "for (size_t i = 0; i < array_size(t->settings); i++) {" NL
    IND2 "struct setting *s = array_get(t->settings, i);" NL
    IND2 "struct file_opt *option = s->fopt;" NL
    IND2 "char *tok = s->tok;" NL
    IND2 "char *name = s->name;" NL
    IND2 "char *line = s->line;" NL
    IND2 "char *dup = s->line;" NL
    IND2 "switch (option->type) {" NL
    IND3 "case 0:" NL
    IND4 "errors += parse_uint_helper(option->loc, option->arr, NULL, tok, \"setting\", name, 0, option->vl, option->lo, option->l.ui, option->vr, option->ro, option->r.ui);" NL
    IND4 "break;" NL
    IND3 "case 1:" NL
    IND4 "errors += parse_int_helper(option->loc, option->arr, NULL, tok, \"setting\", name, 0, option->vl, option->lo, option->l.i, option->vr, option->ro, option->r.i);" NL
    IND4 "break;" NL
    IND3 "case 2:" NL
    IND4 "errors += parse_int_helper(option->loc, option->arr, NULL, tok, \"setting\", name, 0, option->vl, option->lo, option->l.d, option->vr, option->ro, option->r.d);" NL
    IND4 "break;" NL
    IND3 "case 3:" NL
    IND4 "errors += parse_quoted_string_helper(option, dup, line, name);" NL
    IND4 "break;" NL
    IND3 "case 4:" NL
    IND4 "errors += parse_bool_string_helper(option, tok, name);" NL
    IND4 "break;" NL
    IND3 "case 5:" NL
    IND4 "errors += parse_enum_helper(option->loc, option->arr, NULL, tok, \"setting\", name, 0, option->enum_args, option->enum_size);" NL
    IND4 "break;" NL
    IND2 "}" NL
    IND "}" NL
    IND "return errors;" NL
    "}" NL
    NL
    "int parse_target(char *string) {" NL
    IND "int errors = 0;" NL
    IND "struct target *t;" NL
    IND "if (!(t = smap_retrieve(target_map, string))) {" NL
    IND2 "printf(\"Target %s not found.\\n\", string);" NL
    IND2 "return 1;" NL
    IND "}" NL
    IND "errors += apply_target(t);" NL
    IND "return errors;" NL
    "}" NL
    NL
    "void free_target(void *t) {" NL
    IND "array_cleanup(((struct target *)t)->settings, &free_setting);" NL
    IND "free(t);" NL
    "}" NL
    NL
    ;

void generate_relative_path(char *src, char *dst, FILE *fp) {
    array *src_parts = ccn_str_split(src, '/');
    array *dst_parts = ccn_str_split(dst, '/');
    size_t src_size = array_size(src_parts);
    size_t dst_size = array_size(dst_parts);

    for (size_t i = 0; i < src_size; i++) {
        if (!ccn_str_equal(array_get(src_parts, i), array_get(dst_parts, i))) {
            for (size_t j = i; j < src_size - 1; j++) {
                out("../");
            }
            for (size_t j = i; j < dst_size - 1; j++) {
                out("%s/", (char *)array_get(dst_parts, j));
            }
            if (i < dst_size) {
                out("%s", (char *)array_get(dst_parts, dst_size - 1));
            }
            break;
        }
    }

    array_cleanup(src_parts, &free);
    array_cleanup(dst_parts, &free);
}

void generate_includes(FILE *fp) {
    out("#include <stdlib.h>" NL
        "#include <stdio.h>" NL
        "#include <stdbool.h>" NL
        "#include <getopt.h>" NL
        "#include <string.h>" NL
        "#include <limits.h>" NL
        "#include <inttypes.h>" NL
        "#include <errno.h>" NL
        NL
        "#include \"lib/str.h\"" NL
        "#include \"lib/smap.h\"" NL
        NL
    );

    out("#include \"");
    generate_relative_path(realpath(globals.outfile, NULL), realpath(globals.headerfile, NULL), fp);
    out("\"" NL NL);
}

void generate_enum_args(Enum *enum_struct, FILE *fp) {
    out("static const char *%s_args[] = {", enum_struct->id);
    for (size_t i = 0; i < array_size(enum_struct->values) - 1; i++) {
        out("\"%s\", ", (char *)array_get(enum_struct->values, i));
    }
    if (array_size(enum_struct->values)) {
        out("\"%s\"", (char *)array_get(enum_struct->values, array_size(enum_struct->values) - 1));
    }
    out("};\n");
}

void generate_enum_args_helper(Configuration *configuration, FILE *fp) {
    for (size_t i = 0; i < array_size(configuration->enums); i++) {
        generate_enum_args(array_get(configuration->enums, i), fp);
    }
    out(NL);
}

void generate_internal_arrays_struct(Config *config, FILE *fp, char *inds) {
    out("%s", inds);
    out("struct {" NL);
    for (size_t i = 0; i < array_size(config->fields); i++) {
        Field *field = (Field *)array_get(config->fields, i);
        if (field->config) {
            generate_internal_arrays_struct(field->config, fp, ccn_str_cat(inds, IND));
            continue;
        }
        if (field->is_list) {
            out("%s", inds);
            out(IND "array *%s;" NL, field->id);
        }
    }
    out("%s", inds);
    out("} %s;" NL, config->id);
}

void generate_internal_arrays_struct_helper(Configuration *configuration, FILE *fp) {
    out("static struct {" NL);
    generate_internal_arrays_struct(configuration->config, fp, IND);
    out("} internal_arrays;" NL NL);
}

void generate_static_helpers(FILE *fp) {
    out("%s", long_static_literal);
    if (globals.generate_configfile) {
        out("%s", long_configfile_literal);
    }
}

void generate_field_value(FieldValue *value, FILE *fp) {
    switch (value->type) {
        case FT_uint:
            out("%" PRIu64, value->value.uint_value);
            break;
        case FT_int:
            out("%" PRId64, value->value.int_value);
            break;
        case FT_float:
            out("%f", value->value.float_value);
            break;
        case FT_string:
            out("strdup(\"%s\")", value->value.string_value);
            break;
        case FT_bool:
            out("%s", value->value.bool_value ? "true" : "false");
            break;
        case FT_enum:
            out("%s", value->value.string_value);
            break;
    }
}

void generate_defaults(Config *config, FILE *fp, char *prefix) {
    prefix = ccn_str_cat(ccn_str_cat(prefix, config->id), ".");
    for (size_t i = 0; i < array_size(config->fields); i++) {
        Field *field = (Field *)array_get(config->fields, i);
        if (field->config) {
            generate_defaults(field->config, fp, prefix);
            continue;
        }
        if (!field->is_argument && !field->is_list) {
            out(IND "%s%s = ", prefix, field->id);
            generate_field_value(field->default_value, fp);
            out(";\n");
        }
    }
}

void generate_initialize_defaults(Configuration *configuration, FILE *fp) {
    out("void initialize_defaults(void) {\n");
    generate_defaults(configuration->config, fp, "");
    out("}\n\n");
}

void generate_new_function(FieldType type, FILE *fp, FieldValue *value) {
    out("new_");
    switch (type) {
        case FT_uint:
            out("uint");
            break;
        case FT_int:
            out("int");
            break;
        case FT_float:
            out("float");
            break;
        case FT_string:
            out("string");
            break;
        case FT_bool:
            out("bool");
            break;
        case FT_enum:
            out("int");
            break;
    }
    out("(");
    generate_field_value(value, fp);
    out(")");
}

void generate_array_initializers(Config *config, FILE *fp, char *prefix) {
    prefix = ccn_str_cat(ccn_str_cat(prefix, config->id), ".");
    for (size_t i = 0; i < array_size(config->fields); i++) {
        Field *field = (Field *)array_get(config->fields, i);
        if (field->config) {
            generate_array_initializers(field->config, fp, prefix);
            continue;
        }
        if (!field->is_list) {
            continue;
        }
        out(IND "internal_arrays.%s%s = create_array();" NL, prefix, field->id);
        array *arr = field->default_value->value.array_value;
        for (size_t j = 0; j < array_size(arr); j++) {
            out(IND "array_append(internal_arrays.%s%s, ", prefix, field->id);
            generate_new_function(field->type, fp, array_get(arr, j));
            out(");" NL);
        }
    }
}

void generate_initialize_arrays(Configuration *configuration, FILE *fp) {
    out("void initialize_arrays(void) {\n");
    generate_array_initializers(configuration->config, fp, "");
    out("}\n\n");
}

size_t generate_file_opts(Config *config, FILE *fp, char *prefix, char *prefix_no_root, bool root) {
    size_t count = 0;
    if (!root) {
        prefix_no_root = ccn_str_cat(ccn_str_cat(prefix_no_root, config->id), ".");
    }
    prefix = ccn_str_cat(ccn_str_cat(prefix, config->id), ".");
    for (size_t i = 0; i < array_size(config->fields); i++) {
        Field *field = (Field *)array_get(config->fields, i);
        if (field->config) {
            count += generate_file_opts(field->config, fp, prefix, prefix_no_root, false);
            continue;
        }
        if (!field->configfile) {
            continue;
        }

        count++;

        out(IND3 "{\"%s%s\", %d, ",
            prefix_no_root, field->id,
            field->type
        );

        if (field->is_list) {
            out("0, 1, &internal_arrays.%s%s, ", prefix, field->id);
        } else {
            out("&%s%s, 0, 0, ", prefix, field->id);
        }

        if (field->range) {
            out("%d, %d, %d, %d, ",
                field->range->left_bound ? 1 : 0,
                field->range->left_open ? 1 : 0,
                field->range->right_bound ? 1 : 0,
                field->range->right_open ? 1 : 0
            );
            FieldValue *l = field->range->left_bound;
            if (l) {
                switch (field->type) {
                    case FT_uint:
                        out(".l={.ui=%" PRIu64 "}, ", l->value.uint_value);
                        break;
                    case FT_int:
                        out(".l={.i=%" PRId64 "}, ", l->value.int_value);
                        break;
                    case FT_float:
                        out(".l={.d=%f}, ", l->value.float_value);
                        break;
                }
            } else {
                out("0, ");
            }
            FieldValue *r = field->range->right_bound;
            if (r) {
                switch (field->type) {
                    case FT_uint:
                        out(".r={.ui=%" PRIu64 "}, ", r->value.uint_value);
                        break;
                    case FT_int:
                        out(".r={.i=%" PRId64 "}, ", r->value.int_value);
                        break;
                    case FT_float:
                        out(".r={.d=%f}, ", r->value.float_value);
                        break;
                }
            } else {
                out("0, ");
            }
        } else {
            out("0, 0, 0, 0, 0, 0, ");
        }

        if (field->enum_struct) {
            out("%s_args, %ld}," NL, field->enum_struct->id, array_size(field->enum_struct->values));
        } else {
            out("0, 0}," NL);
        }
    }

    return count;
}

void generate_parse_configuration_file(Configuration *configuration, FILE *fp) {
    out("int parse_configuration_file(FILE *fp) {" NL
        IND "int errors = 0;" NL
        IND "static struct file_opt options[] =" NL
        IND IND "{" NL);

    size_t count = generate_file_opts(configuration->config, fp, "", "", true);

    out(IND2 "};" NL
        IND "char *line = NULL;" NL
        IND "size_t len = 0;" NL
        IND "char *tok;" NL
        IND "size_t i;" NL
        IND "size_t linenr = 0;" NL
        IND "char *name;" NL
        IND "struct target *current_target = NULL;" NL
        IND "char *current_target_name = NULL;" NL
        IND "while (getline(&line, &len, fp) != -1) {"NL
        IND2 "if ((name = tok = strtok(line, \" \\t\\n\"))) {" NL
        IND3 "if (ccn_str_equal(tok, \"//\")) {" NL
        IND4 "continue;" NL
        IND3 "}" NL
        IND3 "if (ccn_str_equal(tok, \"target\")) {" NL
        IND4 "if (!(current_target_name = tok = strtok(NULL, \" \\t\\n\"))) {" NL
        IND5 "printf(\"Unexpected end of line on line %%zu.\\n\", linenr);" NL
        IND5 "continue;" NL
        IND4 "}" NL
        IND4 "if (!(tok = strtok(NULL, \" \\t\\n\"))) {" NL
        IND5 "current_target = make_target(NULL); smap_insert(target_map, current_target_name, current_target);" NL
        IND5 "continue;" NL
        IND4 "}" NL
        IND4 "if (!ccn_str_equal(tok, \"extends\")) {" NL
        IND5 "printf(\"Unrecognized sequence on line %%zu.\\n\", linenr);" NL
        IND5 "continue;" NL
        IND4 "}" NL
        IND4 "if (!(tok = strtok(NULL, \" \\t\\n\"))) {" NL
        IND5 "printf(\"Unexpected end of line on line %%zu.\\n\", linenr);" NL
        IND5 "continue;" NL
        IND4 "}" NL
        IND4 "if (!smap_retrieve(target_map, tok)) {" NL
        IND5 "printf(\"Target %%s not found\\n\", tok);" NL
        IND5 "continue;" NL
        IND4 "}" NL
        IND4 "current_target = make_target(smap_retrieve(target_map, tok));" NL
        IND4 "smap_insert(target_map, current_target_name, current_target);" NL
        IND4 "continue;" NL
        IND3 "}" NL
        IND3 "for (i = 0; i < %zu; i++) {" NL
        IND4 "if (ccn_str_equal(tok, options[i].str)) {" NL
        IND5 "break;" NL
        IND4 "}" NL
        IND3 "}" NL
        IND3 "if (i >= %zu) {" NL
        IND4 "printf(\"Unrecognized option %%s on line %%zu.\\n\", tok, linenr);" NL
        IND4 "continue;" NL
        IND3 "}" NL
        IND3 "if (!(tok = strtok(NULL, \" \\t\\n\"))) {" NL
        IND4 "printf(\"Unexpected end of line on line %%zu.\\n\", linenr);" NL
        IND4 "continue;" NL
        IND3 "}" NL
        IND3 "if ((!options[i].list && !ccn_str_equal(tok, \"=\")) || (options[i].list && !ccn_str_equal(tok, \"+=\"))) {" NL
        IND4 "printf(\"Unrecognized sequence on line %%zu.\\n\", linenr);" NL
        IND4 "continue;" NL
        IND3 "}" NL
        IND3 "char *dup = strdup(tok);" NL
        IND3 "if (!(tok = strtok(NULL, \" \\t\\n\"))) {" NL
        IND4 "printf(\"Unexpected end of line on line %%zu.\\n\", linenr);" NL
        IND4 "free(dup);" NL
        IND4 "continue;" NL
        IND3 "}" NL
        IND3 "if (current_target) {" NL
        IND4 "array_append(current_target->settings, make_setting(&options[i], strdup(tok), strdup(name), strdup(line), strdup(dup)));" NL
        IND4 "free(dup);" NL
        IND4 "continue;" NL
        IND3 "}" NL
        IND3 "switch (options[i].type) {" NL
        IND4 "case 0:" NL
        IND5 "errors += parse_uint_helper(options[i].loc, options[i].arr, NULL, tok, \"setting\", name, 0, options[i].vl, options[i].lo, options[i].l.ui, options[i].vr, options[i].ro, options[i].r.ui);" NL
        IND5 "break;" NL
        IND4 "case 1:" NL
        IND5 "errors += parse_int_helper(options[i].loc, options[i].arr, NULL, tok, \"setting\", name, 0, options[i].vl, options[i].lo, options[i].l.i, options[i].vr, options[i].ro, options[i].r.i);" NL
        IND5 "break;" NL
        IND4 "case 2:" NL
        IND5 "errors += parse_int_helper(options[i].loc, options[i].arr, NULL, tok, \"setting\", name, 0, options[i].vl, options[i].lo, options[i].l.d, options[i].vr, options[i].ro, options[i].r.d);" NL
        IND5 "break;" NL
        IND4 "case 3:" NL
        IND5 "errors += parse_quoted_string_helper(&options[i], dup, line, name);" NL
        IND5 "break;" NL
        IND4 "case 4:" NL
        IND5 "errors += parse_bool_string_helper(&options[i], tok, name);" NL
        IND5 "break;" NL
        IND4 "case 5:" NL
        IND5 "errors += parse_enum_helper(options[i].loc, options[i].arr, NULL, tok, \"setting\", name, 0, options[i].enum_args, options[i].enum_size);" NL
        IND5 "break;" NL
        IND3 "}" NL
        IND3 "free(dup);" NL
        IND2 "}" NL
        IND2 "linenr++;" NL
        IND "}" NL
        IND "free(line);" NL
        IND "return errors;" NL
        "}" NL NL,
        count, count
    );
}

char *generate_long_options(Config *config, FILE *fp, int *count) {
    char *short_opts = "";

    for (size_t i = 0; i < array_size(config->fields); i++) {
        Field *field = (Field *)array_get(config->fields, i);
        if (field->config) {
            short_opts = ccn_str_cat(short_opts, generate_long_options(field->config, fp, count));
            continue;
        }
        if (field->is_argument) {
            continue;
        }

        bool created_long = false;

        for (size_t j = 0; j < array_size(field->options); j++) {
            char *option = array_get(field->options, j);
            if (strlen(option) == 1) {
                short_opts = ccn_str_cat(short_opts, option);
                if (field->type != FT_bool) {
                    short_opts = ccn_str_cat(short_opts, ":");
                }
            } else {
                out(IND IND IND "{\"%s\", %s_argument, 0, %d},\n", option, field->type == FT_bool ? "no" : "required", *count);
                created_long = true;
            }
        }
        if (created_long) {
            (*count)++;
        }
        if (field->type == FT_bool) {
            created_long = false;
            for (size_t j = 0; j < array_size(field->disable_options); j++) {
                char *option = array_get(field->disable_options, j);
                if (strlen(option) == 1) {
                    short_opts = ccn_str_cat(ccn_str_cat(short_opts, option), ":");
                } else {
                    out(IND IND IND "{\"%s\", %s_argument, 0, %d},\n", option, field->type == FT_bool ? "no" : "required", *count);
                    created_long = true;
                }
            }
            if (created_long) {
                (*count)++;
            }
        }
    }

    return short_opts;
}

char *generate_long_options_mo(array *multioptions, FILE *fp, int *count) {
    char *short_opts = "";

    for (size_t i = 0; i < array_size(multioptions); i++) {
        MultiOption *multioption = (MultiOption *)array_get(multioptions, i);
        bool created_long = false;
        for (size_t j = 0; j < array_size(multioption->options); j++) {
            char *option = (char *)array_get(multioption->options, j);
            if (strlen(option) == 1) {
                short_opts = ccn_str_cat(short_opts, option);
                if (multioption->takes_argument) {
                    short_opts = ccn_str_cat(short_opts, ":");
                }
            } else {
                if (multioption->takes_argument) {
                    out(IND IND IND "{\"%s\", required_argument, 0, %d},\n", option, *count);
                } else {
                    out(IND IND IND "{\"%s\", no_argument, 0, %d},\n", option, *count);
                }
                created_long = true;
            }
        }
        if (created_long) {
            (*count)++;
        }
    }

    return short_opts;
}

char *generate_long_options_os(array *optionsets, FILE *fp, int *count) {
    char *short_opts = "";

    for (size_t i = 0; i < array_size(optionsets); i++) {
        OptionSet *optionset = (OptionSet *)array_get(optionsets, i);
        bool created_long = false;
        for (size_t j = 0; j < array_size(optionset->options); j++) {
            char *option = (char *)array_get(optionset->options, j);
            if (strlen(option) == 1) {
                short_opts = ccn_str_cat(short_opts, option);
            } else {
                out(IND IND IND "{\"%s\", required_argument, 0, %d},\n", option, *count);
                created_long = true;
            }
        }
        if (created_long) {
            (*count)++;
        }
    }

    return short_opts;
}

char *generate_long_options_to(array *targetoptions, FILE *fp, int *count) {
    char *short_opts = "";

    for (size_t i = 0; i < array_size(targetoptions); i++) {
        char *option = (char *)array_get(targetoptions, i);
        if (strlen(option) == 1) {
            short_opts = ccn_str_cat(short_opts, option);
            short_opts = ccn_str_cat(short_opts, ":");
        } else {
            out(IND IND IND "{\"%s\", required_argument, 0, %d},\n", option, *count);
            (*count)++;
        }
    }

    return short_opts;
}

void generate_range_numbers(Field *field, FILE *fp) {
    if (field->range) {
        out("%d, %d, ",
            field->range->left_bound ? 1 : 0,
            field->range->left_open ? 1 : 0
        );
        FieldValue *l = field->range->left_bound;
        if (l) {
            switch (field->type) {
                case FT_uint:
                    out("%" PRIu64 ", ", l->value.uint_value);
                    break;
                case FT_int:
                    out("%" PRId64 ", ", l->value.int_value);
                    break;
                case FT_float:
                    out("%f, ", l->value.float_value);
                    break;
            }
        } else {
            out("0, ");
        }
        out("%d, %d, ",
            field->range->right_bound ? 1 : 0,
            field->range->right_open ? 1 : 0
        );
        FieldValue *r = field->range->right_bound;
        if (r) {
            switch (field->type) {
                case FT_uint:
                    out("%" PRIu64, r->value.uint_value);
                    break;
                case FT_int:
                    out("%" PRId64, r->value.int_value);
                    break;
                case FT_float:
                    out("%f", r->value.float_value);
                    break;
            }
        } else {
            out("0");
        }
    } else {
        out("0, 0, 0, 0, 0, 0");
    }
}

void generate_field_parser(Field *field, FILE *fp, char *prefix, char *bool_val, char *string, char *source, char *name, char *argind, char *inds) {
    out("%s", inds);
    out("errors += ");
    switch (field->type) {
        case FT_uint:
            if (field->is_list) {
                out("parse_uint_helper(0, &internal_arrays.%s%s, ", prefix, field->id);
                if (field->separator) {
                    out("\"%s\", ", field->separator);
                } else {
                    out("0, ");
                }
            } else {
                out("parse_uint_helper(&%s%s, 0, 0, ", prefix, field->id);
            }
            out("%s, %s, %s, %s, ", string, source, name, argind);
            generate_range_numbers(field, fp);
            out(");" NL);
            break;
        case FT_int:
            if (field->is_list) {
                out("parse_int_helper(0, &internal_arrays.%s%s, ", prefix, field->id);
                if (field->separator) {
                    out("\"%s\", ", field->separator);
                } else {
                    out("0, ");
                }
            } else {
                out("parse_int_helper(&%s%s, 0, 0, ", prefix, field->id);
            }
            out("%s, %s, %s, %s, ", string, source, name, argind);
            generate_range_numbers(field, fp);
            out(");" NL);
            break;
        case FT_float:
            if (field->is_list) {
                out("parse_float_helper(0, &internal_arrays.%s%s, ", prefix, field->id);
                if (field->separator) {
                    out("\"%s\", ", field->separator);
                } else {
                    out("0, ");
                }
            } else {
                out("parse_float_helper(&%s%s, 0, 0, ", prefix, field->id);
            }
            out("%s, %s, %s, %s, ", string, source, name, argind);
            generate_range_numbers(field, fp);
            out(");" NL);
            break;
        case FT_string:
            if (field->is_list) {
                out("parse_string_helper(0, &internal_arrays.%s%s, ", prefix, field->id);
                if (field->separator) {
                    out("\"%s\", ", field->separator);
                } else {
                    out("0, ");
                }
            } else {
                out("parse_string_helper(&%s%s, 0, 0, ", prefix, field->id);
            }
            out("%s);" NL, string);
            break;
        case FT_bool:
            out("parse_bool_helper(&%s%s, 0, %s);" NL, prefix, field->id, bool_val);
            break;
        case FT_enum:
            if (field->is_list) {
                out("parse_enum_helper(0, &internal_arrays.%s%s, ", prefix, field->id);
                if (field->separator) {
                    out("\"%s\", ", field->separator);
                } else {
                    out("0, ");
                }
            } else {
                out("parse_enum_helper(&%s%s, 0, 0, ", prefix, field->id);
            }
            out("%s, %s, %s, %s, %s_args, %ld);" NL,
                string,
                source,
                name,
                argind,
                field->enum_id,
                array_size(field->enum_struct->values)
            );
            break;
    }
}

void generate_getopt_cases(Config *config, FILE *fp, int *count, char *prefix) {
    prefix = ccn_str_cat(ccn_str_cat(prefix, config->id), ".");
    for (size_t i = 0; i < array_size(config->fields); i++) {
        Field *field = (Field *)array_get(config->fields, i);
        if (field->config) {
            generate_getopt_cases(field->config, fp, count, prefix);
            continue;
        }
        if (field->is_argument) {
            continue;
        }

        bool create_long = false;

        for (size_t j = 0; j < array_size(field->options); j++) {
            char *option = array_get(field->options, j);
            if (strlen(option) == 1) {
                out(IND IND IND "case '%c':\n", option[0]);
                generate_field_parser(field, fp, prefix, "true", "optarg", "\"option\"", option, "0", IND4);
                out(IND IND IND IND "break;\n");
            } else {
                create_long = true;
            }
        }
        if (create_long) {
            out(IND IND IND "case %d:\n", *count);
            generate_field_parser(field, fp, prefix, "true", "optarg", "\"option\"", "long_options[long_index].name", "0", IND4);
            out(IND IND IND IND "break;\n");
            (*count)++;
        }
        if (field->type == FT_bool) {
            create_long = false;
            for (size_t j = 0; j < array_size(field->disable_options); j++) {
                char *option = array_get(field->disable_options, j);
                if (strlen(option) == 1) {
                    out(IND IND IND "case '%c':\n", option[0]);
                    generate_field_parser(field, fp, prefix, "false", "optarg", "\"option\"", option, "0", IND4);
                    out(IND IND IND IND "break;\n");
                } else {
                    create_long = true;
                }
            }
            if (create_long) {
                out(IND IND IND "case %d:\n", *count);
                generate_field_parser(field, fp, prefix, "false", "optarg", "\"option\"", "long_options[long_index].name", "0", IND4);
                out(IND IND IND IND "break;\n");
                (*count)++;
            }
        }
    }
}

char *generate_prefix(array *ids, FILE *fp, char *root_id) {
    char *prefix = ccn_str_cat(root_id, ".");
    for (size_t i = 0; i < array_size(ids) - 1; i++) {
        prefix = ccn_str_cat(prefix, array_get(ids, i));
        prefix = ccn_str_cat(prefix, ".");
    }
    return prefix;
}

void generate_setters(array *setters, FILE *fp, char *root_id, char *inds, char *shortopt) {
    for (size_t i = 0; i < array_size(setters); i++) {
        Setter *setter = (Setter *)array_get(setters, i);
        if (setter->value) {
            if (setter->field->is_list) {
                array *arr = setter->value->value.array_value;
                for (size_t j = 0; j < array_size(arr); j++) {
                    out("%s", inds);
                    out("array_append(internal_arrays.%s%s, ", generate_prefix(setter->ids, fp, root_id), setter->field->id);
                    generate_new_function(setter->field->type, fp, array_get(arr, j));
                    out(");" NL);
                }
            } else {
                out("%s", inds);
                out("%s%s = ", generate_prefix(setter->ids, fp, root_id), setter->field->id);
                generate_field_value(setter->value, fp);
                out(";" NL);
            }
        } else {
            if (shortopt) {
                generate_field_parser(setter->field, fp, generate_prefix(setter->ids, fp, root_id), NULL, "optarg", "\"option\"", ccn_str_cat(ccn_str_cat("'", shortopt), "'"), "0", inds);
            } else {
                generate_field_parser(setter->field, fp, generate_prefix(setter->ids, fp, root_id), NULL, "optarg", "\"option\"", "long_options[long_index].name", "0", inds);
            }
        }
    }
}

void generate_getopt_cases_mo(array *multioptions, FILE *fp, int *count, char *root_id) {
    for (size_t i = 0; i < array_size(multioptions); i++) {
        MultiOption *multioption = (MultiOption *)array_get(multioptions, i);

        bool create_long = false;

        for (size_t j = 0; j < array_size(multioption->options); j++) {
            char *option = array_get(multioption->options, j);
            if (strlen(option) == 1) {
                out(IND IND IND "case '%c':\n", option[0]);
                generate_setters(multioption->fields, fp, root_id, IND4, option);
                out(IND IND IND IND "break;\n");
            } else {
                create_long = true;
            }
        }

        if (create_long) {
            out(IND IND IND "case %d:\n", *count);
            generate_setters(multioption->fields, fp, root_id, IND4, NULL);
            out(IND IND IND IND "break;\n");
            (*count)++;
        }
    }
}

void generate_optionset_scanner(OptionSet *optionset, FILE *fp, char *root_id, char *shortopt) {
    if (optionset->separator) {
        out(IND4 "strings = ccn_str_split(optarg, '%c');" NL, optionset->separator[0]);
        out(IND4 "for (size_t i = 0; i < array_size(strings); i++) {" NL);
        for (size_t i = 0; i < array_size(optionset->tokens); i++) {
            Token *token = (Token *)array_get(optionset->tokens, i);
            out(IND5 "if (ccn_str_equal(\"%s\", array_get(strings, i))) {" NL, token->token);
            generate_setters(token->setters, fp, root_id, IND6, shortopt);
            out(IND6 "continue;" NL
                IND5 "}" NL
            );
        }
        if (shortopt) {
            out(IND5 "printf(\"%%s is not a valid value for option %s\\n\", (char *)array_get(strings, i), argv[if (source]);" NL
                IND5 "errors++;" NL
                IND4 "}" NL
                IND4 "array_cleanup(strings, &free);" NL,
                shortopt
            );
        } else {
            out(IND5 "printf(\"%%s is not a valid value for option %%s\\n\", (char *)array_get(strings, i), long_options[long_index].name);" NL
                IND5 "errors++;" NL
                IND4 "}" NL
                IND4 "array_cleanup(strings, &free);" NL
            );
        }
    } else {
        out(IND4 "for (size_t i = 0; i < strlen(optarg); i++) {" NL);
        for (size_t i = 0; i < array_size(optionset->tokens); i++) {
            Token *token = (Token *)array_get(optionset->tokens, i);
            out(IND5 "if ('%c' == optarg[i]) {" NL, token->token[0]);
            generate_setters(token->setters, fp, root_id, IND6, shortopt);
            out(IND6 "continue;" NL
                IND5 "}" NL
            );
        }
        if (shortopt) {
            out(IND5 "printf(\"%%c is not a valid value for option %s\\n\", optarg[i]);" NL
                IND5 "errors++;" NL
                IND4 "}" NL,
                shortopt
            );
        } else {
            out(IND5 "printf(\"%%c is not a valid value for option %%s\\n\", optarg[i], long_options[long_index].name);" NL
                IND5 "errors++;" NL
                IND4 "}" NL
            );
        }
    }
}

void generate_getopt_cases_os(array *optionsets, FILE *fp, int *count, char *root_id) {
    for (size_t i = 0; i < array_size(optionsets); i++) {
        OptionSet *optionset = (OptionSet *)array_get(optionsets, i);

        bool create_long = false;

        for (size_t j = 0; j < array_size(optionset->options); j++) {
            char *option = array_get(optionset->options, j);
            if (strlen(option) == 1) {
                out(IND IND IND "case '%c':\n", option[0]);
                generate_optionset_scanner(optionset, fp, root_id, option);
                out(IND IND IND IND "break;\n");
            } else {
                create_long = true;
            }
        }

        if (create_long) {
            out(IND IND IND "case %d:\n", *count);
            generate_optionset_scanner(optionset, fp, root_id, NULL);
            out(IND IND IND IND "break;\n");
            (*count)++;
        }
    }
}

void generate_getopt_cases_to(array *targetoptions, FILE *fp, int *count) {
    for (size_t i = 0; i < array_size(targetoptions); i++) {
        char *option = (char *)array_get(targetoptions, i);
        if (strlen(option) == 1) {
            out(IND3 "case '%c':" NL
                IND4 "errors += parse_target(optarg);" NL
                IND4 "break;" NL,
                option[0]
            );
        } else {
            out(IND3 "case %d:" NL
                IND4 "errors += parse_target(optarg);" NL
                IND4 "break;" NL,
                *count
            );
            (*count)++;
        }
    }
}

void generate_arguments_loop(Config *config, FILE *fp, char *prefix) {
    prefix = ccn_str_cat(ccn_str_cat(prefix, config->id), ".");
    for (size_t i = 0; i < array_size(config->fields); i++) {
        Field *field = (Field *)array_get(config->fields, i);
        if (field->config) {
            generate_arguments_loop(field->config, fp, prefix);
            continue;
        }
        if (!field->is_argument) {
            continue;
        }
        if (field->is_list) {
            out(IND "while (optind < argc) {\n");
            generate_field_parser(field, fp, prefix, NULL, "argv[optind++]", "\"argument\"", "NULL", "optind", IND2);
            out(IND "}\n");
        } else {
            out(IND "if (optind < argc) {\n");
            generate_field_parser(field, fp, prefix, NULL, "argv[optind++]", "\"argument\"", "NULL", "optind", IND2);
            out(IND "} else {\n"
                IND IND "printf(\"Too few arguments.\\n\");\n"
                IND IND "errors++;\n"
                IND "}\n");
        }
    }
}

void generate_parse_command_line(Configuration *configuration, FILE *fp) {
    out("int parse_command_line(int argc, char *argv[]) {\n"
        IND "int errors = 0;\n"
        IND "static struct option long_options[] =\n"
        IND IND "{\n"
    );

    int count = 256;
    char *short_opts = generate_long_options(configuration->config, fp, &count);
    short_opts = ccn_str_cat(short_opts, generate_long_options_mo(configuration->multioptions, fp, &count));
    short_opts = ccn_str_cat(short_opts, generate_long_options_os(configuration->optionsets, fp, &count));
    if (globals.generate_configfile) {
        short_opts = ccn_str_cat(short_opts, generate_long_options_to(configuration->targetoptions, fp, &count));
    }

    out(IND IND IND "{0, 0, 0, 0}\n"
        IND IND "};\n\n"
        IND "int c;\n"
        IND "int long_index;\n"
        IND "int res;\n"
        IND "unsigned long ul;\n"
        IND "array *strings;\n"
        IND "while (true) {\n"
        IND IND "c = getopt_long(argc, argv, \"%s\", long_options, &long_index);\n"
        IND IND "if (c == -1) {\n"
        IND IND IND "break;\n"
        IND IND "}\n\n"
        IND IND "switch (c) {\n"
        IND IND IND "case '?':\n"
        IND IND IND IND "errors++;\n"
        IND IND IND IND "break;\n",
        short_opts
    );

    count = 256;
    generate_getopt_cases(configuration->config, fp, &count, "");
    generate_getopt_cases_mo(configuration->multioptions, fp, &count, configuration->config->id);
    generate_getopt_cases_os(configuration->optionsets, fp, &count, configuration->config->id);
    if (globals.generate_configfile) {
        generate_getopt_cases_to(configuration->targetoptions, fp, &count);
    }

    out(IND IND "}\n"
        IND "}\n"
    );

    generate_arguments_loop(configuration->config, fp, "");

    out(IND "return errors;\n"
        "}\n\n"
    );
}

void generate_mallocs(Config *config, FILE *fp, char *prefix) {
    prefix = ccn_str_cat(ccn_str_cat(prefix, config->id), ".");
    for (size_t i = 0; i < array_size(config->fields); i++) {
        Field *field = (Field *)array_get(config->fields, i);
        if (field->config) {
            generate_mallocs(field->config, fp, prefix);
            continue;
        }
        if (!field->is_list) {
            continue;
        }
        out(IND "arr = internal_arrays.%s%s;\n"
            IND "%s%s = malloc(array_size(arr) * sizeof(",
            prefix, field->id, prefix, field->id);
        if (field->enum_id) {
            out("CFG_%s", field->enum_id);
        } else if (field->type == FT_uint) {
            out("unsigned int");
        } else if (field->type == FT_string) {
            out("char *");
        } else {
            out("%s", FieldType_name(field->type));
        }
        out("));\n"
            IND "for (size_t i = 0; i < array_size(arr); i++) {\n"
            IND IND "%s%s[i] = *(", prefix, field->id);
        if (field->enum_id) {
            out("CFG_%s ", field->enum_id);
        } else if (field->type == FT_uint) {
            out("unsigned int ");
        } else if (field->type == FT_int) {
            out("int ");
        } else if (field->type == FT_string) {
            out("char *");
        } else {
            out("%s ", FieldType_name(field->type));
        }
        out("*)array_get(arr, i);\n"
            IND "}\n"
            IND "%s%s_length = array_size(arr);\n",
            prefix, field->id
        );
    }
}

void generate_mallocs_helper(Configuration *configuration, FILE *fp) {
    out("void malloc_arrays(void) {\n"
    IND "array *arr;\n");
    generate_mallocs(configuration->config, fp, "");
    out("}\n\n");
}

void generate_internal_array_frees(Config *config, FILE *fp, char *prefix) {
    prefix = ccn_str_cat(ccn_str_cat(prefix, config->id), ".");
    for (size_t i = 0; i < array_size(config->fields); i++) {
        Field *field = (Field *)array_get(config->fields, i);
        if (field->config) {
            generate_internal_array_frees(field->config, fp, prefix);
            continue;
        }
        if (!field->is_list) {
            continue;
        }
        out(IND "array_cleanup(internal_arrays.%s%s, &free);" NL,
            prefix, field->id
        );
    }
}

void generate_cleanup_function(Configuration *configuration, FILE *fp) {
    out("void cleanup(void) {" NL);
    if (globals.generate_configfile) {
        out(IND "smap_free_values(target_map, &free_target);" NL
            IND "smap_free(target_map);" NL
        );
    }
    generate_internal_array_frees(configuration->config, fp, "");
    out("}" NL NL);
}

void generate_parse_function(FILE *fp) {
    if (globals.generate_configfile) {
        out("CCNM_RESULT ccnm_parse(int argc, char *argv[], FILE *fp) {\n");
    } else {
        out("CCNM_RESULT ccnm_parse(int argc, char *argv[]) {\n");
    }
    out(IND "initialize_defaults();\n"
        IND "initialize_arrays();\n"
    );
    if (globals.generate_configfile) {
        out(IND "target_map = smap_init(32);\n"
            IND "if (fp && parse_configuration_file(fp)) {\n"
            IND IND "cleanup();\n"
            IND IND "return CCNM_FILE_ERROR;\n"
            IND "}\n"
        );
    }
    out(IND "if (parse_command_line(argc, argv)) {\n"
        IND IND "cleanup();\n"
        IND IND "return CCNM_CLI_ERROR;\n"
        IND "}\n"
        IND "malloc_arrays();\n"
        IND "cleanup();\n"
        IND "return CCNM_SUCCESS;\n"
        "}\n\n"
    );
}

void generate_frees(Config *config, FILE *fp, char *prefix) {
    prefix = ccn_str_cat(ccn_str_cat(prefix, config->id), ".");
    for (size_t i = 0; i < array_size(config->fields); i++) {
        Field *field = (Field *)array_get(config->fields, i);
        if (field->config) {
            generate_frees(field->config, fp, prefix);
            continue;
        }
        if (field->is_list) {
            if (field->type == FT_string) {
                out(IND "for (size_t i = 0; i < %s%s_length; i++) {\n"
                    IND IND "free(%s%s[i]);\n"
                    IND "}\n", prefix, field->id, prefix, field->id);
            }
            out(IND "free(%s%s);\n", prefix, field->id);
        } else if (field->type == FT_string) {
            out(IND "free(%s%s);\n", prefix, field->id);
        }
    }
}

void generate_free_function(Configuration *configuration, FILE *fp) {
    out("void ccnm_free(void) {\n");
    generate_frees(configuration->config, fp, "");
    out("}\n\n");
}

void generate_option_print(array *options, FILE *fp, char *info, array *disable) {
    // option format = 4/24/4/48 whitespace/name/whitespace/info
    char *optionstring = "";

    if (!array_size(options)) {
        return;
    }

    bool first = true;

    for (size_t i = 0; i < array_size(options); i++) {
        char *option = (char *)array_get(options, i);
        if (!first) {
            optionstring = ccn_str_cat(optionstring, " ");
        }
        if (strlen(option) == 1) {
            optionstring = ccn_str_cat(ccn_str_cat(optionstring, "-"), option);
        } else {
            optionstring = ccn_str_cat(ccn_str_cat(optionstring, "--"), option);
        }
        first = false;
    }

    if (disable) {
        for (size_t i = 0; i < array_size(disable); i++) {
            char *option = (char *)array_get(disable, i);
            if (!first) {
                optionstring = ccn_str_cat(optionstring, " ");
            }
            if (strlen(option) == 1) {
                optionstring = ccn_str_cat(ccn_str_cat(optionstring, "-"), option);
            } else {
                optionstring = ccn_str_cat(ccn_str_cat(optionstring, "--"), option);
            }
            first = false;
        }
    }

    int noptionlines = (strlen(optionstring) + 23) / 24;
    int ninfolines = (strlen(info) + 47) / 48;
    int nlines = noptionlines > ninfolines ? noptionlines : ninfolines;

    for (size_t i = 0; i < nlines; i++) {
        out(IND2 "   \"");
        if (i == noptionlines - 1) {
            out("    %s", &(optionstring[i * 24]));
            if (i < ninfolines) {
                for (size_t j = 0; j < 24 - strlen(&(optionstring[i * 24])); j++) {
                    out(" ");
                }
            }
        } else if (i < noptionlines) {
            out("    %.*s",
                24,
                &(optionstring[i * 24])
            );
        } else {
            if (i < ninfolines) {
                out(IND6 IND);
            }
        }
        if (i < ninfolines) {
            out("    %.*s",
                48,
                &(info[i * 48])
            );
        }
        out("\\n\"" NL);
    }

    out(IND2 "   \"\\n\"" NL);
}

void generate_main_options_print(Config *config, FILE *fp, Configuration *configuration) {
    if (!configuration) {
        out(IND2 "   \"");
        for (size_t i = 0; i < strlen(config->id); i++) {
            out("%c", toupper(config->id[i]));
        }

        out(" OPTIONS:\\n\"" NL);

        out(IND2 "   \"\\n\"" NL);

        int nlines = (strlen(config->info) + 79) / 80;
        for (size_t i = 0; i < nlines; i++) {
            out(IND2 "   \"%.*s\\n\"" NL,
                80,
                &(config->info[i * 80])
            );
        }

        out(IND2 "   \"\\n\"" NL);
    }
    for (size_t i = 0; i < array_size(config->fields); i++) {
        Field *field = (Field *)array_get(config->fields, i);
        if (field->config) {
            continue;
        }
        if (field->is_argument) {
            continue;
        }
        generate_option_print(field->options, fp, field->info, field->disable_options);
    }

    if (configuration) {
        for (size_t i = 0; i < array_size(configuration->multioptions); i++) {
            MultiOption *multioption = (MultiOption *)array_get(configuration->multioptions, i);
            generate_option_print(multioption->options, fp, multioption->info, NULL);
        }
        for (size_t i = 0; i < array_size(configuration->optionsets); i++) {
            OptionSet *optionset = (OptionSet *)array_get(configuration->optionsets, i);
            generate_option_print(optionset->options, fp, optionset->info, NULL);
        }
        if (globals.generate_configfile) {
            generate_option_print(configuration->targetoptions, fp, "Apply target defined in configuration file", NULL);
        }
    }

    for (size_t i = 0; i < array_size(config->fields); i++) {
        Field *field = (Field *)array_get(config->fields, i);
        if (field->config) {
            generate_main_options_print(field->config, fp, NULL);
        }
    }
}

void generate_print_usage_function(Configuration *configuration, FILE *fp) {
    out("void ccnm_print_usage(void) {" NL
        IND "printf(\"OPTIONS:\\n\"" NL
        IND2 "   \"\\n\"" NL
    );
    generate_main_options_print(configuration->config, fp, configuration);
    out(IND ");" NL
        "}\n");
}

void generate_parser(Configuration *configuration, FILE *fp) {
    generate_includes(fp);
    generate_enum_args_helper(configuration, fp);
    generate_internal_arrays_struct_helper(configuration, fp);
    generate_static_helpers(fp);
    generate_initialize_defaults(configuration, fp);
    generate_initialize_arrays(configuration, fp);
    if (globals.generate_configfile) {
        generate_parse_configuration_file(configuration, fp);
    }
    generate_parse_command_line(configuration, fp);
    generate_mallocs_helper(configuration, fp);
    generate_cleanup_function(configuration, fp);
    generate_parse_function(fp);
    generate_free_function(configuration, fp);
    generate_print_usage_function(configuration, fp);
}
