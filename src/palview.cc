/*
 *	palview.cc
 *	Palette (PLAYPAL & COLORMAP) viewer
 *	AYM 1999-11-11
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
#include <X11/Xlib.h>
#include "colour.h"
#include "gcolour2.h"
#include "gfx.h"
#include "palview.h"
#include "rgb.h"
#include "wadfile.h"
#include "wads.h"
#include "ytime.h"

/*
 *	Palette viewer::run
 *	The only public method of the palette viewer.
 */
// One COLORMAP entry. Wrapped in struct to avoid array<->pointer problems
typedef struct { uint8_t c[DOOM_COLOURS]; } colormap_entry_t;

void Palette_viewer::run ()
{
    int       lines    = (ncolours + columns - 1) / columns;
    const int pwidth   = columns * (pixels + 1);
    const int pheight  = lines * (pixels + 1);
    int       width    = 2 * BOX_BORDER + 2 * WIDE_HSPACING + pwidth;
    int       height   = 2 * BOX_BORDER + 3 * WIDE_VSPACING + pheight + 7 * FONTH;
    int       x0       = (ScrMaxX - width) / 2;
    int       y0       = (ScrMaxY - height) / 2;
    int       nmaps    = 0;  // Number of entries in the COLORMAP lump
    colormap_entry_t **colormap = 0;
    rgb_c    *playpal  = 0;

    // Load the PLAYPAL lump
    do
    {
        playpal = new rgb_c[DOOM_COLOURS];
        for (size_t n = 0; n < DOOM_COLOURS; n++)
        playpal[n].set (0, 0, 0);
        const char *lump_name = "PLAYPAL";
        MDirPtr dir = FindMasterDir (MasterDir, lump_name);
        if (dir == NULL)
        {
            warn ("%s: lump not found\n", lump_name);
            break;
        }
        const Wad_file *wf = dir->wadfile;
        if (dir->dir.size % (3 * DOOM_COLOURS) != 0)
        {
            warn ("%s has weird size (%ld, not mult of %d), ignoring tail\n",
            lump_name, (long) dir->dir.size, (int) (DOOM_COLOURS * 3));
        }
        wf->seek (dir->dir.start);
        if (wf->error ())
        {
            warn ("%s: seek error\n", lump_name);
            break;
        }
        playpal = new rgb_c[DOOM_COLOURS];
        for (size_t n = 0; n < DOOM_COLOURS; n++)
        {
            char buf[3];
            wf->read_bytes (buf, sizeof buf);
            playpal[n].set (buf[0], buf[1], buf[2]);
        }
        if (wf->error ())
            warn ("%s: read error\n", lump_name);
    }
    while (0);

    // Load the COLORMAP lump
    do
    {
        const char *lump_name = "COLORMAP";
        MDirPtr dir = FindMasterDir (MasterDir, lump_name);
        if (dir == NULL)
        {
            warn ("%s: lump not found\n", lump_name);
            break;
        }
        const Wad_file *wf = dir->wadfile;
        nmaps = dir->dir.size / DOOM_COLOURS;
        if ((long) DOOM_COLOURS * nmaps != dir->dir.size)
        {
            warn ("%s: has weird size (%ld, not mult of %d), ignoring tail\n",
            lump_name, (long) dir->dir.size, (int) DOOM_COLOURS);
        }
        if (nmaps > 200)
        {
            warn ("%s has too many (%d) entries, keeping only 200 first\n",
            lump_name, nmaps);
            nmaps = 200;
        }
        wf->seek (dir->dir.start);
        if (wf->error ())
        {
            warn ("%s: seek error\n", lump_name);
            break;
        }
        colormap = new colormap_entry_t *[nmaps];
        for (int n = 0; n < nmaps; n++)
        {
            colormap[n] = new colormap_entry_t;
            wf->read_bytes (colormap[n]->c, sizeof colormap[n]->c);
        }
        if (wf->error ())
            warn ("%s: read error\n", lump_name);
    }
    while (0);

    // On to the real business
    ix0 = x0 + BOX_BORDER + WIDE_HSPACING;
    iy0 = y0 + BOX_BORDER + WIDE_VSPACING;
    int tx0 = ix0;					// Top left corner of text
    int ty0 = y0 + BOX_BORDER + 2 * WIDE_VSPACING + pheight;
    push_colour (0);				// Save current colour
#define DECIDX(i,n) do { i = (i - n + ncolours) % ncolours; } while (0)
#define INCIDX(i,n) do { i = (i + n           ) % ncolours; } while (0)
    int           mapno         = 0;
    bool          mapping       = true;
    int           is_drawn      = 0;
    const int     YID_WINDOW    = 0x01;
    const int     YID_CURSOR    = 0x02;
    const int     YID_PALETTE   = 0x04;
    const int     YID_TEXT      = 0x08;
    int           cursor_phase  = 0;
    int           display_phase = 0;
    unsigned long cursor_time   = 0;
    i   = 0;
    ofs = 0;

    for (;;)
    {
        int mi = colormap[mapno]->c[i];	// Mapped index
        int ei = mapping ? mi : i;		// Effective index
        int mapped_to = 0;			// N. distinct colours that map to i
        for (int n = 0; n < ncolours; n++)
            for (int m = 0; m < nmaps; m++)
                if (colormap[m]->c[n] == i)
                {
                    mapped_to++;
                    break;        // Don't count the same mapper twice
                }
        int maps_to = 0;			// N. distinct colours that i maps to
        {
            bitvec_c mappee (ncolours);
            for (int m = 0; m < nmaps; m++)
            mappee.set (colormap[m]->c[i]);
            for (int n = 0; n < ncolours; n++)
            if (mappee.get (n))
            maps_to++;
        }

        // Draw the window
        if (! (is_drawn & YID_WINDOW))
        {
            DrawScreenBox3D (x0, y0, x0 + width - 1, y0 + height - 1);
            is_drawn = YID_WINDOW;  // Redraw everything else
        }

        // Draw the cursor (frame around the current cell)
        {
            const int cycle = 800;  // 800 ms
            unsigned long current_time = y_milliseconds ();
            unsigned long elapsed_time = current_time - cursor_time;
            cursor_time = current_time;
            cursor_phase = (cursor_phase + elapsed_time) % cycle;
            if ((cursor_phase >= cycle / 2) != (display_phase >= cycle / 2))
                is_drawn &= ~YID_CURSOR;
            if (! (is_drawn & YID_CURSOR))
            {
                draw_cursor (WINFG, cursor_phase >= cycle / 2);
                display_phase = cursor_phase;
                is_drawn |= YID_CURSOR;
            }
        }

        // Draw a (pixels x pixels) square for each colour
        if (! (is_drawn & YID_PALETTE))
        {
            int x = 0;  // Initialized only to prevent GCC from warning
            int y = 0;  // Initialized only to prevent GCC from warning
            for (int n = 0; n < ncolours; n++)
            {
                if (n % columns == 0)
                {
                    x = ix0;
                    if (n == 0)
                    y = iy0;
                    else
                    y += pixels + 1;
                }
                else
                    x += pixels + 1;

                if (game_colour == 0)  // If PLAYPAL not found
                    set_pcolour (0);
                else
                {
                    if (mapping)
                        set_pcolour (game_colour[colormap[mapno]->c[(n + ofs) % ncolours]]);
                    else
                        set_pcolour (game_colour[(n + ofs) % ncolours]);
                }
                DrawScreenBoxwh (x, y, pixels, pixels);
            }
            is_drawn |= YID_PALETTE;
            set_colour (WINFG_DIM);  // Just to force the next set_colour() to do sth
        }

        // Draw the "caption"
        if (! (is_drawn & YID_TEXT))
        {
            set_colour (WINBG);
            DrawScreenBoxwh (tx0, ty0, pwidth, 7 * FONTH);
            set_colour (WINFG);
            DrawScreenText (tx0, ty0, "Index        %3d", i);
            push_colour (mapping ? WINFG : WINFG_DIM);
            DrawScreenText (tx0, -1,  "Mapped index %3d", mi);
            pop_colour ();
            DrawScreenText (tx0, -1,  "R            %3d", playpal[ei].r);
            DrawScreenText (tx0, -1,  "G            %3d", playpal[ei].g);
            DrawScreenText (tx0, -1,  "B            %3d", playpal[ei].b);
            DrawScreenText (tx0, -1,  "Mapped to by %3d", mapped_to);
            DrawScreenText (tx0, -1,  "Maps to      %3d", maps_to);
            push_colour (mapping ? WINFG : WINFG_DIM);
            DrawScreenText (tx0 + 18 * FONTW, ty0, "Colormap %3d", mapno);
            pop_colour ();
            is_drawn |= YID_TEXT;
        }

        // Process any events
        get_input_status ();
        if (is.key == YK_PU)			// [Pgup] previous colormap
        {
            mapno--;
            if (mapno < 0)
                mapno = nmaps - 1;
            is_drawn &= ~(YID_PALETTE | YID_TEXT);
        } else if (is.key == YK_PD)		// [Pgdn] next colormap
        {
            mapno++;
            if (mapno >= nmaps)
            mapno = 0;
            is_drawn &= ~(YID_PALETTE | YID_TEXT);
        } else if (is.key == YK_LEFT)		// [Left] previous palette entry
        {
            draw_cursor (WINBG, false);
            DECIDX (i, 1);
            is_drawn &= ~(YID_PALETTE | YID_TEXT | YID_CURSOR);
            cursor_phase = 0;
        } else if (is.key == YK_RIGHT)		// [Right] next palette entry
        {
            draw_cursor (WINBG, false);
            INCIDX (i, 1);
            is_drawn &= ~(YID_TEXT | YID_CURSOR);
            cursor_phase = 0;
        } else if (is.key == YK_UP)		// [Up] previous palette row
        {
            draw_cursor (WINBG, false);
            DECIDX (i, columns);
            is_drawn &= ~(YID_TEXT | YID_CURSOR);
            cursor_phase = 0;
        } else if (is.key == YK_DOWN)		// [Down] next palette row
        {
            draw_cursor (WINBG, false);
            INCIDX (i, columns);
            is_drawn &= ~(YID_TEXT | YID_CURSOR);
            cursor_phase = 0;
        } else if (is.key == YK_END		// [End], [$]: end of current line
            || is.key == '$')
        {
            draw_cursor (WINBG, false);
            i += columns - i % columns - 1;
            is_drawn &= ~(YID_TEXT | YID_CURSOR);
            cursor_phase = 0;
        } else if (is.key == YK_HOME		// [Home], [0], [^]: start of cur. line
            || is.key == '^'
            || is.key == '0')
        {
            draw_cursor (WINBG, false);
            i -= i % columns;
            is_drawn &= ~(YID_TEXT | YID_CURSOR);
            cursor_phase = 0;
        } else if (is.key == YK_RETURN)		// [Return]: beginning of next line
        {
            draw_cursor (WINBG, false);
            i += columns - i % columns;
            is_drawn &= ~(YID_TEXT | YID_CURSOR);
            cursor_phase = 0;
        } else if (is.key == 'G' 		// [G], [L]: beginning of last line
            || is.key == 'L')
        {
            draw_cursor (WINBG, false);
            i = (lines  - 1) * columns;
            is_drawn &= ~(YID_TEXT | YID_CURSOR);
            cursor_phase = 0;
        } else if (is.key == 'H')		// [H] beginning of first line
        {
            draw_cursor (WINBG, false);
            i = 0;
            is_drawn &= ~(YID_TEXT | YID_CURSOR);
            cursor_phase = 0;
        } else if (is.key == 'm')		// [m] toggle mapping
        {
            mapping = ! mapping;
            is_drawn &= ~(YID_PALETTE | YID_TEXT);
        } else if (is.key == 'M')		// [M] beginning of middle line
        {
            draw_cursor (WINBG, false);
            i = (lines / 2) * columns;
            is_drawn &= ~(YID_TEXT | YID_CURSOR);
            cursor_phase = 0;
        } else if (is.key == '+' || is.key == '=')  // [+] increment offset
        {
            INCIDX (ofs, 1);
            INCIDX (i, 1);
            is_drawn &= ~(YID_PALETTE | YID_TEXT);
        } else if (is.key == '-')		// [-] decrement offset
        {
            DECIDX (ofs, 1);
            DECIDX (i, 1);
            is_drawn &= ~(YID_PALETTE | YID_TEXT);
        } else if (is.key == YK_ESC)		// [Esc] quit
            break;
        else if (is.key == YE_EXPOSE)
            is_drawn = 0;  // Redraw everything
        else
            ;
    }

    pop_colour ();				// Restore current colour
    delete[] playpal;
    for (int n = 0; n < nmaps; n++)
        delete colormap[n];
    delete[] colormap;
}


void Palette_viewer::draw_cursor (int c, bool phase)
{
    const int a = (i + ncolours - ofs) % ncolours;
    const int side = pixels + 2;
    const int x0 = ix0 - 1 + (side - 1) * (a % columns);
    const int y0 = iy0 - 1 + (side - 1) * (a / columns);
    const int x1 = x0 + side - 1;
    const int y1 = y0 + side - 1;
    if (c == WINBG)
    {
        set_colour (c);
        DrawScreenRect (x0, y0, side, side);
    } else
    {
        const int l1 = side / 2;
        const int l2 = side - l1;
        // FIXME this cursor looks ugly
        set_colour (phase ? BLACK : WHITE);
        DrawScreenLineLen (x0, y0,  l1,   0);
        DrawScreenLineLen (x0, y0,   0,  l1);
        DrawScreenLineLen (x1, y1,   0, -l1);
        DrawScreenLineLen (x1, y1, -l1,   0);
        set_colour (phase ? WHITE : BLACK);
        DrawScreenLineLen (x1, y0, -l2,   0);
        DrawScreenLineLen (x1, y0,   0,  l2);
        DrawScreenLineLen (x0, y1,   0, -l2);
        DrawScreenLineLen (x0, y1,  l2,   0);
    }
}
