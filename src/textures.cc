/*
 *	textures.cc
 *	Trevor Phillips, RQ and Christian Johannes Schladetsch
 *	sometime in 1994.
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
#include "dialog.h"
#include "game.h"      /* yg_picture_format */
#include "gfx.h"
#include "lists.h"
#include "patchdir.h"
#include "pic2img.h"
#include "sticker.h"
#include "textures.h"
#include "wadfile.h"
#include "wads.h"
#include "wstructs.h"


/*
   display a wall texture ("TEXTURE1" or "TEXTURE2" object)
   at coords c->x0, c->y0
*/
void DisplayWallTexture (hookfunc_comm_t *c)
{
MDirPtr  dir = 0;	// Main directory pointer to the TEXTURE* entries
int      n;		// General counter
int16_t      width, height;	// Size of the texture
int16_t      npatches;	// Number of patches in the textures
int32_t      numtex;	// number of texture names in TEXTURE* list
int32_t      texofs;	// Offset in the wad file to the texture data
char     tname[WAD_TEX_NAME + 1];	/* texture name */
char     picname[WAD_PIC_NAME + 1];	/* wall patch name */
bool     have_dummy_bytes;
int      header_size;
int      item_size;

// So that, on failure, the caller clears the display area
c->disp_x0 = c->x1;
c->disp_y0 = c->y1;
c->disp_x1 = c->x0;
c->disp_y1 = c->y0;

// Iwad-dependant details
if (yg_texture_format == YGTF_NAMELESS)
   {
   have_dummy_bytes = true;
   header_size      = 14;
   item_size        = 10;
   }
else if (yg_texture_format == YGTF_NORMAL)
   {
   have_dummy_bytes = true;
   header_size      = 14;
   item_size        = 10;
   }
else if (yg_texture_format == YGTF_STRIFE11)
   {
   have_dummy_bytes = false;
   header_size      = 10;
   item_size        = 6;
   }
else
   {
   nf_bug ("Bad texture format %d.", (int) yg_texture_format);
   return;
   }

// Offset for texture we want
texofs = 0;
// Doom alpha 0.4 : "TEXTURES", no names
if (yg_texture_lumps == YGTL_TEXTURES && yg_texture_format == YGTF_NAMELESS)
   {
   const char *lump_name = "TEXTURES";
   dir = FindMasterDir (MasterDir, lump_name);
   if (dir != NULL)
      {
      const Wad_file *wf = dir->wadfile;
      wf->seek (dir->dir.start);
      if (wf->error ())
	 {
	 warn ("%s: can't seek to lump\n", lump_name);
	 goto alpha04_done;
	 }
      wf->read_int32_t (&numtex);
      if (wf->error ())
	 {
	 warn ("%s: error reading texture count\n", lump_name);
	 goto alpha04_done;
	 }
      if (WAD_TEX_NAME < 7) nf_bug ("WAD_TEX_NAME too small");  // Sanity
      if (! y_strnicmp (c->name, "TEX", 3)
	    && isdigit (c->name[3])
	    && isdigit (c->name[4])
	    && isdigit (c->name[5])
	    && isdigit (c->name[6])
	    && c->name[7] == '\0')
	 {
	 long num;
	 if (sscanf (c->name + 3, "%4ld", &num) == 1
	       && num >= 0 && num < numtex)
	    {
	    wf->seek (dir->dir.start + 4 + 4 * num);
	    if (wf->error ())
	       {
	       // FIXME print name of offending texture (or #)
	       warn ("%s: can't seek to offsets table entry\n", lump_name);
	       goto alpha04_done;
	       }
	    wf->read_int32_t (&texofs);
	    if (wf->error ())
	       {
	       warn ("%s: error reading texture offset\n", lump_name);
	       goto alpha04_done;
	       }
	    texofs += dir->dir.start;
	    }
	 }
      }
   alpha04_done:
   ;
   }
// Doom alpha 0.5 : only "TEXTURES"
else if (yg_texture_lumps == YGTL_TEXTURES
   && (yg_texture_format == YGTF_NORMAL || yg_texture_format == YGTF_STRIFE11))
   {
   const char *lump_name = "TEXTURES";
   dir = FindMasterDir (MasterDir, lump_name);
   if (dir != NULL)  // (Theoretically, it should always exist)
      {
      int32_t *offsets = NULL;  // Array of offsets to texture names
      const Wad_file *wf = dir->wadfile;

      wf->seek (dir->dir.start);
      if (wf->error ())
	 {
	 warn ("%s: can't seek to lump\n", lump_name);
	 goto textures_done;
	 }
      wf->read_int32_t (&numtex);
      if (wf->error ())
	 {
	 warn ("%s: error reading texture count\n", lump_name);
	 goto textures_done;
	 }
      // Read in the offsets for texture1 names and info
      offsets = (int32_t *) malloc((long) numtex * 4);
      wf->read_int32_t (offsets, numtex);
      if (wf->error ())
	 {
	 warn ("%s: error reading offsets table\n", lump_name);
	 goto textures_done;
	 }
      for (long n = 0; n < numtex && !texofs; n++)
	 {
	 wf->seek (dir->dir.start + offsets[n]);
	 if (wf->error ())
	    {
	    warn ("%s: error seeking to definition of texture #%ld\n",
	      lump_name, n);
	    break;
	    }
	 wf->read_bytes (&tname, WAD_TEX_NAME);
	 if (wf->error ())
	    {
	    warn ("%s: error reading name of texture #%ld\n", lump_name, n);
	    break;
	    }
	 if (!y_strnicmp (tname, c->name, WAD_TEX_NAME))
	    texofs = dir->dir.start + offsets[n];
	 }
      textures_done:
      if (offsets != NULL)
	 free(offsets);
      }
   }
// Other iwads : "TEXTURE1" and "TEXTURE2"
else if (yg_texture_lumps == YGTL_NORMAL
   && (yg_texture_format == YGTF_NORMAL || yg_texture_format == YGTF_STRIFE11))
   {
   // Is it in TEXTURE1 ?
   {
   const char *lump_name = "TEXTURE1";
   dir = FindMasterDir (MasterDir, lump_name);
   if (dir != NULL)  // (Theoretically, it should always exist)
      {
      const Wad_file *wf = dir->wadfile;
      int32_t *offsets = NULL;  // Array of offsets to texture names

      wf->seek (dir->dir.start);
      if (wf->error ())
	 {
	 warn ("%s: can't seek to lump\n", lump_name);
	 goto texture1_done;
	 }
      wf->read_int32_t (&numtex);
      if (wf->error ())
	 {
	 warn ("%s: error reading texture count\n", lump_name);
	 goto texture1_done;
	 }
      // Read in the offsets for texture1 names and info
      offsets = (int32_t *) malloc((long) numtex * 4);
      wf->read_int32_t (offsets, numtex);
      if (wf->error ())
	 {
	 warn ("%s: error reading offsets table\n", lump_name);
	 goto texture1_done;
	 }
      for (long n = 0; n < numtex && !texofs; n++)
	 {
	 wf->seek (dir->dir.start + offsets[n]);
	 if (wf->error ())
	    {
	    warn ("%s: error seeking to definition of texture #%ld\n",
	      lump_name, n);
	    break;
	    }
	 wf->read_bytes (&tname, WAD_TEX_NAME);
	 if (wf->error ())
	    {
	    warn ("%s: error reading name of texture #%ld\n", lump_name, n);
	    break;
	    }
	 if (!y_strnicmp (tname, c->name, WAD_TEX_NAME))
	    texofs = dir->dir.start + offsets[n];
	 }
      texture1_done:
      if (offsets != NULL)
	 free(offsets);
      }
   }
   // Well, then is it in TEXTURE2 ?
   if (texofs == 0)
      {
      const char *lump_name = "TEXTURE2";
      int32_t *offsets = NULL;  // Array of offsets to texture names

      dir = FindMasterDir (MasterDir, lump_name);
      if (dir != NULL)  // Doom II has no TEXTURE2
	 {
	 const Wad_file * wf = dir->wadfile;
	 wf->seek (dir->dir.start);
	 if (wf->error ())
	    {
	    warn ("%s: can't seek to lump\n", lump_name);
	    goto texture2_done;
	    }
	 wf->read_int32_t (&numtex);
	 if (wf->error ())
	    {
	    warn ("%s: error reading texture count\n", lump_name);
	    goto texture2_done;
	    }
	 // Read in the offsets for TEXTURE2 names
	 offsets = (int32_t *) malloc((long) numtex * 4);
	 wf->read_int32_t (offsets, numtex);
	 if (wf->error ())
	    {
	    warn ("%s: error reading offsets table\n", lump_name);
	    goto texture2_done;
	    }
	 for (long n = 0; n < numtex && !texofs; n++)
	    {
	    wf->seek (dir->dir.start + offsets[n]);
	    if (wf->error ())
	       {
	       warn ("%s: error seeking to definition of texture #%ld\n",
		 lump_name, n);
	       break;
	       }
	    wf->read_bytes (&tname, WAD_TEX_NAME);
	    if (wf->error ())
	       {
	       warn ("%s: error reading name of texture #%ld\n", lump_name, n);
	       break;
	       }
	    if (!y_strnicmp (tname, c->name, WAD_TEX_NAME))
	       texofs = dir->dir.start + offsets[n];
	    }
	 texture2_done:
	    free(offsets);
	 }
      }
   }
else
   nf_bug ("Invalid texture_format/texture_lumps combination.");

// Texture name not found
if (texofs == 0)
   return;

// Read the info for this texture
int32_t header_ofs;
if (yg_texture_format == YGTF_NAMELESS)
   header_ofs = texofs;
else
   header_ofs = texofs + WAD_TEX_NAME;
dir->wadfile->seek     (header_ofs + 4);
// FIXME
dir->wadfile->read_int16_t (&width);
// FIXME
dir->wadfile->read_int16_t (&height);
// FIXME
if (have_dummy_bytes)
   {
   int16_t dummy;
   dir->wadfile->read_int16_t (&dummy);
   dir->wadfile->read_int16_t (&dummy);
   }
dir->wadfile->read_int16_t (&npatches);
// FIXME

c->width    = width;
c->height   = height;
c->npatches = npatches;
c->flags   |= HOOK_SIZE_VALID | HOOK_DISP_SIZE;

/* Clip the texture to size. Done *after* setting c->width and
   c->height so that the selector shows the unclipped size. */
width  = y_min (width,  c->x1 - c->x0 + 1);
height = y_min (height, c->y1 - c->y0 + 1);

// Compose the texture
c->img.resize (width, height);
c->img.set_opaque (false);

/* Paste onto the buffer all the patches that the texture is
   made of. Unless c->npatches is non-zero, in which case we
   paste only the first maxpatches ones. */
int maxpatches = npatches;
if (c->maxpatches != 0 && c->maxpatches <= npatches)
   maxpatches = c->maxpatches;
for (n = 0; n < maxpatches; n++)
   {
   hookfunc_comm_t subc;
   int16_t xofs, yofs;	// Offset in texture space for the patch
   int16_t pnameind;	// Index of patch in PNAMES

   dir->wadfile->seek (header_ofs + header_size + (long) n * item_size);
   dir->wadfile->read_int16_t (&xofs);
   dir->wadfile->read_int16_t (&yofs);
   dir->wadfile->read_int16_t (&pnameind);
   if (have_dummy_bytes)
      {
      int16_t stepdir;
      int16_t colormap;
      dir->wadfile->read_int16_t (&stepdir);   // Always 1, unused.
      dir->wadfile->read_int16_t (&colormap);  // Always 0, unused.
      }

   /* AYM 1998-08-08: Yes, that's weird but that's what Doom
      does. Without these two lines, the few textures that have
      patches with negative y-offsets (BIGDOOR7, SKY1, TEKWALL1,
      TEKWALL5 and a few others) would not look in the texture
      viewer quite like in Doom. This should be mentioned in
      the UDS, by the way. */
   if (yofs < 0)
      yofs = 0;

   Lump_loc loc;
   {
     wad_pic_name_t *name = patch_dir.name_for_num (pnameind);
     if (name == 0)
       {
       warn ("texture \"%.*s\": patch %2d has bad index %d.\n",
	  WAD_TEX_NAME, tname, (int) n, (int) pnameind);
       continue;
       }
     patch_dir.loc_by_name ((const char *) *name, loc);
     *picname = '\0';
     strncat (picname, (const char *) *name, sizeof picname - 1);
#ifdef DEBUG
     printf ("Texture \"%.*s\": Patch %2d: #%3d %-8.8s (%d, %d)\n",
       c->name, (int) n, (int) pnameind, picname, (int) xofs, (int) yofs);
#endif
   }

   subc.x0 = y_max (c->x0, c->x0 + xofs);
   subc.y0 = y_min (c->y0, c->y0 + yofs);
   subc.x1 = c->x1;
   subc.x1 = c->y1;
   subc.name = picname;

   if (LoadPicture (c->img, picname, loc, xofs, yofs, 0, 0))
      warn ("texture \"%.*s\": patch \"%.*s\" not found.\n",
	  WAD_TEX_NAME, tname, WAD_PIC_NAME, picname);
   }

// Display the texture
Sticker sticker (c->img, true);		// Use opaque because it's faster
sticker.draw (drw, 't', c->x0, c->y0);
c->flags |= HOOK_DRAWN;
c->disp_x0 = c->x0 + c->xofs;
c->disp_y0 = c->y0 + c->yofs;
c->disp_x1 = c->disp_x0 + width - 1;
c->disp_y1 = c->disp_y0 + height - 1;

}


