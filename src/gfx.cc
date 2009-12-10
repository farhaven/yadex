/*
 *    gfx.cc
 *    Graphics routines.
 *    BW & RQ sometime in 1993 or 1994.
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
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "acolours.h"
#include "gcolour1.h"
#include "gcolour2.h"
#include "gcolour3.h"
#include "gfx.h"
#include "levels.h"  /* Level */
#include "x11.h"

/* Parameters set by the command line args and config file */
const char *font_name = NULL;    // X: the name of the font to load
Win_dim initial_window_width ("90%");    // X: the name says it all
Win_dim initial_window_height ("90%");    // X: the name says it all
int   no_pixmap;        // X: use no pixmap -- direct window output

/* Global variables */
int GfxMode = 0; // graphics mode number, or 0 for text
                 // 1 = 320x200, 2 = 640x480, 3 = 800x600, 4 = 1kx768
                 // positive = 16 colors, negative = 256 colors
int OrigX;       // Map X-coord of centre of screen/window
int OrigY;       // Map Y-coord of centre of screen/window
float Scale;     // the scale value
int ScrMaxX;     // Maximum display X-coord of screen/window
int ScrMaxY;     // Maximum display Y-coord of screen/window
int ScrCenterX;  // Display X-coord of centre of screen/window
int ScrCenterY;  // Display Y-coord of centre of screen/window
unsigned FONTH;
unsigned FONTW;
int font_xofs;
int font_yofs;

Display *dpy;        // The X display
int      scn;        // The X screen number
Colormap cmap = 0;   // The X colormap
GC       gc;         // Default GC as set by set_colour(), SetDrawingMode()
                     // and SetLineThickness()
GC       pixmap_gc;  // The GC used to clear the pixmap
Window   win;        // The X window
Pixmap   pixmap;     // The X pixmap (if any)
Drawable drw;        // Points to either <win> or <pixmap>
int      drw_mods;   // Number of modifications to drw since last call to
                     // update_display(). Number of modifications to drw
                     // plus 1 since last call to ClearScreen().
Visual  *win_vis;    // The visual for win */
xpv_t    win_r_mask; // The RGB masks for win's visual */
xpv_t    win_g_mask;
xpv_t    win_b_mask;
int      win_r_bits; // The RGB masks' respective lengths */
int      win_g_bits;
int      win_b_bits;
int      win_r_ofs;  // The RGB masks' respective offsets relative to b0 */
int      win_g_ofs;
int      win_b_ofs;
int      win_ncolours; // The number of possible colours for win's visual.
                       // If win_vis_class is TrueColor or DirectColor,
                       // it's the number of bits in the biggest subfield.
int      win_vis_class; // The class of win's visual
VisualID win_vis_id;    // The ID of win's visual
int      win_depth;     // The depth of win in bits
int      x_server_big_endian = 0; // Is the X server big endian ?
int      ximage_bpp;    // Number of bytes per pixels in XImages
int      ximage_quantum;// Pad XImages lines to a multiple of that many bytes
static pcolour_t *app_colour = 0; // Pixel values for the app. colours
static int DrawingMode    = 0;    // 0 = copy, 1 = xor
static int LineThickness  = 0;    // 0 = thin, 1 = thick
int      text_dot         = 0;    // DrawScreenText() debug flag
static acolour_t colour_stack[4];
static int       colour_stack_pointer = 0;
static Font      font_xfont;
static bool      default_font = true;

/*
 *    Prototypes
 */

/*
 *    InitGfx - initialize the graphics display
 *
 *    Return 0 on success, non-zero on failure
 */
