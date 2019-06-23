#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "lib/errors.h"
#include "lib/print.h"
#include "lib/str.h"

#include "cfg.ccnm.h"

#include "cfg.h"
#include "print-cfg.h"
#include "convert-cfg.h"
#include "check-cfg.h"
#include "gen-header.h"
#include "gen-parser.h"

// Defined in parser
extern Configuration *parse(FILE *fp);
extern char *yy_filename;

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
    FILE *fp;
    int ret = 0;

    if (ccnm_parse(argc, argv, NULL)) {
        return 1;
    }

    if (globals.version) {
        version();
        return 0;
    }

    if (globals.help_flag) {
        ccnm_print_usage();
        return 0;
    }

    if (!globals.infile) {
        ccnm_print_usage();
        return 1;
    }

    if (globals.auto_case != ACM_kebab) {
        print_error_no_loc("Pascal and none case modes not supported at this time");
        return 1;
    }

    if (globals.parse_mode != APM_GNU) {
        print_error_no_loc("Autoexpand, extended, and exact case modes not supported at this time");
        return 1;
    }

    if (globals.gnu_autoformat == false) {
        print_error_no_loc("GNU autoformat must be enabled in this version");
        return 1;
    }

    fp = open_input_file(globals.infile);

    Configuration *configuration = parse(fp);

    fclose(fp);

    int errors = 0;
    // has no errors unless -Werror is on, nothing depends on this
    errors += check_and_convert_attributes(configuration);
    // this can actually error
    errors += check_configuration(configuration);
    if (errors) {
        exit_compile_error();
    }

    if (globals.verbose_flag) {
        print_configuration(configuration);
    }

    fp = fopen(globals.headerfile, "w");
    if (!fp) {
        perror("Opening file failed");
        exit(CANNOT_OPEN_FILE);
    }
    generate_header(configuration, fp);

    fp = fopen(globals.outfile, "w");
    if (!fp) {
        perror("Opening file failed");
        exit(CANNOT_OPEN_FILE);
    }
    generate_parser(configuration, fp);

    // free_configuration(configuration);

    ccnm_free();

    return ret;
}
