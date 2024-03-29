%{

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "cfg.h"
#include "create-cfg.h"

#include "lib/array.h"
#include "lib/imap.h"
#include "lib/print.h"

#include "coconutman.lexer.h"

extern bool yy_lex_keywords;
extern bool yy_lex_options;

/* Array to append config entries to during reducing */
static array *config_enums;
static array *config_multioptions;
static array *config_optionsets;
static array *config_targetoptions;
static struct Config *config_config;

static struct Configuration* parse_result = NULL;

char *yy_filename;
array *yy_lines;
imap_t *yy_parser_locations;

void yyerror(const char* s);
int yydebug = 1;

#define YYLTYPE YYLTYPE
typedef struct ParserLocation YYLTYPE;

struct ParserLocation yy_parser_location;

static void new_location(void *ptr, struct ParserLocation *loc);
static array *list_append(struct array *array, void *element,  struct ParserLocation *loc);

// Override YYLLOC_DEFAULT so we can set yy_parser_location
// to the current location
#define YYLLOC_DEFAULT(Cur, Rhs, N)                         \
    if (N) {                                                \
        (Cur).first_line   = YYRHSLOC(Rhs, 1).first_line;   \
        (Cur).first_column = YYRHSLOC(Rhs, 1).first_column; \
        (Cur).last_line    = YYRHSLOC(Rhs, N).last_line;    \
        (Cur).last_column  = YYRHSLOC(Rhs, N).last_column;  \
    } else {                                                \
        (Cur).first_line   = (Cur).last_line   =            \
          YYRHSLOC(Rhs, 0).last_line;                       \
        (Cur).first_column = (Cur).last_column =            \
          YYRHSLOC(Rhs, 0).last_column;                     \
    }                                                       \
    yy_parser_location = (Cur);

%}

%union {
    int64_t intval;
    uint64_t uintval;
    long double fval;
    char *string;
    bool boolval;
    struct array *array;
    struct Configuration *configuration;
    struct FieldValue *field_value;
    struct Attribute *attribute;
    struct Enum *enum_type;
    struct MultiOption *multioption;
    struct Setter *setter;
    struct OptionSet *option_set;
    struct Token *token;
    struct Config *config;
    struct Field *field;
    struct Range *range;
    enum FieldType type;
}

%error-verbose
%locations

%token<intval> T_INTVAL "integer value"
%token<uintval> T_UINTVAL "unsigned integer value"
%token<fval> T_FLOATVAL "float value"
%token<string> T_STRINGVAL "string value"
%token<string> T_ID "identifier"
%token<string> T_OPTION "option name"
%token<boolval> T_BOOLVAL "boolean value"

%token T_INT "int"
%token T_UINT "uint"

%token T_BOOL "bool"

%token T_ENUM "enum"
%token T_PREFIX "prefix"
%token T_INFO "info"
%token T_FLOAT "float"
%token T_STRING "string"
%token T_VALUES "values"
%token END 0 "End-of-file (EOF)"

%token T_ARROW "arrow"

%token T_CONFIG "config"
%token T_FIELDS "fields"
%token T_CONFIGFILE "configfile"
%token T_OPTIONS "options"
%token T_DISABLE "disable"
%token T_RANGE "range"
%token T_MULTIOPTION "multioption"
%token T_OPTIONSET "optionset"
%token T_TOKENS "tokens"
%token T_TARGETOPTIONS "targetoptions"
%token T_ARGUMENT "argument"
%token T_SEPARATOR "separator"
%token T_LIST "list"

%type<configuration> root
%type<enum_type> enum
%type<array> enum_attributes values multioption_attributes options multioption_fields setters optionset_attributes attribute_tokens tokens targetoptions config_attributes config_fields fields field_attributes idlist optionlist nested_id vallist disable
%type<attribute> enum_attribute multioption_attribute optionset_attribute config_attribute field_attribute
%type<string> prefix separator info id option
%type<multioption> multioption
%type<setter> setter
%type<option_set> optionset
%type<token> token
%type<config> config
%type<boolval> configfile argument left_open right_open
%type<field> field
%type<range> range_attribute range
%type<type> type
%type<field_value> defaultval val

%start root

%%

/* Root of the config, creating the final config */
root: entry { parse_result = create_configuration(config_enums, config_multioptions, config_optionsets, config_targetoptions, config_config); };

