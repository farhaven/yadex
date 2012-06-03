/*
 *	acolours.cc
 *	Allocate and free the application colours.
 *
 *	By "application colours", I mean the colours used to draw
 *	the windows, the menus and the map, as opposed to the
 *	"game colours" which depend on the game (they're in the
 *	PLAYPAL lump) and are used to draw the game graphics
 *	(flats, textures, sprites...).
 *
 *	The game colours are handled in gcolour1.cc and gcolour2.cc.
 *
 *	AYM 1998-11-29
 */

/*
This file is part of Yadex.

Yadex incorporates code from DEU 5.21 that was put in the public domain in
1994 by Raphaël Quinet and Brendon Wyber.

The rest of Yadex is Copyright © 1997-2003 André Majorel and others.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307, USA.
*/

#include "yadex.h"
#include "acolours.h"
#include "gfx.h"
#include "rgb.h"

typedef struct
{
    rgb_c rgb;
    char deleted;
} ac_table_entry_t;

static ac_table_entry_t *table     = 0;  // The list
static acolour_t        table_size = 0;  // The size of the list
static acolour_t        ac_count   = 0;  // The number of entries really used

/*
 *	add_app_colour
 *	Add colour <rgb> to the list of application colours
 *	and return a brand new application colour# for it.
 */
pcolour_t add_app_colour (rgb_c rgb)
{
    size_t i;

    for (i = 0; i < table_size; i++)
        if (table[i].deleted)
            break;

    if (i == table_size)
    {
        table_size++;
        table = (ac_table_entry_t *) realloc (table, table_size * sizeof *table);
        if (table == NULL)
            fatal_error (msg_nomem);
    }
    ac_count++;
    table[i].rgb     = rgb;
    table[i].deleted = 0;
    return i;
}

/*
 *	delete_app_colour
 *	Remove colour# <acn> from the list of application colours.
 */
void delete_app_colour (acolour_t acn)
{
    if (acn >= table_size)
        fatal_error ("delete_app_colour called with non-existent colour %d", acn);
    if (table[acn].deleted)
        fatal_error ("colour %d deleted twice", acn);
    ac_count--;
    table[acn].deleted = 1;
}

/* FIXME a very quick-and-dirty way of preventing
   changes to the list done between commit_() and
   uncommit_() to corrupt things. */
static size_t committed_colours = 0;

/*
 *	commit_app_colours
 *	Return an array containing the physical colour numbers
 *	for the application colours in the list.
 */
pcolour_t *commit_app_colours ()
{
    verbmsg ("colours: committing %d colours\n", ac_count);

    /* First create an array of RGB values
    for all the colours in the list. */
    verbmsg ("colours: rgb_values %p\n");
    rgb_c *rgb_values = new rgb_c[ac_count];
    rgb_c *rgb = rgb_values;
    int items_on_line = 0;
    for (size_t n = 0; n < table_size; n++)
        if (! table[n].deleted)
        {
            if (items_on_line == 0)
                verbmsg ("colours: committing: ");
            verbmsg ("%d ", int (rgb - rgb_values));
            *rgb++ = table[n].rgb;
            if (++items_on_line == 16)
            {
                verbmsg ("\n");
                items_on_line = 0;
            }
        }
    if (items_on_line != 0)
        verbmsg ("\n");

    // Then do the actual allocation.
    committed_colours = ac_count;
    pcolour_t *app_colours = alloc_colours (rgb_values, committed_colours);
    delete[] rgb_values;
    return app_colours;
}

/*
 *	uncommit_app_colours
 *	Free all the colours that were allocated by alloc_app_colours().
 *	They are _not_ removed from the list !
 */
void uncommit_app_colours (pcolour_t *app_colours)
{
    free_colours (app_colours, committed_colours);
    committed_colours = 0;
}
