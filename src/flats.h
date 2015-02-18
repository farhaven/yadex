/*
 *	flats.h
 *	AYM 1998-nn-nn
 */


#ifndef YH_FLATS  /* DO NOT INSERT ANYTHING BEFORE THIS LINE */
#define YH_FLATS

#include <string>
#include <vector>

#include "lists.h"

using std::string;
using std::vector;

string ChooseFloorTexture (int, int, const char *, vector<string>, string name);
void DisplayFloorTexture (hookfunc_comm_t *c);
void display_flat_depressed (hookfunc_comm_t *c);


#endif  /* DO NOT ADD ANYTHING AFTER THIS LINE */
