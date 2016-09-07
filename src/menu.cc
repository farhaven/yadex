/*
 *	menu.cc
 *	AYM 1998-12-01
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


/*
These are the pop-up and pull-down menus.

Events that are not understood (E.G. [Left] and [Right])
are put back in the input buffer before the
function returns.
*/


#include "yadex.h"
#include "events.h"
#include "gfx.h"
#include "menu.h"
#include "menudata.h"


const char *MI_SEPARATION = "";

const unsigned char MIF_MACTIVE = 0x03;
const unsigned char MIF_MTICK = 0x0c;
const unsigned char MIF_SEPAR = 0x10;

const short VSPACE = 2;				// Pixels between items


/* 
 *	Menu_item - a menu item class
 */
class Menu_item
{
  public :
    Menu_item ()
    {
      flags          = MIF_NTICK | MIF_NACTIVE;
      tilde_key      = YK_;
      shortcut_key   = YK_;
      str            = 0;
    }

    unsigned char flags;	// Logical-or of MIF_?ACTIVE MIF_?TICK MIF_SEPAR
    inpev_t       tilde_key;
    inpev_t       shortcut_key;
    short         y;		// Top of item, relative to ly0. If there is a
				// separation, it is above y.
    const char    *str;
    union
    {
      bool s;			// Static
      bool *v;			// Pointer to variable
      struct
      {
	micb_t f;		// Pointer to callback function
	micbarg_t a;		// Argument
      } f;
    }
    tick;
    union
    {
      bool s;			// Static
      bool *v;			// Pointer to variable
      struct
      {
	micb_t f;		// Pointer to callback function
	micbarg_t a;		// Argument
      } f;
    }
    active;
};


/*
 *	Menu_priv - Menu's private data
 */
class Menu_priv {
	public :
		Menu_priv () {
			flags         = 0;
			title         = "";
			title_len     = 0;
			items_ks_len  = 0;
			items_len     = 0;
			items.clear ();
			user_ox0      = -1;
			user_oy0      = -1;
			ox0           = -1;
			oy0           = -1;
			need_geom     = true;
			visible       = false;
			visible_disp  = false;
			line          = 0;
			line_disp     = UINT_MAX;
			item_height   = FONTH + VSPACE;
		}

		void vinit (Menu& m, const string title, va_list argp);
		void cook ();
		void draw ();
		void draw_one_line (size_t line, bool highlighted);
		void geom ();
		int process_event (const input_status_t *is);

    // Menu data
		unsigned char flags;	// MF_MENUDATA
				//          The strings come from a Menu_data.
				// MF_POPUP Used as popup (not pull-down) menu
				//          If set, the title is shown and
				//          button releases are treated
				//          differently.
				// MF_TICK  At least one item can be ticked
				//          (has the MIF_[SVF]TICK flag).
				// MF_SHORTCUT  At least one item has a key
				//          shortcut key.
				// MF_NUMS  Force the use of standard
				//          shortcuts [0-9a-zA-Z] even if
				//          index/key shortcuts exist.
		string title;		// empty if no title
		size_t     title_len;	// Length of <title> (or 0 if none)
		size_t     items_ks_len;	// Length of the longest item counting key sc.
		size_t     items_len;	// Length of the longest item not counting k.s.
		const Menu_data *menudata;
		std::vector<Menu_item> items;

    // Geometry as the user would like it to be
		int    user_ox0;		// Top left corner. < 0 means centered.
		int    user_oy0;		// Top left corner. < 0 means centered.

		// Geometry as it should be
		int    ix0, iy0, ix1, iy1;	// Corners of inner edge of window
		int    ox0, oy0, ox1, oy1;	// Corners of outer edge of window
		int    width;		// Overall width of window
		int    height;		// Overall height of window
		int    ty0;			// Y-pos of title
		int    ly0;			// Y-pos of first item

		// Geometry as it is displayed
		int    ox0_disp;
		int    oy0_disp;
		int    width_disp;
		int    height_disp;

		// Status
		bool   need_geom;		// Need to call geom() again
		bool   visible;		// Should the menu be visible ?
		bool   visible_disp;	// Is it really visible ?
		size_t line;		// Which line should be highlighted ?
		size_t line_disp;		// Which line is actually highlighted ?
		inpev_t _last_shortcut_key;	// Shortcut key for last selected item.