entry: entry enum { array_append(config_enums, $2); }
    | entry multioption { array_append(config_multioptions, $2); }
    | entry optionset { array_append(config_optionsets, $2); }
    | enum { array_append(config_enums, $1); }
    | multioption { array_append(config_multioptions, $1); }
    | optionset { array_append(config_optionsets, $1); }
    | entry targetoptions
    {
        if (config_targetoptions) {
            yyerror("Targetoptions are already defined.");
        }
        config_targetoptions = $2;
    }
    | targetoptions
    {
        if (config_targetoptions) {
            yyerror("Targetoptions are already defined.");
        }
        config_targetoptions = $1;
    }
    | entry config
    {
        if (config_config) {
            yyerror("A root config is already defined.");
        }
        config_config = $2;
    }
    | config
    {
        if (config_config) {
            yyerror("A root config is already defined.");
        }
        config_config = $1;
    }
    ;

enum: T_ENUM id '{' enum_attributes '}'
    {
        $$ = create_enum($2, $4);
        new_location($$, &@$);
        new_location($2, &@2);
    }
    ;

enum_attributes: enum_attributes ',' enum_attribute
    {
        $$ = list_append($1, $3, &@3);
    }
    | enum_attribute
    {
        $$ = list_append(NULL, $1, &@1);
    }
    ;

enum_attribute: prefix { $$ = create_attribute_string(E_prefix, $1); new_location($$, &@$); }
    | values { $$ = create_attribute_array(E_values, $1); new_location($$, &@$); }
    ;

prefix: T_PREFIX '=' id
    {
        $$ = $3;
        new_location($3, &@3);
    }
    ;

values: T_VALUES '='
    {
        yy_lex_keywords = false;
    }
    '{' idlist '}'
    {
        $$ = $5; new_location($5, &@4);
        yy_lex_keywords = true;
    };

multioption: T_MULTIOPTION '{' info multioption_attributes '}'
    {
        $$ = create_multioption($3, $4);
        new_location($$, &@$);
    }
    ;

multioption_attributes: multioption_attributes ',' multioption_attribute
    {
        $$ = list_append($1, $3, &@3);
    };
    | multioption_attribute
    {
        $$ = list_append(NULL, $1, &@1);
    }
    ;

multioption_attribute: options { $$ = create_attribute_array(MO_options, $1); new_location($$, &@$); }
    | multioption_fields { $$ = create_attribute_array(MO_fields, $1); new_location($$, &@$); }
    ;

optseq: T_OPTIONS '=' { yy_lex_options = true; };

options: optseq option
    {
        $$ = create_array();
        array_append($$, $2);
        new_location($2, &@2);
        yy_lex_options = false;
    }
    | optseq '{' optionlist '}'
    {
        $$ = $3;
        yy_lex_options = false;
    }
    | optseq '{' '}'
    {
        $$ = create_array();
        yy_lex_options = false;
    }
    ;

disableeq: T_DISABLE '=' { yy_lex_options = true; };

disable: disableeq option
    {
        $$ = create_array();
        array_append($$, $2);
        new_location($2, &@2);
        yy_lex_options = false;
    }
    | disableeq '{' optionlist '}'
    {
        $$ = $3;
        yy_lex_options = false;
    }
    | disableeq '{' '}'
    {
        $$ = create_array();
        yy_lex_options = false;
    }
    ;

multioption_fields: T_FIELDS '=' '{' setters '}' { $$ = $4; };

setters: setters ',' setter
    {
        $$ = list_append($1, $3, &@3);
    }
    | setter
    {
        $$ = list_append(NULL, $1, &@1);
    }
    ;

setter: nested_id '=' val
    {
        $$ = create_setter($1, $3);
        new_location($3, &@3);
    }
    | nested_id '=' '?'
    {
        $$ = create_setter($1, NULL);
    }
    | nested_id '=' '{' vallist '}'
    {
        FieldValue *fv = create_field_value_array($4);
        $$ = create_setter($1, fv);
        new_location(fv, &@3);
    }
    ;

optionset: T_OPTIONSET '{' info optionset_attributes '}'
    {
        $$ = create_option_set($3, $4);
        new_location($$, &@$);
    }
    ;

optionset_attributes: optionset_attributes ',' optionset_attribute
    {
        $$ = list_append($1, $3, &@3);
    }
    | optionset_attribute
    {
        $$ = list_append(NULL, $1, &@1);
    }
    ;

optionset_attribute: options { $$ = create_attribute_array(OS_options, $1); new_location($$, &@$); }
    | attribute_tokens { $$ = create_attribute_array(OS_tokens, $1); new_location($$, &@$); }
    | separator { $$ = create_attribute_string(OS_separator, $1); new_location($$, &@$); }
    ;

