/*
 *	levels.h
 *	BW & RQ sometime in 1993 or 1994
 *	FIXME all those variables should become members of a
 *	"Level" class.
 */


#ifndef YH_LEVELS  /* Prevent multiple inclusion */
#define YH_LEVELS  /* Prevent multiple inclusion */

#include <string>
#include <vector>

#include "wstructs.h"
#include "things.h"

using std::string;
using std::vector;

// Defined in levels.cc
extern MDirPtr Level;		/* master dictionary entry for the level */

extern int   NumThings;		/* number of things */
extern TPtr  Things;		/* things data */
extern int   NumLineDefs;	/* number of linedefs */
extern LDPtr LineDefs;		/* linedefs data */
extern vector<SideDef> SideDefs;
extern int   NumVertices;	/* number of vertices */
extern VPtr  Vertices;		/* vertices data */
extern int   NumSegs;		/* number of segments */
extern int   NumSectors;	/* number of sectors */
extern SPtr  Sectors;		/* sectors data */
extern uint8_t*   Behavior;
extern int   BehaviorSize;

// FIXME should be somewhere else
extern vector<string> WTexture;	/* vector of wall texture names */
extern size_t NumFTexture;	/* number of floor/ceiling textures */
typedef struct
{
  char            name[WAD_NAME + 1];	// Name of flat
  const Wad_file *wadfile;		// Pointer on wad where flat comes from
  int32_t             offset;		// Offset of flat in wad
} flat_list_entry_t;			// Length is implicit (always 4096)
extern flat_list_entry_t *flat_list;	// List of all flats in the directory

extern int   MapMaxX;		/* maximum X value of map */
extern int   MapMaxY;		/* maximum Y value of map */
extern int   MapMinX;		/* minimum X value of map */
extern int   MapMinY;		/* minimum Y value of map */
extern bool  MadeChanges;	/* made changes? */
extern bool  MadeMapChanges;	/* made changes that need rebuilding? */

extern unsigned long things_angles;  /* Used to know whether a list of
				   things sorted by type and angle would
				   need to be rebuilt. Incremented
				   whenever a thing is created, deleted
				   or has its type or angle changed.
				   Presently, no such list exists but
				   there will be one if
				   draw_things_sprites() ever draws
				   sprites according to their angles. */

extern unsigned long things_types;  /* Used to know whether the list of
				   things sorted by type that drawmap.cc
				   maintains should be rebuilt.
				   Incremented whenever a thing is
				   created, deleted or has its type
				   changed. */

extern char Level_name[WAD_NAME + 1]; /* The name of the level (E.G.
				   "MAP01" or "E1M1"), followed by a
				   NUL. If the Level has been created as
				   the result of a "c" command with no
				   argument, an empty string. The name
				   is not necesarily in upper case but
				   it always a valid lump name, not a
				   command line shortcut like "17". */

extern y_file_name_t Level_file_name;  /* The name of the file in which
				   the level would be saved. If the
				   level has been created as the result
				   of a "c" command, with or without
				   argument, an empty string. */

extern y_file_name_t Level_file_name_saved;  /* The name of the file in
				   which the level was last saved. If
				   the Level has never been saved yet,
				   an empty string. */

int ReadLevelData (string); /* SWAP! */
void EmptyLevelData(string levelname);
void update_level_bounds ();


#endif	/* DO NOT ADD ANYTHING AFTER THIS LINE */
