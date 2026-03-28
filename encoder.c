#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encoder.h"
#include "parser.h"

// Inverse Operator Mapping
const char* OP_STR[] = {
    "", "=", "<", "<=", ">", ">=", "!=", "*", "/", "%", "+", "-", "&", "^"
};

// --- HELPERS PARA O FORMATO TXT DO EMULADOR ---
// Converte o enum Format de volta para a string do TXT
const char* get_format_txt(Format format) {
    switch(format) {
        case FORMAT_SCORE: return "SCORE";
        case FORMAT_TIME_FRAMES: return "TIME";
        case FORMAT_TIME_CENTISECONDS: return "MILLISECS";
        case FORMAT_TIME_SECONDS: return "TIMESECS";
        case FORMAT_TIME_MINUTES: return "MINUTES";
        case FORMAT_TIME_SECONDS_AS_MINUTES: return "SECS_AS_MINS";
        case FORMAT_VALUE: return "VALUE";
        case FORMAT_VALUE_UNSIGNED: return "UNSIGNED";
        case FORMAT_VALUE_TENS: return "TENS";
        case FORMAT_VALUE_HUNDREDS: return "HUNDREDS";
        case FORMAT_VALUE_THOUSANDS: return "THOUSANDS";
        case FORMAT_VALUE_FIXED1: return "FIXED1";
        case FORMAT_VALUE_FIXED2: return "FIXED2";
        case FORMAT_VALUE_FIXED3: return "FIXED3";
        default: return "VALUE";
    }
}

// Converte o Achievement_type para os IDs numéricos usados pelo RAIntegration
int get_achievement_type_txt(Achievement_type type) {
    switch(type) {
        case ACHIEVEMENT_TYPE_NONE: return 0; // Fica vazio no TXT
        case ACHIEVEMENT_TYPE_PROGRESSION: return 3;
        case ACHIEVEMENT_TYPE_WIN_CONDITION: return 4;
        case ACHIEVEMENT_TYPE_MISSABLE: return 5;
        default: return 0;
    }
}

int encode_numeral(char *buffer, size_t max_size, struct NUMERAL *num) {
    if (num->type == TYPE_VALUE) {
        return snprintf(buffer, max_size, "%d", num->value);
    }
    if (num->type == TYPE_RECALL) {
        return snprintf(buffer, max_size, "{recall}");
    }
    if (num->type == TYPE_FLOAT) {
        float f;
        memcpy(&f, &num->value, sizeof(float));
        // %g omits unnecessary decimal zeros
        return snprintf(buffer, max_size, "f%g", f); 
    }

    int offset = 0;

    // 1. Type Prefix (Delta 'd', Prior 'p', BCD 'b', Invert '~')
    // Note: TYPE_DELTA is value 2 in the enum, so the index is type - 2
    if (num->type >= TYPE_DELTA && num->type <= TYPE_INVERT && num->type != TYPE_FLOAT) {
        offset += snprintf(buffer + offset, max_size - offset, "%c", char_type[num->type - 2]);
    }

    // 2. Memory Prefix
    // Float-type memories start with 'f'. Others start with '0x'
    int is_float_size = (num->size >= SIZE_FLOAT && num->size <= SIZE_MBF32_LE);
    if (is_float_size) {
        offset += snprintf(buffer + offset, max_size - offset, "f");
    } else {
        offset += snprintf(buffer + offset, max_size - offset, "0x");
    }

    // 3. Size
    // SIZE_BIT0 is value 1 in the enum, so the index is size - 1
    if (num->size != SIZE_NONE) {
        char s_char = char_size[num->size - 1];
        // We omit the character for SIZE_WORD (' ') to keep the string cleaner ("0x1234" vs "0x 1234")
        offset += snprintf(buffer + offset, max_size - offset, "%c", s_char);
    }

    // 4. Address / Value (Standard Hexadecimal format with a minimum of 4 digits)
    offset += snprintf(buffer + offset, max_size - offset, "%04x", num->value);

    return offset;
}

int encode_condition(char *buffer, size_t max_size, struct CONDITION *cond) {
    int offset = 0;

    // Add the Flag (e.g., "R:")
    if (cond->flag != FLAG_NONE) {
        offset += snprintf(buffer + offset, max_size - offset, "%c:", char_flag[cond->flag - 1]);
    }

    // Left Hand Side (LHS)
    offset += encode_numeral(buffer + offset, max_size - offset, &cond->lhs);

    // Operator and Right Hand Side (RHS)
    if (cond->op != OP_NONE) {
        offset += snprintf(buffer + offset, max_size - offset, "%s", OP_STR[cond->op]);
        offset += encode_numeral(buffer + offset, max_size - offset, &cond->rhs);

        // Hit Target
        if (cond->hit_target > 0) {
            offset += snprintf(buffer + offset, max_size - offset, ".%d.", cond->hit_target);
        }
    }

    return offset;
}

int encode_group(char *buffer, size_t max_size, struct GROUP *group) {
    int offset = 0;
    struct CONDITION *head = group->condition_head;
    while (head) {
        if (head->prev != NULL) {
            offset += snprintf(buffer + offset, max_size - offset, "%c", condition_separator);
        }
        offset += encode_condition(buffer + offset, max_size - offset, head);
        head = head->next;
    }
    return offset;
}