attribute_tokens: T_TOKENS '=' '{' tokens '}' { $$ = $4; };

tokens: tokens ',' token
    {
        $$ = list_append($1, $3, &@3);
    }
    | token
    {
        $$ = list_append(NULL, $1, &@1);
    }
    ;

token: id '=' '{' setters '}'
    {
        $$ = create_token($1, $4);
        new_location($1, &@1);
    };

separator: T_SEPARATOR '=' T_STRINGVAL
    {
        $$ = $3;
        new_location($3, &@3);
    };

toptseq: T_TARGETOPTIONS '='
    {
        yy_lex_options = true;
    };

targetoptions: toptseq '{' optionlist '}'
    {
        $$ = $3;
        yy_lex_options = false;
    };

config: T_CONFIG id '{' info config_attributes '}'
    {
        $$ = create_config($2, $4, $5);
        new_location($2, &@2);
    }
    ;

config_attributes: config_attributes ',' config_attribute
    {
        $$ = list_append($1, $3, &@3);
    }
    | config_attribute
    {
        $$ = list_append(NULL, $1, &@1);
    }
    ;

config_attribute: configfile { $$ = create_attribute_bool(C_configfile, $1); new_location($$, &@$); }
    | config_fields { $$ = create_attribute_array(C_fields, $1); new_location($$, &@$); };
    ;

configfile: T_CONFIGFILE '=' T_BOOLVAL { $$ = $3; };

config_fields: T_FIELDS '=' '{' fields '}' { $$ = $4; };

fields: fields ',' field
    {
        $$ = list_append($1, $3, &@3);
    }
    | field
    {
        $$ = list_append(NULL, $1, &@1);
    }
    ;

field: config
    {
        $$ = create_field_config($1);
    }
    | type T_LIST id defaultval '{' info field_attributes '}'
    {
        $$ = create_field($3, $6, $1, true, $4, $7);
        new_location($3, &@3);
    }
    | type id defaultval '{' info field_attributes '}'
    {
        $$ = create_field($2, $5, $1, false, $3, $6);
        new_location($2, &@2);
    }
    | type T_LIST id defaultval '{' info '}'
    {
        $$ = create_field($3, $6, $1, true, $4, create_array());
        new_location($3, &@3);
    }
    | type id defaultval '{' info '}'
    {
        $$ = create_field($2, $5, $1, false, $3, create_array());
        new_location($2, &@2);
    }
    | type T_LIST id defaultval
    {
        $$ = create_field($3, NULL, $1, true, $4, create_array());
        new_location($3, &@3);
    }
    | type id defaultval
    {
        $$ = create_field($2, NULL, $1, false, $3, create_array());
        new_location($2, &@2);
    }
    | id T_LIST id defaultval '{' info field_attributes '}'
    {
        $$ = create_field_enum($3, $6, FT_enum, true, $4, $7, $1);
        new_location($3, &@3);
    }
    | id id defaultval '{' info field_attributes '}'
    {
        $$ = create_field_enum($2, $5, FT_enum, false, $3, $6, $1);
        new_location($2, &@2);
    }
    | id T_LIST id defaultval '{' info '}'
    {
        $$ = create_field_enum($3, $6, FT_enum, true, $4, create_array(), $1);
        new_location($3, &@3);
    }
    | id id defaultval '{' info '}'
    {
        $$ = create_field_enum($2, $5, FT_enum, false, $3, create_array(), $1);
        new_location($2, &@2);
    }
    | id T_LIST id defaultval
    {
        $$ = create_field_enum($3, NULL, FT_enum, true, $4, create_array(), $1);
        new_location($1, &@1);
        new_location($3, &@3);
    }
    | id id defaultval
    {
        $$ = create_field_enum($2, NULL, FT_enum, false, $3, create_array(), $1);
        new_location($1, &@1);
        new_location($2, &@2);
    }
    ;

field_attributes: field_attributes ',' field_attribute
    {
        $$ = list_append($1, $3, &@3);
    }
    | field_attribute
    {
        $$ = list_append(NULL, $1, &@1);
    }
    ;

field_attribute: configfile { $$ = create_attribute_bool(F_configfile, $1); new_location($$, &@$); }
    | options
    {
        $$ = create_attribute_array(F_options, $1);
        new_location($1, &@1);
        new_location($$, &@$);
    }
    | disable
    {
        $$ = create_attribute_array(F_disable, $1);
        new_location($1, &@1);
        new_location($$, &@$);
    }
    | argument { $$ = create_attribute_bool(F_argument, $1); new_location($$, &@$); }
    | separator { $$ = create_attribute_string(F_separator, $1); new_location($$, &@$); }
    | range_attribute { $$ = create_attribute_range(F_range, $1); new_location($$, &@$); }
    ;

