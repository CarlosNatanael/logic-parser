#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "parser.h"
#include "achievement.h"
#include "cJSON.h"

const char condition_separator = '_';
const char group_separator = 'S';
const char char_flag[] =
  {
    'P',    // Pause If
    'R',    // Reset If
    'Z',    // Reset Next If
    'A',    // Add Source
    'B',    // Sub Source
    'C',    // Add Hits
    'D',    // Sub Hits
    'I',    // Add Address
    'N',    // And Next
    'O',    // Or Next
    'M',   // Measured
    'Q',   // Measured If
    'T',   // Trigger
    'K',   // Remember
  };
const char char_type[] =
  {
    'd',    // Delta
    'p',    // Prior
    'b',    // BCD
    'f',    // Float
    '~',    // Invert
  };
const char char_size[] =
  {
    'M',    // Bit0
    'N',    // Bit1
    'O',    // Bit2
    'P',    // Bit3
    'Q',    // Bit4
    'R',    // Bit5
    'S',    // Bit6
    'T',    // Bit7
    'L',    // Lower4
    'U',    // Upper4
    'H',   // 8-bit
    ' ',   // 16-bit
    'W',   // 24-bit
    'X',   // 32-bit
    'I',   // 16-bit BE
    'J',   // 24-bit BE
    'G',   // 32-bit BE
    'K',   // BitCount
     // requires Float type
    'F',   // Float
    'B',   // Float BE
    'H',   // Double32
    'I',   // Double32 BE
    'M',   // MBF32
    'L'    // MBF32 BE
  };

int is_recall(char *numeral, size_t len)
{
  return numeral[0] == '{';
}

Type get_type(char *numeral, size_t len)
{
  // Value is the only type that can be this length
  if (len <= 2) return TYPE_VALUE;
  if (is_recall(numeral, len)) return TYPE_RECALL;
  // Mem are written like 0xABCDEF so numeral[1] must be 'x'
  if (numeral[1] == 'x') return TYPE_MEM;
  // Floats are either f4.5 or fFABCDEF. first one is of type Float, second is Mem
  if (numeral[0] == 'f')
  {
    for (int i = SIZE_REQUIRE_TYPE_FLOAT; i < sizeof(char_size); i++)
      if (char_size[i] == numeral[1]) return TYPE_MEM;
    return TYPE_FLOAT;
  }

  // in any other case, the type is the first character of the numeral, like d0xABCDEF or ~0xABCDEF
  for (int i = 0; i < sizeof(char_type); i ++)
    if (char_type[i] == numeral[0]) return i + 2;

  return TYPE_VALUE;
}

Size get_size(char *numeral, size_t len, Type type)
{
  // Value, Recall, and Floats do not hold any size
  if (type == TYPE_VALUE || type == TYPE_RECALL || type == TYPE_FLOAT) return SIZE_NONE;

  int is_float =
    (type == TYPE_MEM && numeral[0] == 'f') ||
    (type != TYPE_MEM && numeral[1] == 'f');

  int index = 1;
  index = index + (type != TYPE_MEM);
  index = index + (!is_float);

  for (int i = 0; i < sizeof(char_size); i ++)
  {
    if (i < SIZE_REQUIRE_TYPE_FLOAT && is_float == 0 && char_size[i] == numeral[index]) return i + 1;
    if (i >= SIZE_REQUIRE_TYPE_FLOAT && is_float == 1 && char_size[i] == numeral[index]) return i + 1;
  }
  return SIZE_WORD; // 16-bit is ' ' but can also be '';
}

long parse_float(char *numeral)
{
  char *start = numeral + 1;

  char *end;
  float f = strtof(start, &end);

  // TODO: add some check with end for error handling

  uint32_t output;
  memcpy(&output, &f, sizeof(f));

  return output;
}

long parse_value(char *numeral)
{
  if (numeral[0] == 'h')
  {
    char *start = numeral + 1;
    char *end;

    long n = strtol(start, &end, 16);
    return n;
  }
  else
  {
    char *start = numeral;
    char *end;

    long n = strtol(start, &end, 10);
    return n;
  }
}

