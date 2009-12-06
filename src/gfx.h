/*
 *	gfx.h
 *	AYM 1998-08-01
 */


#ifndef YH_GFX  /* Prevent multiple inclusion */
#define YH_GFX  /* Prevent multiple inclusion */


/* Width and height of font cell. Those figures are not meant to
   represent the actual size of the character but rather the step
   between two characters. For example, for the original 8x8 BGI
   font, FONTW was 8 and FONTH was 10. DrawScreenText() is supposed
   to draw characters properly _centered_ within that space, taking
   into account the optional underscoring. */
extern unsigned FONTW;
extern unsigned FONTH;
extern int font_xofs;
extern int font_yofs;


/* This number is the Y offset to use when underscoring a character.
   For example, if you want to draw an underscored "A" at (x,y),
   you should do :
     DrawScreenString (x, y,         "A");
     DrawScreenString (x, y + FONTU, "_"); */
#define FONTU  1

/* Narrow spacing around text. That's the distance in pixels
   between the characters and the inner edge of the box. */
#define NARROW_HSPACING  4
#define NARROW_VSPACING  2

/* Wide spacing around text. That's the distance in pixels
   between the characters and the inner edge of the box. */
#define WIDE_HSPACING  FONTW
#define WIDE_VSPACING  (FONTH / 2)

/* Boxes */
#define BOX_BORDER    2	 // Offset between outer and inner edges of a 3D box
#define NARROW_BORDER 1	 // Same thing for a shallow 3D box
#define HOLLOW_BORDER 1	 // Same thing for a hollow box
#define BOX_VSPACING  WIDE_VSPACING  // Vertical space between two hollow boxes

/* Parameters set by command line args and configuration file */
extern const char *BGIDriver;	// BGI: default extended BGI driver
extern bool  CirrusCursor;	// use HW cursor on Cirrus Logic VGA cards
extern bool  FakeCursor;	// use a "fake" mouse cursor
extern const char *font_name;	// X: the name of the font to load
				// (if NULL, use the default)
extern Win_dim initial_window_width;// X: the name says it all
extern Win_dim initial_window_height;// X: the name says it all
extern int   no_pixmap;		// X: use no pixmap -- direct window output
extern int   VideoMode;		// BGI: default video mode for VESA cards

/* Global variables */
extern int   GfxMode;		// current graphics mode, or 0 for text
extern int   OrigX;		// Map X-coord of centre of screen/window
extern int   OrigY;		// Map Y-coord of centre of screen/window
extern int   ScrCenterX;	// Display X-coord of center of screen/window
extern int   ScrCenterY;	// Display Y-coord of center of screen/window
#ifdef Y_X11
typedef unsigned long xpv_t;	// The type of a pixel value in X's opinion
#ifdef X_PROTOCOL
extern Display *dpy;
extern int      scn;
extern Colormap cmap;		// The X colormap
extern Window   win;
extern Drawable drw;
extern GC       gc;
extern Visual  *win_vis;	// The visual for win
extern int      win_depth;	// The depth of win in bits
extern int      win_bpp;	// The depth of win in bytes
extern int	x_server_big_endian;	// Is the X server big-endian ?
extern int      ximage_bpp;	// Number of bytes per pixels in XImages
extern int      ximage_quantum;	// Pad XImage lines to a mult of that many B.
#endif  // ifdef X_PROTOCOL
#endif  // ifdef Y_X11
extern int	text_dot;     // DrawScreenText()/DrawScreenString() debug flag

/* gfx.cc */
int InitGfx (void);
void SwitchToVGA256 (void);
void SwitchToVGA16 (void);
void TermGfx (void);
void SetWindowSize (int width, int height);
void ClearScreen (void);
void update_display ();
void force_window_not_pixmap ();
void SetLineThickness (int thick);
void SetDrawingMode (int _xor);
void DrawMapCircle (int, int, int);
void DrawMapLine (int mapx1, int mapy1, int mapx2, int mapy2);
void DrawMapVector (int, int, int, int);
void DrawMapArrow (int, int, unsigned);
void DrawScreenLine (int, int, int, int);
void DrawScreenRect (int x, int y, int width, int height);
void DrawScreenBox (int, int, int, int);
void DrawScreenBoxwh (int scrx0, int scry0, int width, int height);
void DrawScreenBox3D (int, int, int, int);
void DrawScreenBox3DShallow (int, int, int, int);
void DrawScreenBoxHollow (int x0, int y0, int x1, int y1, acolour_t colour);
void draw_box_border (int x, int y, int width, int height,
   int thickness, int raised);
void DrawScreenText (int, int, const char *, ...);
void DrawScreenString (int, int, const char *);
void DrawScreenChar (int x, int y, char c);
void DrawPointer (bool);
void DrawScreenMeter (int, int, int, int, float);
void DrawScreenLineLen (int x, int y, int width, int height);
int TranslateToDoomColor (int);
#if defined Y_BGI && defined CIRRUS_PATCH
void SetHWCursorPos (unsigned, unsigned);
void SetHWCursorCol (long, long);
void SetHWCursorMap (char *);
#endif /* Y_BGI && CIRRUS_PATCH */
#ifdef PCOLOUR_NONE
void set_pcolour (pcolour_t colour);
#endif
acolour_t get_colour ();
void set_colour (acolour_t);
void push_colour (acolour_t colour);
void pop_colour (void);
void draw_point (int x, int y);
void draw_map_point (int mapx, int mapy);


#endif  /* DO NOT ADD ANYTHING AFTER THIS LINE */
