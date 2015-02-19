/*
 *	r_render.cc
 *	3D Rendering
 *	AJA 2002-04-21
 */


/*
This file is part of Yadex.

Yadex incorporates code from DEU 5.21 that was put in the public domain in
1994 by Raphaël Quinet and Brendon Wyber.

The rest of Yadex is Copyright © 1997-2000 André Majorel.

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
#include <vector>
#include <map>
#include <algorithm>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "levels.h"
#include "wstructs.h"
#include "gfx.h"
#include "img.h"
#include "sticker.h"
#include "gamesky.h"
#include "things.h"
#include "wadres.h"
#include "objid.h"
#include "objects.h"
#include "pic2img.h"
#include "rgb.h"
#include "gcolour2.h"
#include "dialog.h"

#include "r_render.h"
#include "r_images.h"


#define ML_UPPER_UNPEGGED  0x08
#define ML_LOWER_UNPEGGED  0x10

#define REND_HEIGHT 400

struct Y_View
{
public:
   int p_type, px, py;
   // player type and position.

   float x, y; 
   int z;
   // view position.

   static const int EYE_HEIGHT = 41;
   // standard height above the floor.

   float angle;
   float Sin, Cos;
   // view direction.

   int sw, sh;
   Img *screen;
   // screen image.

   bool texturing;
   bool sprites;
   bool walking;

   ImageCache *im_ch;

   int *thing_floors;

   Y_View () { memset (this, 0, sizeof *this);}

   void SetAngle (float new_ang)
      {
      angle = new_ang;

      if (angle >= TWOPI)
         angle -= TWOPI;
      else if (angle < 0)
         angle += TWOPI;

      Sin = sin (angle);
      Cos = cos (angle);
      }

   void CalcViewZ ()
      {
        Objid o;
        GetCurObject (o, OBJ_SECTORS, int (x), int (y));
      int secnum = o.num;
      if (secnum >= 0)
         z = Sectors[secnum].floorh + EYE_HEIGHT;
      }

   void ClearScreen ()
      {
      memset (screen->wbuf (), colour0, sw * sh);
      }

   void PutScreen (int x, int y)
      {
      DrawScreenBox3D (x, y, x + BOX_BORDER*2 + sw, y + BOX_BORDER*2 + sh);

      Sticker sticker (*screen, true);

      sticker.draw (drw, 't', x + BOX_BORDER, y + BOX_BORDER);
      }

   void FindThingFloors ()
   {
   thing_floors = new int[NumThings];

   for (int i = 0; i < NumThings; i++)
      {
        Objid o;
        GetCurObject (o, OBJ_SECTORS, Things[i].xpos, 
            Things[i].ypos);
      int secnum = o.num;
      
      if (secnum < 0)
         thing_floors[i] = 0;
      else
         thing_floors[i] = Sectors[secnum].floorh;
      }
   }
};


static Y_View view;


struct DrawSurf
{
public:
   enum
      {
      K_INVIS = 0,
      K_FLAT,
      K_TEXTURE
      };
   int kind;  

   int h1, h2, tex_h;
   // heights for the surface (h1 is above h2).

   Img *img;
   img_pixel_t col;  /* used if img is zero */

   enum
      {
      SOLID_ABOVE = 1,
      SOLID_BELOW = 2
      };
   int y_clip;

   /* CTor */

   DrawSurf () { kind = K_INVIS; img = 0; }

   void FindFlat (const wad_flat_name_t& fname, Sector *sec)
      {
      if (view.texturing)
         {
         img = view.im_ch->GetFlat (fname);

         if (img != 0)
            return;
         }
      col = 0x70 + ((sec - Sectors) % 48);
      }

   void FindTex (const string& tname, LineDef *ld)
      {
      if (view.texturing)
         {
         img = view.im_ch->GetTex (tname);

         if (img != 0)
            return;
         }
      col = 0x30 + ((ld - LineDefs) % 64);

      if (col >= 0x60)
         col += 0x70;
      }
};


struct DrawWall
{
public:
   typedef std::vector<struct DrawWall *> vec_t;

   Thing *th;
   // when `th' is non-zero, this is actually a sprite, and `ld' and
   // `sd' will be zero.  Sprites use the info in the `ceil' surface.

   LineDef *ld;
   SideDef *sd;
   Sector *sec;

   int side;
   // which side this wall faces (0 right, 1 left)