/*
   Function to get the size of a wall texture
*/
void GetWallTextureSize (int16_t *width, int16_t *height, const char *texname)
{
MDirPtr  dir = 0;			// Pointer in main directory to texname
int32_t    *offsets;			// Array of offsets to texture names
int      n;				// General counter
int32_t      numtex;			// Number of texture names in TEXTURE*
int32_t      texofs;			// Offset in wad for the texture data
char     tname[WAD_TEX_NAME + 1];	// Texture name

// Offset for texture we want
texofs = 0;
// Search for texname in TEXTURE1 (or TEXTURES)
if (yg_texture_lumps == YGTL_TEXTURES && yg_texture_format == YGTF_NAMELESS)
   {
   dir = FindMasterDir (MasterDir, "TEXTURES");
   if (dir != NULL)
      {
      dir->wadfile->seek (dir->dir.start);
      dir->wadfile->read_int32_t (&numtex);
      if (WAD_TEX_NAME < 7) nf_bug ("WAD_TEX_NAME too small");  // Sanity
      if (! y_strnicmp (texname, "TEX", 3)
	    && isdigit (texname[3])
	    && isdigit (texname[4])
	    && isdigit (texname[5])
	    && isdigit (texname[6])
	    && texname[7] == '\0')
	 {
	 long num;
	 if (sscanf (texname + 3, "%4ld", &num) == 1
	       && num >= 0 && num < numtex)
	    {
	    dir->wadfile->seek (dir->dir.start + 4 + 4 * num);
	    dir->wadfile->read_int32_t (&texofs);
	    }
	 }
      }
   }
else if (yg_texture_format == YGTF_NORMAL
    || yg_texture_format == YGTF_STRIFE11)
   {
   if (yg_texture_lumps == YGTL_TEXTURES)
      dir = FindMasterDir (MasterDir, "TEXTURES");  // Doom alpha 0.5
   else if (yg_texture_lumps == YGTL_NORMAL)
      dir = FindMasterDir (MasterDir, "TEXTURE1");
   else
      {
      dir = 0;
      nf_bug ("Invalid texture_format/texture_lumps combination.");
      }
   if (dir != NULL)
      {
      dir->wadfile->seek (dir->dir.start);
      dir->wadfile->read_int32_t (&numtex);
      // Read in the offsets for texture1 names and info
      offsets = (int32_t *) malloc((long) numtex * 4);
      dir->wadfile->read_int32_t (offsets, numtex);
      for (n = 0; n < numtex && !texofs; n++)
	 {
	 dir->wadfile->seek (dir->dir.start + offsets[n]);
	 dir->wadfile->read_bytes (&tname, WAD_TEX_NAME);
	 if (!y_strnicmp (tname, texname, WAD_TEX_NAME))
	    texofs = dir->dir.start + offsets[n];
	 }
      free(offsets);
      }
   if (texofs == 0 && yg_texture_lumps == YGTL_NORMAL)
      {
      // Search for texname in TEXTURE2
      dir = FindMasterDir (MasterDir, "TEXTURE2");
      if (dir != NULL)  // Doom II has no TEXTURE2
	 {
	 dir->wadfile->seek (dir->dir.start);
	 dir->wadfile->read_int32_t (&numtex);
	 // Read in the offsets for texture2 names
	 offsets = (int32_t *) malloc((long) numtex * 4);
	 dir->wadfile->read_int32_t (offsets);
	 for (n = 0; n < numtex && !texofs; n++)
	    {
	    dir->wadfile->seek (dir->dir.start + offsets[n]);
	    dir->wadfile->read_bytes (&tname, WAD_TEX_NAME);
	    if (!y_strnicmp (tname, texname, WAD_TEX_NAME))
	       texofs = dir->dir.start + offsets[n];
	    }
	 free(offsets);
	 }
      }
   }
else
   nf_bug ("Invalid texture_format/texture_lumps combination.");

if (texofs != 0)
   {
   // Read the info for this texture
   if (yg_texture_format == YGTF_NAMELESS)
      dir->wadfile->seek (texofs + 4L);
   else
      dir->wadfile->seek (texofs + 12L);
   dir->wadfile->read_int16_t (width);
   dir->wadfile->read_int16_t (height);
   }
else
   {
   // Texture data not found
   *width  = -1;
   *height = -1;
   }
}


/*
   choose a wall texture
*/
string ChooseWallTexture (int x0, int y0, const char *prompt, int listsize, char **list, string name) {
	return InputNameFromListWithFunc (x0, y0, prompt, listsize, list, 9, name,
			512, 256, DisplayWallTexture);
}
