#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encoder.h"
#include "parser.h"

// Inverse Operator Mapping
const char* OP_STR[] = {
    "", "=", "<", "<=", ">", ">=", "!=", "*", "/", "%", "+", "-", "&", "^"
};

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