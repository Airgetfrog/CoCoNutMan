/* Handles the checking of the config.
 *
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cfg.h"
#include "cfg-names.h"
#include "create-cfg.h"
#include "lib/memory.h"
#include "lib/print.h"
#include "lib/smap.h"
#include "lib/str.h"

#define BQS "\033[1m‘%s’\033[0m"

struct Info {
    smap_t *enum_id;
    smap_t *enum_prefix;
    smap_t *options;

    Config *config;
};

static struct Info *create_info(void) {
    struct Info *info = (struct Info *)mem_alloc(sizeof(struct Info));

    info->enum_id = smap_init(32);
    info->enum_prefix = smap_init(32);
    info->options = smap_init(32);
    return info;
}

static void free_info(struct Info *info) {
    smap_free(info->enum_id);
    smap_free(info->enum_prefix);
    smap_free(info->options);
    mem_free(info);
}

/* Generates an option from an id.
 * This (should) depend on configuration. (TODO)
 * Used for fields without options that are not arguments.
 */
char *generate_option(char *id) {
    char *option = mem_alloc(sizeof(char) * strlen(id) + 3);
    memmove(option + 2, id, strlen(id) + 1);
    option[0] = '-';
    option[1] = '-';
    for (size_t i = 0; i < strlen(option); i++) {
        if (option[i] == '_') {
            option[i] = '-';
        }
    }
    return option;
}

int check_enum(struct Info *info, Enum *enum_struct) {
    int errors = 0;

    if (!enum_struct->values) {
        print_error(enum_struct, "attribute " BQS " missing from enum " BQS, Attribute_name(E_values), enum_struct->id);
        errors++;
    }

    // We enforce use of prefixes to avoid checking overlap between
    //  enums with and without prefix.
    if (!enum_struct->prefix) {
        print_error(enum_struct, "attribute " BQS " missing from enum " BQS, Attribute_name(E_prefix), enum_struct->id);
        errors++;
    }

    if (errors) {
        return errors;
    }

    if (smap_retrieve(info->enum_id, enum_struct->id)) {
        print_error(enum_struct->id, "redefinition of " BQS, enum_struct->id);
        print_note(((Enum *)smap_retrieve(info->enum_id, enum_struct->id))->id, "previous definition of " BQS " was here", enum_struct->id);
        errors++;
    }
    smap_insert(info->enum_id, enum_struct->id, enum_struct);

    if (smap_retrieve(info->enum_prefix, enum_struct->prefix)) {
        print_error(enum_struct->prefix, "duplicate prefix " BQS, enum_struct->prefix);
        print_note(smap_retrieve(info->enum_prefix, enum_struct->prefix), "previous use of " BQS " was here", enum_struct->prefix);
        errors++;
    }
    smap_insert(info->enum_prefix, enum_struct->prefix, enum_struct->prefix);

    smap_t *value_map = smap_init(32);
    for (size_t i = 0; i < array_size(enum_struct->values); i++) {
        char *id = array_get(enum_struct->values, i);
        if (smap_retrieve(value_map, id)) {
            print_error(id, "duplicate value " BQS, id);
            print_note(smap_retrieve(value_map, id), "first use of " BQS " was here", id);
            errors++;
        }
        smap_insert(value_map, id, id);
    }
    smap_free(value_map);

    return errors;
}

/* Checks a value in relation to a field.
 * Returns in multiple places.
 */
int check_value(struct Info *info, FieldValue *value, Field *field, bool is_list) {
    int errors = 0;

    if (is_list && value->type != FT_list) {
        print_error(value, "value is of type " BQS " but field is of type " BQS, FieldType_name(value->type), FieldType_name(FT_list));
        print_note(field, "field " BQS " defined here", field->id);
        return 1;
    }

    if (!is_list) {
        switch (field->type) {
            case FT_uint:
                errors += value->type != FT_uint;
                break;
            case FT_int:
                errors += value->type != FT_uint && value->type != FT_int;
                break;
            case FT_float:
                errors += value->type != FT_uint && value->type != FT_int && value->type != FT_float;
                break;
            default:
                errors += value->type != field->type;
        }
    }

    if (errors) {
        print_error(value, "value is of type " BQS " but field is of type " BQS, FieldType_name(value->type), FieldType_name(field->type));
        print_note(field, "field " BQS " defined here", field->id);
        return errors;
    }

    // types now guaranteed to match

    if (value->type == FT_enum) {
        Enum *enum_struct = (Enum *)smap_retrieve(info->enum_id, field->enum_id);
        if (!enum_struct) {
            // This is just a hint, an error will have been printed for
            //  the enum not existing.
            print_warning(value, "could not check value " BQS " of undefined enum " BQS, value->value.string_value, field->enum_id);
        } else {
            // simply check if the value is valid
            char *fvid = value->value.string_value;
            bool found = false;

            for (size_t i = 0; i < array_size(enum_struct->values); i++) {
                char *evid = (char *)array_get(enum_struct->values, i);
                if (ccn_str_equal(fvid, evid)) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                print_error(value, BQS " is not a valid value for enum " BQS, fvid, field->enum_id);
                print_note(enum_struct, "enum " BQS " defined here", field->enum_id);
                errors++;
            }
        }
    }

    if (is_list && value->type == FT_list) {
        // check each element individually (lists dont recurse)
        array *arr = value->value.array_value;
        for (size_t i = 0; i < array_size(arr); i++) {
            errors += check_value(info, array_get(arr, i), field, false);
        }
    }

    return errors;
}