		// Misc
		unsigned short item_height;
};


/* First subscript :  0 = normal, 1 = greyed out
   Second subscript : 0 = normal, 1 = highlighted */
const colour_pair_t menu_colour[2][2] =
{
  {
    { WINBG,    WINFG        },    // Normal entry
    { WINBG_HL, WINFG_HL     }     // Normal entry, highlighted
  },
  {
    { WINBG,    WINFG_DIM    },    // Greyed out entry
    { WINBG_HL, WINFG_DIM_HL }     // Greyed out entry, highlighted
  }
};

const unsigned char MF_POPUP    = 0x01;
const unsigned char MF_TICK     = 0x02;
const unsigned char MF_TILDE    = 0x04;
const unsigned char MF_SHORTCUT = 0x08;
const unsigned char MF_NUMS     = 0x10;
const unsigned char MF_MENUDATA = 0x20;


/*
 *	Menu::Menu - ctor from arguments in variable number
 *
 *	Another ctor for Menu, suited to the creation of pull
 *	down menus.
 *
 *	Expects the title of the menu followed by repeats of
 *	either of the following sets of arguments, in any order :
 *
 *	1) Menu item
 *
 *	  A menu item is made of 3 or more arguments :
 *
 *	    const char *
 *			A NUL-terminated string that will be
 *			used to display the item. The first
 *			occurrence of a tilde ("~") in the
 *			string indicates that the following
 *			character will be used as a local
 *			shortcut for this item.
 *
 *			That string must remain valid and should
 *			not change during the lifetime of the
 *			Menu object.
 *
 *	    inpev_t	The global shortcut key associated to
 *			this item. That key will be displayed on
 *			the right of the menu.
 *
 *	    <option> ...
 *			Zero or more options.
 *
 *	    (unsigned char) 0
 *			The end of the item.
 *
 *	  Where <option> is any of the following pairs of
 *	  arguments :
 *
 *	    (unsigned char) MIF_STICK
 *	    bool ticked
 *			Indicates that this item can be ticked.
 *			If <ticked> is true, the item will be
 *			initially ticked. To change the state of
 *			the item, call set_ticked().
 *
 *	    (unsigned char) MIF_VTICK
 *	    bool *ticked
 *			Indicates that this item can be ticked.
 *			The argument is a pointer to a boolean
 *			variable. Whenever this item is
 *			displayed, the variable is read and the
 *			item is shown as ticked or unticked
 *			depending on whether the function
 *			returned true or false.
 *
 *	    (unsigned char) MIF_FTICK
 *	    boolcb_t ticked
 *	    void *arg
 *			Indicates that this item can be ticked.
 *			The argument is a pointer to a function
 *			taking only one argument (<arg>) and
 *			returning bool.  Whenever this item is
 *			displayed, the function is called and
 *			the item is shown as ticked or unticked
 *			depending on whether the function
 *			returned true or false.
 *
 *	    (unsigned char) MIF_SACTIVE
 *	    bool active
 *			Indicates that this item can be greyed
 *			out. If <active> is false the item
 *			will be initially greyed out. To change
 *			the state of the item, call
 *			set_active().
 *
 *	    (unsigned char) MIF_VACTIVE
 *	    bool *active
 *			Indicates that this item can be greyed
 *			out. The argument is a pointer to a
 *			boolean variable. Whenever this item is
 *			displayed, the variable is read and the
 *			item is shown as greyed out or not
 *			depending on whether the function
 *			returned false or true.
 *
 *	    (unsigned char) MIF_FACTIVE
 *	    boolcb_t active
 *			Indicates that this item can be greyed
 *			out. The argument is a pointer to a
 *			function taking only one argument
 *			(<arg>) and returning bool. Whenever
 *			this item is displayed, the function is
 *			called and the item is shown as greyed
 *			out or not depending on whether the
 *			function returned false or true.
 *
 *	2) Separation
 *
 *	  A separation is made of exactly 1 argument :
 *
 *	    (const char *) MI_SEPARATION
 *
 *	Pass a NULL pointer after the last group of arguments.
 *
 *	For a given item, the options MIF_STICK, MIF_VTICK and
 *	MIF_FTICK are mutually exclusive. For a given item, the
 *	options MIF_SACTIVE, MIF_VACTIVE and MIF_FACTIVE are
 *	mutually exclusive.
 *
 *	A MI_SEPARATION argument at the end of the list (i.e.
 *	not followed by an item) is ignored.
 *
 *	The ticked and greyed out states can change at any time
 *	but will not be reflected until the menu is redrawn from
 *	scratch.
 */
