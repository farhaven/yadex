/*
 *	wadnamec.h
 *	AYM 2000-04-11
 */


#ifndef YH_WADNAMEC
#define YH_WADNAMEC


/*
 *	Wad_name_c - a lump name
 */
struct Wad_name_c
{
  Wad_name_c (const char *string);
  Wad_name_c (const char *fmt, ...);
  int cmp (wad_name_t name);
  char name[WAD_NAME + 1];
};


#endif
