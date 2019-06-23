#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>
#include <inttypes.h>
#include <errno.h>

#include "cfg.ccnm.h"
#include "lib/str.h"
#include "lib/smap.h"

static const char *argument_parse_mode_args[] = {"GNU", "autoexpand", "extended", "exact"};
static const char *auto_case_mode_args[] = {"kebab", "pascal", "none"};

static struct {
    struct {
        struct {
            array *test;
            array *test3;
        } nested;
    } globals;
} internal_arrays;

struct file_opt {
    char *str;
    uint8_t type;
    void *loc;
    bool list;
    array **arr;
    bool vl;
    bool lo;
    bool vr;
    bool ro;
    union {
        unsigned int ui;
        int i;
        double d;
    } l;
    union {
        unsigned int ui;
        int i;
        double d;
    } r;
    const char **enum_args;
    size_t enum_size;
};

int parse_uint_helper(void *dest, array **arr, char *sep, char *string, char *source, const char *name, int argind, bool vl, bool lo, unsigned int l, bool vr, bool ro, unsigned int r) {
    if (sep) {
        int errors = 0;
        array *strings = ccn_str_split(string, sep[0]);
        for (size_t i = 0; i < array_size(strings); i++) {
            errors += parse_uint_helper(dest, arr, NULL, array_get(strings, i), source, name, argind, vl, lo, l, vr, ro, r);
        }
        array_cleanup(strings, &free);
        return errors;
    }
    unsigned long ul;
    errno = 0;
    if (errno == ERANGE || (ul = strtoul(string, NULL, 10)) > UINT_MAX) {
        printf("Value %s could not be converted.\n", string);
        return 1;
    }
    if ((vl && ((lo && ul < l) || ul <= l)) || (vr && ((ro && ul > r) || ul >= r))) {
        if (source) {
            printf("Invalid value %lu for %s %s.\n", ul, source, name);
        } else {
            printf("Invalid value %lu for argument %d.\n", ul, argind);
        }
        return 1;
    }
    if (arr) {
        dest = malloc(sizeof(unsigned int));
        array_append(*arr, dest);
    }
    *(unsigned int *)dest = ul;
    return 0;
}

int parse_int_helper(void *dest, array **arr, char *sep, char *string, char *source, const char *name, int argind, bool vl, bool lo, int l, bool vr, bool ro, int r) {
    if (sep) {
        int errors = 0;
        array *strings = ccn_str_split(string, sep[0]);
        for (size_t i = 0; i < array_size(strings); i++) {
            errors += parse_int_helper(dest, arr, NULL, array_get(strings, i), source, name, argind, vl, lo, l, vr, ro, r);
        }
        array_cleanup(strings, &free);
        return errors;
    }
    long sl;
    errno = 0;
    if (errno == ERANGE || (sl = strtol(string, NULL, 10)) > INT_MAX || sl < INT_MIN) {
        printf("Value %s could not be converted.\n", string);
        return 1;
    }
    if ((vl && ((lo && sl < l) || sl <= l)) || (vr && ((ro && sl > r) || sl >= r))) {
        if (source) {
            printf("Invalid value %ld for %s %s.\n", sl, source, name);
        } else {
            printf("Invalid value %ld for argument %d.\n", sl, argind);
        }
        return 1;
    }
    if (arr) {
        dest = malloc(sizeof(int));
        array_append(*arr, dest);
    }
    *(int *)dest = sl;
    return 0;
}

int parse_float_helper(void *dest, array **arr, char *sep, char *string, char *source, const char *name, int argind, bool vl, bool lo, double l, bool vr, bool ro, double r) {
    if (sep) {
        int errors = 0;
        array *strings = ccn_str_split(string, sep[0]);
        for (size_t i = 0; i < array_size(strings); i++) {
            errors += parse_float_helper(dest, arr, NULL, array_get(strings, i), source, name, argind, vl, lo, l, vr, ro, r);
        }
        array_cleanup(strings, &free);
        return errors;
    }
    double d = strtof(string, NULL);
    if ((vl && ((lo && d < l) || d <= l)) || (vr && ((ro && d > r) || d >= r))) {
        if (source) {
            printf("Invalid value %f for %s %s.\n", d, source, name);
        } else {
            printf("Invalid value %f for argument %d.\n", d, argind);
        }
        return 1;
    }
    if (arr) {
        dest = malloc(sizeof(double));
        array_append(*arr, dest);
    }
    *(double *)dest = d;
    return 0;
}

