#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>
#include <inttypes.h>
#include <errno.h>

#include "lib/str.h"
#include "lib/smap.h"

#include "cfg.ccnm.h"

static const char *argument_parse_mode_args[] = {"GNU", "autoexpand", "extended", "exact"};
static const char *auto_case_mode_args[] = {"kebab", "pascal", "none"};

static struct {
    struct {
    } globals;
} internal_arrays;

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

void initialize_defaults(void) {
    globals.outfile = strdup("cfg.ccnm.c");
    globals.headerfile = strdup("cfg.ccnm.h");
    globals.verbose_flag = false;
    globals.dry_run = false;
    globals.help_flag = false;
    globals.version = false;
    globals.posixly_correct = false;
    globals.auto_disable_prefix = strdup("no-");
    globals.auto_prefix = strdup("--");
    globals.auto_case = ACM_kebab;
    globals.parse_mode = APM_GNU;
    globals.gnu_autoformat = true;
    globals.generate_configfile = true;
}

void initialize_arrays(void) {
}

int parse_command_line(int argc, char *argv[]) {
    int errors = 0;
    static struct option long_options[] =
        {
            {"outfile", required_argument, 0, 256},
            {"headerfile", required_argument, 0, 257},
            {"verbose", no_argument, 0, 258},
            {"dry", no_argument, 0, 259},
            {"dry-run", no_argument, 0, 259},
            {"no-dry-run", no_argument, 0, 260},
            {"help", no_argument, 0, 261},
            {"print-help", no_argument, 0, 261},
            {"version", no_argument, 0, 262},
            {"posixly-correct", no_argument, 0, 263},
            {"no-posixly-correct", no_argument, 0, 264},
            {"auto-disable-prefix", required_argument, 0, 265},
            {"auto-prefix", required_argument, 0, 266},
            {"auto-case", required_argument, 0, 267},
            {"parse-mode", required_argument, 0, 268},
            {"gnu-autoformat", no_argument, 0, 269},
            {"no-gnu-autoformat", no_argument, 0, 270},
            {"generate-configfile", no_argument, 0, 271},
            {"no-generate-configfile", no_argument, 0, 272},
            {0, 0, 0, 0}
        };

    int c;
    int long_index;
    int res;
    unsigned long ul;
    array *strings;
    while (true) {
        c = getopt_long(argc, argv, "vdh", long_options, &long_index);
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
            case 'd':
                errors += parse_bool_helper(&globals.dry_run, 0, true);
                break;
            case 259:
                errors += parse_bool_helper(&globals.dry_run, 0, true);
                break;
            case 260:
                errors += parse_bool_helper(&globals.dry_run, 0, false);
                break;
            case 'h':
                errors += parse_bool_helper(&globals.help_flag, 0, true);
                break;
            case 261:
                errors += parse_bool_helper(&globals.help_flag, 0, true);
                break;
            case 262:
                errors += parse_bool_helper(&globals.version, 0, true);
                break;
            case 263:
                errors += parse_bool_helper(&globals.posixly_correct, 0, true);
                break;
            case 264:
                errors += parse_bool_helper(&globals.posixly_correct, 0, false);
                break;
            case 265:
                errors += parse_string_helper(&globals.auto_disable_prefix, 0, 0, optarg);
                break;
            case 266:
                errors += parse_string_helper(&globals.auto_prefix, 0, 0, optarg);
                break;
            case 267:
                errors += parse_enum_helper(&globals.auto_case, 0, 0, optarg, "option", long_options[long_index].name, 0, auto_case_mode_args, 3);
                break;
            case 268:
                errors += parse_enum_helper(&globals.parse_mode, 0, 0, optarg, "option", long_options[long_index].name, 0, argument_parse_mode_args, 4);
                break;
            case 269:
                errors += parse_bool_helper(&globals.gnu_autoformat, 0, true);
                break;
            case 270:
                errors += parse_bool_helper(&globals.gnu_autoformat, 0, false);
                break;
            case 271:
                errors += parse_bool_helper(&globals.generate_configfile, 0, true);
                break;
            case 272:
                errors += parse_bool_helper(&globals.generate_configfile, 0, false);
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
}

void cleanup(void) {
}

CCNM_RESULT ccnm_parse(int argc, char *argv[]) {
    initialize_defaults();
    initialize_arrays();
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
}

void ccnm_print_usage(void) {
    printf("OPTIONS:\n"
           "\n"
           "    --outfile                   Location for output .c file\n"
           "\n"
           "    --headerfile                Location for output .h file\n"
           "\n"
           "    -v --verbose                Print configuration tree before outputting files\n"
           "\n"
           "    -d --dry --dry-run --no-    Do not output files\n"
           "    dry-run\n"
           "\n"
           "    -h --help --print-help      Print this message\n"
           "\n"
           "    --version                   Print version info\n"
           "\n"
           "    --posixly-correct --no-p    Parse options only up to first non-option argume\n"
           "    osixly-correct              nt (not implemented)\n"
           "\n"
           "    --auto-disable-prefix       String used to generate disable flag names\n"
           "\n"
           "    --auto-prefix               String used to generate option names\n"
           "\n"
           "    --auto-case                 Case conversion used to generate option names fr\n"
           "                                om field names (not implemented)\n"
           "\n"
           "    --parse-mode                Generated CLI parsing mode (not implemented)\n"
           "\n"
           "    --gnu-autoformat --no-gn    Automatically prepend option names with hyphens \n"
           "    u-autoformat                in GNU mode\n"
           "\n"
           "    --generate-configfile --    Enable generation of configuration file parsing \n"
           "    no-generate-configfile      functionality\n"
           "\n"
    );
}
