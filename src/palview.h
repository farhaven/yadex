/*
 *	palview.h
 *	Palette (PLAYPAL & COLORMAP) viewer
 *	AYM 1999-11-11
 */


#ifndef YH_PALVIEW
#define YH_PALVIEW


class Palette_viewer
{
  public :
    void run ();

  private :
    void draw_cursor (int c, bool phase);

    static const int ncolours = DOOM_COLOURS;
    static const int pixels   = 16;
    static const int columns  = 16;
    int              i;
    int              ofs;
    int              ix0;
    int              iy0;
};


#endif
