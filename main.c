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
  char *input_buffer = malloc(65535);
  while (1)
  {
    fgets(input_buffer, 65535, stdin);
    input_buffer[strcspn(input_buffer, "\n")] = '\0';
    if (strcmp(input_buffer, "exit") == 0) break;

    struct ACHIEVEMENT *achievement = get_achievement(input_buffer, strlen(input_buffer));

    int id_collumn_width = 4;
    int flag_collumn_width = 14;
    int type_collumn_width = 8;
    int size_collumn_width = 12;
    int value_collumn_width = 11;
    int op_collumn_width = 2;
    int hit_target_collumn_width = 10;
    int alt_group_line_length = 86;


    printf("\n");
    printf("| ID |     FLAG     | TYPE  |   SIZE    |  VALUE   |OP| TYPE  |   SIZE    |  VALUE   | HIT TARGET |\n");

    for (int i = 0; i < achievement->group_count; i ++)
    {
      struct GROUP *group = achievement->groups[i];

      printf("|-------------------------------------------------------------------------------------------------|\n");
      if (group->id == 0)
        printf("| CORE GROUP                                                                                      |\n");
      else
        printf("| ALT GROUP %-*d|\n", alt_group_line_length, group->id);
      printf("|-------------------------------------------------------------------------------------------------|\n");


      for (int j = 0; j < group->condition_count; j ++)
      {
        printf("|%-*d| ", id_collumn_width, group->conditions[j]->id);
        printf("%-*s", flag_collumn_width, FLAGS[group->conditions[j]->flag]);
        printf("%-*s", type_collumn_width, TYPES[group->conditions[j]->lhs.type]);
        printf("%-*s", size_collumn_width, SIZES[group->conditions[j]->lhs.size]);
        if (group->conditions[j]->lhs.type != TYPE_RECALL)
          printf("%-#*x", value_collumn_width, group->conditions[j]->lhs.value);
        else printf ("%*c", value_collumn_width, ' ');
        if (group->conditions[j]->op != OP_NONE)
        {
          printf("%-*s ", op_collumn_width, OP[group->conditions[j]->op]);
          printf("%-*s", type_collumn_width, TYPES[group->conditions[j]->rhs.type]);
          printf("%-*s", size_collumn_width, SIZES[group->conditions[j]->rhs.size]);
          if (group->conditions[j]->rhs.type != TYPE_RECALL)
            printf("%-#*x", value_collumn_width, group->conditions[j]->rhs.value);
          else printf("%*c", value_collumn_width, ' ');
        }
        else
        {
          printf("%*c ", op_collumn_width, ' ');
          printf("%*c", type_collumn_width, ' ');
          printf("%*c", size_collumn_width, ' ');
          printf("%*c", value_collumn_width, ' ');
        }
        if (group->conditions[j]->flag != FLAG_ADD_SOURCE
         && group->conditions[j]->flag != FLAG_SUB_SOURCE
         && group->conditions[j]->flag != FLAG_ADD_ADDRESS
         && group->conditions[j]->flag != FLAG_REMEMBER)
        {
          char hit_target_str[100];
          sprintf(hit_target_str, "%d", group->conditions[j]->hit_target);
          printf("(%s)", hit_target_str);
          printf("%*c|\n", (int)(hit_target_collumn_width - strlen(hit_target_str)), ' ');
        }
        else printf("%*c|\n", hit_target_collumn_width + 2, ' '); // adding 2 for the '()'
      }
    }

    printf("|-------------------------------------------------------------------------------------------------|\n");
  }

  return EXIT_SUCCESS;
}
