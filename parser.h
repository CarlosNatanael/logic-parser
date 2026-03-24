#ifndef PARSER_H
#define PARSER_H

#include "achievement.h"

#define MINIMUM_LEADERBOARD_LENGTH 34
#define SIZE_REQUIRE_TYPE_FLOAT 18

extern const char condition_separator;
extern const char group_separator;
extern const char char_flag[];
extern const char char_type[];
extern const char char_size[];

int is_recall(char numeral[], size_t len);

Type get_type(char numeral[], size_t len);
Size get_size(char numeral[], size_t len, Type type);

long parse_float(char numeral[]);
long parse_value(char numeral[]);
long parse_address(struct NUMERAL *input, char numeral[]);

struct NUMERAL *get_numeral(char numeral[], size_t len);

Flag get_flag(char condition[]);
Operator get_op(char condition[], size_t len, int *index);
long get_hit_target(char condition[], size_t len);
struct CONDITION *get_condition(char condition[], size_t len);

struct GROUP *get_group(char group[], size_t len);
struct ACHIEVEMENT *get_achievement(char achievement[], size_t len);

struct LEADERBOARD *get_leaderboard(char leaderboard[], size_t len);

#endif // !PARSER_H