int InitGfx (void)
{
    // Initialization is in fact not necessary
    int width;
    int height;

    /*
    *    Open display and get screen number
    */
    dpy = XOpenDisplay (0);
    if (! dpy)
    {
        err ("Can't open display");
        return 1;
    }
    scn = DefaultScreen (dpy);
    {
        verbmsg ("X: server endianness: ");
        int r = ImageByteOrder (dpy);
        if (r == LSBFirst)
        {
            verbmsg ("little-endian\n");
            x_server_big_endian = 0;
        } else if (r == MSBFirst)
        {
            verbmsg ("big-endian\n");
            x_server_big_endian = 1;
        } else // Can't happen
        {
            verbmsg ("unknown\n");
            warn ("don't understand X server's endianness code %d\n", r);
            warn ("assuming same endianness as CPU.\n");
            x_server_big_endian = cpu_big_endian;
        }
        }
        int screen_width  = DisplayWidth  (dpy, scn);
        int screen_height = DisplayHeight (dpy, scn);
        // On QNX 6, XFree returns silly values. Use a plausible default instead.
        if (screen_width == 16383 && screen_height == 16383)
        {
            warn ("QNX XFree bug detected (width %d, height %d)\n",
            screen_width, screen_height);
            screen_width  = 1024;
            screen_height = 768;
        }

        /*
        *    Create the window
        */
        width  = initial_window_width.pixels  (screen_width);
        height = initial_window_height.pixels (screen_height);
        win = XCreateSimpleWindow (dpy, DefaultRootWindow (dpy),
                                   10, 10, width, height, 0, 0, 0);
        //win = DefaultRootWindow (dpy);
        {
            XWindowAttributes wa;
            XVisualInfo model;
            XVisualInfo *vis_info;
            int nvisuals;

            XGetWindowAttributes (dpy, win, &wa);
            win_vis   = wa.visual;
            win_depth = wa.depth;
            verbmsg ("X: window depth: %d b\n", win_depth);

            /*
            *    Retrieve info regarding win's visual
            */
            model.visualid = XVisualIDFromVisual (win_vis);
            vis_info = XGetVisualInfo (dpy, VisualIDMask, &model, &nvisuals);
            if (! vis_info)
                fatal_error ("XGetVisualInfo returned NULL for ID %d", model.visualid);
            if (nvisuals != 1)
                fatal_error ("XGetVisualInfo returned %d visuals", nvisuals);
            if (vis_info->depth != win_depth)
                fatal_error ("Visual depth %d <> win depth %d", vis_info->depth, win_depth);
            win_vis_id    = vis_info->visualid;
            // #if defined _cplusplus || defined __cplusplus
            win_vis_class = vis_info->c_class;
            /*#elif
            win_vis_class = vis_info->class;
#endif */
            win_ncolours  = vis_info->colormap_size;
            win_r_mask    = vis_info->red_mask;
            win_g_mask    = vis_info->green_mask;
            win_b_mask    = vis_info->blue_mask;
            XFree (vis_info);
            verbmsg ("X: visual: id %xh, class ", (unsigned) win_vis_id);
            if (win_vis_class == PseudoColor)
                verbmsg ("PseudoColor");
            else if (win_vis_class == TrueColor)
                verbmsg ("TrueColor");
            else if (win_vis_class == DirectColor)
                verbmsg ("DirectColor");
            else if (win_vis_class == StaticColor)
                verbmsg ("StaticColor");
            else if (win_vis_class == GrayScale)
                verbmsg ("GrayScale");
            else if (win_vis_class == StaticGray)
                verbmsg ("StaticGray");
            else
                verbmsg ("unknown (%d)", win_vis_class);
            verbmsg (", colours %d, masks %08lX %08lX %08lX\n",
            win_ncolours, win_r_mask, win_g_mask, win_b_mask);

            // Compute win_[rgb]_bits and win_[rgb]_ofs
            /* FIXME can enter infinite loop if MSb of either mask is set
            and >> sign extends. */
            if (win_r_mask)
            {
                xpv_t mask;
                for (mask = win_r_mask, win_r_ofs = 0; ! (mask & 1); mask >>= 1)
                    win_r_ofs++;
                for (win_r_bits = 0; mask & 1; mask >>= 1)
                    win_r_bits++;
            }
            if (win_g_mask)
            {
                xpv_t mask;
                for (mask = win_g_mask, win_g_ofs = 0; ! (mask & 1); mask >>= 1)
                    win_g_ofs++;
                for (win_g_bits = 0; mask & 1; mask >>= 1)
                    win_g_bits++;
            }
            if (win_b_mask)
            {
                xpv_t mask;
                for (mask = win_b_mask, win_b_ofs = 0; ! (mask & 1); mask >>= 1)
                    win_b_ofs++;
                for (win_b_bits = 0; mask & 1; mask >>= 1)
                    win_b_bits++;
            }
            verbmsg ("X: visual: "
            "r_ofs %d, r_bits %d, ", win_r_ofs, win_r_bits);
            verbmsg ("g_ofs %d, g_bits %d, ", win_g_ofs, win_g_bits);
            verbmsg ("b_ofs %d, b_bits %d\n", win_b_ofs, win_b_bits);
        }

    /*
    *    Get info relevant to XImages
    */
    ximage_bpp     = 0;
    ximage_quantum = 0;
    {
        int nformats = 0;
        XPixmapFormatValues *format = XListPixmapFormats (dpy, &nformats);
        if (format == 0 || nformats < 1)
        {
            warn ("XListPixmapFormats() trouble (ret=%p, n=%d).\n", format, nformats);
            goto ximage_done;
        }
        /* Pick the best possible pixmap format. Prefer one that has the
        same depth as the drawable, as it is likely to be the fastest
        one. Should there be several formats that satisfy that
        requirement (I don't think that ever happens), prefer the one
        where the line quantum is equal to the number of bits per
        pixel, as that is easier to work with. */
        {
            const XPixmapFormatValues *best = 0;
            for (int n = 0; n < nformats; n++)
            {
                const XPixmapFormatValues *current = format + n;
                verbmsg ("X: pixmap format:"
                " #%d, depth %2d, bits_per_pixel %2d, scanline_pad %2d\n",
                n, current->depth, current->bits_per_pixel, current->scanline_pad);
                if (best == 0)
                {
                    best = current;
                    continue;
                }
                int cgoodness = 2 * (current->depth == win_depth)
                                + 1 * (current->scanline_pad == current->bits_per_pixel);
                int bgoodness = 2 * (best->depth == win_depth)
                                + 1 * (best->scanline_pad == best->bits_per_pixel);
                if (cgoodness > bgoodness)
                    best = current;
            }
            verbmsg ("X: pixmap format: best is #%d\n", int (best - format));
            if (best != 0)
            {
                int bits_per_pixel = best->bits_per_pixel;
                if (bits_per_pixel % 8)  // Paranoia
                {
                    round_up (bits_per_pixel, 8);
                    warn ("XImage format has bad bits_per_pixel %d. Rounding up to %d.\n",
                          best->bits_per_pixel, bits_per_pixel);
                }
                int scanline_pad = best->scanline_pad;
                if (best->scanline_pad % 8)  // Paranoia
                {
                    round_up (scanline_pad, 8);
                    warn ("XImage format has bad scanline_pad %d. Rounding up to %d.\n",
                          best->scanline_pad, scanline_pad);
                }
                ximage_bpp     = bits_per_pixel / 8;
                ximage_quantum = scanline_pad   / 8;
            }
        }
        if (ximage_bpp == 0 || ximage_quantum == 0)
        {
            warn ("XListPixmapFormats() returned no suitable formats.\n"); 
            goto ximage_done;
        }
ximage_done:
        if (format != 0)
            XFree (format);
    }
    /* Could not obtain authoritative/good values. Warn and guess
    plausible values. */
    if (ximage_bpp == 0 || ximage_quantum == 0)
    {
        if (ximage_bpp == 0)
            ximage_bpp = (win_depth + 7) / 8;
        if (ximage_quantum == 0)
            ximage_quantum = 4;
        warn ("guessing format. Images will probably not be displayed correctly\n");
    }
    verbmsg ("X: pixmap format: %d B per pixel, %d B quantum.\n",
    ximage_bpp, ximage_quantum);

    /*
    *    Further configure the window
    */
#if 0
    {
    XSetWindowAttributes wa;
    wa.win_gravity = CenterGravity;
    XChangeWindowAttributes (dpy, win, CWWinGravity, &wa);
    }
#endif

    XStoreName (dpy, win, "Yadex");  // Temporary name -- will be overwritten
    XSelectInput (dpy, win,
                  KeyPressMask | KeyReleaseMask
                  | ButtonPressMask | ButtonReleaseMask
                  | PointerMotionMask
                  | EnterWindowMask | LeaveWindowMask
                  | ExposureMask
                  | StructureNotifyMask);

    /*
    *    Possibly load and query the font
    */
    {
        XFontStruct *xqf;

        // Load the font or use the default font.
        default_font = true;
        if (font_name != NULL)
        {
            x_catch_on ();  // Catch errors in XLoadFont()
            font_xfont = XLoadFont (dpy, font_name);
            if (const char *err_msg = x_error ())
            {
                warn("can't load font \"%s\" (%s).\n", font_name, err_msg);
                warn("using default font instead.\n");
            }
            else
                default_font = false;
            x_catch_off ();
        }

        // Query the font we'll use for FONTW, FONTH and FONTYOFS.
        xqf = XQueryFont (dpy,
        default_font ? XGContextFromGC (DefaultGC (dpy, scn)) : font_xfont);
        if (xqf->direction != FontLeftToRight)
            warn ("this font is not left-to-right !\n");
        if (xqf->min_byte1 != 0 || xqf->max_byte1 != 0)
            warn ("this is not a single-byte font !\n");
        if (xqf->min_char_or_byte2 > 32 || xqf->max_char_or_byte2 < 126)
            warn ("this font does not support the ASCII character set !\n");
        if (xqf->min_bounds.width != xqf->max_bounds.width)
            warn ("this is not a fixed-width font !\n");
        FONTW     = xqf->max_bounds.width;
        FONTH     = xqf->ascent + xqf->descent;
        font_xofs = xqf->min_bounds.lbearing;
        font_yofs = xqf->max_bounds.ascent;
        XFreeFontInfo (NULL, xqf, 1);
        verbmsg ("X: font: metrics: %dx%d, xofs %d, yofs %d\n",
        FONTW, FONTH, font_xofs, font_yofs);
    }

    /*
    *    Get/create the colormap
    *    and allocate the colours.
    */
    if (win_vis_class == PseudoColor)
    {
        verbmsg ("X: running on PseudoColor visual, using private Colormap\n");
        cmap = XCreateColormap (dpy, win, win_vis, AllocNone);
    }
    else
        cmap = DefaultColormap (dpy, scn);

    XSetWindowColormap (dpy, win, cmap);
    game_colour = alloc_game_colours (0);
    app_colour = commit_app_colours ();
    if (win_depth == 24)
        game_colour_24.refresh (game_colour, x_server_big_endian);

    /*
    *    Create the GC
    */
    {
        XGCValues gcv;
        unsigned long mask;

        mask = GCForeground | GCFunction | GCLineWidth;
        if (! default_font)
        {
            mask |= GCFont;
            gcv.font = font_xfont;
        }
        gcv.foreground = app_colour[0];  // Default colour
        gcv.line_width = 0;
        gcv.function   = GXcopy;
        gc = XCreateGC (dpy, win, mask, &gcv);
        if (gc == 0)
            fatal_error ("XCreateGC() returned NULL");
    }

    /*
    *    More stuff
    */
    XSetWindowBackground (dpy, win, app_colour[0]);
    XMapWindow (dpy, win);

    // Unless no_pixmap is set, create the pixmap and its own pet GC.
    if (no_pixmap)
        drw = win;
    else
    {
        XGCValues gcv;

        pixmap = XCreatePixmap (dpy, win, width, height, win_depth);
        gcv.foreground = app_colour[0];
        gcv.graphics_exposures = False;  // We don't want NoExpose events
        pixmap_gc = XCreateGC (dpy, pixmap, GCForeground | GCGraphicsExposures, &gcv);
        drw = win;
        drw_mods = 0;  // Force display the first time
    }
    XSync (dpy, False);

    GfxMode = 1;

    SetWindowSize (width, height);
    return 0;
}