Menu::Menu (const string title, ...)
{
  priv = new Menu_priv;
  va_list argp;
  va_start (argp, title);
  priv->vinit (*this, title, argp);
  va_end (argp);
  priv->cook ();
}


/*
 *	Menu::Menu - ctor from an argument list
 *
 *	This is the same thing as Menu::Menu (const char
 *	*title, ...) except that it expects a pointer on the
 *	list of arguments.
 */
Menu::Menu (const string title, va_list argp)
{
  priv = new Menu_priv;
  priv->vinit (*this, title, argp);
  priv->cook ();
}


/*
 *	Menu::Menu - ctor from a list
 *
 *	Another ctor for Menu, suited to the creation of the
 *	menu from a list.
 */
Menu::Menu (
		const string title,
		al_llist_t   *list,
		const char   *(*getstr) (void *)) {
	priv = new Menu_priv;
	set_title (title);
	size_t nitems = y_min (al_lcount (list), 100);
	priv->items.resize (nitems);
	size_t line;
	for (al_lrewind (list), line = 0;
			! al_leol (list) && line < nitems;
			al_lstep (list), line++)
		priv->items[line].str = getstr (al_lptr (list));
	priv->cook();
}


/*
 *	Menu::Menu - ctor from a Menu_data
 */
Menu::Menu (const string title, const Menu_data& menudata) {
	priv = new Menu_priv;
	priv->menudata = &menudata;
	priv->flags |= MF_MENUDATA;
	set_title (title);
	size_t nitems = menudata.nitems ();
	priv->items.resize (nitems);
	priv->cook ();
}


/*
 *	Menu::~Menu - dtor
 */
Menu::~Menu () {
	delete priv;
}


/*
 *	Menu_priv::vinit - initialize the menu from an argument list
 */
void Menu_priv::vinit (Menu& m, const string title, va_list argp) {
	bool tick = false;

	m.set_title (title);

	while (items.size () < 100) {
		Menu_item i;

		const char *str = va_arg (argp, const char *);
		while (str == MI_SEPARATION) {
			i.flags |= MIF_SEPAR;
			str = va_arg (argp, const char *);
		}
		if (str == 0)
			break;

		i.str            = str;
		i.shortcut_key   = (inpev_t) va_arg (argp, int);
		unsigned char flag;
		while ((flag = (unsigned char) va_arg (argp, int)) != 0) {
			if (flag == MIF_SACTIVE) {
				i.flags  = (i.flags & ~MIF_MACTIVE) | MIF_SACTIVE;
				i.active.s = (bool) va_arg (argp, int);
			} else if (flag == MIF_VACTIVE) {
				i.flags  = (i.flags & ~MIF_MACTIVE) | MIF_VACTIVE;
				i.active.v = va_arg (argp, bool *);
			} else if (flag == MIF_FACTIVE) {
				i.flags  = (i.flags & ~MIF_MACTIVE) | MIF_FACTIVE;
				i.active.f.f = va_arg (argp, micb_t);
				i.active.f.a = va_arg (argp, micbarg_t);
			} else if (flag == MIF_STICK) {
				tick = true;
				i.flags  = (i.flags & ~MIF_MTICK) | MIF_STICK;
				i.tick.s = (bool) va_arg (argp, int);
			} else if (flag == MIF_VTICK) {
				tick = true;
				i.flags  = (i.flags & ~MIF_MTICK) | MIF_VTICK;
				i.tick.v = va_arg (argp, bool *);
			} else if (flag == MIF_FTICK) {
				tick = true;
				i.flags  = (i.flags & ~MIF_MTICK) | MIF_FTICK;
				i.tick.f.f = va_arg (argp, micb_t);
				i.tick.f.a = va_arg (argp, micbarg_t);
			} else if (flag != 0) {
				nf_bug ("Menu::ctor: flag %d", (int) flag);
			}
		}
		items.push_back (i);
	}

	if (tick)
		flags |= MF_TICK;
}


/*
 *	Menu_priv::cook - parse the menu item strings
 *
 *	Compute items_len, items_ks_len and prepare the cooked
 *	tilde shortcuts.
 */