int check_field(struct Info *info, Field *field) {
    int errors = 0;

    // There is no use case for this.
    if (field->is_argument && array_size(field->options)) {
        print_error(field->options, "field " BQS " has options but is an argument", field->id);
        errors++;
    }

    if (!field->options) {
        field->options = create_array();
        if (!field->is_argument) {
            array_append(field->options, generate_option(field->id));
        }
    }

    for (size_t i = 0; i < array_size(field->options); i++) {
        char *option = array_get(field->options, i);
        if (smap_retrieve(info->options, option)) {
            print_error(option, "duplicate option " BQS, option);
            print_note(smap_retrieve(info->options, option), "previous use of " BQS " was here", option);
            errors++;
        }
        smap_insert(info->options, option, field);
    }

    if (field->is_list && !field->separator) {
        print_error(field, "attribute " BQS " missing from field " BQS "of type " BQS, Attribute_name(F_separator), field->id, FieldType_name(FT_list));
        errors++;
    }

    if (field->enum_id) {
        Enum *enum_struct = (Enum *)smap_retrieve(info->enum_id, field->enum_id);
        if (!enum_struct) {
            print_error(field->enum_id, "enum type " BQS " is undefined", field->enum_id);
            errors++;
        }
    }

    // only now add defaults because we have enum values
    if (field->default_value) {
        errors += check_value(info, field->default_value, field, field->is_list);
    } else if (field->is_list) {
        field->default_value = create_field_value_array(create_array());
    } else if (field->type == FT_enum) {
        field->default_value = create_field_value_enum(array_get(smap_retrieve(info->enum_id, field->enum_id), 0));
    } else {
        switch (field->type) {
            case FT_uint:
            case FT_int:
                field->default_value = create_field_value_int(0);
                break;
            case FT_float:
                field->default_value = create_field_value_float(0);
                break;
            case FT_string:
                field->default_value = create_field_value_string(NULL);
                break;
            case FT_bool:
                field->default_value = create_field_value_bool(false);
                break;
        }
    }

    if (field->range) {
        errors += field->range->left_bound && check_value(info, field->range->left_bound, field, false);
        errors += field->range->right_bound && check_value(info, field->range->right_bound, field, false);
    }

    return errors;
}

int check_config(struct Info *info, Config *config) {
    int errors = 0;

    if (!config->fields) {
        print_error(config, "attribute " BQS " missing from config " BQS, Attribute_name(C_fields), config->id);
        return 1;
    }

    smap_t *smap = smap_init(32);

    for (size_t i = 0; i < array_size(config->fields); i++) {
        Field *field = (Field *)array_get(config->fields, i);
        Field *result = (Field *)smap_retrieve(smap, field->id);
        if (result) {
            print_error(field->id, "redefinition of field " BQS, field->id);
            print_note(result->id, "previous definition of " BQS " was here", field->id);
            errors++;
        }
        smap_insert(smap, field->id, field);
    }

    config->smap = smap;

    for (size_t i = 0; i < array_size(config->fields); i++) {
        Field *field = (Field *)array_get(config->fields, i);
        if (field->config) {
            errors += check_config(info, field->config);
        } else {
            errors += check_field(info, array_get(config->fields, i));
        }

    }

    return errors;
}

int check_setter(struct Info *info, Setter *setter) {
    Config *config = info->config;
    // everything before the end should be config, last should be field
    for (size_t i = 0; i < array_size(setter->ids) - 1; i++) {
        char *id = (char *)array_get(setter->ids, i);
        Field *field = (Field *)smap_retrieve(config->smap, id);
        if (!field) {
            print_error(id, "config " BQS " does not exist", id);
            return 1;
        }
        if (!field->config) {
            print_error(id, "field " BQS " is not a config", id);
            print_note(field, "field " BQS " defined here", id);
            return 1;
        }
        config = field->config;
    }

    char *id = (char *)array_get(setter->ids, array_size(setter->ids) - 1);
    Field *field = (Field *)smap_retrieve(config->smap, id);
    if (!field) {
        print_error(id, "field " BQS " does not exist", id);
        return 1;
    }
    if (field->config) {
        print_error(id, "config " BQS " is not a field", id);
        return 1;
    }

    setter->field = field;

    if (setter->value) {
        return check_value(info, setter->value, field, field->is_list);
    }

    return 0;
}