/*
 *    TermGfx - terminate the graphics display
 */
void TermGfx ()
{
    verbmsg ("TermGfx: GfxMode=%d\n", GfxMode);
    if (GfxMode)
    {
        int r;

        if (! no_pixmap)
        {
            XFreePixmap (dpy, pixmap);
            XFreeGC     (dpy, pixmap_gc);
        }
        r = XDestroyWindow (dpy, win);
        verbmsg ("X: XDestroyWindow returned %d\n", r);
        free_game_colours (game_colour);
        game_colour = 0;
        uncommit_app_colours (app_colour);
        app_colour = 0;
        if (cmap != DefaultColormap (dpy, scn))
        {
            verbmsg ("X: freeing Colormap\n");
            XFreeColormap  (dpy, cmap);
        }
        if (! default_font)
        {
            verbmsg ("X: unloading font\n");
            XUnloadFont (dpy, font_xfont);
        }
        XFreeGC (dpy, gc);
        gc = 0;
        // FIXME there is surely more to do...
        XCloseDisplay (dpy);
        GfxMode = 0;
    }
}

/*
 *    SetWindowSize - set the size of the edit window
 */
void SetWindowSize (int width, int height)
{
    // Am I called uselessly ?
    if (width == ScrMaxX + 1 && height == ScrMaxY + 1)
        return;

    ScrMaxX = width - 1;
    ScrMaxY = height - 1;
    ScrCenterX = ScrMaxX / 2;
    ScrCenterY = ScrMaxY / 2;

    // Replace the old pixmap by another of the new size
    if (! no_pixmap)
    {
        XFreePixmap (dpy, pixmap);
        pixmap = XCreatePixmap (dpy, win, width, height, win_depth);
        drw = pixmap;
    }
}

