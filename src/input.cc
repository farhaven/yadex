/*
 *	input.cc
 *	User input (mouse and keyboard)
 *	AYM 1998-06-16
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
#include <time.h>	// nanosleep ()
#include <X11/Xlib.h>
#include <X11/Xutil.h>  // XLookupString
#include <X11/keysym.h>
#include "gfx.h"

/* This variable needs to be global because get_input_status() is called
   from different places and some state needs to be saved (shift, ctrl,
   alt...). I think there should be one instance of this type for each
   window that can have focus. But one instance for all windows might be
   enough. Multi-threading (one thread per edit window) is also an
   issue. Have to think of it. */
input_status_t is;

/*
 *	init_input_status
 *	Initialize <is>. Must be called before using <is> or calling
 *	get_input_status().
 */
void init_input_status ()
{
    is.in_window   = 0;
    is.width       = -1;
    is.height      = 0;
    is.butl        = 0;
    is.butm        = 0;
    is.butr        = 0;
    is.shift       = 0;
    is.ctrl        = 0;
    is.alt         = 0;
    is.scroll_lock = 0;
}


/*
 *	Static table for get_input_status to convert keysyms to one
 *	of the YK_-something codes.
 */
typedef struct
{
    KeySym ks;
    inpev_t key;
} key_info_t;

static const key_info_t key_info[] =
{
    { XK_BackSpace,	YK_BACKSPACE	},
#ifdef XK_ISO_Left_Tab  /* OpenServer 5.0 X11R5 doesn't have XK_ISO_Left_Tab */
    { XK_ISO_Left_Tab,	YK_BACKTAB	},
#endif
    { XK_Delete,    YK_DEL,		},
    { XK_Down,		YK_DOWN,	},
    { XK_End,		YK_END,		},
    { XK_Escape,	YK_ESC,		},
    { XK_F1,		YK_F1,		},
    { XK_F2,		YK_F2,		},
    { XK_F3,		YK_F3,		},
    { XK_F4,		YK_F4,		},
    { XK_F5,		YK_F5,		},
    { XK_F6,		YK_F6,		},
    { XK_F7,		YK_F7,		},
    { XK_F8,		YK_F8,		},
    { XK_F9,		YK_F9,		},
    { XK_F10,		YK_F10,		},
    { XK_Home,		YK_HOME,	},
    { XK_Insert,	YK_INS,		},
    { XK_Left,		YK_LEFT,	},
    { XK_Linefeed,	YK_RETURN,	},
#ifdef XK_Page_Down	/* HP-UX 10 doesn't have XK_Page_Down */
    { XK_Page_Down,	YK_PD,		},
#endif
#ifdef XK_Page_Up	/* HP-UX 10 doesn't have XK_Page_Up */
    { XK_Page_Up,	YK_PU,		},
#endif
    { XK_Return,	YK_RETURN,	},
    { XK_Right,		YK_RIGHT,	},
    { XK_Tab,		YK_TAB,		},
    { XK_Up,		YK_UP,		},
};

/*
 *	get_input_status
 *	Get the next event and update <is> accordingly.
 *	If no event is available, waits for idle_sleep_ms ms
 *	and returns (it's used for the autoscroll feature).
 */