int check_multioption(struct Info *info, MultiOption *multioption) {
    int errors = 0;

    for (size_t i = 0; i < array_size(multioption->options); i++) {
        char *option = array_get(multioption->options, i);
        if (smap_retrieve(info->options, option)) {
            print_error(option, "duplicate option " BQS, option);
            print_note(smap_retrieve(info->options, option), "previous use of " BQS " was here", option);
            errors++;
        }
        smap_insert(info->options, option, option);
    }

    if (!multioption->fields) {
        print_error(multioption, "attribute " BQS " missing from multioption", Attribute_name(MO_fields));
        errors++;
        return errors;
    }

    Setter *first = NULL;
    for (size_t i = 0; i < array_size(multioption->fields); i++) {
        Setter *setter = (Setter *)array_get(multioption->fields, i);
        errors += check_setter(info, setter);
        if (setter->field && !setter->value) {
            if (!first) {
                first = setter;
            } else if (first->field->type != setter->field->type) {
                print_error(setter, "setter takes argument but field type does not match previously seen setter");
                print_note(first, "previous setter has type " BQS " but current setter has type " BQS, FieldType_name(first->field->type), FieldType_name(setter->field->type));
                errors++;
            }
        }
    }

    return errors;
}

int check_token(struct Info *info, Token *token) {
    int errors = 0;

    for (size_t i = 0; i < array_size(token->setters); i++) {
        Setter *setter = (Setter *)array_get(token->setters, i);
        errors += check_setter(info, setter);
        if (!setter->value) {
            print_error(setter, "valueless setter not allowed in optionset token");
            errors++;
        }
    }

    return errors;
}

int check_optionset(struct Info *info, OptionSet *optionset) {
    int errors = 0;

    for (size_t i = 0; i < array_size(optionset->options); i++) {
        char *option = array_get(optionset->options, i);
        if (smap_retrieve(info->options, option)) {
            print_error(option, "duplicate option " BQS, option);
            print_note(smap_retrieve(info->options, option), "previous use of " BQS " was here", option);
            errors++;
        }
        smap_insert(info->options, option, option);
    }

    if (!optionset->tokens) {
        print_error(optionset, "attribute " BQS " missing from optionset", Attribute_name(OS_tokens));
        errors++;
    }

    if (errors) {
        return errors;
    }

    bool sep_required = false;
    smap_t *smap = smap_init(32);

    for (size_t i = 0; i < array_size(optionset->tokens); i++) {
        Token *token = (Token *)array_get(optionset->tokens, i);
        if (smap_retrieve(smap, token->token)) {
            print_error(token->token, "duplicate token " BQS, token->token);
            print_note(smap_retrieve(smap, token->token), "previous use of " BQS " was here", token->token);
            errors++;
        }
        smap_insert(smap, token->token, token->token);
        sep_required = sep_required || strlen(token->token) > 1;

        errors += check_token(info, token);
    }

    smap_free(smap);

    if (sep_required && !optionset->separator) {
        print_error(optionset, "attribute " BQS " missing in optionset with token of length greater than 1", Attribute_name(OS_separator));
        errors++;
        return errors;
    }

    for (size_t i = 0; i < strlen(optionset->separator); i++) {
        if (optionset->separator[i] == ' ') {
            print_error(optionset->separator, "spaces not allowed in separator");
            errors++;
            break;
        }
    }

    return errors;
}

int check_targetoptions(struct Info *info, array *targetoptions) {
    int errors = 0;

    for (size_t i = 0; i < array_size(targetoptions); i++) {
        char *option = array_get(targetoptions, i);
        if (smap_retrieve(info->options, option)) {
            print_error(option, "duplicate option " BQS, option);
            print_note(smap_retrieve(info->options, option), "previous use of " BQS " was here", option);
            errors++;
        }
        smap_insert(info->options, option, option);
    }

    return errors;
}

int check_configuration(Configuration *configuration) {
    int errors = 0;

    struct Info *info = create_info();

    for (size_t i = 0; i < array_size(configuration->enums); i++) {
        errors += check_enum(info, array_get(configuration->enums, i));
    }

    if (configuration->config) {
        info->config = configuration->config;
        errors += check_config(info, configuration->config);
    }

    for (size_t i = 0; i < array_size(configuration->multioptions); i++) {
        errors += check_multioption(info, array_get(configuration->multioptions, i));
    }

    for (size_t i = 0; i < array_size(configuration->optionsets); i++) {
        errors += check_optionset(info, array_get(configuration->optionsets, i));
    }

    if (array_size(configuration->targetoptions)) {
        errors += check_targetoptions(info, configuration->targetoptions);
    }

    free_info(info);

    return errors;
}
