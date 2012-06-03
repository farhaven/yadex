/*
 *	imgscale.cc
 *	AYM 2000-06-16
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
#include "img.h"


/*
 *	scale_img - scale a game image
 *
 *	<img> is the source image, <omg> is the destination
 *	image. <scale> is the scaling factor (> 1.0 to magnify).
 *	A scaled copy of <img> is put in <omg>. <img> is not
 *	modified. Any previous data in <omg> is lost.
 *
 *	Example:
 *
 *	  Img raw;
 *	  Img scaled;
 *	  LoadPicture (raw, ...);
 *	  scale_img (raw, 2, scaled);
 *	  display_img (scaled, ...);
 *
 *	The implementation is mediocre in the case of scale
 *	factors < 1 because it uses only one source pixel per
 *	destination pixel. On certain patterns, it's likely to
 *	cause a visible loss of quality.
 *
 *	In the case of scale factors > 1, the algorithm is
 *	suboptimal.
 */
void scale_img (const Img& img, double scale, Img& omg)
{
    img_dim_t iwidth  = img.width ();
    img_dim_t owidth  = (img_dim_t) (img.width () * scale + 0.5);
    img_dim_t oheight = (img_dim_t) (img.height () * scale + 0.5);
    omg.resize (owidth, oheight);
    const img_pixel_t *const ibuf = img.buf ();
    img_pixel_t       *const obuf = omg.wbuf ();
    if (scale <= 2.0)
    {
        img_pixel_t *orow = obuf;
        int *ix = new int[owidth];
        for (int ox = 0; ox < owidth; ox++)
            ix[ox] = (int) (ox / scale);
        const int *const ix_end = ix + owidth;
        for (int oy = 0; oy < oheight; oy++)
        {
            int iy = (int) (oy / scale);
            const img_pixel_t *const irow = ibuf + iwidth * iy;
            for (const int *i = ix; i < ix_end; i++)
            *orow++ = irow[*i];
        }
        delete[] ix;
    }
    // (Slightly) optimized version for large zoom factors.
    else
    {
        size_t pixels_at_a_time = (int) (scale + 0.99999999999);
        int *ox = new int[iwidth];
        for (int ix = 0; ix < iwidth; ix++)
            ox[ix] = (int) (ix * scale);
        for (int oy = 0; oy < oheight; oy++)
        {
            int iy = (int) (oy / scale);
            const img_pixel_t *const irow = ibuf + iwidth * iy;
            img_pixel_t       *const orow = obuf + owidth * oy;
            for (int ix = 0; ix < iwidth; ix++)
                memset (orow + ox[ix], irow[ix], pixels_at_a_time);
        }
        fflush (stdout);
        delete[] ox;
    }
}