/*
 *    ClearScreen - clear the screen
 */
void ClearScreen ()
{
    if (no_pixmap)
        XClearWindow (dpy, win);
    else
    {
        XFillRectangle (dpy, pixmap, pixmap_gc, 0, 0, ScrMaxX + 1, ScrMaxY + 1);
        drw = pixmap;  // Redisplaying from scratch so let's use the pixmap
    }
}

/*
 *    update_display - update the physical display
 *
 *    Make sure the physical bitmap display (the X window)
 *    is up to date WRT the logical bitmap display (the X
 *    pixmap).
 *
 *    If <drw> == <win>, it means that only partial
 *    changes were made and that they were made directly on
 *    the window, not on the pixmap so no need to copy the
 *    pixmap onto the window.
 */
void update_display ()
{
    //if (drw_mods == 0)  // Nothing to do, display is already up to date
    //   return;
    //printf (" [");
    //fflush (stdout);
    if (! no_pixmap && drw == pixmap)
    {
        //putchar ('*');
        XCopyArea (dpy, pixmap, win, pixmap_gc, 0, 0, ScrMaxX+1, ScrMaxY+1, 0, 0);
    }
    XFlush (dpy);
    //printf ("] ");
    //fflush (stdout);
    drw_mods = 0;
    drw = win;  // If they don't like it, they can call ClearScreen() [HHOS]
}