char* encode_logic(struct ACHIEVEMENT_LOGIC *logic) {
    if (!logic || logic->group_head == NULL) return NULL;

    // Allocate 4KB by default (sufficient for the vast majority of achievements)
    size_t buf_size = 4096;
    char *buffer = malloc(buf_size);
    if (!buffer) return NULL;

    buffer[0] = '\0';
    int offset = 0;

    struct GROUP *head = logic->group_head;
    while (head) {
        if (head->prev != NULL) {
            // Separate Alternate Groups using 'S'
            offset += snprintf(buffer + offset, buf_size - offset, "%c", group_separator);
        }
        offset += encode_group(buffer + offset, buf_size - offset, head);
        head = head->next;
    }

    return buffer;
}

char* encode_leaderboard(struct LEADERBOARD *leaderboard) {
    if (!leaderboard) return NULL;

    // Encode the 4 parts using your existing function
    char *start_str = encode_logic(leaderboard->start);
    char *cancel_str = encode_logic(leaderboard->cancel);
    char *submit_str = encode_logic(leaderboard->submit);
    char *value_str = encode_logic(leaderboard->value);

    // Protection: if any logic returns NULL, we treat it as an empty string ""
    const char *s_str = start_str ? start_str : "";
    const char *c_str = cancel_str ? cancel_str : "";
    const char *sub_str = submit_str ? submit_str : "";
    const char *v_str = value_str ? value_str : "";

    // Calculate the exact size needed for the final string
    // Tag sizes (STA:=4, ::CAN:=6, ::SUB:=6, ::VAL:=6) -> 22 bytes
    // + the size of each string + 1 byte for the final '\0'
    size_t total_len = 22 + strlen(s_str) + strlen(c_str) + strlen(sub_str) + strlen(v_str) + 1;

    // Allocate the exact memory
    char *buffer = malloc(total_len);
    if (buffer) {
        snprintf(buffer, total_len, "STA:%s::CAN:%s::SUB:%s::VAL:%s", 
                 s_str, c_str, sub_str, v_str);
    }

    // Free the intermediate strings that were allocated by encode_logic
    if (start_str) free(start_str);
    if (cancel_str) free(cancel_str);
    if (submit_str) free(submit_str);
    if (value_str) free(value_str);

    return buffer;
}

// --- EXPORT FUNCTIONS (.TXT) ---

char* encode_leaderbaord_txt(struct LEADERBOARD *leaderboard) {
    if (!leaderboard) return NULL;

    // Generates the logic using the base functions
    char *start_str = encode_logic(leaderboard->start);
    char *cancel_str = encode_logic(leaderboard->cancel);
    char *submit_str = encode_logic(leaderboard->submit);
    char *value_str = encode_logic(leaderboard->value);

    // Protection against nulls
    const char *s_str = start_str ? start_str: "";
    const char *c_str = cancel_str ? cancel_str: "";
    const char *sub_str = submit_str ? submit_str: "";
    const char *v_str = value_str ? value_str: "";

    const char *format_str = get_format_txt(leaderboard->format);
    const char *title = leaderboard->title ? leaderboard->title : "";
    const char *desc = leaderboard->description ? leaderboard->description : "";

    // Calculate size for the string: L_ID:"Start":"Cancel":"Submit":"Value":FORMAT:Title:Description:LowerIsBetter
    size_t total_len = 50 + strlen(s_str) + strlen(c_str) + strlen(sub_str) + strlen(v_str) +
                       strlen(format_str) + strlen(title) + strlen(desc);

    char *buffer = malloc(total_len);
    if (buffer) {
        snprintf(buffer, total_len, "L%d:\"%s\":\"%s\":\"%s\":\"%s\":%s:%s:%s:%d",
                 leaderboard->id, s_str, c_str, sub_str, v_str, 
                 format_str, title, desc, leaderboard->lower_is_better);
    }

    // Free up intermediate memory
    if (start_str) free(start_str);
    if (cancel_str) free(cancel_str);
    if (submit_str) free(submit_str);
    if (value_str) free(value_str);

    return buffer;

}

char* encode_achievement_txt(struct ACHIEVEMENT *achievement) {
    if (!achievement) return NULL;

    char *logic_str = encode_logic(achievement->logic);
    const char *l_str = logic_str ? logic_str : "";
    const char *title = achievement->title ? achievement->title : "";
    const char *desc = achievement->description ? achievement->description : "";
    int type_val = get_achievement_type_txt(achievement->type);

    // Calculate size: ID:"Logic":"Title":"Description":::Type:Author:Points:::::Badge
    size_t total_len = 60 + strlen(l_str) + strlen(title) + strlen(desc);

    char *buffer = malloc(total_len);
    if (buffer) {
        // If the type is 0 (None), RA leaves this field completely empty (::::)
        // We set the "Author" to Unknown and the default "Badge" to 00000 to be valid in the emulator.
        if (type_val == 0) {
            snprintf(buffer, total_len, "%d:\"%s\":\"%s\":\"%s\"::::Unknown:%d:::::00000",
                     achievement->id, l_str, title, desc, achievement->points);
        } else {
            snprintf(buffer, total_len, "%d:\"%s\":\"%s\":\"%s\":::%d:Unknown:%d:::::00000",
                     achievement->id, l_str, title, desc, type_val, achievement->points);
        }
    }

    if (logic_str) free(logic_str);

    return buffer;
}