argument: T_ARGUMENT { $$ = true; };

range_attribute: T_RANGE '=' range { $$ = $3; new_location($3, &@3); };

range: left_open val T_ARROW val right_open
    {
        $$ = create_range($1, $5, $2, $4);
    }
    | '(' T_ARROW val right_open
    {
        $$ = create_range(true, $4, NULL, $3);
    }
    | left_open val T_ARROW ')'
    {
        $$ = create_range($1, true, $2, NULL);
    }
    | '(' T_ARROW ')'
    {
        $$ = create_range(true, true, NULL, NULL);
    }
    ;

left_open: '[' { $$ = false; }
    | '(' { $$ = true; }
    ;

right_open: ']' { $$ = false; }
    | ')' { $$ = true; }
    ;

type: T_BOOL { $$ = FT_bool; }
    | T_UINT { $$ = FT_uint; }
    | T_INT { $$ = FT_int; }
    | T_FLOAT { $$ = FT_float; }
    | T_STRING { $$ = FT_string; }
    ;

defaultval: '=' val { $$ = $2; }
    | '=' '{' vallist '}'
    {
        $$ = create_field_value_array($3);
        new_location($$, &@2);
    }
    | %empty { $$ = NULL; }
    ;

val: T_BOOLVAL { $$ = create_field_value_bool($1); new_location($$, &@$); }
    | T_UINTVAL { $$ = create_field_value_uint($1); new_location($$, &@$); }
    | T_INTVAL { $$ = create_field_value_int($1); new_location($$, &@$); }
    | T_FLOATVAL { $$ = create_field_value_float($1); new_location($$, &@$); }
    | T_STRINGVAL { $$ = create_field_value_string($1); new_location($$, &@$); }
    | id { $$ = create_field_value_enum($1); new_location($$, &@$); }
    ;

vallist: vallist ',' val
    {
        $$ = list_append($1, $3, &@3);
    }
    | val
    {
        $$ = list_append(NULL, $1, &@1);
    }
    ;

/* Comma separated list of identifiers. */
idlist: idlist ',' id
    {
        array_append($1, $3);
        $$ = $1;
        // $$ is an array and should not be added to location list.
        new_location($3, &@3);
    }
    | id
    {
        array *tmp = create_array();
        array_append(tmp, $1);
        $$ = tmp;
        // $$ is an array and should not be added to location list.
        new_location($1, &@1);
    }
    ;

id: T_ID { $$ = $1; }
    ;

optionlist: optionlist ',' option
    {
        $$ = list_append($1, $3, &@3);
    }
    | option
    {
        $$ = list_append(NULL, $1, &@1);
    }
    ;

option: T_OPTION { $$ = $1; }
    ;

nested_id: nested_id '.' id
    {
        $$ = list_append($1, $3, &@3);
    }
    | id
    {
        $$ = list_append(NULL, $1, &@1);
    }
    ;

info: T_INFO '=' T_STRINGVAL ','
    {
        $$ = $3;
        new_location($$, &@$);
        new_location($3, &@3);
    }
    | T_INFO '=' T_STRINGVAL
    {
        $$ = $3;
        new_location($$, &@$);
        new_location($3, &@3);
    }
    | %empty
    {
        $$ = NULL;
    }
    ;
%%

static void new_location(void *ptr, struct ParserLocation *loc) {
    struct ParserLocation *loc_copy = malloc(sizeof(struct ParserLocation));
    memcpy(loc_copy, loc, sizeof(struct ParserLocation));

    imap_insert(yy_parser_locations, ptr, loc_copy);
}

static array *list_append(struct array *array, void *element,  struct ParserLocation *loc) {
    if (!array) {
        array = create_array();
    }
    array_append(array, element);
    new_location(element, loc);
    return array;
}

struct Configuration* parse(FILE *fp) {
    yyin = fp;
    config_enums = create_array();
    config_multioptions = create_array();
    config_optionsets = create_array();

    yy_lines = array_init(32);
    yy_parser_locations = imap_init(128);

    print_init_compilation_messages(NULL, yy_filename,
        yy_lines, yy_parser_locations);
    yyparse();
    yylex_destroy();
    return parse_result;
}