long parse_address(struct NUMERAL *input, char *numeral)
{
  int8_t starting_index = 0;
  if (input->size >= SIZE_FLOAT)
    starting_index = 2; // f_ABCDEF
  else if (input->size == SIZE_WORD && numeral[2] != ' ')
    starting_index = 2; // 0xABCDEF
  else
    starting_index = 3; // 0x_ABCDEF

  if (input->type != TYPE_MEM) starting_index ++; // _f_ABCDEF or _0xABCDEF or _0x_ABCDEF

  char *start = numeral + starting_index;
  char *end;

  long address = strtol(start, &end, 16);
  return address;
}

struct NUMERAL *get_numeral(char *numeral, size_t len)
{
  if (len == 0) return NULL;

  struct NUMERAL *output = malloc(sizeof(struct NUMERAL));

  output->type = get_type(numeral, len);
  output->size = get_size(numeral, len, output->type);

  switch (output->type)
  {
    case TYPE_RECALL:
      // ignore output->value
      break;
    case TYPE_FLOAT:
      output->value = parse_float(numeral);
      break;
    case TYPE_VALUE:
      output->value = parse_value(numeral);
      break;
    default:
      output->value = parse_address(output, numeral);
      break;
  }
  return output;
}

Flag get_flag(char *condition)
{
  if (condition[1] == ':')
  {
    for (int i = 0; i < sizeof(char_flag); i ++)
      if (char_flag[i] == condition[0]) return i + 1;
    return FLAG_NONE; // NOTE: should handle it as an error here instead of just returning none
  }
  else return FLAG_NONE;
}

Operator get_op(char *condition, size_t len, int *index)
{
  for(int i = 0; i < len; i ++)
    switch (condition[i])
    {
      case '=':
        *index = i;
        return OP_EQUAL;
        break;
      case '<':
        *index = i;
        if (i < len - 1 && condition[i + 1] == '=')
        {
          *index = *index + 1;
          return OP_INF_EQUAL;
        }
        else
          return OP_INF;
        break;
      case '>':
        *index = i;
        if (i < len - 1 && condition[i + 1] == '=')
        {
          *index = *index + 1;
          return OP_SUP_EQUAL;
        }
        else
          return OP_SUP;
        break;
      case '!':
        *index = i + 1;
        return OP_NOT_EQUAL;
        break;
      case '*':
        *index = i;
        return OP_FACTOR;
        break;
      case '/':
        *index = i;
        return OP_QUOTIENT;
        break;
      case '%':
        *index = i;
        return OP_MOD;
        break;
      case '+':
        *index = i;
        return OP_PLUS;
        break;
      case '-':
        if (i == 0 || condition[i - 1] == 'f')
          break;
        else
        {
          *index = i;
          return OP_MINUS;
        }
      case '&':
        *index = i;
        return OP_AND;
        break;
      case '^':
        *index = i;
        return OP_XOR;
        break;
    }
  *index = len;
  return OP_NONE;
}

long get_hit_target(char *condition, size_t len)
{
  if(condition[len - 1] != '.')
    return 0;
  else
  {
    char *start = condition + len - 1;
    char *end;

    // NOTE: should handle case where len < 3
    for (int i = len - 3; i >= 0; i --)
    {
      if (condition[i] == '.') start = condition + i + 1;
    }
    return strtol(start, &end, 10);
  }
}

struct CONDITION *get_condition(char *condition, size_t len)
{
  struct CONDITION *output = malloc(sizeof(struct CONDITION));
  int op_index;
  output->flag = get_flag(condition);
  output->op = get_op(condition, len, &op_index);
  output->hit_target = get_hit_target(condition, len);

  int lhs_start = (output->flag != FLAG_NONE) * 2;
  int lhs_end = op_index;
  int lhs_len = lhs_end - lhs_start;
  char *lhs = malloc(lhs_len + 1);
  memcpy(lhs, condition + lhs_start, lhs_len);
  lhs[lhs_len] = '\0';
  output->lhs = *get_numeral(lhs, lhs_len);
  free(lhs);

  if (output-> op == OP_NONE)
  {
    output->rhs.type = TYPE_VALUE;
    output->rhs.size = SIZE_NONE;
    output->rhs.value = 0;
    output->hit_target = 0;
    return output;
  }