int parse_string_helper(void *dest, array **arr, char *sep, char *string) {
    if (sep) {
        int errors = 0;
        array *strings = ccn_str_split(string, sep[0]);
        for (size_t i = 0; i < array_size(strings); i++) {
            errors += parse_string_helper(dest, arr, NULL, array_get(strings, i));
        }
        array_cleanup(strings, &free);
        return errors;
    }
    if (arr) {
        dest = malloc(sizeof(char *));
        array_append(*arr, dest);
        *(char **)dest = strdup(string);
    } else {
        free(*(char **)dest);
        *(char **)dest = strdup(string);
    }
    return 0;
}

int parse_bool_helper(void *dest, array **arr, bool val) {
    if (arr) {
        dest = malloc(sizeof(bool));
        array_append(*arr, dest);
    }
    *(bool *)dest = val;
    return 0;
}

int parse_enum_helper(void *dest, array **arr, char *sep, char *string, char *source, const const char *name, int argind, const char *args[], int n) {
    if (sep) {
        int errors = 0;
        array *strings = ccn_str_split(string, sep[0]);
        for (size_t i = 0; i < array_size(strings); i++) {
            errors += parse_enum_helper(dest, arr, NULL, array_get(strings, i), source, name, argind, args, n);
        }
        array_cleanup(strings, &free);
        return errors;
    }
    if (arr) {
        dest = malloc(sizeof(int));
        array_append(*arr, dest);
    }
    for (size_t i = 0; i < n; i++) {
        if (ccn_str_equal(string, args[i])) {
            *(int *)dest = i;
            return 0;
        }
    }
    if (source) {
        printf("Invalid value %s for %s %s.\n", string, source, name);
    } else {
        printf("Invalid value %s for argument %d.\n", string, argind);
    }
    return 1;
}

int parse_quoted_string_helper(struct file_opt *fopt, char *tok, char *line, const char *name) {
    if (tok[0] != '"') {
        printf("Invalid value %s for setting %s.\n", tok, name);
        return 1;
    }
    bool escaped = false;
    char *rest = strtok(NULL, "");
    if (rest) {
        line = ccn_str_cat(tok, rest);
    } else {
        line = strdup(tok);
    }
    char *dup = malloc(strlen(line) * sizeof(char));
    size_t put = 0;
    for (size_t i = 1; i < strlen(line); i++) {
        if (escaped) {
            dup[put++] = line[i];
            escaped = false;
            continue;
        }
        if (line[i] == '\\') {
            escaped = true;
            continue;
        }
        if (line[i] == '"') {
            dup[put] = '\0';
            parse_string_helper(fopt->loc, fopt->arr, NULL, dup);
            free(dup);
            free(line);
            return 0;
        }
        dup[put++] = line[i];
    }
    printf("Invalid value %s for setting %s.\n", tok, name);
    free(dup);
    free(line);
    return 1;
}

int parse_bool_string_helper(struct file_opt *fopt, char *tok, const char *name) {
    if (ccn_str_equal(tok, "true")) {
        parse_bool_helper(fopt->loc, fopt->arr, true);
    } else if (ccn_str_equal(tok, "false")) {
        parse_bool_helper(fopt->loc, fopt->arr, false);
    } else {
        printf("Invalid value %s for setting %s.\n", tok, name);
        return 1;
    }
    return 0;
}

unsigned int *new_uint(unsigned int val) {
    unsigned int *ui = malloc(sizeof(unsigned int));
    *ui = val;
    return ui;
}

int *new_int(int val) {
    int *i = malloc(sizeof(int));
    *i = val;
    return i;
}

double *new_float(double val) {
    double *d = malloc(sizeof(double));
    *d = val;
    return d;
}

char **new_string(char *val) {
    char **s = malloc(sizeof(char *));
    *s = val;
    return s;
}

bool *new_bool(bool val) {
    bool *b = malloc(sizeof(bool));
    *b = val;
    return b;
}

struct setting {
    struct file_opt *fopt;
    char *tok;
    char *name;
    char *line;
};