   float ang1, dang, cur_ang;
   float base_ang;
   // clipped angles

   float dist, t_dist;
   float normal;
   // line constants

   double iz1, diz, cur_iz; 
   double mid_iz;
   // distance values (inverted, so they can be lerped)

   float spr_tx1;
   // translate coord, for sprite

   int sx1, sx2;
   // screen X coordinates
 
   int oy1, oy2;
   // for sprites, the remembered open space to clip to

   /* surfaces */
   
   DrawSurf ceil;
   DrawSurf upper;
   DrawSurf lower;
   DrawSurf floor;

   static constexpr double IZ_EPSILON = 0.000001;

   /* PREDICATES */

   struct MidDistCmp
      {
      inline bool operator() (const DrawWall * A, const DrawWall * B) const
         {
         return A->mid_iz > B->mid_iz;
         }
      };

   struct DistCmp
      {
      inline bool operator() (const DrawWall * A, const DrawWall * B) const
         {
         if (fabs (A->cur_iz - B->cur_iz) < IZ_EPSILON)
            return A->diz > B->diz;

         return A->cur_iz > B->cur_iz;
         }
      };

   struct SX1Cmp
      {
      inline bool operator() (const DrawWall * A, const DrawWall * B) const
         {
         return A->sx1 < B->sx1;
         }

      inline bool operator() (const DrawWall * A, int x) const
         {
         return A->sx1 < x;
         }

      inline bool operator() (int x, const DrawWall * A) const
         {
         return x < A->sx1;
         }
      };

   struct SX2Less
      {
      int x;

      SX2Less (int _x) : x (_x) { }

      inline bool operator() (const DrawWall * A) const
         {
         return A->sx2 < x;
         }
      };

   /* methods */

   void ComputeWallSurface ()
      {
      Sector *front = sec;
      Sector *back  = 0;

      if (is_obj (side ? ld->sidedef1 : ld->sidedef2))
         {
         SideDef *bsd = &SideDefs[side ? ld->sidedef1 : ld->sidedef2];

         if (is_obj (bsd->sector))
            back = Sectors + bsd->sector;
         }

      bool sky_upper = back && is_sky (front->ceilt) && is_sky (back->ceilt);

      if ((front->ceilh > view.z || is_sky (front->ceilt)) && ! sky_upper) 
         {
         ceil.kind = DrawSurf::K_FLAT;
         ceil.h1 = +99999;
         ceil.h2 = front->ceilh;
         ceil.tex_h = ceil.h2;
         ceil.y_clip = DrawSurf::SOLID_ABOVE;

         if (is_sky (front->ceilt))
            ceil.col = sky_colour;
         else
            ceil.FindFlat (front->ceilt, front);
         }

      if (front->floorh < view.z)
         {
         floor.kind = DrawSurf::K_FLAT;
         floor.h1 = front->floorh;
         floor.h2 = -99999;
         floor.tex_h = floor.h1;
         floor.y_clip = DrawSurf::SOLID_BELOW;

         if (is_sky (front->floort))
            floor.col = sky_colour;
         else
            floor.FindFlat (front->floort, front);
         }

      if (! back)
         {
         /* ONE-sided line */

         lower.kind = DrawSurf::K_TEXTURE;
         lower.h1 = front->ceilh;
         lower.h2 = front->floorh;
         lower.y_clip = DrawSurf::SOLID_ABOVE | DrawSurf::SOLID_BELOW;

         lower.FindTex (sd->tex3, ld);

         if (lower.img && (ld->flags & ML_LOWER_UNPEGGED))
            lower.tex_h = lower.h2 + lower.img->height ();
         else
            lower.tex_h = lower.h1;
         }
      else
         {
         /* TWO-sided line */

         if (back->ceilh < front->ceilh && ! sky_upper)
            {
            upper.kind = DrawSurf::K_TEXTURE;
            upper.h1 = front->ceilh;
            upper.h2 = back->ceilh;
            upper.tex_h = upper.h1;
            upper.y_clip = DrawSurf::SOLID_ABOVE;

            upper.FindTex (sd->tex1, ld);

            if (upper.img && ! (ld->flags & ML_UPPER_UNPEGGED))
               upper.tex_h = upper.h2 + upper.img->height ();
            else
               upper.tex_h = upper.h1;
            }

         if (back->floorh > front->floorh)
            {
            lower.kind = DrawSurf::K_TEXTURE;
            lower.h1 = back->floorh;
            lower.h2 = front->floorh;
            lower.y_clip = DrawSurf::SOLID_BELOW;

            lower.FindTex (sd->tex2, ld);

            if (ld->flags & ML_LOWER_UNPEGGED)
               lower.tex_h = front->ceilh;
            else
               lower.tex_h = lower.h1;
            }
         }
      }
};


