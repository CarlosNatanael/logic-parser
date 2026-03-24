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

    struct LEADERBOARD *leaderboard = get_leaderboard(input_buffer, strlen(input_buffer));

    int id_collumn_width = 4;
    int flag_collumn_width = 14;
    int type_collumn_width = 8;
    int size_collumn_width = 12;
    int value_collumn_width = 11;
    int op_collumn_width = 2;
    int hit_target_collumn_width = 10;
    int alt_group_line_length = 86;




    for (int i = 0; i < 4; i ++)
    {
      struct ACHIEVEMENT *achievement;
      switch (i)
      {
        case 0:
          achievement = leaderboard->start;
          printf("\n\n\n");
          printf("|-------------------------------------------------------------------------------------------------|\n");
          printf("| START                                                                                           |\n");
          printf("|-------------------------------------------------------------------------------------------------|\n");
          break;
        case 1:
          achievement = leaderboard->cancel;
          printf("\n\n");
          printf("|-------------------------------------------------------------------------------------------------|\n");
          printf("| CANCEL                                                                                          |\n");
          printf("|-------------------------------------------------------------------------------------------------|\n");
          break;
        case 2:
          achievement = leaderboard->submit;
          printf("\n\n");
          printf("|-------------------------------------------------------------------------------------------------|\n");
          printf("| SUBMIT                                                                                          |\n");
          printf("|-------------------------------------------------------------------------------------------------|\n");
          break;
        case 3:
          achievement = leaderboard->value;
          printf("\n\n");
          printf("|-------------------------------------------------------------------------------------------------|\n");
          printf("| VALUE                                                                                           |\n");
          printf("|-------------------------------------------------------------------------------------------------|\n");
          break;
      }

      printf("| #  |     Flag     | Type  |   Size    |  Value   |op| Type  |   Size    |  Value   | Hit Target |\n");

      for (int j = 0; j < achievement->group_count; j ++)
      {
        struct GROUP *group = achievement->groups[j];

        printf("|-------------------------------------------------------------------------------------------------|\n");
        if (group->id == 0)
          printf("| Core Group                                                                                      |\n");
        else
          printf("| Alt Group %-*d|\n", alt_group_line_length, group->id);
        printf("|-------------------------------------------------------------------------------------------------|\n");


        for (int k = 0; k < group->condition_count; k ++)
        {
          printf("|%-*d| ", id_collumn_width, group->conditions[k]->id);
          printf("%-*s", flag_collumn_width, FLAGS[group->conditions[k]->flag]);
          printf("%-*s", type_collumn_width, TYPES[group->conditions[k]->lhs.type]);
          printf("%-*s", size_collumn_width, SIZES[group->conditions[k]->lhs.size]);
          if (group->conditions[k]->lhs.type != TYPE_RECALL)
            printf("%-#*x", value_collumn_width, group->conditions[k]->lhs.value);
          else printf ("%*c", value_collumn_width, ' ');
          if (group->conditions[k]->op != OP_NONE)
          {
            printf("%-*s ", op_collumn_width, OP[group->conditions[k]->op]);
            printf("%-*s", type_collumn_width, TYPES[group->conditions[k]->rhs.type]);
            printf("%-*s", size_collumn_width, SIZES[group->conditions[k]->rhs.size]);
            if (group->conditions[k]->rhs.type != TYPE_RECALL)
              printf("%-#*x", value_collumn_width, group->conditions[k]->rhs.value);
            else printf("%*c", value_collumn_width, ' ');
          }
          else
          {
            printf("%*c ", op_collumn_width, ' ');
            printf("%*c", type_collumn_width, ' ');
            printf("%*c", size_collumn_width, ' ');
            printf("%*c", value_collumn_width, ' ');
          }
          if (group->conditions[k]->flag != FLAG_ADD_SOURCE
           && group->conditions[k]->flag != FLAG_SUB_SOURCE
           && group->conditions[k]->flag != FLAG_ADD_ADDRESS
           && group->conditions[k]->flag != FLAG_REMEMBER)
          {
            char hit_target_str[100];
            sprintf(hit_target_str, "%d", group->conditions[k]->hit_target);
            printf("(%s)", hit_target_str);
            printf("%*c|\n", (int)(hit_target_collumn_width - strlen(hit_target_str)), ' ');
          }
          else printf("%*c|\n", hit_target_collumn_width + 2, ' '); // adding 2 for the '()'
        }
      }

    printf("|-------------------------------------------------------------------------------------------------|\n");

    }
  }

  return EXIT_SUCCESS;
}