void Menu_priv::cook ()
{
  items_len    = 0;
  items_ks_len = 0;
  short y      = 0;

  for (size_t line = 0; line < items.size (); line++)
  {
    Menu_item& i = items[line];
    if (i.shortcut_key != YK_)
      flags |= MF_SHORTCUT;
    if ((i.flags & MIF_MTICK) != MIF_NTICK)
      flags |= MF_TICK;
    if (i.flags & MIF_SEPAR)
      y += 2 * (NARROW_VSPACING + NARROW_BORDER);
    i.y = y;
    y += item_height;
    size_t len = 0;
    i.tilde_key = YK_;
    const char *str = (flags & MF_MENUDATA) ? (*menudata)[line] : i.str;
    for (const char *p = str; *p != '\0'; p++)
      if (p[0] == '~' && p[1] != '\0' && i.tilde_key == YK_)
      {
	i.tilde_key = tolower (p[1]);
	flags |= MF_TILDE;
      }
      else
	len++;
    size_t len_ks = len;
    if (i.shortcut_key != YK_)
      len_ks += strlen (key_to_string (i.shortcut_key)) + 2;
    if (len > items_len)
      items_len = len;
    if (len_ks > items_ks_len)
      items_ks_len = len_ks;
  }

  if (flags & MF_TICK)
  {
    items_len    += 2;		// Tick mark
    items_ks_len += 2;
  }
  if (flags & MF_SHORTCUT)
  {
    items_len    += 4;		// Space between strings and shortcut
    items_ks_len += 4;
  }
  if (! (flags & MF_TILDE) && ! (flags & MF_SHORTCUT))
    flags |= MF_NUMS;
  if (flags & MF_NUMS)
  {
    items_len    += 4;		// [1-9a-zA-Z] prefix
    items_ks_len += 4;
  }
}


/*
 *	Menu::set_title - set the title
 *
 *	Set the title of the menu (it's ignored unless the menu
 *	is set in popup mode).
 *
 *	Bug: changing the title does not take effect until the
 *	next display from scratch.
 */
void Menu::set_title (const string title) {
	priv->title = title;
	size_t title_len = title.length();

	/* If the length of the title has changed,
		force geom() to be called again. */
	if (title_len != priv->title_len)
		priv->need_geom = true;

	priv->title_len = title_len;
}


/*
 *	Menu::set_coords - position or reposition the menu window.
 *
 *	(<x0>,<y0>) is the top left corner.
 *	If <x0> is < 0, the window is horizontally centred.
 *	If <y0> is < 0, the window is vertically centred.
 */
void Menu::set_coords (int x0, int y0)
{
  if (x0 != priv->ox0 || y0 != priv->oy0)
    priv->need_geom = true;  // Force geom() to be called

  priv->user_ox0 = x0;
  priv->user_oy0 = y0;
}


/*
 *	Menu::set_item_no - set the current line
 *
 *	The current line number is set to <item_no>. The first
 *	line bears number 0.
 */
void Menu::set_item_no (int item_no)
{
  priv->line = item_no;
}


/*
 *	Menu::set_popup - set the popup flag
 *
 *	If <popup> is true, the popup flag is set. If <popup> is
 *	false, the popup flag is cleared.
 */
void Menu::set_popup (bool popup)
{
  if (popup != !! (priv->flags & MF_POPUP))
    priv->need_geom = true;  // Force geom() to be called
  if (popup)
    priv->flags |= MF_POPUP;
  else
    priv->flags &= ~MF_POPUP;
}


/*
 *	Menu::set_force_numbers - set the force_numbers flags
 *
 *	If <force_numbers> is true, the force_numbers flag is
 *	set. If <force_numbers> is false, the force_numbers flag
 *	is cleared.
 *
 *	The effect of the <force_numbers> flag is to disable key
 *	shortcuts and tilde shortcuts and to add automatic
 *	numbering of items ([1-9a-zA-Z]).
 *
 *	If none of the items has a tilde or key shortcut,
 *	<force_numbers> is automatically set. Otherwise, is it
 *	off by default.
 */
void Menu::set_force_numbers (bool force_numbers)
{
  if (force_numbers != !! (priv->flags & MF_NUMS))
    priv->need_geom = true;  // Force geom() to be called.
  if (force_numbers)
    priv->flags |= MF_NUMS;
  else
    priv->flags &= ~MF_NUMS;
}