/*
 *    force_window_not_pixmap - force graphic ops to use window
 *
 *    Redirect graphic output to window, not pixmap. Used only
 *    in yadex.cc, before calling the sprite viewer.
 *
 *    FIXME this is not a clean way to do things.
 */
void force_window_not_pixmap ()
{
    drw = win;
}

/*
 *    set_pcolour - set the current drawing colour
 *
 *    <colour> must be a physical colour number (a.k.a. pixel value).
 */
void set_pcolour (pcolour_t colour)
{
    XSetForeground (dpy, gc, (xpv_t) colour);
}

// Last application colour set by set_colour()
static acolour_t current_acolour = 0;

/*
 *    get_colour - get the current drawing colour
 */
acolour_t get_colour ()
{
    return current_acolour;
}

/*
 *    set_colour - set the current drawing colour
 *
 *    <colour> must be an application colour number.
 */
void set_colour (acolour_t colour)
{
    if (colour != current_acolour)
    {
        current_acolour = colour;
        XSetForeground (dpy, gc, app_colour[colour]);
    }
}

/*
 *    push_colour - push a colour into the colour stack
 *
 *    Like set_colour() except that it will only last until
 *    the next call to pop_colour().
 */
void push_colour (acolour_t colour)
{
    if (colour_stack_pointer >= (int) (sizeof colour_stack / sizeof *colour_stack))
    {
        nf_bug ("Colour stack overflow");
        return;
    }
    colour_stack[colour_stack_pointer] = current_acolour;
    colour_stack_pointer++;
    set_colour (colour);
}

/*
 *    pop_colour - pop the colour stack
 *
 *    Cancel the effect of the last call to push_colour().
 */
void pop_colour (void)
{
    if (colour_stack_pointer < 1)
    {
        nf_bug ("Colour stack underflow");
        return;
    }
    colour_stack_pointer--;
    set_colour (colour_stack[colour_stack_pointer]);
}

/*
 *    SetLineThickness - set the line style (thin or thick)
 */
void SetLineThickness (int thick)
{
    if (!! thick != LineThickness)
    {
        LineThickness = !! thick;
        XGCValues gcv;
        gcv.line_width = LineThickness ? 3 : (DrawingMode ? 1 : 0);
        // ^ It's important to use a line_width of 1 when in xor mode.
        // See note (1) in the hacker's guide.
        XChangeGC (dpy, gc, GCLineWidth, &gcv);
    }
}

/*
 *    SetDrawingMode - set the drawing mode (copy or xor)
 */
void SetDrawingMode (int _xor)
{
    if (!! _xor != DrawingMode)
    {
        DrawingMode = !! _xor;
        XGCValues gcv;
        gcv.function = DrawingMode ? GXxor : GXcopy;
        gcv.line_width = LineThickness ? 3 : (DrawingMode ? 1 : 0);
        // ^ It's important to use a line_width of 1 when in xor mode.
        // See note (1) in the hacker's guide.
        XChangeGC (dpy, gc, GCFunction | GCLineWidth, &gcv);
    }
}

/*
 *    draw_point - draw a point at display coordinates
 *
 *    The point is drawn at display coordinates (<x>, <y>).
 */
void draw_point (int x, int y)
{
    XDrawPoint (dpy, drw, gc, x, y);
}

/*
 *    draw_map_point - draw a point at map coordinates
 *
 *    The point is drawn at map coordinates (<mapx>, <mapy>)
 */
void draw_map_point (int mapx, int mapy)
{
    XDrawPoint (dpy, drw, gc, SCREENX (mapx), SCREENY (mapy));
    drw_mods++;
}

/*
 *    DrawMapLine - draw a line on the screen from map coords
 */
void DrawMapLine (int mapx1, int mapy1, int mapx2, int mapy2)
{
    XDrawLine (dpy, drw, gc, SCREENX (mapx1), SCREENY (mapy1),
    SCREENX (mapx2), SCREENY (mapy2));
    drw_mods++;
}

/*
 *    DrawMapCircle - draw a circle on the screen from map coords
 */
void DrawMapCircle (int mapx, int mapy, int mapradius)
{
    XDrawArc (dpy, drw, gc, SCREENX (mapx - mapradius), SCREENY (mapy + mapradius),
              (unsigned int) (2 * mapradius * Scale),
              (unsigned int) (2 * mapradius * Scale), 0, 360*64);
    drw_mods++;
}

