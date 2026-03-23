#include <string.h>
#include <stdio.h>

#include "parser.h"
#include "achievement.h"

const char *FLAGS[] = {
  "",
  "Pause If",
  "Reset If",
  "Reset Next If",
  "Add Source",
  "Sub Source",
  "Add Hits",
  "Sub Hits",
  "Add Address",
  "And Next",
  "Or Next",
  "Measured",
  "Measured If",
  "Trigger",
  "Remember",
};

const char *TYPES[] = {
  "Mem",
  "Value",
  "Delta",
  "Prior",
  "BCD",
  "Float",
  "Invert",
  "Recall"
};

const char *SIZES[] = {
 "",
 "Bit0",
 "Bit1",
 "Bit2",
 "Bit3",
 "Bit4",
 "Bit5",
 "Bit6",
 "Bit7",
 "Lower4",
 "Upper4",
 "8-bit",
 "16-bit",
 "24-bit",
 "32-bit",
 "16-bit BE",
 "24-bit BE",
 "32-bit BE",
 "BitCount",
 "Float",
 "Float BE",
 "Double32",
 "Double32 BE",
 "MBF32",
 "MBF32 LE"
};

const char *OP[] = {
  "",
  "=",
  "<",
  "<=",
  ">",
  ">=",
  "!=",
  "*",
  "/",
  "%",
  "+",
  "-",
  "&",
  "^"
};

int main(int argc, char *argv[])
{
  char input[256];
  while (1)
  {
    scanf("%255s", input);
    if (strcmp(input, "exit") == 0) break;

    struct GROUP *group = get_group(input, strlen(input));

    int id_collumn_width = 4;
    int flag_collumn_width = 14;
    int type_collumn_width = 8;
    int size_collumn_width = 12;
    int value_collumn_width = 11;
    int op_collumn_width = 2;
    int hit_target_collumn_width = 10;

    /*printf("%s  ", FLAGS[group->conditions[0]->flag]);
    printfintf("%s  ", TYPES[group->conditions[0]->lhs.type]);
    printf("%s  ", SIZES[group->conditions[0]->lhs.size]);
    printf("%#x\n", group->conditions[0]->lhs.value);*/

    printf("| ID |     FLAG     |  TYPE  |    SIZE    |   VALUE   |OP|  TYPE  |    SIZE    |   VALUE   | HIT TARGET |\n");
    printf("---------------------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < group->condition_count; i ++)
    {
      printf("|%-*d", id_collumn_width, group->conditions[i]->id);
      printf("|%-*s", flag_collumn_width, FLAGS[group->conditions[i]->flag]);
      printf("|%-*s", type_collumn_width, TYPES[group->conditions[i]->lhs.type]);
      printf("|%-*s", size_collumn_width, SIZES[group->conditions[i]->lhs.size]);
      //if (group->conditions[i]->lhs.type != TYPE_RECALL)
        printf("|%-#*x", value_collumn_width, group->conditions[i]->lhs.value);
      //else printf ("|%*c", value_collumn_width, ' ');
      //if (group->conditions[i]->op != OP_NONE)
      {
        printf("|%-*s", op_collumn_width, OP[group->conditions[i]->op]);
        printf("|%-*s", type_collumn_width, TYPES[group->conditions[i]->rhs.type]);
        printf("|%-*s", size_collumn_width, SIZES[group->conditions[i]->rhs.size]);
        if (group->conditions[i]->rhs.type != TYPE_RECALL)
          printf("|%-#*x", value_collumn_width, group->conditions[i]->rhs.value);
        else printf("|%*c", value_collumn_width, ' ');
      }
      /*else
      {
        printf("|%*c", op_collumn_width, ' ');
        printf("|%*c", type_collumn_width, ' ');
        printf("|%*c", size_collumn_width, ' ');
        printf("|%*c", value_collumn_width, ' ');
      }
      if (group->conditions[i]->flag != FLAG_ADD_SOURCE
       && group->conditions[i]->flag != FLAG_SUB_SOURCE
       && group->conditions[i]->flag != FLAG_ADD_ADDRESS
       && group->conditions[i]->flag != FLAG_REMEMBER)*/
      {
        char hit_target_str[100];
        sprintf(hit_target_str, "%d", group->conditions[i]->hit_target);
        printf("|(%s)", hit_target_str);
        printf("%*c|\n", (int)(hit_target_collumn_width - strlen(hit_target_str)), ' ');
      }
      //else printf("|%*c|\n", hit_target_collumn_width, ' ');
    }

    printf("---------------------------------------------------------------------------------------------------------\n");
  }

  return EXIT_SUCCESS;
}