/*
 *	Menu::set_visible - set the visible flag
 *
 *	If <visible> is true, the visible flag is set. If
 *	<visible> is false, the visible flag is cleared.
 */
void Menu::set_visible (bool visible)
{
  priv->visible = visible;
}


/*
 *	Menu::set_ticked - tick or untick a menu item
 *
 *	If <ticked> is true, item number <item_no> is ticked. If
 *	<ticked> is false, item number <item_no> is unticked.
 *	If the menu item was not created with the MIF_STICK
 *	option, emit a warning and return without doing
 *	anything.
 */
void Menu::set_ticked (size_t item_no, bool ticked)
{
  if (item_no >= priv->items.size ())
  {
    nf_bug ("Menu::set_ticked: item_no %lu", (unsigned long) item_no);
    return;
  }
  Menu_item& i = priv->items[item_no];
  if ((i.flags & MIF_MTICK) != MIF_STICK)
  {
    nf_bug ("Menu::set_ticked: flags %02X", i.flags);
    return;
  }
  i.tick.s = ticked;
}


/*
 *	Menu::set_active - grey-out or ungrey-out a menu
 *
 *	If <active> is false, item number <item_no> becomes
 *	greyed out. If <active> is true, item number <item_no>
 *	ceases to be greyed out. If the item was not created
 *	with with the MIF_SACTIVE option, emit a warning and
 *	return without doing anything.
 */
void Menu::set_active (size_t item_no, bool active)
{
  if (item_no >= priv->items.size ())
  {
    nf_bug ("Menu::set_active: item_no %lu", (unsigned long) item_no);
    return;
  }
  Menu_item& i = priv->items[item_no];
  if ((i.flags & MIF_MACTIVE) != MIF_SACTIVE)
  {
    nf_bug ("Menu::set_active: flags %02Xh", i.flags);
    return;
  }
  i.active.s = active;
}


/*
 *	Menu_priv::geom - recalculate the screen coordinates etc.
 */
void Menu_priv::geom () {
	size_t width_chars = 0;
	if (!title.empty() && (flags & MF_POPUP))
		width_chars = y_max (width_chars, title_len);
	if (flags & MF_NUMS)
		width_chars = y_max (width_chars, items_len + 4);
	else
		width_chars = y_max (width_chars, items_ks_len);
	int title_height = !title.empty() && (flags & MF_POPUP) ? (int) (1.5 * FONTH) : 0;

	width  = 2 * BOX_BORDER + 2 * WIDE_HSPACING + width_chars * FONTW;
	height = 2 * BOX_BORDER + 2 * WIDE_VSPACING + title_height
		+ items.back().y
		+ item_height;

	if (user_ox0 < 0)
		ox0 = (ScrMaxX - width) / 2;
	else
		ox0 = user_ox0;
	ix0 = ox0 + BOX_BORDER;
	ix1 = ix0 + 2 * WIDE_HSPACING + width_chars * FONTW - 1;
	ox1 = ix1 + BOX_BORDER;
	if (ox1 > ScrMaxX) {
		int overlap = ox1 - ScrMaxX;
		ox0 -= overlap;
		ix0 -= overlap;
		ix1 -= overlap;
		ox1 -= overlap;
	}

	if (user_oy0 < 0)
		oy0 = (ScrMaxY - height) / 2;
	else
		oy0 = user_oy0;
	iy0 = oy0 + BOX_BORDER;
	ty0 = iy0 + FONTH / 2;				// Title of menu
	ly0 = ty0 + title_height;				// First item of menu

	oy1 = oy0 + height - 1;
	iy1 = oy1 - BOX_BORDER;

	need_geom = false;
}


/*
 *	Menu::process_event - process an input event
 *
 *	Process event in *<is>.
 *
 *	Return one of the following :
 *	- MEN_CANCEL: user pressed [Esc] or clicked outside
 *	  the menu. The caller should delete the menu.
 *	- MEN_INVALID: we didn't understand the event so we put it
 *	  back in the input buffer.
 *	- MEN_OTHER: we understood the event and processed it.
 *	- the number of the item that was validated.
 */
int Menu::process_event (const input_status_t *is)
{
  return priv->process_event (is);
}