  int rhs_start = op_index + 1;
  int rhs_end = len;
  if (output->hit_target != 0)
  {
    rhs_end = rhs_end - 2;
    rhs_end = rhs_end - ((int)floor(log10(output->hit_target)) + 1);
  }
  int rhs_len = rhs_end - rhs_start;
  char *rhs = malloc(rhs_len + 1);
  memcpy(rhs, condition + rhs_start, rhs_len);
  rhs[rhs_len] = '\0';
  output->rhs = *get_numeral(rhs, rhs_len);
  free(rhs);

  return output;
}

struct GROUP *get_group(char *group, size_t len)
{
  size_t max_condition = 1;
  for (int i = 0; i < len; i ++)
    if (group[i] == condition_separator) max_condition ++;

  struct GROUP *output = malloc(sizeof(struct GROUP) + max_condition * sizeof(struct CONDITION *));

  int current_id = 0;
  int last_separator_index = 0;
  for (int i=0; i < len; i ++)
  {
    if (group[i] != condition_separator && i != len - 1) continue;

    int condition_len = i - last_separator_index;

    if (i == len - 1 && group[i] != condition_separator) condition_len++;

    char *condition_str = malloc(condition_len + 1);

    memcpy(condition_str, group + last_separator_index, condition_len);
    condition_str[condition_len] = '\0';

    struct CONDITION *condition = get_condition(condition_str, condition_len);
    free(condition_str);

    condition->id = current_id + 1; // lines ID are 1 indexed to stay consistent with RAInt
    output->conditions[current_id] = condition;
    current_id ++;

    last_separator_index = i + 1;
  }

  output->condition_count = max_condition;
  return output;
}

struct ACHIEVEMENT *get_achievement(char *achievement, size_t len)
{
  size_t max_group = 1;
  for (int i = 1; i < len; i ++)
    if (achievement[i] == group_separator && achievement[i - 1] != 'x') max_group ++;

  struct ACHIEVEMENT *output = malloc(sizeof(struct ACHIEVEMENT) + sizeof(struct ACHIEVEMENT_LOGIC *));

  int current_id = 0;
  int last_separator_index = 0;
  for (int i = 1; i < len; i ++)
  {
    if (achievement[i] != group_separator && i < len - 1) continue;
    if (achievement[i - 1] == 'x') continue; // 'S' is also a size marker for Bit6

    size_t group_len = i - last_separator_index;

    if (achievement[i] != group_separator && i == len - 1) group_len ++;

    char *group_str = malloc(group_len + 1);

    memcpy(group_str, achievement + last_separator_index, group_len);
    group_str[group_len] = '\0';

    struct GROUP *group = get_group(group_str, group_len);
    free(group_str);

    group->id = current_id;

    output->logic = malloc(sizeof(struct ACHIEVEMENT_LOGIC) + sizeof(struct GROUP *) * max_group);

    output->logic->groups[current_id] = group;
    current_id++;

    last_separator_index = i + 1;
  }

  output->logic->group_count = max_group;
  return output;
}


// NOTE: this was made quickly, a rework of it wouldn't hurt.
struct LEADERBOARD *get_leaderboard(char *leaderboard, size_t len)
{
  struct LEADERBOARD *output = malloc(sizeof(struct LEADERBOARD));

  int start_index = 4;
  int start_len;
  int cancel_index = 0;
  int cancel_len;
  int submit_index = 0;
  int submit_len;
  int value_index = 0;
  int value_len;

  if (len < MINIMUM_LEADERBOARD_LENGTH)
  {free(output); return NULL;} // TODO: add diag "leaderboard length is too small : $len"
  if (strncmp(leaderboard, "STA:", 4) != 0)
  {free(output); return NULL;} // TODO: add diag "leaderboard does not start with the START global_group"

  for (int i = 4; i < len - 6; i ++)
  {
    if (strncmp(leaderboard + i, "::CAN:", 6) == 0)
    {
      cancel_index = i + 6;
      start_len = i - start_index;
    }
    else if (strncmp(leaderboard + i, "::SUB:", 6) == 0)
    {
      submit_index = i + 6;
      if (cancel_index == 0)
      {free(output); return NULL;} // TODO: add diag "wrong order of global_groups in leaderboard"
      cancel_len = i - cancel_index;
    }
    else if (strncmp(leaderboard + i, "::VAL:", 6) == 0)
    {
      value_index = i + 6;
      if (submit_index == 0)
      {free(output); return NULL;} // TODO: add diag "wrong order of global_groups in leaderboard"
      submit_len = i - submit_index;
    }
  }

