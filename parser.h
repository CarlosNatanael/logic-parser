#ifndef PARSER_H
#define PARSER_H

#include "achievement.h"

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

int is_recall(char numeral[], size_t len);

Type get_type(char numeral[], size_t len);
Size get_size(char numeral[], size_t len, Type type);

long parse_float(char numeral[]);
long parse_value(char numeral[]);
long parse_address(struct NUMERAL *input, char numeral[]);

struct NUMERAL *get_numeral(char numeral[], size_t len);

Flag get_flag(char condition);
int get_hit_target(char condition[], size_t len);
struct CONDITION *get_condition(char condition[], size_t len);

struct GROUP *get_group(char group[], size_t len);
struct ACHIEVEMENT *get_achievement(char achievement[], size_t len);

#endif // !PARSER_H