void get_input_status ()
{
    XEvent ev;

    is.key = 0;

    if (! dpy)  /* Sanity check */
    fatal_error ("get_input_status() called before XOpenDisplay()");
    if (XPending (dpy) == 0)
    {
        // No event ? Wait for <idle_sleep_ms> ms before polling again.
        struct timespec treq = { 0, 1000000l * idle_sleep_ms };
        struct timespec trem;
        nanosleep (&treq, &trem);
        return;
    }

    XNextEvent (dpy, &ev);

    switch (ev.type)
    {
        /* Exposure */
        case Expose :
            if (ev.xexpose.window == win && ev.xexpose.count == 0)
                is.key = YE_EXPOSE;
            break;

        /* Resize */
        case ConfigureNotify :
            if (is.width < 0 || ev.xconfigure.width != is.width
            || ev.xconfigure.height != is.height)
            {
                is.key    = YE_RESIZE;
                is.width  = ev.xconfigure.width;
                is.height = ev.xconfigure.height;
            }
            break;

        /* Mouse motion */
        case EnterNotify :
            is.key       = YE_ENTER;
            is.time      = ev.xcrossing.time;
            is.in_window = 1;
            is.x = ev.xcrossing.x;
            is.y = ev.xcrossing.y;
            break;
        case LeaveNotify :
            is.key       = YE_LEAVE;
            is.time      = ev.xcrossing.time;
            is.in_window = 0;  /* Should probably "release" buttons */
            return;
            break;
        case MotionNotify :
            is.key  = YE_MOTION;
            is.time = ev.xmotion.time;
            is.x = ev.xmotion.x;
            is.y = ev.xmotion.y;
#ifdef DEBUG
            {
                static bool first_time = true;
                static int dxmin = INT_MAX;
                static int dxmax = INT_MIN;
                static int dymin = INT_MAX;
                static int dymax = INT_MIN;
                static int prevx = 0;
                static int prevy = 0;
                bool change = false;
                if (! first_time)
                {
                    int dx = prevx - ev.xmotion.x;
                    int dy = prevy - ev.xmotion.y;

                    if (dx < dxmin)
                    {
                        dxmin = dx;
                        change = true;
                    }
                    if (dx > dxmax)
                    {
                        dxmax = dx;
                        change = true;
                    }
                    if (dy < dymin)
                    {
                        dymin = dy;
                        change = true;
                    }
                    if (dy > dymax)
                    {
                        dymax = dy;
                        change = true;
                    }
                }
                prevx = ev.xmotion.x;
                prevy = ev.xmotion.y;
                first_time = false;
                if (change)
                    printf ("Mouse: xmin=%d, xmax=%d, ymin=%d, ymax=%d\n",
                dxmin, dxmax, dymin, dymax);
            }
#endif
            // DEBUG
            break;

        /* Mouse buttons */
        case ButtonPress :
        case ButtonRelease :
            {
                is.time = ev.xbutton.time;
                int press = (ev.type == ButtonPress);
                if (ev.xbutton.button == Button1)
                {
                    is.key = press ? YE_BUTL_PRESS : YE_BUTL_RELEASE;
                    is.butl = press;
                } else if (ev.xbutton.button == Button2)
                {
                    is.key = press ? YE_BUTM_PRESS : YE_BUTM_RELEASE;
                    is.butm = press;
                } else if (ev.xbutton.button == Button3)
                {
                    is.key = press ? YE_BUTR_PRESS : YE_BUTR_RELEASE;
                    is.butr = press;
                } else if (ev.xbutton.button == Button4)
                {
                    is.key = press ? YE_WHEEL_UP : 0;
                } else if (ev.xbutton.button == Button5)
                {
                    is.key = press ? YE_WHEEL_DOWN : 0;
                }
                break;
            }

        /*
        * Keyboard
        * FIXME: need to handle NotifyKeymap event as well.
        */
        case KeyPress :
        case KeyRelease :
        {
            KeySym ks;
            int press;
            unsigned char c;
            int has_string;

            is.time = ev.xkey.time;

            press = (ev.type == KeyPress);

            /* Convert keycode -> keysym + char. The keysym is useful for keys
            such as cursor arrows that don't have an ASCII code. */
            has_string = XLookupString ((XKeyEvent *) &ev, (char *) &c, 1, &ks, NULL);

            /* The event says that Ctrl, Alt and Shift are not in the state we
            thought they were. Don't panic ; it's just that we missed the
            modifier key press/release event as it happened when we didn't
            have focus. Adjust ourselves. */
            if (!! (ev.xkey.state & ShiftMask) != is.shift)
            is.shift = !! (ev.xkey.state & ShiftMask);
            if (!! (ev.xkey.state & ControlMask) != is.ctrl)
            is.ctrl = !! (ev.xkey.state & ControlMask);
            if (!! (ev.xkey.state & Mod1Mask) != is.alt)
            is.alt = !! (ev.xkey.state & Mod1Mask);

            /* It's a modifier ? Remember its state */
            switch (ks)
            {
                case XK_Shift_L:
                case XK_Shift_R:
                    is.shift = press;
                    break;
                case XK_Control_L:
                case XK_Control_R:
                    is.ctrl = press;
                    break;
                case XK_Alt_L:
                case XK_Alt_R:
                case XK_Meta_L:
                case XK_Meta_R:
                    is.alt = press;
                    break;
            }

            /* Process ordinary keys */
            if (press)
            {
                size_t n;
                if (has_string)
                    is.key = c;
                for (n = 0; n < sizeof key_info / sizeof *key_info; n++)
                    if (key_info[n].ks == ks)
                    {
                        is.key = key_info[n].key;
                        break;
                    }
                if (is.key >= YK_ && is.key != YK_BACKTAB && is.shift)
                    is.key |= YK_SHIFT;
                if (is.key >= YK_ && is.ctrl)
                    is.key |= YK_CTRL;
                if (is.key != 0 && is.alt)
                    is.key |= YK_ALT;
            }
            break;
        }
    }  /* switch (ev.type) */
}