  if (value_index == 0)
  {free(output); return NULL;} // TODO: add diag "no VALUE global_group found in leaderboard"
  value_len = len - value_index;

  char *start_str = malloc(start_len + 1);
  memcpy(start_str, leaderboard + start_index, start_len);
  start_str[start_len] = '\0';

  char *cancel_str = malloc(cancel_len + 1);
  memcpy(cancel_str, leaderboard + cancel_index, cancel_len);
  cancel_str[cancel_len] = '\0';

  char *submit_str = malloc(submit_len + 1);
  memcpy(submit_str, leaderboard + submit_index, submit_len);
  submit_str[submit_len] = '\0';

  char *value_str = malloc(value_len + 1);
  memcpy(value_str, leaderboard + value_index, value_len);
  value_str[value_len] = '\0';

  output->start = get_achievement(start_str, start_len)->logic;
  output->cancel = get_achievement(cancel_str, cancel_len)->logic;
  output->submit = get_achievement(submit_str, submit_len)->logic;
  output->value = get_achievement(value_str, value_len)->logic;

  free(start_str);
  free(cancel_str);
  free(submit_str);
  free(value_str);

  return output;
}

struct ACHIEVEMENT *get_achievement_from_json(const cJSON *json_achievement, int *status)
{
  const cJSON *id = cJSON_GetObjectItemCaseSensitive(json_achievement, "ID");
  const cJSON *logic = cJSON_GetObjectItemCaseSensitive(json_achievement, "MemAddr");
  const cJSON *title = cJSON_GetObjectItemCaseSensitive(json_achievement, "Title");
  const cJSON *description = cJSON_GetObjectItemCaseSensitive(json_achievement, "Description");
  const cJSON *points = cJSON_GetObjectItemCaseSensitive(json_achievement, "Points");
  const cJSON *type = cJSON_GetObjectItemCaseSensitive(json_achievement, "Type");

  if
  (
    cJSON_IsNumber(id) == 0 ||
    cJSON_IsString(logic) == 0 ||
    cJSON_IsString(title) == 0 ||
    cJSON_IsString(description) == 0 ||
    cJSON_IsNumber(points) == 0 ||
    (cJSON_IsString(type) == 0 && cJSON_IsNull(type) == 0)
  )
  {
    *status = WRONG_JSON_OBJECT_TYPE;
    return NULL;
  }

  struct ACHIEVEMENT *achievement = get_achievement(logic->valuestring, strlen(logic->valuestring));
  achievement->id = id->valueint;

  achievement->title = malloc(strlen(title->valuestring) + 1);
  memcpy(achievement->title, title->valuestring, strlen(title->valuestring));
  achievement->title[strlen(title->valuestring)] = '\0';

  achievement->description = malloc(strlen(description->valuestring) + 1);
  memcpy(achievement->description, description->valuestring, strlen(description->valuestring));
  achievement->description[strlen(description->valuestring)] = '\0';

  achievement->points = points->valueint;

  if (cJSON_IsNull(type)) achievement->type = ACHIEVEMENT_TYPE_NONE;
  else if (strcmp(type->valuestring, "progression") == 0) achievement->type = ACHIEVEMENT_TYPE_PROGRESSION;
  else if (strcmp(type->valuestring, "win_condition") == 0) achievement->type = ACHIEVEMENT_TYPE_WIN_CONDITION;
  else if (strcmp(type->valuestring, "missable") == 0) achievement->type = ACHIEVEMENT_TYPE_MISSABLE;
  else
  {
    free(achievement);
    *status = UNKNOWN_ACHIEVEMENT_TYPE;
    return NULL;
  }

  return achievement;
}