struct RendInfo
{
public:
   DrawWall::vec_t walls;
   // complete set of walls/sprites to draw.

   DrawWall::vec_t active;
   // the active list.  Pointers here are always duplicates of ones in
   // the walls list (no need to `delete' any of them).

   std::vector<double> depth_x;  
   // inverse distances over X range, 0 when empty.

   int open_y1;
   int open_y2;

   static constexpr double Y_SLOPE = 1.70;

   static void DeleteWall (DrawWall *P)
      {
      delete P;
      }

   ~RendInfo ()
      {
      std::for_each (walls.begin (), walls.end (), DeleteWall);
      
      walls.clear ();
      active.clear ();
      }

   void InitDepthBuf (int width)
      {
      depth_x.resize (width);

      std::fill_n (depth_x.begin (), width, 0);
      }

   static inline float PointToAngle (float x, float y)
      {
      if (-0.01 < x && x < 0.01)
         return (y > 0) ? HALFPI : (3 * HALFPI);

      float angle = atan2(y, x);

      if (angle < 0)
         angle += TWOPI;

      return angle;
      }

   static inline int AngleToX (float ang)
      {
      float t = tan (HALFPI - ang);

      int x = int (view.sw * t);

      x = (view.sw + x) / 2;

      if (x < 0)
         x = 0;
      else if (x > view.sw)
         x = view.sw;

      return x;
      }

   static inline float XToAngle (int x)
      {
      x = x * 2 - view.sw;

      float ang = HALFPI + atan (x / float (view.sw));

      if (ang < 0)
         ang = 0;
      else if (ang > ONEPI)
         ang = ONEPI;

      return ang;
      }

   static inline int DeltaToX (double iz, float tx)
      {
      int x = int (view.sw * tx * iz);

      x = (x + view.sw) / 2;

      return x;
      }

   static inline float XToDelta (int x, double iz)
      {
      x = x * 2 - view.sw;

      float tx = x / iz / view.sw;

      return tx;
      }

   static inline int DistToY (double iz, int sec_h)
      {
      if (sec_h > 32770)
         return -9999;

      if (sec_h < -32770)
         return +9999;

      sec_h -= view.z;

      int y = int (view.sh * sec_h * iz * Y_SLOPE);

      y = (view.sh - y) / 2;

      return y;
      }

   static inline float YToDist (int y, int sec_h)
      {
      sec_h -= view.z;

      y = y * 2 - view.sh;

      if (y == 0)
         return 999999;

      return view.sh * sec_h * Y_SLOPE / y;
      }

   static inline float YToSecH (int y, double iz)
      {
      y = y * 2 - view.sh;

      return view.z - (float (y) / view.sh / iz / Y_SLOPE);
      }

