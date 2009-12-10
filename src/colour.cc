/*
 *	colour1.cc
 *	getcolour()
 *	AYM 1998-01-27
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
#include "rgb.h"
#include <X11/Xlib.h>
#include "colour.h"
#include "gfx.h"
#include "x11.h"

#define RGB_DIGITS 2  /* R, G and B are 8-bit wide each */

/* This table contains all the physical colours allocated,
   with their rgb value and usage count. */
typedef struct 
{
    pcolour_t pcn;	// The physical colour# (pixel value).
    rgb_c rgb;		// Its RGB value.
    int usage_count;	// Number of logical colours that use it.
} pcolours_table_entry_t;
pcolours_table_entry_t *pcolours = 0;
size_t physical_colours = 0;  // Number of entries in <pcolours>

static void dump_pcolours ();

/*
 *	getcolour
 *	Decode an "rgb:<r>/<g>/<b>" colour specification
 *	Returns :
 *	  0    OK
 *	  <>0  malformed colour specification
 */
int getcolour (const char *s, rgb_c *rgb)
{
    int i;
    int digit;
    int rdigits;
    int gdigits;
    int bdigits;
    unsigned r;
    unsigned g;
    unsigned b;
    int globaldigits;

    if (strncmp (s, "rgb:", 4))
        return 1;

    for (i = 4, r = 0, rdigits = 0; (digit = hextoi (s[i])) >= 0; i++, rdigits++)
        r = (r << 4) | digit;
    if (s[i++] != '/')
        return 2;

    for (g = 0, gdigits = 0; (digit = hextoi (s[i])) >= 0; i++, gdigits++)
        g = (g << 4) | digit;
    if (s[i++] != '/')
        return 3;

    for (b = 0, bdigits = 0; (digit = hextoi (s[i])) >= 0; i++, bdigits++)
        b = (b << 4) | digit;
    if (s[i++] != '\0')
        return 4;

    // Force to 8 bits (RGB_DIGITS hex digits) by scaling up or down
    globaldigits = rdigits;
    globaldigits = y_max (globaldigits, gdigits);
    globaldigits = y_max (globaldigits, bdigits);
    for (; globaldigits < RGB_DIGITS; globaldigits++)
    {
        r <<= 4;
        g <<= 4;
        b <<= 4;
    }
    for (; globaldigits > RGB_DIGITS; globaldigits--)
    {
        r >>= 4;
        g >>= 4;
        b >>= 4;
    }
    rgb->set (r, g, b);
    return 0;
}

/*
 *	irgb2rgb
 *	Convert an IRGB colour (16-colour VGA) to an 8-bit-per-component
 *	RGB colour.
 */
void irgb2rgb (int c, rgb_c *rgb)
{
    if (c == 8)  // Special case for DARKGREY
        rgb->r = rgb->g = rgb->b = 0x40;
    else
    {
        rgb->r = (c & 4) ? ((c & 8) ? 0xff : 0x80) : 0;
        rgb->g = (c & 2) ? ((c & 8) ? 0xff : 0x80) : 0;
        rgb->b = (c & 1) ? ((c & 8) ? 0xff : 0x80) : 0;
    }
}

/*
 *	eight2sixteen
 *	Convert an 8-bit RGB component value to a 16-bit one.
 *	Will convert 00h to 0000h, 80h to 8080h and FFh to FFFFh.
 */
inline u16 eight2sixteen (u8 v)
{
    return (v << 8) | v;
}

/*
 *	alloc_colours
 *	Allocate a group of <count> rgb values and return an array of
 *	<count> physical colour numbers.
 */
pcolour_t *alloc_colours (rgb_c rgb_values[], size_t count)
{
    verbmsg ("colours: alloc_colours: count %d\n", count);

    pcolour_t *pcn_table = (pcolour_t *) malloc (count * sizeof *pcn_table);
    if (pcn_table == NULL)
        fatal_error (msg_nomem);

    /* Allocate the physical colours if necessary. Should not do
    it for static visuals (StaticColor, TrueColor). It does no
    harm but it's useless. */
    for (size_t n = 0; n < count; n++)
    {
        // Is there already a physical colour for this RGB value ?
        pcn_table[n] = PCOLOUR_NONE;
        for (size_t i = 0; i < physical_colours; i++)
        {
            if (pcolours[i].rgb == rgb_values[n])
            {
                // There is. Reuse it.
                pcn_table[n] = pcolours[i].pcn;
                pcolours[i].usage_count++;
                break;
            }
        }

        // There isn't. Try to create a new physical colour.
        if (pcn_table[n] == PCOLOUR_NONE)
        {
            XColor xc;
            xc.red   = eight2sixteen (rgb_values[n].r);
            xc.green = eight2sixteen (rgb_values[n].g);
            xc.blue  = eight2sixteen (rgb_values[n].b);
            Status r = XAllocColor (dpy, cmap, &xc);

            /* Allocation successful. Add a new entry to
            the table of physical colours. */
            if (r != 0)
            {
                pcn_table[n] = (pcolour_t) xc.pixel;
                physical_colours++;
                pcolours = (pcolours_table_entry_t *)
                realloc (pcolours, physical_colours * sizeof *pcolours);
                if (pcolours == NULL)
                    fatal_error (msg_nomem);
                pcolours[physical_colours - 1].pcn         = (pcolour_t) xc.pixel;
                pcolours[physical_colours - 1].rgb         = rgb_values[n];
                pcolours[physical_colours - 1].usage_count = 1;
            }
            /* Couldn't allocate (the colormap is full).
            Reuse the nearest existing physical colour. */
            else
            {
                size_t best_fit = 0;
                int best_delta = INT_MAX;

                for (size_t m = 0; m < physical_colours; m++)
                {
                    int delta = pcolours[m].rgb - rgb_values[n];
                    if (delta < best_delta)
                    {
                        best_fit = m;
                        best_delta = delta;
                    }
                }
                verbmsg ("colours: alloc_colours %d/%d/%d: reused %d/%d/%d, delta=%d\n",
                rgb_values[n].r, rgb_values[n].g, rgb_values[n].b, 
                pcolours[best_fit].rgb.r, pcolours[best_fit].rgb.g,
                pcolours[best_fit].rgb.b,
                best_delta);
                pcn_table[n] = pcolours[best_fit].pcn;
                pcolours[best_fit].usage_count++;
            }
        }
    }
    return pcn_table;
}

