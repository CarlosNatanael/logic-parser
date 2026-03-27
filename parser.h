#ifndef PARSER_H
#define PARSER_H

#include "achievement.h"
#include "cJSON.h"

#define MINIMUM_LEADERBOARD_LENGTH 34 // STA:0=0::CAN:0=0::SUB:0=0::VAL:M:0
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

struct ACHIEVEMENT_LOGIC *get_achievement(char achievement[], size_t len);

struct LEADERBOARD *get_leaderboard(char leaderboard[], size_t len);

// json parsing status:
#define SUCCESS 0
#define UNKNOWN_ACHIEVEMENT_TYPE 1
#define UNKNOWN_LEADERBOARD_FORMAT 2
#define UNKNOWN_SET_TYPE 3
#define WRONG_JSON_OBJECT_TYPE 4

struct ACHIEVEMENT *get_achievement_from_json(const cJSON *json_achievement, int *status);
struct LEADERBOARD *get_leaderboard_from_json(const cJSON *json_leaderboard, int *status);
struct ACHIEVEMENT_SET *get_achievement_set_from_json(const cJSON *achievement_set, int *status);
struct GAME *get_game_from_json(char *path);

struct ACHIEVEMENT_SET *get_achievement_set_from_txt(char *path);
struct ACHIEVEMENT_SET *get_achievement_set_from_web(int id);

struct ACHIEVEMENT_SET *combine_sets(struct ACHIEVEMENT_SET **sets);


void free_numeral(struct NUMERAL *numeral);
void free_condition(struct CONDITION *condition);
void free_group(struct GROUP *group);
void free_achievement(struct ACHIEVEMENT *achievement);
void free_leaderboard(struct LEADERBOARD *leaderboard);
void free_achievement_set(struct ACHIEVEMENT_SET *achievement_set);

#endif // !PARSER_H