/*
 *	has_input_event
 *	Tells whether there are events in the input queue
 */
int has_input_event ()
{
    XEvent xev;
    if (XCheckMaskEvent (dpy, 0xffffffff, &xev) == True)
    {
        XPutBackEvent (dpy, &xev);
        return 1;
    }
    return 0;
}

/*
 *	have_key
 *	Return 0 if there is no key press in the input queue, <>0 else.
 *	This is a convenience function to replace bioskey(1).
 */
int have_key ()
{
    return 1;  /* FIXME!! */
}

/*
 *	get_key
 *	Wait until the user presses a key and returns its code.
 *	This is a convenience function to replace bioskey(0).
 */
int get_key ()
{
    do
       get_input_status ();
    while (! event_is_key (is.key));
    return is.key;
}

/*
 *	get_key_or_click
 *	Wait until the user presses a key or clicks the left button.
 *	In most cases, you should use this and not get_key().
 */
void get_key_or_click ()
{
    do
       get_input_status ();
    while (! event_is_key (is.key) && is.key != YE_BUTL_PRESS);

    is.key = 0;  // FIXME Shouldn't have to do that but EditorLoop() is broken
}

/*
 *	key_to_string
 *	Return a string corresponding to the key number k.
 *	Examples :
 *	- for k equal to 'a', returns "a",
 *	- for k equal to YK_ALT + 'a', returns "Alt-a".
 *	The string returned is guaranteed to have a length <= 50.
 */
typedef struct
{
    inpev_t key;
    const char *string;
} key_string_t;

static const key_string_t key_string[] =
{
   { ' ',           "Space"     },
   { YK_BACKSPACE, 	"BS"		},
   { YK_BACKTAB,	"Shift-Tab"	},
   { YK_DEL,       	"Del"		},
   { YK_DOWN,      	"Down"		},
   { YK_END,		"End"		},
   { YK_ESC,		"Esc"		},
   { YK_F1,		    "F1"		},
   { YK_F2,		    "F2"		},
   { YK_F3,		    "F3"		},
   { YK_F4,		    "F4"		},
   { YK_F5,		    "F5"		},
   { YK_F6,		    "F6"		},
   { YK_F7,		    "F7"		},
   { YK_F8,		    "F8"		},
   { YK_F9,		    "F9"		},
   { YK_F10,		"F10"		},
   { YK_HOME,		"Home"		},
   { YK_INS,		"Ins"		},
   { YK_LEFT,		"Left"		},
   { YK_PD,		    "Pgdn"		},
   { YK_PU,		    "Pgup"		},
   { YK_RETURN,		"Return"	},
   { YK_RIGHT,		"Right"		},
   { YK_TAB,		"Tab"		},
   { YK_UP,		    "Up"		},
};

const char *key_to_string (inpev_t k)
{
    static char buf[51];

    // Is one of the special keys ?
    size_t n;
    const size_t nmax = sizeof key_string / sizeof *key_string;
    for (n = 0; n < nmax; n++)
        if (key_string[n].key == k)
            break;

    *buf = '\0';
    if (k & YK_CTRL || (n == nmax && k <= 31))
    {
        strlcat(buf, "Ctrl-", sizeof(buf));
        if (k & YK_CTRL)
            k ^= YK_CTRL;
        if (k <= 31)
            k += 96;  // Heavy ASCII-ism : 01h (^A) -> 61h ("a")
    }
    if (k & YK_ALT)
    {
        strlcat(buf, "Alt-", sizeof(buf));
        k ^= YK_ALT;
    }
    if (k & YK_SHIFT)
    {
        strlcat(buf, "Shift-", sizeof(buf));
        k ^= YK_SHIFT;
    }

    if (n == nmax)
    {
        if (k <= UCHAR_MAX && isprint (k))
            al_sapc (buf, k, sizeof buf - 1);
        else
        {
            al_sapc (buf, al_adigits[(k >> 12) & 15], sizeof buf - 1);
            al_sapc (buf, al_adigits[(k >>  8) & 15], sizeof buf - 1);
            al_sapc (buf, al_adigits[(k >>  4) & 15], sizeof buf - 1);
            al_sapc (buf, al_adigits[(k >>  0) & 15], sizeof buf - 1);
        }
    }
    else
        strlcat(buf, key_string[n].string, sizeof(buf));

    buf[sizeof buf - 1] = '\0';  /* Paranoia */
    return buf;
}

