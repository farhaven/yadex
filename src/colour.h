/*
 *	colour.h
 *	AYM 1998-11-28
 */

#ifndef YH_COLOUR  // Prevent multiple inclusion
#define YH_COLOUR  // Prevent multiple inclusion

class rgb_c;

/* pcolour_t -- a physical colour number.
   The value of a pixel in the opinion of the output library.
   The exact size and meaning of this type vary.
   With X DirectColor visuals, it's an RGB value.
   With X PseudoColor visuals, it's a palette index.
 */

typedef unsigned long pcolour_t;	// X11: up to 32 BPP.
#define PCOLOUR_NONE  0xffffffff	/* An "impossible" colour no. */

pcolour_t *alloc_colours (rgb_c rgb_values[], size_t count);
void free_colours (pcolour_t *pc, size_t count);
size_t get_pcolours_count ();
pcolour_t get_pcolour_pcn (size_t i);

#endif	// Prevent multiple inclusions
