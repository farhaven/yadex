/* decorate.h
    definitions for parsing decorate files
*/

#ifndef YH_DECORATE
#define YH_DECORATE

#define STATE "spawn" // state to read the first graphic from
#define STATELEN 4    // length of the state name
#define SPRITELEN 4   // length of the sprites name
#define MAX_TOKENS 6

void SaveEntryToRawFile (FILE *, const char *);
void read_decorate();
#endif
