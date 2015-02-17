/*
 *	textures.h
 *	AYM 2000-04-29
 */


#ifndef YH_TEXTURES  /* DO NOT ADD ANYTHING BEFORE THIS LINE */
#define YH_TEXTURES


#include "lists.h"


/* textures.cc */
void DisplayPic (hookfunc_comm_t *c);
string ChooseWallTexture (int, int, const char *, int, char **, string);
void GetWallTextureSize (int16_t *, int16_t *, const char *);


#endif