/*
 *    DrawMapVector - draw an arrow on the screen from map coords
 */
void DrawMapVector (int mapx1, int mapy1, int mapx2, int mapy2)
{
    int    scrx1   = SCREENX (mapx1);
    int    scry1   = SCREENY (mapy1);
    int    scrx2   = SCREENX (mapx2);
    int    scry2   = SCREENY (mapy2);
    double r       = hypot ((double) (scrx1 - scrx2), (double) (scry1 - scry2));

    int    scrXoff = (r >= 1.0) ? (int) ((scrx1 - scrx2) * 8.0 / r * (Scale / 2)) : 0;
    int    scrYoff = (r >= 1.0) ? (int) ((scry1 - scry2) * 8.0 / r * (Scale / 2)) : 0;

    XDrawLine (dpy, drw, gc, scrx1, scry1, scrx2, scry2);
    scrx1 = scrx2 + 2 * scrXoff;
    scry1 = scry2 + 2 * scrYoff;
    XDrawLine (dpy, drw, gc, scrx1 - scrYoff, scry1 + scrXoff, scrx2, scry2);
    XDrawLine (dpy, drw, gc, scrx1 + scrYoff, scry1 - scrXoff, scrx2, scry2);
    drw_mods++;
}

/*
 *    DrawMapArrow - draw an arrow on the screen from map coords and angle (0 - 65535)
 */
void DrawMapArrow (int mapx1, int mapy1, unsigned angle)
{
    int    mapx2   = mapx1 + (int) (50 * cos (angle / 10430.37835));
    int    mapy2   = mapy1 + (int) (50 * sin (angle / 10430.37835));
    int    scrx1   = SCREENX (mapx1);
    int    scry1   = SCREENY (mapy1);
    int    scrx2   = SCREENX (mapx2);
    int    scry2   = SCREENY (mapy2);
    double r       = hypot (scrx1 - scrx2, scry1 - scry2);

    int    scrXoff = (r >= 1.0) ? (int) ((scrx1 - scrx2) * 8.0 / r * (Scale / 2)) : 0;
    int    scrYoff = (r >= 1.0) ? (int) ((scry1 - scry2) * 8.0 / r * (Scale / 2)) : 0;

    XDrawLine (dpy, drw, gc, scrx1, scry1, scrx2, scry2);
    scrx1 = scrx2 + 2 * scrXoff;
    scry1 = scry2 + 2 * scrYoff;
    XDrawLine (dpy, drw, gc, scrx1 - scrYoff, scry1 + scrXoff, scrx2, scry2);
    XDrawLine (dpy, drw, gc, scrx1 + scrYoff, scry1 - scrXoff, scrx2, scry2);
    drw_mods++;
}

/*
 *    DrawScreenLine - draw a line on the screen from screen coords
 */
void DrawScreenLine (int Xstart, int Ystart, int Xend, int Yend)
{
    XDrawLine (dpy, drw, gc, Xstart, Ystart, Xend, Yend);
    drw_mods++;
}

/*
 *    DrawScreenLineLen - draw a line on the screen
 */
void DrawScreenLineLen (int x, int y, int width, int height)
{
    if (width > 0)
        width--;
    else if (width < 0)
        width++;
    if (height > 0)
        height--;
    else if (height < 0)
        height++;
    XDrawLine (dpy, drw, gc, x, y, x + width, y + height);
    drw_mods++;
}

/*
 *    DrawScreenRect - draw a rectangle
 *
 *    Unlike most functions here, the 3rd and 4th parameters
 *    specify lengths, not coordinates.
 */
void DrawScreenRect (int x, int y, int width, int height)
{
    XDrawRectangle (dpy, drw, gc, x, y, width - 1, height - 1);
    drw_mods++;
}

/*
 *    DrawScreenBox - draw a filled in box on the screen from screen coords
 *
 *    (scrx1, scry1) is the top left corner
 *    (scrx2, scry2) is the bottom right corner
 *    If scrx2 < scrx1 or scry2 < scry1, the function does nothing.
 */
void DrawScreenBox (int scrx1, int scry1, int scrx2, int scry2)
{
    if (scrx2 < scrx1 || scry2 < scry1)
        return;
    // FIXME missing gc fill_style
    XFillRectangle (dpy, drw, gc, scrx1, scry1,
    scrx2 - scrx1 + 1, scry2 - scry1 + 1);
    drw_mods++;
}

/*
 *    DrawScreenBoxwh - draw a filled rectangle of width x height pixels
 *
 *    (scrx0, scry0) is the top left corner
 *    (width, height) is the obvious
 *    If width < 1 or height < 1, does nothing.
 */
