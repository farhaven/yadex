/*
 *	x11.h
 *	X11-specific functions
 *	AYM 1999-08-03
 */


/* Declarations that don't rely on the X11 headers */
#if defined Y_X11 && ! defined Y_X11_H1
#define Y_X11_H1
void x_bell ();
void x_catch_on ();
void x_catch_off ();
void x_clear_error ();
const char *x_error ();
#endif


/* Declarations that do */
#if defined Y_X11 && defined X_PROTOCOL && ! defined Y_X11_H2
#define Y_X11_H2
int x_error_handler (Display *dpy, XErrorEvent *e);
#endif