   void AddLine (int linenum)
      {
      LineDef *ld = LineDefs + linenum;

      if (! is_obj (ld->start) || ! is_obj (ld->end))
         return;

      float x1 = Vertices[ld->start].x - view.x;
      float y1 = Vertices[ld->start].y - view.y;
      float x2 = Vertices[ld->end].x - view.x;
      float y2 = Vertices[ld->end].y - view.y;

      float tx1 = x1 * view.Sin - y1 * view.Cos;
      float ty1 = x1 * view.Cos + y1 * view.Sin;
      float tx2 = x2 * view.Sin - y2 * view.Cos;
      float ty2 = x2 * view.Cos + y2 * view.Sin;

      // reject line if complete behind viewplane
      if (ty1 <= 0 && ty2 <= 0)
         return;

      float angle1 = PointToAngle (tx1, ty1);
      float angle2 = PointToAngle (tx2, ty2);
      float span = angle1 - angle2;

      if (span < 0)
         span += TWOPI;

      int side = 0;
      SideDef *sd;

      if (span >= ONEPI)
         side = 1;

      // ignore the line when there is no facing sidedef
      if (! is_obj (side ? ld->sidedef2 : ld->sidedef1))
         return;

      sd = &SideDefs[side ? ld->sidedef2 : ld->sidedef1];

      if (! is_obj (sd->sector))
         return;

      if (side == 1)
         {
         float tmp = angle1;
         angle1 = angle2;
         angle2 = tmp;
         }

      // clip angles to view volume

      float base_ang = angle1;

      float leftclip  = (3 * ONEPI / 4);
      float rightclip = ONEPI / 4;

      float tspan1 = angle1 - rightclip;
      float tspan2 = leftclip - angle2;

      if (tspan1 < 0) tspan1 += TWOPI;
      if (tspan2 < 0) tspan2 += TWOPI;

      if (tspan1 > HALFPI)
         {
         // Totally off the left edge?
         if (tspan2 >= ONEPI)
            return;

         angle1 = leftclip;
         }

      if (tspan2 > HALFPI)
         {
         // Totally off the left edge?
         if (tspan1 >= ONEPI)
            return;

         angle2 = rightclip;
         }

      // convert angles to on-screen X positions
      int sx1 = AngleToX (angle1);
      int sx2 = AngleToX (angle2) - 1;

      if (sx1 > sx2)
         return;

      // compute distance from eye to wall
      float wdx = x2 - x1;
      float wdy = y2 - y1;

      float wlen = sqrt (wdx * wdx + wdy * wdy);
      float dist = fabs ((y1 * wdx / wlen) - (x1 * wdy / wlen));

      if (dist < 0.01)
         return;

      // compute normal of wall (translated coords)
      float normal;

      if (side == 1)
         normal = PointToAngle (ty2 - ty1, tx1 - tx2);
      else
         normal = PointToAngle (ty1 - ty2, tx2 - tx1);

      // compute inverse distances
      double iz1 = cos (normal - angle1) / dist / cos (HALFPI - angle1);
      double iz2 = cos (normal - angle2) / dist / cos (HALFPI - angle2);

      double diz = (iz2 - iz1) / y_max (1, sx2 - sx1);

      // create drawwall structure

      DrawWall *dw = new DrawWall;

      dw->th = 0;
      dw->ld = ld;
      dw->sd = sd;
      dw->sec = Sectors + sd->sector;

      dw->side = side;

      dw->base_ang = base_ang;
      dw->ang1 = angle1;
      dw->dang = (angle2 - angle1) / y_max (1, sx2 - sx1);

      dw->dist = dist;
      dw->normal = normal;
      dw->t_dist = tan (base_ang - normal) * dist;

      dw->iz1 = iz1;
      dw->diz = diz;
      dw->mid_iz = iz1 + (sx2 - sx1 + 1) * diz / 2;

      dw->sx1 = sx1;  dw->sx2 = sx2;

      walls.push_back (dw);
      }

   void AddThing (int thingnum)
      {
      Thing *th = Things + thingnum;

      float x = th->xpos - view.x;
      float y = th->ypos - view.y;

      float tx = x * view.Sin - y * view.Cos;
      float ty = x * view.Cos + y * view.Sin;

      // reject sprite if complete behind viewplane
      if (ty < 4)
         return;

      Img *sprite = view.im_ch->GetSprite (th->type);
      if (! sprite)
         return;

      float tx1 = tx - sprite->width () / 2.0;
      float tx2 = tx + sprite->width () / 2.0;

      double iz = 1 / ty;

      int sx1 = DeltaToX (iz, tx1);
      int sx2 = DeltaToX (iz, tx2) - 1;

      if (sx1 < 0)
         sx1 = 0;

      if (sx2 >= view.sw)
         sx2 = view.sw - 1;

      if (sx1 > sx2)
         return;

      int h2 = view.thing_floors[thingnum];
      int h1 = h2 + sprite->height ();

      // create drawwall structure

      DrawWall *dw = new DrawWall;

      dw->th = th;
      dw->ld = 0;
      dw->sd = 0;
      dw->sec = 0;

      dw->spr_tx1 = tx1;

      dw->ang1 = dw->dang = 0;

      dw->iz1 = dw->mid_iz = iz;
      dw->diz = 0;

      dw->sx1 = sx1;  dw->sx2 = sx2;

      dw->ceil.img = sprite;
      dw->ceil.h1  = h1;
      dw->ceil.h2  = h2;

      walls.push_back (dw);
      }

