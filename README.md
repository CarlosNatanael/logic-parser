# Logic Parser for RetroAchievement
---
[RetroAchievement](https://retroachievements.org/) implements its achievement system through [RAIntegration](https://github.com/RetroAchievements/RAIntegration)   
It uses a string encoding to store the logic for an achievement.   
I highly recommend that you check the [RetroAchievement Dev Docs](https://docs.retroachievements.org/developer-docs/) before using this parser.   

---
## How to Use
   
- `main.c` is only used for printing logic as readable data to the terminal. Unless that's what you want to do, do not add this file your project.<br>

- `achievement.h` is where the achievement `structs` and `enums` are stored. It is included by `parser.h` so you have to download it.
- `parser.h` and `parser.c` is where the main logic for the parsing resides.<br>

Add the following line to any C-based script to gain access to the parsing functions.
```c
#include "path/to/achievement.h"
```
Then you can use `get_achievement(char *achievement, size_t len)` to get the whole achievement parsed.   
It returns an object of type `struct ACHIEVEMENT *`   

#### Parsing Structure
   
- `NUMERAL` is a hand side of any comparison or operation, it has a type, a size, and a value.   
Since RAIntegration is a 32-bit DLL, the internal accumulator is 32-bit, so a numeral's value cannot go beyond that.   
```c
struct NUMERAL
{
  Type type;
  Size size;
  int32_t value;
};
```
<br>

- `CONDITION` represents any line of logic, with a left and right hand side. It can either represents a comparison or an operation.   
Each condition is indexed following a 1 based indexation   
```c
struct CONDITION
{
  int id;
  Flag flag;
  Operator op; // either an operator or a comparator, such as '=' or '+'
  int hit_target

  struct NUMERAL lhs;
  struct NUMERAL rhs;
};
```
<br>

- `GROUP` represents a set of `CONDITION`. Each achievement must have a Core group (represented here by a `GROUP` where `GROUP.id == 0`) and an arbitrary number of Alt Groups.
>[!NOTE] Any achievement is unlocked when every condition in its Core Group AND in at least one Alt Group is true   
```c
struct GROUP
{
  int id;
  size_t condition_count;
  struct CONDITION *conditions[];
};
```
<br>

- `ACHIEVEMENT` represents a set of `GROUP`. Each of them has an id, title, and description which are currently not supported.   
```c
struct ACHIEVEMENT
{
  int id;
  char *title;
  char *description;

  size_t group_count;
  struct GROUP *groups;
};
```
<br>

- `LEADERBOARD` represents an array of 4 `ACHIEVEMENT` called respectively `START`, `CANCEL`, `SUBMIT` and `VALUE`
>[!NOTE] Each of these aren't technically achievements, but they can be represented as is
> when `START` is true, the leaderboard start a new entry
> when `CANCEL` is true, the leaderboard cancel any on going entries
> when `SUBMIT` is true, the leaderboard submit the entry to [retroachievements](https://retroachievements.org)
> `VALUE` is the part that require the format. It doesn't act like a regular achievement, as it isn't true or false. It is just measuring a value.
```c
struct LEADERBOARD
{
  int id;
  char *title;
  char *description;
  Format format;
  int lower_is_better;

  struct ACHIEVEMENT *start;
  struct ACHIEVEMENT *cancel;
  struct ACHIEVEMENT *submit;
  struct ACHIEVEMENT *value;
};
```
