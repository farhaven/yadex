/*
 *	img.cc - Game image object (255 colours + transparency)
 *	AYM 2000-06-13
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
#include "help1.h"
#include "img.h"
#include "wadfile.h"
#include "wads.h"


// Holds the private data members
class Img_priv
{
  public:
    Img_priv () { buf = 0; width = 0; height = 0; opaque = false; }
    ~Img_priv () { if (buf != 0) delete[] buf; }
    img_pixel_t *buf;
    img_dim_t    width;
    img_dim_t    height;
    bool         opaque;
};


/*
 *	Img::Img - default constructor
 *
 *	The new image is a null image.
 */
Img::Img ()
{
  p = new Img_priv;
}


/*
 *	Img::Img - constructor with dimensions
 *
 *	The new image is set to the specified dimensions.
 */
Img::Img (img_dim_t width, img_dim_t height, bool opaque)
{
  p = new Img_priv;
  resize (width, height);
  set_opaque (opaque);
}


/*
 *	Img::~Img - dtor
 */
Img::~Img ()
{
  delete p;
}


/*
 *	Img::is_null - return true iff this is a null image
 */
bool Img::is_null () const
{
  return p->buf == 0;
}


/*
 *	Img::width - return the current width
 *
 *	If the image is null, return 0.
 */
img_dim_t Img::width () const
{
  return p->width;
}


/*
 *	Img::height - return the current height
 *
 *	If the image is null, return 0.
 */
img_dim_t Img::height () const
{
  return p->height;
}


/*
 *	Img::buf - return a const pointer on the buffer
 *
 *	If the image is null, return a null pointer.
 */
const img_pixel_t *Img::buf () const
{
  return p->buf;
}


/*
 *	Img::wbuf - return a writable pointer on the buffer
 *
 *	If the image is null, return a null pointer.
 */
img_pixel_t *Img::wbuf ()
{
  return p->buf;
}


/*
 *	Img::clear - clear the image
 */
void Img::clear ()
{
  if (p->buf != 0)
    memset (p->buf, IMG_TRANSP, p->width * p->height);
}


/*
 *	Img::set_opaque - set or clear the opaque flag
 */
void Img::set_opaque (bool opaque)
{
  p->opaque = opaque;
}

 
/*
 *	Img::resize - resize the image
 *
 *	If either dimension is zero, the image becomes a null
 *	image.
 */
void Img::resize (img_dim_t width, img_dim_t height)
{
  if (width == p->width && height == p->height)
    return;

  // Unallocate old buffer
  if (p->buf != 0)
  {
    delete[] p->buf;
    p->buf = 0;
  }

  // Is it a null image ?
  if (width == 0 || height == 0)
  {
    p->width  = 0;
    p->height = 0;
    return;
  }

  // Allocate new buffer
  p->width  = width;
  p->height = height;
  p->buf = new img_pixel_t[width * height + 10];  // Some slack
  clear ();
}


/*
 *	Img::save - save an image to file in packed PPM format
 *
 *	Return 0 on success, non-zero on failure
 *
 *	If an error occurs, errno is set to:
 *	- ECHILD if PLAYPAL could not be loaded
 *	- whatever fopen() or fclose() set it to
 */
int Img::save (const char *filename) const
{
  int rc = 0;
  FILE *fp = 0;

  // Load palette 0 from PLAYPAL
  MDirPtr dir = FindMasterDir (MasterDir, "PLAYPAL");
  if (dir == 0)
  {
    errno = ECHILD;
    return 1;
  }
  unsigned char *pal = new unsigned char[768];
  dir->wadfile->seek (dir->dir.start);
  if (dir->wadfile->error ())
  {
    /*warn ("%s: can't seek to %lXh\n",
	dir->wadfile->filename, (unsigned long) ftell (dir->wadfile->fp));
    warn ("PLAYPAL: seek error\n");*/
    rc = 1;
    errno = ECHILD;
    goto byebye;
  }
  dir->wadfile->read_bytes (pal, 768);
  if (dir->wadfile->error ())
  {
    /*warn ("%s: read error", dir->wadfile->where ());
    warn ("PLAYPAL: read error\n");*/
    rc = 1;
    errno = ECHILD;
    goto byebye;
  }

  // Create PPM file
  fp = fopen (filename, "wb");
  if (fp == NULL)
  {
    rc = 1;
    goto byebye;
  }
  fputs ("P6\n", fp);
  fprintf (fp, "# %s\n", what ());
  fprintf (fp, "%d %d 255\n", p->width, p->height);
  {
    const img_pixel_t *pix    = p->buf;
    const img_pixel_t *pixmax = pix + (unsigned long) p->width * p->height;
    for (; pix < pixmax; pix++)
    {
      if (*pix == IMG_TRANSP && ! p->opaque)
      {
	putc ( 0, fp);	// DeuTex convention, rgb:0/2f/2f
	putc (47, fp);
	putc (47, fp);
      }
      else
      {
	putc (pal[3 * *pix    ], fp);
	putc (pal[3 * *pix + 1], fp);
	putc (pal[3 * *pix + 2], fp);
      }
    }
  }
  if (ferror (fp))
    rc = 1;

byebye:
  if (fp != 0)
    if (fclose (fp))
      rc = 1;
  delete[] pal;
  return rc;
}


