#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "lib/errors.h"
#include "lib/print.h"

#include "cfg.h"
#include "print-cfg.h"
#include "convert-cfg.h"
#include "check-cfg.h"
#include "gen-header.h"

// Defined in parser
extern Configuration *parse(FILE *fp);
extern char *yy_filename;

static void usage(char *program) {
    char *program_bin = strrchr(program, '/');
    if (program_bin)
        program = program_bin + 1;

    printf("Usage: %s [options] [file]\n", program);
    printf("Options:\n");
}

static void version(void) {
    printf("coconutman 0.1\nCoCoNut Configuration Manager\n");
}

static FILE *open_input_file(char *path) {

    struct stat path_stat;
    if (stat(path, &path_stat) != 0) {
        print_error_no_loc("%s: cannot open file: %s", path, strerror(errno));
        exit(CANNOT_OPEN_FILE);
    }

    // Test if file a regular file.
    if (S_ISREG(path_stat.st_mode) != 1) {
        print_error_no_loc("%s: cannot open file: "
                           "specified path is not a regular file.",
                           path);
        exit(CANNOT_OPEN_FILE);
    }

    FILE *f = fopen(path, "r");
    if (f == NULL) {
        print_error_no_loc("%s: cannot open file: %s", yy_filename,
                           strerror(errno));
        exit(CANNOT_OPEN_FILE);
    }

    return f;
}

void exit_compile_error(void) {
    PRINT_COLOR(MAGENTA);
    fprintf(stderr, "Errors were found, code generation terminated.\n");
    PRINT_COLOR(RESET_COLOR);
    exit(INVALID_CONFIG);
}

int main(int argc, char *argv[]) {
    int verbose_flag = 0;
    int list_gen_files_flag = 0;
    int ret = 0;
    int option_index;
    int c = 0;
    char *output_dir = NULL;
    char *dot_dir = NULL;

    struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 20},
        {0, 0, 0, 0}};

    while (1) {
        c = getopt_long(argc, argv, "v", long_options, &option_index);

        // End of options
        if (c == -1)
            break;

        switch (c) {
        case 20:
            version();
            return 0;
        case 'v':
            verbose_flag = 1;
            break;
        case 'h':
            usage(argv[0]);
            return 0;
        case '?':
            usage(argv[0]);
            return 1;
        }
    }

    if (optind == argc - 1) {
        yy_filename = argv[optind];
    } else {
        usage(argv[0]);
        return 1;
    }

    if (output_dir == NULL)
        output_dir = "generated/";

    FILE *f = open_input_file(yy_filename);

    Configuration *configuration = parse(f);

    fclose(f);

    int errors = 0;
    // has no errors unless -Werror is on, nothing depends on this
    errors += check_and_convert_attributes(configuration);
    // this can actually error
    errors += check_configuration(configuration);
    if (errors) {
        exit_compile_error();
    }

    if (verbose_flag) {
        print_configuration(configuration);
    }

    FILE *fp = fopen("cfg.ccnm.h", "w");
    if (!fp) {
        perror("Opening file failed");
        exit(CANNOT_OPEN_FILE);
    }
    generate_header(configuration, fp);

    // free_configuration(configuration);

    return ret;
}
