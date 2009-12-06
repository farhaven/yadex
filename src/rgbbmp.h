/*
 *	rgbbmp.h
 *	Rgbbmp - An RGB bitmap image class.
 *
 *	This is a simple bitmap where each pixel is an RGB
 *	triplet. Each component is coded as an 8-bit unsigned
 *	integer (of type u8).
 *
 *	AYM 1999-06-06
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
 
 
#ifndef YH_RGBBMP
#define YH_RGBBMP


typedef struct
{
  u8 r;
  u8 g;
  u8 b;
} Rgbbmp_pixel_t;


class Rgbbmp
{
  public :

    Rgbbmp ()
    {
      _width = 0;
      _height = 0;
      pixel = 0;
    }

    void resize (int width, int height)  // Must be defined before first use
    {
      _width  = width;
      _height = height;
      if (pixel)
	delete[] pixel;
      if (width * height > 0)
        pixel = new Rgbbmp_pixel_t[width * height];
      else
	pixel = 0;
    };

    Rgbbmp (int width, int height)
    {
      pixel = 0;
      resize (width, height);
    }

    ~Rgbbmp ()
    {
      if (pixel)
	delete[] pixel;
    }

    int width () const
    {
      return _width;
    }

    int height () const
    {
      return _height;
    }

    void clear ()
    {
      if (pixel)
	memset (pixel, 0, _width * _height * sizeof *pixel);
    }

    void get (int x, int y, u8 &r, u8 &g, u8 &b) const
    {
      r = pixel[y * _width + x].r;
      g = pixel[y * _width + x].g;
      b = pixel[y * _width + x].b;
    }

    u8 get_r (int x, int y) const
    {
      return pixel[y * _width + x].r;
    }

    u8 get_g (int x, int y) const
    {
      return pixel[y * _width + x].g;
    }

    u8 get_b (int x, int y) const
    {
      return pixel[y * _width + x].b;
    }

    void set (int x, int y, u8 r, u8 g, u8 b)
    {
      pixel[y * _width + x].r = r;
      pixel[y * _width + x].g = g;
      pixel[y * _width + x].b = b;
    }

    void set_r (int x, int y, u8 r)
    {
      pixel[y * _width + x].r = r;
    }

    void set_g (int x, int y, u8 g)
    {
      pixel[y * _width + x].g = g;
    }

    void set_b (int x, int y, u8 b)
    {
      pixel[y * _width + x].b = b;
    }

  private :

    int _width;
    int _height;
    Rgbbmp_pixel_t *pixel;
};


#endif  /* DO NOT ADD ANYTHING AFTER THIS LINE */
