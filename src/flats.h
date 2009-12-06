/*
 *	flats.h
 *	AYM 1998-nn-nn
 */


#ifndef YH_FLATS  /* DO NOT INSERT ANYTHING BEFORE THIS LINE */
#define YH_FLATS


#include "lists.h"


void ChooseFloorTexture (int, int, const char *, int, char **, char *);
void DisplayFloorTexture (hookfunc_comm_t *c);
void display_flat_depressed (hookfunc_comm_t *c);


#endif  /* DO NOT ADD ANYTHING AFTER THIS LINE */