void DrawScreenBoxwh (int scrx0, int scry0, int width, int height)
{
    if (width < 1 || height < 1)
        return;
    // FIXME missing gc fill_style
    XFillRectangle (dpy, drw, gc, scrx0, scry0, width, height);
    drw_mods++;
}

/*
 *    DrawScreenBox3D - draw a filled-in 3D box on the screen
 *
 *    The 3D border is rather wide (BOX_BORDER pixels wide).
 */
void DrawScreenBox3D (int scrx1, int scry1, int scrx2, int scry2)
{
    DrawScreenBox3DShallow (scrx1, scry1, scrx2, scry2);

    push_colour (WINBG_DARK);
    XDrawLine (dpy, drw, gc, scrx1 + 1, scry2 - 1, scrx2 - 1, scry2 - 1);
    XDrawLine (dpy, drw, gc, scrx2 - 1, scry1 + 1, scrx2 - 1, scry2 - 1);

    set_colour (WINBG_LIGHT);
    XDrawLine (dpy, drw, gc, scrx1 + 1, scry1 + 1, scrx1 + 1, scry2 - 1);
    XDrawLine (dpy, drw, gc, scrx1 + 1, scry1 + 1, scrx2 - 1, scry1 + 1);
    drw_mods++;

    pop_colour ();
}

/*
 *    DrawScreenBox3DShallow - draw a filled-in 3D box on the screen
 *
 *    Same thing as DrawScreenBox3D but shallow (the 3D border
 *    is NARROW_BORDER pixels wide).
 */
void DrawScreenBox3DShallow (int scrx1, int scry1, int scrx2, int scry2)
{
    push_colour (WINBG);
    XFillRectangle (dpy, drw, gc, scrx1+1, scry1+1, scrx2-scrx1, scry2-scry1);
    set_colour (WINBG_DARK);
    XDrawLine (dpy, drw, gc, scrx1, scry2, scrx2, scry2);
    XDrawLine (dpy, drw, gc, scrx2, scry1, scrx2, scry2);
    set_colour (WINBG_LIGHT);
    XDrawLine (dpy, drw, gc, scrx1, scry1, scrx2, scry1);
    XDrawLine (dpy, drw, gc, scrx1, scry1, scrx1, scry2);
    drw_mods++;
    pop_colour ();
}

/*
 *    draw_box_border - draw the 3D border of a box.
 *
 *    (x, y) is the outer top left corner.
 *    (width, height) are the outer dimensions.
 *    (thickness) is the thickness of the border in pixels.
 *    (raised) is zero for depressed, non-zero for raised.
 */
void draw_box_border (int x, int y, int width, int height,
      int thickness, int raised)
{
    int n;
    XPoint points[3];

    // We want offsets, not distances
    width--;
    height--;

    // Draw the right and bottom edges
    push_colour (raised ? WINBG_DARK : WINBG_LIGHT);
    points[0].x = x + width;
    points[0].y = y;
    points[1].x = 0;
    points[1].y = height;
    points[2].x = -width;
    points[2].y = 0;
    for (n = 0; n < thickness; n++)
    {
        XDrawLines (dpy, drw, gc, points, 3, CoordModePrevious);
        points[0].x--;
        points[0].y++;
        points[1].y--;
        points[2].x++;
    }

    // Draw the left and top edges
    set_colour (raised ? WINBG_LIGHT : WINBG_DARK);
    points[0].x = x;
    points[0].y = y + height;
    points[1].x = 0;
    points[1].y = -height;
    points[2].x = width;
    points[2].y = 0;
    for (n = 0; n < thickness; n++)
    {
        XDrawLines (dpy, drw, gc, points, 3, CoordModePrevious);
        points[0].x++;
        points[0].y--;
        points[1].y++;
        points[2].x--;
    }

    pop_colour ();
}

/*
 *    DrawScreenBoxHollow - draw a hollow 3D box on the screen
 *
 *    The 3D border is HOLLOW_BORDER pixels wide.
 */
void DrawScreenBoxHollow (int scrx1, int scry1, int scrx2, int scry2, acolour_t colour)
{
      push_colour (colour);
      XFillRectangle (dpy, drw, gc,
          scrx1 + HOLLOW_BORDER, scry1 + HOLLOW_BORDER,
          scrx2 + 1 - scrx1 - 2 * HOLLOW_BORDER, scry2 + 1 - scry1 - 2 * HOLLOW_BORDER);
      set_colour (WINBG_LIGHT);
      XDrawLine (dpy, drw, gc, scrx1, scry2, scrx2, scry2);
      XDrawLine (dpy, drw, gc, scrx2, scry1, scrx2, scry2);
      set_colour (WINBG_DARK);
      XDrawLine (dpy, drw, gc, scrx1, scry1, scrx2, scry1);
      XDrawLine (dpy, drw, gc, scrx1, scry1, scrx1, scry2);
      drw_mods++;
      pop_colour ();
}

