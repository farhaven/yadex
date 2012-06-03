/*
 *	gamesky.h
 *	AYM 2000-04-24
 */


#ifndef YH_GAMESKY  /* DO NOT INSERT ANYTHING BEFORE THIS LINE */
#define YH_GAMESKY


#include "wadname.h"


extern Wad_name sky_flat;


/*
 *	is_sky - is this flat a sky
 */
inline bool is_sky (const char *flat)
{
  return sky_flat == flat;
}


#endif  /* DO NOT ADD ANYTHING AFTER THIS LINE */
