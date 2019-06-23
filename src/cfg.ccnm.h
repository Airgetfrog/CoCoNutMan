#ifndef __CFG_CCNM_H__
#define __CFG_CCNM_H__

#include <stdio.h>
#include <stdbool.h>

typedef enum CCNM_RESULT {
    CCNM_SUCCESS = 0,
    CCNM_IO_ERROR,
    CCNM_FILE_ERROR,
    CCNM_CLI_ERROR
} CCNM_RESULT;

typedef enum {
    APM_GNU = 0,
    APM_autoexpand,
    APM_extended,
    APM_exact
} CFG_argument_parse_mode;

typedef enum {
    ACM_kebab = 0,
    ACM_pascal,
    ACM_none
} CFG_auto_case_mode;

struct globals {
    char *infile;
    char *outfile;
    char *headerfile;
    bool verbose_flag;
    bool dry_run;
    bool help_flag;
    bool version;
    bool posixly_correct;
    char *auto_disable_prefix;
    char *auto_prefix;
    CFG_auto_case_mode auto_case;
    CFG_argument_parse_mode parse_mode;
    bool gnu_autoformat;
    bool generate_configfile;
} globals;

CCNM_RESULT ccnm_parse(int argc, char *argv[]);
void ccnm_print_usage(void);
void ccnm_free(void);

#endif
