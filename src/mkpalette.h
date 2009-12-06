/*
 *	mkpalette.h
 *	Make palette files from lump PLAYPAL
 *	AYM 1998-12-29
 */


int make_gimp_palette (int playpalnum, const char *filename);
int make_palette_ppm (int playpalnum, const char *filename);
int make_palette_ppm_2 (int playpalnum, const char *filename);