   void ComputeSurfaces ()
      {
      DrawWall::vec_t::iterator S;

      for (S = walls.begin (); S != walls.end (); S++)
         if ((*S)->ld)
            (*S)->ComputeWallSurface ();
      }

   void ClipSolids ()
      {
      // perform a rough depth sort of the walls and sprites.

      std::sort (walls.begin (), walls.end (), DrawWall::MidDistCmp ());

      // go forwards, from closest to furthest away

      DrawWall::vec_t::iterator S;

      for (S = walls.begin (); S != walls.end (); S++)
         {
         DrawWall *dw = (*S);

         if (! dw)
            continue;

         int one_sided = dw->ld && ! is_obj (dw->ld->sidedef2);
         int vis_count = dw->sx2 - dw->sx1 + 1;

         for (int x = dw->sx1; x <= dw->sx2; x++)
            {
            double iz = dw->iz1 + (dw->diz * (x - dw->sx1));

            if (iz < depth_x[x])
               vis_count--;
            else if (one_sided)
               depth_x[x] = iz;
            }

         if (vis_count == 0)
            {
            delete dw;
            (*S) = 0;
            }
         }

      // remove null pointers

      S = std::remove (walls.begin (), walls.end (), (DrawWall *) 0);

      walls.erase (S, walls.end ());
      }

   void RenderFlatColumn (DrawWall *dw, DrawSurf& surf,
         int x, int y1, int y2)
      {
      img_pixel_t *buf = view.screen->wbuf ();
      img_pixel_t *wbuf = surf.img->wbuf ();

      int tw = surf.img->width ();
      int th = surf.img->height ();

      float ang = XToAngle (x);
      float modv = cos (ang - HALFPI);

      float t_cos = cos (ONEPI + -view.angle + ang) / modv;
      float t_sin = sin (ONEPI + -view.angle + ang) / modv;

      buf += x + y1 * view.sw;

      for (; y1 <= y2; y1++, buf += view.sw)
         {
         float dist = YToDist (y1, surf.tex_h);

         int tx = int ( view.x + t_sin * dist) & (tw - 1);
         int ty = int (-view.y - t_cos * dist) & (th - 1);

         *buf = wbuf[ty * tw + tx];
         }
      }

   void RenderTexColumn (DrawWall *dw, DrawSurf& surf,
         int x, int y1, int y2)
      {
      img_pixel_t *buf = view.screen->wbuf ();
      img_pixel_t *wbuf = surf.img->wbuf ();

      int tw = surf.img->width ();
      int th = surf.img->height ();

      /* compute texture X coord */

      int tx = int (dw->t_dist - tan (dw->cur_ang - dw->normal) * dw->dist);

      tx = (dw->sd->xoff + tx) & (tw - 1);

      /* compute texture Y coords */

      float base_h = surf.tex_h + dw->sd->yoff;

      float h1 = base_h - YToSecH (y1, dw->cur_iz);
      float dh = base_h - YToSecH (y2, dw->cur_iz);

      dh = (dh - h1) / y_max (1, y2 - y1);
       
      buf  += x + y1 * view.sw;
      wbuf += tx;

      for (; y1 <= y2; y1++, h1 += dh, buf += view.sw)
         {
         int ty = int (h1) % th;

         // handle negative values (use % twice)
         ty = (ty + th) % th;

         *buf = wbuf[ty * tw];
         }
      }

   void RenderSolidColumn (DrawWall *w, DrawSurf& surf,
         int x, int y1, int y2)
      {
      img_pixel_t *buf = view.screen->wbuf ();

      buf += x + y1 * view.sw;
       
      for (; y1 <= y2; y1++, buf += view.sw)
         {
         *buf = surf.col;
         }
      }

   inline void RenderWallSurface (DrawWall *dw, DrawSurf& surf, 
         int x)
      {
      if (surf.kind == DrawSurf::K_INVIS)
         return;

      int y1 = DistToY (dw->cur_iz, surf.h1);
      int y2 = DistToY (dw->cur_iz, surf.h2) - 1;

      if (y1 < open_y1)
         y1 = open_y1;

      if (y2 > open_y2)
         y2 = open_y2;

      if (y1 > y2)
         return;

      /* clip the open region */

      if (surf.y_clip & DrawSurf::SOLID_ABOVE)
         if (y2 > open_y1)
            open_y1 = y2;

      if (surf.y_clip & DrawSurf::SOLID_BELOW)
         if (y1 < open_y2)
            open_y2 = y1;

      /* fill pixels */

      if (! surf.img)
         {
         RenderSolidColumn (dw, surf, x, y1, y2);
         }
      else switch (surf.kind)
         {
         case DrawSurf::K_FLAT:
            RenderFlatColumn (dw, surf, x, y1, y2);
            break;

         case DrawSurf::K_TEXTURE:
            RenderTexColumn  (dw, surf, x, y1, y2);
            break;
         }
      }