int Menu_priv::process_event (const input_status_t *is)
{
  size_t mouse_line;
  char status;

  if ((int) is->x < ix0 || (int) is->x > ix1 || (int) is->y < ly0)
    mouse_line = items.size ();
  else
  {
    for (mouse_line = 0; mouse_line < items.size (); mouse_line++)
      if ((int) is->y >= ly0 + items[mouse_line].y
	&& (int) is->y < ly0 + items[mouse_line].y + item_height)
	break;
  }

  status = 'i';

  // Clicking left button on an item: validate it.
  if (is->key == YE_BUTL_PRESS && mouse_line < items.size ())
  {
    line = mouse_line;  // Useless ?
    status = 'v';
  }

  // Moving over the box sets current line.
  else if (is->key == YE_MOTION && mouse_line < items.size ())
  {
    line = mouse_line;
    status = 'o';
  }

  /* Releasing the button while standing on an item: has a
     different effect depending on whether we're in pull-down or
     pop-up mode.

     In pull-down mode, the button was normally last pressed on
     the menu bar or on an item of this menu. So the current
     item is selected upon button release.

     In pop-up mode, if the button was pressed, it was most
     likely to exit a submenu (cf. the "thing type" menu) so we
     ignore the event. */
  else if (is->key == YE_BUTL_RELEASE
	&& mouse_line < items.size ()
	&& ! (flags & MF_POPUP))
    status = 'v';

  // [Enter], [Return]: accept selection
  else if (is->key == YK_RETURN)
    status = 'v';

  // [Esc]: cancel
  else if (is->key == YK_ESC)
    status = 'c';

  // [Up]: select previous line
  else if (is->key == YK_UP)
  {
    if (line > 0)
      line--;
    else
      line = items.size () - 1;
    status = 'o';
  }

  // [Down]: select next line
  else if (is->key == YK_DOWN)
  {
    if (line < items.size () - 1)
      line++;
    else
      line = 0;
    status = 'o';
  }

  // [Home]: select first line
  else if (is->key == YK_HOME)
  {
    line = 0;
    status = 'o';
  }

  // [End]: select last line
  else if (is->key == YK_END)
  {
    line = items.size () - 1;
    status = 'o';
  }

  // [Pgup]: select line - 5
  else if (is->key == YK_PU)
  {
    if (line >= 5)
      line -= 5;
    else
      line = 0;
    status = 'o';
  }

  // [Pgdn]: select line + 5
  else if (is->key == YK_PD)
  {
    if (line + 5 < items.size ())
      line += 5;
    else
      line = items.size () - 1;
    status = 'o';
  }

  // [1]-[9]: select items 0 through 8
  else if ((flags & MF_NUMS)
    && is->key < YK_
    && within (dectoi (is->key), 1, items.size ()))
  {
    line = dectoi (is->key) - 1;
    status = 'o';
    send_event (YK_RETURN);
  }

  // [a]-[z]: select items 9 through 34
  else if ((flags & MF_NUMS)
    && is->key < YK_
    && islower (is->key)
    && within (b36toi (is->key), 10, items.size ()))
  {
    line = b36toi (is->key) - 1;
    status = 'o';
    send_event (YK_RETURN);
  }

  // [A]-[Z]: select items 35 through 60
  else if ((flags & MF_NUMS)
    && is->key < YK_
    && isupper (is->key)
    && within (b36toi (is->key) + 26, 36, items.size ()))
  {
    line = b36toi (is->key) + 25;
    status = 'o';
    send_event (YK_RETURN);
  }

  // A shortcut ?
  else
  {
    /* First, check the list of tilde shortcuts
       (only if is->key is a regular key) */
    if ((flags & MF_TILDE)
        && ! (flags & MF_NUMS)
        && is->key == (unsigned char) is->key)
    {
      for (size_t n = 0; n < items.size (); n++)
	if (items[n].tilde_key != YK_
	    && items[n].tilde_key == tolower (is->key))
	{
	  line = n;
	  status = 'o';
	  send_event (YK_RETURN);
	  break;
	}
    }
    /* If no tilde shortcut matched, check the list of shortcut
       keys. It's important to do the tilde shortcuts first so
       that you can override a shortcut key (normally global)
       with a tilde shortcut (normally local). */
    if (status == 'i' && (flags & MF_SHORTCUT) && ! (flags & MF_NUMS))
    {
      for (size_t n = 0; n < items.size (); n++)
	if (items[n].shortcut_key != YK_
	    && items[n].shortcut_key == is->key)
	{
	  line = n;
	  status = 'o';
	  send_event (YK_RETURN);
	  break;
	}
    }
  }

  // See last_shortcut_key()
  if (status == 'v')
    _last_shortcut_key = (flags & MF_SHORTCUT) ? items[line].shortcut_key : 0;

  /* Return
     - the item# if validated,
     - MEN_CANCEL if cancelled,
     - MEN_OTHER or MEN_INVALID if neither. */
  if (status == 'v')
    return (int) line;
  else if (status == 'c')
    return MEN_CANCEL;
  else if (status == 'o')
    return MEN_OTHER;
  else if (status == 'i')
    return MEN_INVALID;
  else
  {
    // Can't happen
    fatal_error ("Menu::process_event: bad status %02Xh", status);
    return 0;  // To please the compiler
  }
}


