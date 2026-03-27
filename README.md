# Logic Parser for RetroAchievement

[RetroAchievement](https://retroachievements.org/) implements its achievement system through [RAIntegration](https://github.com/RetroAchievements/RAIntegration)   
It uses a string encoding to store the logic for an achievement.   
I highly recommend that you check the [RetroAchievement Dev Docs](https://docs.retroachievements.org/developer-docs/) before using this parser.   


## How to Use
   

- `achievement.h` is where the achievement `structs` and `enums` are stored. It is included by `parser.h` so you have to download it.
- `RA_Consoles.h` is a header file taken directly from [RAInterface](https://github.com/RetroAchievements/RAInterface) and hold an `enum` for console IDs.
- `parser.h` and `parser.c` is where the main logic for the parsing resides.
- `encoder.h` and `encoder.c` is what you'll want to refer to if you plan on encoding the parsed logic back to a string.<br>

Add the following line to any C-based script to gain access to the parsing functions.
```c
#include "path/to/achievement.h"
```
Then you have multiple possibilities.
- `get_achievement_logic(char *logic, size_t len)` will return an object of type `struct ACHIEVEMENT_LOGIC *` which hold a list of `struct GROUP`.
- `get_leaderboard(char *logic, size_t len)` will return an object of type `struct LEADERBOARD *` which hold 4 objects of type `struct ACHIEVEMENT_LOGIC *` :
  - `leaderboard->start`
  - `leaderboard->cancel`
  - `leaderboard->submit`
  - `leaderboard->value`
- `get_game_from_json(char *path)` will return an object of type `struct GAME *` which holds a list of `struct ACHIEVEMENT` and a list of `struct LEADERBOARD`.

### Parsing Structure
>[!NOTE]
> Everything here is mapped as doubly linked lists. Inserting or appending structures will always be simpler this way. Each listed structures hold two pointers `next` and `prev` which will allow you to navigate through the list. Furthermore, each structures which holds a list as a member, will have two pointer members `head` and `tail`
   
- `NUMERAL` is a hand side of any comparison or operation, it has a type, a size, and a value.   
Since RAIntegration is a 32-bit DLL, the internal accumulator is 32-bit, so a numeral's value cannot go beyond that.   
>[!NOTE]
> If your numeral's type is `TYPE_FLOAT` or his size is one of the 6 floats size, you'll need to copy `value` into a float.
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
  Operator op; // either an operator or a comparator, such as '+' or '='
  int hit_target

  struct CONDITION *next;
  struct CONDITION *prev;

  struct NUMERAL lhs;
  struct NUMERAL rhs;
};
```
<br>

- `GROUP` represents a set of `CONDITION`. Each achievement must have a Core group (represented here by a `GROUP` where `GROUP.id == 0`) and an arbitrary number of Alt Groups.
>[!NOTE]
> Any achievement is unlocked when every condition in its Core Group AND in at least one Alt Group is true   
```c
struct GROUP
{
  int id;

  struct GROUP *next;
  struct GROUP *prev;

  struct CONDITION *condition_head;
  struct CONDITION *condition_tail;
};
```
<br>

- `ACHIEVEMENT_LOGIC` represents a set of `GROUP` and that is all. It does not hold any meta data.
```c
struct ACHIEVEMENT_LOGIC
{
  struct GROUP *group_head;
  struct GROUP *group_tail;
}
```

- `ACHIEVEMENT` adds some meta data to an `ACHIEVEMENT_LOGIC` such as an id, title or description which are parsed from a JSON.
```c
struct ACHIEVEMENT
{
  int id;
  char *title;
  char *description;
  int points;
  Achievement_type type;

  struct ACHIEVEMENT *next;
  struct ACHIEVEMENT *prev;

  struct ACHIEVEMENT_LOGIC *logic;
};
```
<br>

- `LEADERBOARD` represents an array of 4 `ACHIEVEMENT_LOGIC` called respectively `START`, `CANCEL`, `SUBMIT` and `VALUE`
>[!NOTE]
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

  struct LEADERBOARD *next;
  struct LEADERBOARD *prev;

  struct ACHIEVEMENT_LOGIC *start;
  struct ACHIEVEMENT_LOGIC *cancel;
  struct ACHIEVEMENT_LOGIC *submit;
  struct ACHIEVEMENT_LOGIC *value;
};
```
<br>

- `ACHIEVEMENT_SET` represents a set of achievements which is a list of achievement and a list of leaderboard. The type is either "core" or "subset".
```c
struct ACHIEVEMENT_SET
{
  Set_type type;

  struct ACHIEVEMENT *achievement_head;
  struct ACHIEVEMENT *achievement_tail;

  struct LEADERBOARD *leaderboard_head;
  struct LEADERBOARD *leaderboard_tail;
};
```
<br>

- Finally, `GAME` is the highest structure possible, it holds all of a game's metadata and all of his sets.
```c
struct GAME
{
  int id;
  char *title;
  ConsoleID consoleID;

  int hub_count;
  int *hubs_id;

  size_t set_count;
  struct ACHIEVEMENT_SET *sets[];
};
```
