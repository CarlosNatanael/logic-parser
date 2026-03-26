#ifndef ACHIEVEMENT_H
#define ACHIEVEMENT_H

#include "RA_Consoles.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum
{
  FLAG_NONE,
  FLAG_PAUSE_IF,
  FLAG_RESET_IF,
  FLAG_RESET_NEXT_IF,
  FLAG_ADD_SOURCE,
  FLAG_SUB_SOURCE,
  FLAG_ADD_HITS,
  FLAG_SUB_HITS,
  FLAG_ADD_ADDRESS,
  FLAG_AND_NEXT,
  FLAG_OR_NEXT,
  FLAG_MEASURED,
  FLAG_MEASURED_IF,
  FLAG_TRIGGER,
  FLAG_REMEMBER
} Flag;

typedef enum
{
  TYPE_MEM,
  TYPE_VALUE,
  TYPE_DELTA,
  TYPE_PRIOR,
  TYPE_BCD,
  TYPE_FLOAT,
  TYPE_INVERT,
  TYPE_RECALL
} Type;

typedef enum
{
  SIZE_NONE,
  SIZE_BIT0,
  SIZE_BIT1,
  SIZE_BIT2,
  SIZE_BIT3,
  SIZE_BIT4,
  SIZE_BIT5,
  SIZE_BIT6,
  SIZE_BIT7,
  SIZE_LOWER4,
  SIZE_UPPER4,
  SIZE_BYTE, // 8-bit
  SIZE_WORD, // 16-bit
  SIZE_TBYTE, // 24-bit
  SIZE_DWORD, // 32-bit
  SIZE_WORD_BE,
  SIZE_TBYTE_BE,
  SIZE_DWORD_BE,
  SIZE_BITCOUNT,
  SIZE_FLOAT,
  SIZE_FLOAT_BE,
  SIZE_DOUBLE32,
  SIZE_DOUBLE32_BE,
  SIZE_MBF32,
  SIZE_MBF32_LE
} Size;

typedef enum
{
  OP_NONE,
  OP_EQUAL,
  OP_INF,
  OP_INF_EQUAL,
  OP_SUP,
  OP_SUP_EQUAL,
  OP_NOT_EQUAL,
  OP_FACTOR,
  OP_QUOTIENT,
  OP_MOD,
  OP_PLUS,
  OP_MINUS,
  OP_AND,
  OP_XOR
} Operator;

struct NUMERAL
{
  Type type;
  Size size;
  int32_t value;
};

struct CONDITION
{
  int id;
  Flag flag;
  Operator op;
  int hit_target;

  struct NUMERAL lhs;
  struct NUMERAL rhs;
};

struct GROUP
{
  int id;
  size_t condition_count;
  struct CONDITION *conditions[];
};

struct ACHIEVEMENT_LOGIC
{
  size_t group_count;
  struct GROUP *groups[];
};

typedef enum
{
  ACHIEVEMENT_TYPE_NONE,
  ACHIEVEMENT_TYPE_PROGRESSION,
  ACHIEVEMENT_TYPE_WIN_CONDITION,
  ACHIEVEMENT_TYPE_MISSABLE
} Achievement_type;

struct ACHIEVEMENT
{
  int id;
  char *title;
  char *description;
  int points;
  Achievement_type type;

  struct ACHIEVEMENT_LOGIC *logic;
};

typedef enum
{
  FORMAT_SCORE,
  FORMAT_TIME_FRAMES,
  FORMAT_TIME_CENTISECONDS,
  FORMAT_TIME_SECONDS,
  FORMAT_TIME_MINUTES,
  FORMAT_TIME_SECONDS_AS_MINUTES,
  FORMAT_VALUE,
  FORMAT_VALUE_UNSIGNED,
  FORMAT_VALUE_TENS,
  FORMAT_VALUE_HUNDREDS,
  FORMAT_VALUE_THOUSANDS,
  FORMAT_VALUE_FIXED1,
  FORMAT_VALUE_FIXED2,
  FORMAT_VALUE_FIXED3
} Format;

struct LEADERBOARD
{
  int id;
  char *title;
  char *description;
  Format format;
  int lower_is_better;

  struct ACHIEVEMENT_LOGIC *start;
  struct ACHIEVEMENT_LOGIC *cancel;
  struct ACHIEVEMENT_LOGIC *submit;
  struct ACHIEVEMENT_LOGIC *value;
};

struct ACHIEVEMENT_SET
{
  int id;
  char *title;
  ConsoleID consoleID;

  size_t hub_count;
  int **hub_ids;

  size_t achievement_count;
  struct ACHIEVEMENT **achievements;
  size_t leaderboard_count;
  struct LEADERBOARD **leaderboards;
};

#endif // !ACHIEVEMENT_H