/*
 *	Menu::last_shortcut_key - shortcut key for last selected item
 *
 *	Return the code of the shortcut key for the last
 *	selected item. This function shouldn't exist : it's just
 *	there because it helps editloop.cc. When real key
 *	bindings are implemented in editloop.cc,
 *	get_shortcut_key() should disappear.
 */
inpev_t Menu::last_shortcut_key ()
{
  return priv->_last_shortcut_key;
}


/*
 *	Menu::draw - display the menu
 *
 *	If necessary, redraw everything from scratch. Else, if
 *	<line> has changed, refresh the highlighted line.
 */
void Menu::draw () {
	priv->draw ();
}

void Menu_priv::draw () {
	bool from_scratch = false;

	if (need_geom)
		geom ();

	// Do we need to redraw everything from scratch ?
	if ((visible && ! visible_disp)
			|| (ox0    != ox0_disp)
			|| (oy0    != oy0_disp)
			|| (width  != width_disp)
			|| (height != height_disp))
		from_scratch = true;

	// Display the static part of the menu
	if (from_scratch) {
		DrawScreenBox3D (ox0, oy0, ox1, oy1);
		set_colour (WINTITLE);
		if ((flags & MF_POPUP) && !title.empty())
			DrawScreenString (ix0 + WIDE_HSPACING, ty0, title.c_str());

		for (size_t l = 0; l < items.size (); l++) {
			set_colour (WINFG);
			draw_one_line (l, false);
		}
		visible_disp = true;
		ox0_disp     = ox0;
		oy0_disp     = oy0;
		width_disp   = width;
		height_disp  = height;
	}

	// Display the "highlight" bar
	if (from_scratch || line != line_disp) {
		if (line_disp < items.size ())
			draw_one_line (line_disp, false);
		if (line < items.size ())
			draw_one_line (line, true);
		line_disp = line;
	}
}


/*
 *	Menu::draw_one_line - display just one line of a menu
 *
 *  	<line> is the number of the option to draw (0 = first
 *  	option). <highlighted> tells whether the option should
 *  	be drawn highlighted.
 */