   inline void RenderSprite (DrawWall *dw, int x)
      {
      int y1 = DistToY (dw->cur_iz, dw->ceil.h1);
      int y2 = DistToY (dw->cur_iz, dw->ceil.h2) - 1;

      if (y1 < dw->oy1)
         y1 = dw->oy1;

      if (y2 > dw->oy2)
         y2 = dw->oy2;

      if (y1 > y2)
         return;

      /* fill pixels */

      img_pixel_t *buf = view.screen->wbuf ();
      img_pixel_t *wbuf = dw->ceil.img->wbuf ();

      int tw = dw->ceil.img->width ();
      int th = dw->ceil.img->height ();

      int tx = int (XToDelta (x, dw->cur_iz) - dw->spr_tx1);

      if (tx < 0 || tx >= tw)
         return;

      float h1 = dw->ceil.h1 - YToSecH (y1, dw->cur_iz);
      float dh = dw->ceil.h1 - YToSecH (y2, dw->cur_iz);

      dh = (dh - h1) / y_max (1, y2 - y1);
       
      buf  += x + y1 * view.sw;
      wbuf += tx;

      for (; y1 <= y2; y1++, h1 += dh, buf += view.sw)
         {
         int ty = int (h1);

         if (ty < 0 || ty >= th)
            continue;

         img_pixel_t pix = wbuf[ty * tw];

         if (pix != IMG_TRANSP)
            *buf = pix;
         }
      }

   void UpdateActiveList (int x)
      {
      DrawWall::vec_t::iterator S, E, P;

      bool changes = false;

      // remove walls that have finished.

      S = active.begin ();
      E = active.end ();

      S = std::remove_if (S, E, DrawWall::SX2Less (x));

      if (S != E)
         {
         active.erase (S, E);
         changes = true;
         }

      // add new walls that start in this column.

      S = walls.begin ();
      E = walls.end ();

      S = std::lower_bound (S, E, x, DrawWall::SX1Cmp ());
      E = std::upper_bound (S, E, x, DrawWall::SX1Cmp ());

      if (S != E)
         changes = true;

      for (; S != E; S++)
         {
         active.push_back (*S);
         }

      // calculate new depth values

      S = active.begin ();
      E = active.end ();

      for (P=S; (P != E); P++)
         {
         DrawWall *dw = (*P);

         dw->cur_iz = dw->iz1 + dw->diz * (x - dw->sx1);

         if (P != S && (*(P-1))->cur_iz < dw->cur_iz)
            changes = true;

         dw->cur_ang = dw->ang1 + dw->dang * (x - dw->sx1);
         }

      // if there are changes, re-sort the active list...

      if (changes)
         {
         std::sort (active.begin (), active.end (), DrawWall::DistCmp ());
         }
      }

   void RenderWalls ()
      {
      // sort walls by their starting column, to allow binary search.

      std::sort (walls.begin (), walls.end (), DrawWall::SX1Cmp ());

      active.clear ();

      for (int x=0; x < view.sw; x++)
         {
         // clear vertical depth buffer

         open_y1 = 0;
         open_y2 = view.sh - 1;
	 // open_y2 = REND_HEIGHT - view.sh - view.view_z;

         UpdateActiveList (x);

         // render, front to back

         DrawWall::vec_t::iterator S, E, P;

         S = active.begin ();
         E = active.end ();

         for (P=S; P != E; P++)
            {
            DrawWall *dw = (*P);

            // for things, just remember the open space
            if (dw->th)
               {
               dw->oy1 = open_y1;
               dw->oy2 = open_y2;
               continue;
               }

            RenderWallSurface (dw, dw->ceil,  x);
            RenderWallSurface (dw, dw->floor, x);
            RenderWallSurface (dw, dw->upper, x);
            RenderWallSurface (dw, dw->lower, x);

            if (open_y1 >= open_y2)
               break;
            }

         // now render things, back to front

         if (P == E)
            P--;

         for (; P != (S-1); P--)
            {
            DrawWall *dw = (*P);

            if (dw->th)
               RenderSprite (dw, x);
            }
         }
      }