struct setting *make_setting(struct file_opt *fopt, char *tok, char *name, char *line) {
    struct setting *s = malloc(sizeof(struct setting));
    s->fopt = fopt;
    s->tok = tok;
    s->name = name;
    s->line = line;
    return s;
}

void free_setting(void *v) {
    struct setting *s = (struct setting *)v;
    free(s->tok);
    free(s->name);
    free(s->line);
    free(s);
}

struct target {
    array *settings;
    struct target *parent;
};

struct target *make_target(struct target *parent) {
    struct target *t = malloc(sizeof(struct target));
    t->settings = create_array();
    t->parent = parent;
    return t;
}

static smap_t *target_map;

int apply_target(struct target *t) {
    int errors = 0;
    if (t->parent) {
        errors += apply_target(t->parent);
    }
    for (size_t i = 0; i < array_size(t->settings); i++) {
        struct setting *s = array_get(t->settings, i);
        struct file_opt *option = s->fopt;
        char *tok = s->tok;
        char *name = s->name;
        char *line = s->line;
        switch (option->type) {
            case 0:
                errors += parse_uint_helper(option->loc, option->arr, NULL, tok, "setting", name, 0, option->vl, option->lo, option->l.ui, option->vr, option->ro, option->r.ui);
                break;
            case 1:
                errors += parse_int_helper(option->loc, option->arr, NULL, tok, "setting", name, 0, option->vl, option->lo, option->l.i, option->vr, option->ro, option->r.i);
                break;
            case 2:
                errors += parse_int_helper(option->loc, option->arr, NULL, tok, "setting", name, 0, option->vl, option->lo, option->l.d, option->vr, option->ro, option->r.d);
                break;
            case 3:
                errors += parse_quoted_string_helper(option, tok, line, name);
                break;
            case 4:
                errors += parse_bool_string_helper(option, tok, name);
                break;
            case 5:
                errors += parse_enum_helper(option->loc, option->arr, NULL, tok, "setting", name, 0, option->enum_args, option->enum_size);
                break;
        }
    }
    return errors;
}

int parse_target(char *string) {
    int errors = 0;
    struct target *t;
    if (!(t = smap_retrieve(target_map, string))) {
        printf("Target %s not found.\n", string);
        return 1;
    }
    errors += apply_target(t);
    return errors;
}

void free_target(void *t) {
    array_cleanup(((struct target *)t)->settings, &free_setting);
    free(t);
}

void initialize_defaults(void) {
    globals.outfile = strdup("cfg.ccnm.c");
    globals.headerfile = strdup("cfg.ccnm.h");
    globals.verbose_flag = false;
    globals.help_flag = false;
    globals.version = false;
    globals.posixly_correct = false;
    globals.auto_disable_prefix = strdup("no-");
    globals.auto_prefix = strdup("--");
    globals.auto_case = ACM_kebab;
    globals.parse_mode = APM_GNU;
    globals.gnu_autoformat = true;
    globals.nested.test2 = 0;
    globals.nested.test4 = strdup("");
}

void initialize_arrays(void) {
    internal_arrays.globals.nested.test = create_array();
    array_append(internal_arrays.globals.nested.test, new_bool(true));
    array_append(internal_arrays.globals.nested.test, new_bool(false));
    array_append(internal_arrays.globals.nested.test, new_bool(false));
    internal_arrays.globals.nested.test3 = create_array();
}