void Menu_priv::draw_one_line (size_t line, bool highlighted)
{
  const Menu_item& i = items[line];
  int x      = ix0 + FONTW;
  int y      = ly0 + i.y;
  int text_y = y + VSPACE / 2;


  // Separation ?
  if (i.flags & MIF_SEPAR)
  {
    push_colour (WINBG_DARK);
    short groove_y = y - NARROW_VSPACING - 2 * NARROW_BORDER;
    DrawScreenLine (ix0, groove_y, ix1, groove_y);
    set_colour (WINBG_LIGHT);
    DrawScreenLine (ix0, groove_y + 1, ix1, groove_y + 1);
    pop_colour ();
  }

  // Greyed out ?
  bool active = true;
  switch (i.flags & MIF_MACTIVE)
  {
    case MIF_NACTIVE:
      active = true;
      break;

    case MIF_SACTIVE:
      active = i.active.s;
      break;

    case MIF_VACTIVE:
      active = *i.active.v;
      break;

    case MIF_FACTIVE:
      active = i.active.f.f (i.active.f.a);
      break;

    default:
      nf_bug ("Menu::draw_one_line: active %02Xh", i.flags);
      break;
  }
  set_colour (menu_colour[! active][highlighted].bg);
  DrawScreenBox (ix0, y, ix1, y + item_height - 1);
  set_colour (menu_colour[! active][highlighted].fg);

  // Tick mark if any
  if (flags & MF_TICK)
  {
    bool have_tick = false;
    bool ticked    = false;
    switch (i.flags & MIF_MTICK)
    {
      case MIF_NTICK:
        have_tick = false;
        break;

      case MIF_STICK:
	have_tick = true;
        ticked    = i.tick.s;
        break;

      case MIF_VTICK:
        have_tick = true;
        ticked    = *i.tick.v;
        break;

      case MIF_FTICK:
        have_tick = true;
        ticked    = i.tick.f.f (i.tick.f.a);
        break;

      default:
        nf_bug ("Menu::draw_one_line: tick %02Xh", i.flags);
        break;
    }
    if (have_tick)
    {
      if (ticked)
      {
	unsigned hside = FONTW * 4 / 5;
	unsigned vside = FONTH * 4 / 5;
	int x0 = x + (FONTW - hside) / 2;
	int y0 = y + (FONTH - vside) / 2;
	DrawScreenLine (x0, y0 + vside / 2, x0 + hside / 2, y0 + vside - 1);
	DrawScreenLine (x0 + hside / 2, y0 + vside - 1, x0 + hside - 1, y0);
      }
      else
      {
	unsigned margin = FONTW / 5;
	DrawScreenLine (x + margin,             y + FONTH / 2,
			x + FONTW - 1 - margin, y + FONTH / 2);
      }
    }
    x += 2 * FONTW;
  }

  // Automatic keys if any
  if (flags & MF_NUMS)
  {
    char c = '\0';
    if (line <= 8)
      c = '1' + line;
    else if (line >= 9 && line < 9 + 26)
      c = 'a' + line - 9;
    else if (line >= 9 + 26 && line < 9 + 26 + 26)
      c = 'A' + line - (9 + 26);
    if (c != '\0')
    {
      push_colour (highlighted ? WINLABEL_HL : WINLABEL);
      DrawScreenString (x, text_y, "[ ]");
      pop_colour ();
      DrawScreenChar (x + FONTW, text_y,         c);
      DrawScreenChar (x + FONTW, text_y + FONTU, '_');
    }
    x += 4 * FONTW;
  }

  // Text
  int tilde_index = -1;
  {
    const char *str = (flags & MF_MENUDATA) ? (*menudata)[line] : i.str;
    char *buf = new char[strlen (str) + 1]; 
    char *d   = buf;
    for (const char *s = str; *s != '\0'; s++)
    {
      if (*s == '~' && tilde_index < 0)
      {
	tilde_index = s - str;
	continue;
      }
      *d++ = *s;
    }
    *d = '\0';
    DrawScreenString (x, text_y, buf);
    delete[] buf;
  }

  // Underscore the tilde shortcut if any
  if (! (flags & MF_NUMS) && tilde_index >= 0)
    DrawScreenString (x + tilde_index * FONTW, text_y + FONTU, "_");

  // Shortcut key if any
  if (! (flags & MF_NUMS) && i.shortcut_key != YK_)
  {
    const char *s = key_to_string (i.shortcut_key);
    DrawScreenString (ix1 + 1 - FONTW - strlen (s) * FONTW, text_y, s);
  }
}


/*
 *	WIDGET METHODS
 */


void Menu::undraw ()
{
  ;  // I can't undraw myself
}


int Menu::can_undraw ()
{
  return 0;  // I can't undraw myself
}


int Menu::need_to_clear ()
{
  return (!priv->visible && priv->visible_disp) || priv->need_geom;
}


void Menu::clear ()
{
  priv->visible_disp = false;
}


int Menu::req_width ()
{
  if (priv->need_geom)
    priv->geom ();
  return priv->width;
}


int Menu::req_height ()
{
  if (priv->need_geom)
    priv->geom ();
  return priv->height;
}


int Menu::get_x0 ()
{
  return priv->ox0_disp;
}


int Menu::get_y0 ()
{
  return priv->oy0_disp;
}


int Menu::get_x1 ()
{
  return priv->ox0_disp + priv->width_disp - 1;
}


int Menu::get_y1 ()
{
  return priv->oy0_disp + priv->height_disp - 1;
}

