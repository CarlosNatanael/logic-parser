#ifndef ACHIEVEMENT_H
#define ACHIEVEMENT_H

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
  char op;
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

struct ACHIEVEMENT
{
  int id;
  char *title;
  char *description;

  size_t group_count;
  struct GROUP **groups[];
};

#endif // !ACHIEVEMENT_H