/*
 *    DrawScreenMeter - draw a meter bar on the screen
 *
 *    In a hollow box; max. value = 1.0
 */
void DrawScreenMeter (int scrx1, int scry1, int scrx2, int scry2, float value)
{
    printf ("DrawScreenMeter()\n");  // FIXME
}

// Shared by DrawScreenText() and DrawScreenString()

/* Coordinates of first character of last string drawn with
   DrawScreenText() or DrawScreenString(). */
static int lastx0;
static int lasty0;

// Where the last DrawScreen{Text,String}() left the "cursor".
static int lastxcur;
static int lastycur;

/*
 *    DrawScreenText - format and display a string
 *
 *    Write text to the screen in printf() fashion.
 *    The top left corner of the first character is at (<scrx>, <scry>)
 *    If <scrx> == -1, the text is printed at the same abscissa
 *    as the last text printed with this function.
 *    If <scry> == -1, the text is printed one line (FONTH pixels)
 *    below the last text printed with this function.
 *    If <msg> == NULL, no text is printed. Useful to set the
 *    coordinates for the next time.
 */
void DrawScreenText (int scrx, int scry, const char *msg, ...)
{
    char temp[120];
    va_list args;

    // <msg> == NULL: print nothing, just set the coordinates.
    if (msg == NULL)
    {
        if (scrx != -1 && scrx != -2)
        {
            lastx0   = scrx;
            lastxcur = scrx;
        }
        if (scry != -1 && scry != -2)
        {
            lasty0   = scry;  // Note: no "+ FONTH"
            lastycur = scry;
        }
        return;
    }

    va_start (args, msg);
    y_vsnprintf (temp, sizeof temp, msg, args);
    DrawScreenString (scrx, scry, temp);
}

/*
 *    DrawScreenString - display a string
 *
 *    Same thing as DrawScreenText() except that the string is
 *    printed verbatim (no formatting or conversion).
 *
 *    A "\1" in the string is not displayed but causes
 *    subsequent characters to be displayed in WINLABEL (or
 *    WINLABEL_DIM if the current colour before the function
 *    was called was WINFG_DIM).
 *
 *    A "\2" in the string is not displayed but causes
 *    subsequent characters to be displayed in the same colour
 *    that was active before the function was called.
 *
 *    The string can contain any number of "\1" and "\2".
 *    Regardless, upon return from the function, the current
 *    colour is restored to what it was before the function
 *    was called.
 *
 *    This colour switching business was hacked in a hurry.
 *    Feel free to improve it.
 */
void DrawScreenString (int scrx, int scry, const char *str)
{
    int x; int y;
    size_t len = strlen (str);

    /* FIXME originally, the test was "< 0". Because it broke
    when the screen was too small, I changed it to a more
    specific "== -1". A quick and very dirty hack ! */
    if (scrx == -1)
        x = lastx0;
    else if (scrx == -2)
        x = lastxcur;
    else
        x = scrx;

    if (scry == -1)
        y = lasty0;
    else if (scry == -2)
        y = lastycur;
    else
        y = scry;

    if (strchr (str, '\1') == 0)
    {
        XDrawString (dpy, drw, gc, x - font_xofs, y + font_yofs, str, len);
    } else
    {
        acolour_t save = get_colour ();
        int xx = x;
        len = 0;
        for (const char *p = str; *p != '\0';)
        {
            int i;
            for (i = 0; p[i] != '\0' && p[i] != '\1' && p[i] != '\2'; i++);
            len += i;
            if (i > 0)
            {
                XDrawString (dpy, drw, gc, xx - font_xofs, y + font_yofs, p, i);
                xx += i * FONTW;
            }
            if (p[i] == '\0')
                break;
            if (p[i] == '\1')
                set_colour (save == WINFG_DIM ? WINLABEL_DIM : WINLABEL);
            else if (p[i] == '\2')
                set_colour (save);
            i++;
            p += i;
        }
        set_colour (save);
    }
    if (text_dot)
    XDrawPoint (dpy, drw, gc, x, y);
    drw_mods++;

    lastxcur = x + FONTW * len;
    lastycur = y;
    if (scrx != -2)
        lastx0 = x;
    if (scry != -2)
        lasty0 = y + FONTH;
}

/*
 *    DrawScreenChar - display a character
 *
 *    Same thing as DrawScreenText() except that the string is
 *    printed verbatim (no formatting or conversion).
 */
void DrawScreenChar (int x, int y, char c)
{
    XDrawString (dpy, drw, gc, x - font_xofs, y + font_yofs, &c, 1);
    if (text_dot)
        XDrawPoint (dpy, drw, gc, x, y);
    drw_mods++;
}