struct LEADERBOARD *get_leaderboard_from_json(const cJSON *json_leaderboard, int *status)
{
  const cJSON *id = cJSON_GetObjectItemCaseSensitive(json_leaderboard, "ID");
  const cJSON *logic = cJSON_GetObjectItemCaseSensitive(json_leaderboard, "Mem");
  const cJSON *format = cJSON_GetObjectItemCaseSensitive(json_leaderboard, "Format");
  const cJSON *lower_is_better = cJSON_GetObjectItemCaseSensitive(json_leaderboard, "LowerIsBetter");
  const cJSON *title = cJSON_GetObjectItemCaseSensitive(json_leaderboard, "Title");
  const cJSON *description = cJSON_GetObjectItemCaseSensitive(json_leaderboard, "Description");

  if
  (
    cJSON_IsNumber(id) == 0 ||
    cJSON_IsString(logic) == 0 ||
    cJSON_IsString(format) == 0 ||
    cJSON_IsBool(lower_is_better) == 0 ||
    cJSON_IsString(title) == 0 ||
    cJSON_IsString(description) == 0
  )
  {
    *status = WRONG_JSON_OBJECT_TYPE;
    return NULL;
  }

  struct LEADERBOARD *leaderboard = get_leaderboard(logic->valuestring, strlen(logic->valuestring));
  leaderboard->id = id->valueint;

  leaderboard->title = malloc(strlen(title->valuestring) + 1);
  memcpy(leaderboard->title, title->valuestring, strlen(title->valuestring) + 1);

  leaderboard->description = malloc(strlen(description->valuestring) + 1);
  memcpy(leaderboard->description, description->valuestring, strlen(description->valuestring) + 1);

  leaderboard->lower_is_better = cJSON_IsTrue(lower_is_better);
 
  const char *format_str = format->valuestring;

  if (strcmp(format_str, "SCORE") == 0) leaderboard->format = FORMAT_SCORE;
  else if (strcmp(format_str, "TIME") == 0) leaderboard->format = FORMAT_TIME_FRAMES;
  else if (strcmp(format_str, "MILLISECS") == 0) leaderboard->format = FORMAT_TIME_CENTISECONDS;
  else if (strcmp(format_str, "TIMESECS") == 0) leaderboard->format = FORMAT_TIME_SECONDS;
  else if (strcmp(format_str, "MINUTES") == 0) leaderboard->format = FORMAT_TIME_MINUTES;
  else if (strcmp(format_str, "SECS_AS_MINS") == 0) leaderboard->format = FORMAT_TIME_SECONDS_AS_MINUTES;
  else if (strcmp(format_str, "VALUE") == 0) leaderboard->format = FORMAT_VALUE;
  else if (strcmp(format_str, "UNSIGNED") == 0) leaderboard->format = FORMAT_VALUE_UNSIGNED;
  else if (strcmp(format_str, "TENS") == 0) leaderboard->format = FORMAT_VALUE_TENS;
  else if (strcmp(format_str, "HUNDREDS") == 0) leaderboard->format = FORMAT_VALUE_HUNDREDS;
  else if (strcmp(format_str, "THOUSANDS") == 0) leaderboard->format = FORMAT_VALUE_THOUSANDS;
  else if (strcmp(format_str, "FIXED1") == 0) leaderboard->format = FORMAT_VALUE_FIXED1;
  else if (strcmp(format_str, "FIXED2") == 0) leaderboard->format = FORMAT_VALUE_FIXED2;
  else if (strcmp(format_str, "FIXED3") == 0) leaderboard->format = FORMAT_VALUE_FIXED3;
  else
  {
    free(leaderboard);
    *status = UNKNOWN_LEADERBOARD_FORMAT;
    return NULL;
  }

  return leaderboard;
}

struct ACHIEVEMENT_SET *get_achievement_set_from_json(const cJSON *achievement_set, int *status)
{
  const cJSON *type = NULL;
  const cJSON *json_achievements = NULL;
  const cJSON *json_achievement = NULL;
  const cJSON *json_leaderboards = NULL;
  const cJSON *json_leaderboard = NULL;

  type = cJSON_GetObjectItemCaseSensitive(achievement_set, "Type");

  json_achievements = cJSON_GetObjectItemCaseSensitive(achievement_set, "Achievements");
  int achievement_count = cJSON_GetArraySize(json_achievements);
  struct ACHIEVEMENT **achievements = malloc(sizeof(struct ACHIEVEMENT *) * achievement_count);

