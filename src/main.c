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

    Configuration *parse_result = parse(f);

    fclose(f);

    int errors = 0;
    // has no errors unless -Werror is on, nothing depends on this
    errors += check_and_convert_attributes(parse_result);
    // this can actually error
    errors += check_configuration(parse_result);
    if (errors) {
        exit_compile_error();
    }

    if (verbose_flag) {
        print_configuration(parse_result);
    }
    //
    // // Set the parse tree for file generation.
    // filegen_init(parse_result, list_gen_files_flag);
    //
    // if (dot_dir) {
    //     filegen_dir(dot_dir);
    //     filegen_generate("ast.dot", generate_dot_definition);
    //     return 0;
    // }
    //
    // // Generated all the header files.
    // filegen_dir(header_dir);
    // filegen_generate("enum.h", generate_enum_definitions);
    // filegen_generate("ast.h", generate_ast_definitions);
    // filegen_all_nodes("ast-%s.h", generate_ast_node_header);
    // filegen_all_nodesets("ast-%s.h", generate_ast_nodeset_header);
    //
    // // Free nodes.
    // filegen_generate("free-ast.h", generate_free_header);
    // filegen_all_nodes("free-%s.h", generate_free_node_header);
    // filegen_all_nodesets("free-%s.h", generate_free_nodeset_header);
    //
    // filegen_generate("create-ast.h", generate_create_header);
    // filegen_all_nodes("create-%s.h", generate_create_node_header);
    // filegen_all_nodesets("create-%s.h", generate_create_nodeset_header);
    //
    // filegen_generate("copy-ast.h", generate_copy_header);
    // filegen_all_nodes("copy-%s.h", generate_copy_node_header);
    // filegen_all_nodesets("copy-%s.h", generate_copy_nodeset_header);
    //
    // filegen_generate("trav-ast.h", generate_trav_header);
    // filegen_generate("trav-core.h", generate_trav_core_header);
    // filegen_all_nodes("trav-%s.h", generate_trav_node_header);
    // // filegen_generate("consistency-ast.h", generate_consistency_header);
    // filegen_generate("phase-driver.h", generate_phase_driver_header);
    //
    // filegen_all_traversals("traversal-%s.h", generate_user_trav_header);
    // filegen_all_passes("pass-%s.h", generate_pass_header);
    //
    // filegen_generate("binary-serialization-util.h",
    //                  generate_binary_serialization_util_header);
    //
    // filegen_all_nodes("serialization-%s.h",
    //                   generate_binary_serialization_node_header);
    //
    // filegen_all_nodesets("serialization-%s.h",
    //                      generate_binary_serialization_nodeset_header);
    //
    // filegen_generate("textual-serialization-util.h",
    //                  generate_textual_serialization_util_header);
    //
    // filegen_generate("serialization-all.h",
    //                  generate_binary_serialization_all_header);
    //
    // filegen_cleanup_old_files();
    //
    // // Genereate all the source files.
    // filegen_dir(source_dir);
    //
    // filegen_all_nodes("free-%s.c", generate_free_node_definitions);
    // filegen_all_nodesets("free-%s.c", generate_free_nodeset_definitions);
    //
    // filegen_all_nodes("create-%s.c", generate_create_node_definitions);
    // filegen_all_nodesets("create-%s.c", generate_create_nodeset_definitions);
    //
    // filegen_all_nodes("copy-%s.c", generate_copy_node_definitions);
    // filegen_all_nodesets("copy-%s.c", generate_copy_nodeset_definitions);
    //
    // /* filegen_generate("trav-ast.c", generate_trav_definitions); */
    // filegen_generate("trav-core.c", generate_trav_core_definitions);
    // filegen_all_nodes("trav-%s.c", generate_trav_node_definitions);
    // // filegen_generate("consistency-ast.c", generate_consistency_definitions);
    // filegen_generate("phase-driver.c", generate_phase_driver_definitions);
    //
    // filegen_generate("binary-serialization-util.c",
    //                  generate_binary_serialization_util);
    // filegen_all_nodes("binary-serialization-%s-write.c",
    //                   generate_binary_serialization_node);
    //
    // filegen_all_nodesets("binary-serialization-%s-write.c",
    //                      generate_binary_serialization_nodeset);
    //
    // filegen_all_nodes("binary-serialization-%s-read.c",
    //                   generate_binary_serialization_read_node);
    //
    // filegen_all_nodesets("binary-serialization-%s-read.c",
    //                      generate_binary_serialization_read_nodeset);
    //
    // filegen_all_nodes("textual-serialization-%s-read.c",
    //                   generate_textual_serialization_read_node);
    //
    // filegen_all_nodesets("textual-serialization-%s-read.c",
    //                      generate_textual_serialization_read_nodeset);
    //
    // filegen_all_nodes("textual-serialization-%s-write.c",
    //                   generate_textual_serialization_write_node);
    //
    // filegen_all_nodesets("textual-serialization-%s-write.c",
    //                      generate_textual_serialization_write_nodeset);
    //
    // filegen_generate("textual-serialization-util.c",
    //                  generate_textual_serialization_util);
    //
    // filegen_cleanup_old_files();
    //
    // filegen_cleanup();
    //
    // free_config(parse_result);

    return ret;
}
