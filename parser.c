#include <stdint.h>
#include <string.h>

#include "parser.h"
#include "achievement.h"

int is_recall(char numeral[], size_t len)
{
  return numeral[0] == '{';
}

Type get_type(char numeral[], size_t len)
{
  // Value is the only type that can be this length
  if (len <= 2) return TYPE_VALUE;
  if (is_recall(numeral, len)) return TYPE_RECALL;
  // Mem are written like 0xABCDEF so numeral[1] must be 'x'
  if (numeral[1] == 'x') return TYPE_MEM;
  // Floats are either f4.5 or fFABCDEF. first one is of type Float, second is Mem
  if (numeral[0] == 'f')
  {
    if (numeral[2] == '.') return TYPE_FLOAT;
    else return TYPE_MEM;
  }

  // in any other case, the type is the first character of the numeral, like d0xABCDEF or ~0xABCDEF
  for (int i = 0; i < sizeof(char_type); i ++)
    if (char_type[i] == numeral[0]) return i + 1;

  return TYPE_VALUE;
}

Size get_size(char numeral[], size_t len, Type type)
{
  // Value, Recall, and Floats do not hold any size
  if (type == TYPE_VALUE || type == TYPE_RECALL || type == TYPE_FLOAT) return SIZE_NONE;

  // a numeral of TYPE_MEM would usually be 0x_ABCDEF where _ is the size
  int8_t size_index = 2;

  // in that case, a numeral will be x0x_ABCDEF where _ is the size
  if (type != TYPE_MEM) size_index = 3;
  // in that case, a numeral would be f_ABCDEF where _ is the size
  else if (numeral[0] == 'f') size_index = 1;

  for (int i = 0; i < sizeof(char_size); i ++)
    if (char_size[i] == numeral[size_index])
    {
      if (i < 18 || size_index == 1)
        return i + 1;
    }
  return SIZE_WORD; // 16-bit is ' ' but can also be '';
}

long parse_float(char *numeral)
{
  char *start = numeral + 1;

  char *end;
  float f = strtof(start, &end);

  // TODO: add some check with end for error handling

  long output;
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

  switch (output->type) {
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