int parse_configuration_file(FILE *fp) {
    int errors = 0;
    static struct file_opt options[] =
        {
            {"infile", 3, &globals.infile, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {"outfile", 3, &globals.outfile, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {"headerfile", 3, &globals.headerfile, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {"verbose_flag", 4, &globals.verbose_flag, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {"help_flag", 4, &globals.help_flag, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {"version", 4, &globals.version, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {"posixly_correct", 4, &globals.posixly_correct, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {"auto_disable_prefix", 3, &globals.auto_disable_prefix, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {"auto_prefix", 3, &globals.auto_prefix, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {"auto_case", 5, &globals.auto_case, 0, 0, 0, 0, 0, 0, 0, 0, auto_case_mode_args, 3},
            {"parse_mode", 5, &globals.parse_mode, 0, 0, 0, 0, 0, 0, 0, 0, argument_parse_mode_args, 4},
            {"gnu_autoformat", 4, &globals.gnu_autoformat, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {"nested.test", 4, 0, 1, &internal_arrays.globals.nested.test, 0, 0, 0, 0, 0, 0, 0, 0},
            {"nested.test2", 0, &globals.nested.test2, 0, 0, 1, 1, 1, 0, .l={.ui=0}, .r={.ui=3}, 0, 0},
            {"nested.test3", 1, 0, 1, &internal_arrays.globals.nested.test3, 1, 1, 1, 0, .l={.i=0}, .r={.i=3}, 0, 0},
            {"nested.test4", 3, &globals.nested.test4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        };
    char *line = NULL;
    size_t len = 0;
    char *tok;
    size_t i;
    size_t linenr = 0;
    char *name;
    struct target *current_target = NULL;
    char *current_target_name = NULL;
    while (getline(&line, &len, fp) != -1) {
        if ((name = tok = strtok(line, " \t\n"))) {
            if (ccn_str_equal(tok, "//")) {
                continue;
            }
            if (ccn_str_equal(tok, "target")) {
                if (!(current_target_name = tok = strtok(NULL, " \t\n"))) {
                    printf("Unexpected end of line on line %zu.\n", linenr);
                    continue;
                }
                if (!(tok = strtok(NULL, " \t\n"))) {
                    current_target = make_target(NULL); smap_insert(target_map, current_target_name, current_target);
                    continue;
                }
                if (!ccn_str_equal(tok, "extends")) {
                    printf("Unrecognized sequence on line %zu.\n", linenr);
                    continue;
                }
                if (!(tok = strtok(NULL, " \t\n"))) {
                    printf("Unexpected end of line on line %zu.\n", linenr);
                    continue;
                }
                if (!smap_retrieve(target_map, tok)) {
                    printf("Target %s not found\n", tok);
                    continue;
                }
                current_target = make_target(smap_retrieve(target_map, tok));
                smap_insert(target_map, current_target_name, current_target);
                continue;
            }
            for (i = 0; i < 16; i++) {
                if (ccn_str_equal(tok, options[i].str)) {
                    break;
                }
            }
            if (i >= 16) {
                printf("Unrecognized option %s on line %zu.\n", tok, linenr);
                continue;
            }
            if (!(tok = strtok(NULL, " \t\n"))) {
                printf("Unexpected end of line on line %zu.\n", linenr);
                continue;
            }
            if ((!options[i].list && !ccn_str_equal(tok, "=")) || (options[i].list && !ccn_str_equal(tok, "+="))) {
                printf("Unrecognized sequence on line %zu.\n", linenr);
                continue;
            }
            if (!(tok = strtok(NULL, " \t\n"))) {
                printf("Unexpected end of line on line %zu.\n", linenr);
                continue;
            }
            if (current_target) {
                array_append(current_target->settings, make_setting(&options[i], strdup(tok), strdup(name), strdup(line)));
                continue;
            }
            switch (options[i].type) {
                case 0:
                    errors += parse_uint_helper(options[i].loc, options[i].arr, NULL, tok, "setting", name, 0, options[i].vl, options[i].lo, options[i].l.ui, options[i].vr, options[i].ro, options[i].r.ui);
                    break;
                case 1:
                    errors += parse_int_helper(options[i].loc, options[i].arr, NULL, tok, "setting", name, 0, options[i].vl, options[i].lo, options[i].l.i, options[i].vr, options[i].ro, options[i].r.i);
                    break;
                case 2:
                    errors += parse_int_helper(options[i].loc, options[i].arr, NULL, tok, "setting", name, 0, options[i].vl, options[i].lo, options[i].l.d, options[i].vr, options[i].ro, options[i].r.d);
                    break;
                case 3:
                    errors += parse_quoted_string_helper(&options[i], tok, line, name);
                    break;
                case 4:
                    errors += parse_bool_string_helper(&options[i], tok, name);
                    break;
                case 5:
                    errors += parse_enum_helper(options[i].loc, options[i].arr, NULL, tok, "setting", name, 0, options[i].enum_args, options[i].enum_size);
                    break;
            }
        }
        linenr++;
    }
    free(line);
    return errors;
}

int parse_command_line(int argc, char *argv[]) {
    int errors = 0;
    static struct option long_options[] =
        {
            {"outfile", required_argument, 0, 256},
            {"headerfile", required_argument, 0, 257},
            {"verbose", no_argument, 0, 258},
            {"help", no_argument, 0, 259},
            {"print-help", no_argument, 0, 259},
            {"this-is-a-very-long-option-wow", no_argument, 0, 259},
            {"version", no_argument, 0, 260},
            {"posixly-correct", no_argument, 0, 261},
            {"no-posixly-correct", no_argument, 0, 262},
            {"auto-disable-prefix", required_argument, 0, 263},
            {"auto-prefix", required_argument, 0, 264},
            {"auto-case", required_argument, 0, 265},
            {"parse-mode", required_argument, 0, 266},
            {"gnu-autoformat", no_argument, 0, 267},
            {"no-gnu-autoformat", no_argument, 0, 268},
            {"test", no_argument, 0, 269},
            {"no-test", no_argument, 0, 270},
            {"test2", required_argument, 0, 271},
            {"test3", required_argument, 0, 272},
            {"test4", required_argument, 0, 273},
            {"opt", required_argument, 0, 274},
            {"os", required_argument, 0, 275},
            {"target", required_argument, 0, 276},
            {0, 0, 0, 0}
        };

    int c;
    int long_index;
    int res;
    unsigned long ul;
    array *strings;
    while (true) {
        c = getopt_long(argc, argv, "vhO:t:", long_options, &long_index);
        if (c == -1) {
            break;
        }

        switch (c) {
            case '?':
                errors++;
                break;
            case 256:
                errors += parse_string_helper(&globals.outfile, 0, 0, optarg);
                break;
            case 257:
                errors += parse_string_helper(&globals.headerfile, 0, 0, optarg);
                break;
            case 'v':
                errors += parse_bool_helper(&globals.verbose_flag, 0, true);
                break;
            case 258:
                errors += parse_bool_helper(&globals.verbose_flag, 0, true);
                break;
            case 'h':
                errors += parse_bool_helper(&globals.help_flag, 0, true);
                break;
            case 259:
                errors += parse_bool_helper(&globals.help_flag, 0, true);
                break;
            case 260:
                errors += parse_bool_helper(&globals.version, 0, true);
                break;
            case 261:
                errors += parse_bool_helper(&globals.posixly_correct, 0, true);
                break;
            case 262:
                errors += parse_bool_helper(&globals.posixly_correct, 0, false);
                break;
            case 263:
                errors += parse_string_helper(&globals.auto_disable_prefix, 0, 0, optarg);
                break;
            case 264:
                errors += parse_string_helper(&globals.auto_prefix, 0, 0, optarg);
                break;
            case 265:
                errors += parse_enum_helper(&globals.auto_case, 0, 0, optarg, "option", long_options[long_index].name, 0, auto_case_mode_args, 3);
                break;
            case 266:
                errors += parse_enum_helper(&globals.parse_mode, 0, 0, optarg, "option", long_options[long_index].name, 0, argument_parse_mode_args, 4);
                break;
            case 267:
                errors += parse_bool_helper(&globals.gnu_autoformat, 0, true);
                break;
            case 268:
                errors += parse_bool_helper(&globals.gnu_autoformat, 0, false);
                break;
            case 269:
                errors += parse_bool_helper(&globals.nested.test, 0, true);
                break;
            case 270:
                errors += parse_bool_helper(&globals.nested.test, 0, false);
                break;
            case 271:
                errors += parse_uint_helper(&globals.nested.test2, 0, 0, optarg, "option", long_options[long_index].name, 0, 1, 1, 0, 1, 0, 3);
                break;
            case 272:
                errors += parse_int_helper(0, &internal_arrays.globals.nested.test3, ".", optarg, "option", long_options[long_index].name, 0, 1, 1, 0, 1, 0, 3);
                break;
            case 273:
                errors += parse_string_helper(&globals.nested.test4, 0, 0, optarg);
                break;
            case 'O':
                array_append(internal_arrays.globals.nested.test3, new_int(1));
                array_append(internal_arrays.globals.nested.test3, new_int(2));
                errors += parse_string_helper(&globals.nested.test4, 0, 0, optarg);
                break;
            case 274:
                array_append(internal_arrays.globals.nested.test3, new_int(1));
                array_append(internal_arrays.globals.nested.test3, new_int(2));
                errors += parse_string_helper(&globals.nested.test4, 0, 0, optarg);
                break;
            case 275:
                strings = ccn_str_split(optarg, '.');
                for (size_t i = 0; i < array_size(strings); i++) {
                    if (ccn_str_equal("a", array_get(strings, i))) {
                        array_append(internal_arrays.globals.nested.test3, new_int(1));
                        continue;
                    }
                    if (ccn_str_equal("b", array_get(strings, i))) {
                        array_append(internal_arrays.globals.nested.test3, new_int(2));
                        continue;
                    }
                    printf("%s is not a valid value for option %s\n", (char *)array_get(strings, i), long_options[long_index].name);
                    errors++;
                }
                array_cleanup(strings, &free);
                break;
            case 't':
                errors += parse_target(optarg);
                break;
            case 276:
                errors += parse_target(optarg);
                break;
        }
    }
    if (optind < argc) {
        errors += parse_string_helper(&globals.infile, 0, 0, argv[optind++]);
    } else {
        printf("Too few arguments.\n");
        errors++;
    }
    return errors;
}

void malloc_arrays(void) {
    array *arr;
    arr = internal_arrays.globals.nested.test;
    globals.nested.test = malloc(array_size(arr) * sizeof(bool));
    for (size_t i = 0; i < array_size(arr); i++) {
        globals.nested.test[i] = *(bool *)array_get(arr, i);
    }
    globals.nested.test_length = array_size(arr);
    arr = internal_arrays.globals.nested.test3;
    globals.nested.test3 = malloc(array_size(arr) * sizeof(int));
    for (size_t i = 0; i < array_size(arr); i++) {
        globals.nested.test3[i] = *(int *)array_get(arr, i);
    }
    globals.nested.test3_length = array_size(arr);
}

void cleanup(void) {
    smap_free_values(target_map, &free_target);
    smap_free(target_map);
    array_cleanup(internal_arrays.globals.nested.test, &free);
    array_cleanup(internal_arrays.globals.nested.test3, &free);
}

CCNM_RESULT ccnm_parse(int argc, char *argv[], FILE *fp) {
    initialize_defaults();
    initialize_arrays();
    target_map = smap_init(32);
    if (fp && parse_configuration_file(fp)) {
        cleanup();
        return CCNM_FILE_ERROR;
    }
    if (parse_command_line(argc, argv)) {
        cleanup();
        return CCNM_CLI_ERROR;
    }
    malloc_arrays();
    cleanup();
    return CCNM_SUCCESS;
}

void ccnm_free(void) {
    free(globals.infile);
    free(globals.outfile);
    free(globals.headerfile);
    free(globals.auto_disable_prefix);
    free(globals.auto_prefix);
    free(globals.nested.test);
    free(globals.nested.test3);
    free(globals.nested.test4);
}
void ccnm_print_usage(void) {
    printf("OPTIONS:\n"
           "\n"
           "    --outfile\n"
           "\n"
           "    --headerfile\n"
           "\n"
           "    -v --verbose                print configuration tree\n"
           "\n"
           "    -h --help --print-help -    print this message very long message wow such he\n"
           "    -this-is-a-very-long-opt    lp very usage wow wow\n"
           "    ion-wow\n"
           "\n"
           "    --version\n"
           "\n"
           "    --posixly-correct\n"
           "\n"
           "    --auto-disable-prefix\n"
           "\n"
           "    --auto-prefix\n"
           "\n"
           "    --auto-case\n"
           "\n"
           "    --parse-mode\n"
           "\n"
           "    --gnu-autoformat\n"
           "\n"
           "    -O --opt\n"
           "\n"
           "    --os\n"
           "\n"
           "    -t --target                 Apply target defined in configuration file\n"
           "\n"
           "NESTED OPTIONS:\n"
           "\n"
           "these are nested\n"
           "\n"
           "    --test\n"
           "\n"
           "    --test2\n"
           "\n"
           "    --test3\n"
           "\n"
           "    --test4\n"
           "\n"
    );
}
