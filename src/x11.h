/*
 *	x11.h
 *	X11-specific functions
 *	AYM 1999-08-03
 */


/* Declarations that don't rely on the X11 headers */
void x_bell ();
void x_catch_on ();
void x_catch_off ();
void x_clear_error ();
const char *x_error ();


/* Declarations that do */
#if defined X_PROTOCOL
int x_error_handler (Display *dpy, XErrorEvent *e);
#endif