  json_leaderboards = cJSON_GetObjectItemCaseSensitive(achievement_set, "Leaderboards");
  int leaderboard_count = cJSON_GetArraySize(json_leaderboards);
  struct LEADERBOARD **leaderboards = malloc(sizeof(struct LEADERBOARD *) * leaderboard_count);

  if
  (
    cJSON_IsString(type) == 0 ||
    cJSON_IsArray(json_achievements) == 0 ||
    cJSON_IsArray(json_leaderboards) == 0
  )
  {
    *status = WRONG_JSON_OBJECT_TYPE;
    return NULL;
  }

  struct ACHIEVEMENT_SET *output = malloc(sizeof(struct ACHIEVEMENT_SET));

  int index = 0;
  cJSON_ArrayForEach(json_achievement, json_achievements)
  {
    achievements[index] = get_achievement_from_json(json_achievement, status);
    if (achievements[index] == NULL)
    {
      return NULL;
    }
    index ++;
  }

  index = 0;
  cJSON_ArrayForEach(json_leaderboard, json_leaderboards)
  {
    leaderboards[index] = get_leaderboard_from_json(json_leaderboard, status);
    if (leaderboards[index] == NULL)
    {
      return NULL;
    }
    index ++;
  }

  if (strcmp(type->valuestring, "core") == 0) output->type = SET_CORE;
  else if (strcmp(type->valuestring, "bonus") == 0) output->type = SET_SUBSET;
  else
  {
    *status = UNKNOWN_SET_TYPE;
    return NULL;
  }

  output->achievement_count = achievement_count;
  output->achievements = achievements;

  output->leaderboard_count = leaderboard_count;
  output->leaderboards = leaderboards;

  return output;
}

struct GAME *get_game_from_json(char *path)
{
  int status = SUCCESS;

  FILE *json_ptr = fopen(path, "r");
  if (json_ptr == NULL)
  {
    // TODO: add diagnostic
    return NULL;
  }

  fseek(json_ptr, 0, SEEK_END);
  long int json_len = ftell(json_ptr);
  rewind(json_ptr);

  char *json_str = malloc(json_len + 1);
  fread(json_str, 1, json_len, json_ptr);
  json_str[json_len] = '\0';
  fclose(json_ptr);

  cJSON *json = cJSON_Parse(json_str);
  if (json == NULL)
  {
    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr != NULL)
    {
      // TODO: add diagnostic
    }
    return NULL;
  }

  const cJSON *gameID = NULL;
  const cJSON *title = NULL;
  const cJSON *consoleID = NULL;

  const cJSON *achievement_sets = NULL;
  const cJSON *achievement_set = NULL;

  gameID = cJSON_GetObjectItemCaseSensitive(json, "GameId");
  title = cJSON_GetObjectItemCaseSensitive(json, "Title");
  consoleID = cJSON_GetObjectItemCaseSensitive(json, "ConsoleId");
  achievement_sets = cJSON_GetObjectItemCaseSensitive(json, "Sets");
  if
  (
    cJSON_IsNumber(gameID) == 0 ||
    cJSON_IsString(title) == 0 ||
    cJSON_IsNumber(consoleID) == 0 ||
    cJSON_IsArray(achievement_sets) == 0
  )
  {
    status = WRONG_JSON_OBJECT_TYPE;
    goto end;
  }
  printf("%d| %s   nb of sets : %d\n", gameID->valueint, title->valuestring, cJSON_GetArraySize(achievement_sets));

  int set_count = cJSON_GetArraySize(achievement_sets);
  struct GAME *output = malloc(sizeof(struct GAME) + sizeof(struct ACHIEVEMENT_SET *) * set_count);

  int index = 0;
  cJSON_ArrayForEach(achievement_set, achievement_sets)
  {
    output->sets[index] = get_achievement_set_from_json(achievement_set, &status);
    if (output->sets[index] == NULL) goto end;
    index ++;
  }

  output->id = gameID->valueint;
  output->consoleID = consoleID->valueint;

  output->title = malloc(strlen(title->valuestring) + 1);
  memcpy(output->title, title->valuestring, strlen(title->valuestring));
  output->title[strlen(title->valuestring)] = '\0';
end:

  // TODO: something with status code for error diagnostic
  cJSON_Delete(json);
  if (status != SUCCESS) return NULL;
  printf("Game parsed successfully\n");
  return output;
}