/*
 *	free_colours
 *	Free the <count> physical colours in <pc>.
 */
void free_colours (pcolour_t *pcn_table, size_t count)
{
    verbmsg ("colours: free_colours: count %d\n", count);
    if (verbose)
        dump_pcolours ();

    if (pcn_table == NULL)  // Sanity
        return;

    /* Decrement the usage count for all those physical colours.
    If the usage count reaches 0, actually free them. */
    for (pcolour_t *pcn = pcn_table; count; count--, pcn++)
    {
        size_t i;

        for (i = 0; i < physical_colours; i++)
            if (pcolours[i].pcn == *pcn)
                break;

        if (i == physical_colours)
            fatal_error ("Trying to free pc[%d]=%ld that does not exist", (int) i, (long) *pcn);

        if (pcolours[i].usage_count < 1)
            fatal_error ("Freeing unused colour %ld", (long) *pcn);

        pcolours[i].usage_count--;
        if (pcolours[i].usage_count == 0)
        {
            unsigned long pixel = (unsigned long) *pcn;
            x_catch_on ();
            XFreeColors (dpy, cmap, &pixel, 1, 0);
            // Should not happen but sometimes does (not reproducible)
            if (const char *err_msg = x_error ())
                warn ("error freeing colour %08lXh (%s).\n", pixel, err_msg);
            x_catch_off ();
            pcolours[i].pcn = PCOLOUR_NONE;
        }
    }

    /* If physical colours have actually been freed,
    remove them from the table of physical colours. */
    size_t new_physical_colours = 0;

    for (size_t i = 0; i < physical_colours; i++)
        if (pcolours[i].pcn != PCOLOUR_NONE)
            new_physical_colours++;  // Number of physical colours still in use.

    verbmsg ("colours: freed %d of %d physical colours\n",
    physical_colours - new_physical_colours, physical_colours);

    if (new_physical_colours == 0)
    {
        free (pcolours);
        pcolours = 0;
    }else
    {
        pcolours_table_entry_t *new_item;
        pcolours_table_entry_t *new_pcolours = (pcolours_table_entry_t *)
        malloc (new_physical_colours * sizeof *new_pcolours);
        if (new_pcolours == NULL)
            fatal_error (msg_nomem);
        new_item = new_pcolours;
        for (size_t i = 0; i < physical_colours; i++)
            if (pcolours[i].pcn != PCOLOUR_NONE)
                memcpy (new_item++, pcolours + i, sizeof *new_pcolours);
        free (pcolours);
        pcolours = new_pcolours;
    }
    physical_colours = new_physical_colours;

    free (pcn_table);
}

/*
 *	get_pcolours_count
 *	Return the number of physical colours allocated
 */
size_t get_pcolours_count ()
{
    return physical_colours;
}

/*
 *	get_pcolour_pcn
 *	Return the physical colour number (a.k.a. pixel value)
 *	for the <i>th physical colour allocated.
 */
pcolour_t get_pcolour_pcn (size_t i)
{
    if (i >= physical_colours)
    {
        nf_bug ("get_pcolour_pcn: i=%d\n", (int) i);
        return ULONG_MAX;
    }
    return pcolours[i].pcn;
}

/*
 *	dump_pcolours
 *	For debugging purposes
 */
static void dump_pcolours ()
{
    int items_on_current_line = 0;

    for (size_t i = 0; i < physical_colours; i++)
    {
        if (items_on_current_line == 0)
            verbmsg ("colours: ");
        verbmsg ("%c",
        (pcolours[i].usage_count == 1) ? '.' : '0' + pcolours[i].usage_count);
        if (++items_on_current_line % 10 == 0)
        {
            if (items_on_current_line == 50)
            {
                items_on_current_line = 0;
                verbmsg ("\n");
            }else
                verbmsg (" ");
        }
    }
    if (items_on_current_line != 0)
        verbmsg ("\n");
}