   void DoRender3D ()
      {
      view.ClearScreen ();

      InitDepthBuf (view.sw);

      for (int i=0; i < NumLineDefs; i++)
         AddLine (i);

      if (view.sprites)
         for (int j=0; j < NumThings; j++)
            AddThing (j);

      ClipSolids ();
      ComputeSurfaces ();
      RenderWalls ();
      }
};


static Thing *FindPlayer (int typenum)
{
for (int i=0; i < NumThings; i++)
   if (Things[i].type == THING_PLAYER1) //typenum)
      return Things + i;

for (int i=0; i < NumThings; i++)
	if (Things[i].type == THING_DEATHMATCH)
     		return Things + i;
		
return 0;
}


/*
 *  Render a 3D view from the player's position. 
 */

void Render3D ()
{
Thing *player = FindPlayer (view.p_type);

if (! player)
{	Notify (-1, -1, "No Player starts have been found.Please add", 
				"either a player 1 or a deathmatch start.");
      	return;
}

if (view.px != player->xpos || view.py != player->ypos)
   {
   // if player moved, re-create view parameters

   view.x = view.px = player->xpos;
   view.y = view.py = player->ypos;

   view.CalcViewZ ();
   view.SetAngle (player->angle * ONEPI / 180.0);
   }

/* create image */

view.sw = 320;
view.sh = 200;


view.screen = new Img ((unsigned short int) view.sw, (unsigned short int) view.sh, false);
view.im_ch = new ImageCache;

view.FindThingFloors ();

bool Redraw = true;

/* input loop */

for (;;)
   {
   /* render image */

   if (Redraw)
      {
      if (view.walking)
         view.CalcViewZ ();

      RendInfo rend;

      rend.DoRender3D ();

      view.PutScreen (40, 40);

      Redraw = false;
      }

   /* handle keypress */

   int key = get_key ();

   if (key == YK_ESC || key == 'q')
      break;

   if ((key & ~YK_SHIFT) == YK_LEFT)
      {
      view.SetAngle (view.angle + ONEPI / ((key & YK_SHIFT) ? 4 : 8));
      Redraw = true;
      }
   else if ((key & ~YK_SHIFT) == YK_RIGHT)
      {
      view.SetAngle (view.angle - ONEPI / ((key & YK_SHIFT) ? 4 : 8));
      Redraw = true;
      }
   else if ((key & ~YK_SHIFT) == YK_UP)
      {
      view.x += view.Cos * ((key & YK_SHIFT) ? 192 : 32);
      view.y += view.Sin * ((key & YK_SHIFT) ? 192 : 32);
      Redraw = true;
      }
   else if ((key & ~YK_SHIFT) == YK_DOWN)
      {
      view.x -= view.Cos * ((key & YK_SHIFT) ? 192 : 32);
      view.y -= view.Sin * ((key & YK_SHIFT) ? 192 : 32);
      Redraw = true;
      }
   else if (key == 'n' || key == 'N')
      {
      view.x -= view.Sin * ((key == 'N') ? 192 : 32);
      view.y += view.Cos * ((key == 'N') ? 192 : 32);
      Redraw = true;
      }
   else if (key == 'm' || key == 'M')
      {
      view.x += view.Sin * ((key == 'M') ? 192 : 32);
      view.y -= view.Cos * ((key == 'M') ? 192 : 32);
      Redraw = true;
      }
   else if (key == 'd' || key == 'D')
      {
      view.z += (key == 'D') ? 128 : 32;
      Redraw = true;
      }
   else if (key == 'c' || key == 'C')
      {
      view.z -= (key == 'C') ? 128 : 32;
      Redraw = true;
      }
   else if (key == 't')
      {
      view.texturing = ! view.texturing;
      Redraw = true;
      }
   else if (key == 's')
      {
      view.sprites = ! view.sprites;
      Redraw = true;
      }
   else if (key == 'w')
      {
      view.walking = ! view.walking;
      Redraw = true;
      }	 
   else if (key)
      {
      // key no good, get another one
      Beep ();
      }
   }

/* all done */

delete view.screen;
view.screen = 0;

delete view.im_ch;
view.im_ch = 0;

delete[] view.thing_floors;
view.thing_floors = 0;
}

