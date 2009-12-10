/*
 *	entry2.cc
 *	A string entry box class
 *	AYM 1999-04-12
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
#include "entry2.h"
#include "gfx.h"


/*
 *	ctor
 */
Entry2::Entry2 (const char *title, const char *fmt, ...)
{
  this->title = title ? title : "(null)";
  nfields = count_widgets (fmt);
  box_len     = new unsigned short[nfields];
  buf         = new char *[nfields];
  buf_max_len = new unsigned short[nfields];
  caption     = new const char *[nfields];
  entry_drawn = new bool[nfields];
  entry_flags = new _field_flags_t[nfields];

  {
    va_list args;
    va_start (args, fmt);
    fill_in_widgets_info (fmt, args);
  }

  background_drawn = false;
  for (size_t f = 0; f < nfields; f++)
    entry_drawn[f] = false;
  field_no = 0;  // This seem redundant with jump_to_field (0)
  		 // but it's not. jump_to_field() really needs
		 // field_no to be initialized.
  jump_to_field (0);
  geom_up_to_date = false;
  win_x0 = -1;
  win_y0 = -1;

  // Geometry constants
  entry_hofs     = HOLLOW_BORDER + NARROW_HSPACING;
  entry_vofs     = HOLLOW_BORDER + NARROW_VSPACING;
  win_hofs       = BOX_BORDER + WIDE_HSPACING;
  win_vofs       = BOX_BORDER + WIDE_VSPACING;
  title_vspacing = FONTH;
}


/*
 *	dtor
 */
Entry2::~Entry2 ()
{
  if (buf)
    delete[] buf;
  if (buf_max_len)
    delete[] buf_max_len;
  if (box_len)
    delete[] box_len;
  if (entry_drawn)
    delete[] entry_drawn;
  if (entry_flags)
    delete[] entry_flags;
  if (caption)
    delete[] caption;
}

 
/*
 *	loop
 *	Block until the user closes the window.
 *	Return 0 on cancel, non-zero on valid.
 */
int Entry2::loop ()
{
  int r;
  for (;;)
  {
    get_input_status ();
    r = process_event (is);
    refresh ();
    if (r != ACT_NONE)
      break;
  }
  is.key = 0;  // FIXME Shouldn't have to do that but EditorLoop() is broken
  if (r == ACT_VALID)
    return 1;
  else if (r == ACT_CANCEL)
    return 0;
  else
  {
    printf ("Internal error: Entry2::process_event returned %d\n", (int) r);
    return 0;
  }
}


/*
 *	process_event
 */
Entry2_action_t Entry2::process_event (const input_status_t &is)
{
  if (is.key == YE_EXPOSE)
  {
    background_drawn = false;
    return ACT_NONE;
  }

  char *s = buf[field_no];
  bool redraw = false;
  size_t max_len = buf_max_len[field_no];
  size_t l = strlen (s);
  int key = is.key;
  _field_flags_t flags = entry_flags[field_no];

  if (first_key && is_ordinary (key))
  {
    *s = '\0';
    l = 0;
    redraw = true;
  }
  first_key = 0;

  // Printable character
  if (is_ordinary (key))
  {
    if (l >= max_len)
      goto reject_key;
    if (is_integer_entry (flags))
    {
      if (l == 1 && s[0] == '0')
      {
	if (key == 'x' || key == 'X')
	  ;
	else if (! isdigit (key))
	  goto reject_key;
        else if (key == '0')
	  goto reject_key;
	else
	  l = 0;
      }
      else if (! isdigit (key))
	goto reject_key;
    }
    if (is_string_entry (flags) && (flags & FF_UPPERCASE))
      key = toupper (key);
    s[l] = key;
    s[l + 1] = '\0';
    redraw = true;
  }
  // [BS]: Backspace
  else if (key == YK_BACKSPACE)
  {
    if (is_integer_entry (flags) && l == 1)
      strcpy (s, "0");
    else if (l > 0)
      s[l-1] = '\0';
    else
      goto reject_key;
    redraw = true;
  }
  // ^A, [Home]: Go to SOL (Emacs/readline, Doskey)
  else if (key == YK_HOME || key == 1)
  {
    ;
  }
  // ^B, [Left]: Go to previous character (Emacs, readline)
  else if (key == YK_LEFT || key == 2)
  {
    ;
  }
  // ^D, [Del]: Delete current character (Emacs, readline)
  else if (key == YK_DEL || key == 4)
  {
    ;
  }
  // ^E, [End]: Go to EOL (Emacs/readline, Doskey)
  else if (key == YK_END || key == 5)
  {
    ;
  }
  // ^F, [Right]: Go to next character (Emacs/readline)
  else if (key == YK_RIGHT || key == 6)
  {
    ;
  }
  // ^K, [Ctrl][End]: Del to EOL (Emacs/readline, Doskey)
  else if (key == YK_END + YK_CTRL || key == 11)
  {
    ;
  }
  // ^U, [Ctrl][Home]: Del to SOL (readline, Doskey)
  else if (key == YK_HOME + YK_CTRL || key == 21)
  {
    if ((flags & FF_SUBTYPE) == FF_INTEGER)
      strcpy (s, "0");
    else
      *s = '\0';
    redraw = true;
  }
  // ^W: Del last word (Unix, readline)
  else if (key == 23)
  {
    while (l > 0 && isspace (s[l - 1]))
      l--;
    while (l > 0 && ! isspace (s[l - 1]))
      l--;
    if (is_integer_entry (flags) && l == 0)
      strcpy (s, "0");
    else
      s[l] = '\0';
    redraw = true;
  }
  // ^X: Decrement (Vim)
  else if (key == 24)
  {
    if (! is_integer_entry (flags))
      goto reject_key;
    //v--;
    //sprintf (s, "%", v);
  }
  // [Tab]: Next field
  else if (key == YK_TAB)
    next_field ();
  // [Shift][Tab]: Previous field
  else if (key == YK_BACKTAB)
    prev_field ();
  // [Return]: Validate
  else if (key == YK_RETURN)
  {
    size_t f;
    for (f = 0; f < nfields; f++)  // No FF_NONEMPTY fields must be empty
      if (   (entry_flags[f] & FF_TYPE) == FF_ENTRY
	  && (entry_flags[f] & FF_SUBTYPE) == FF_STRING
	  && (entry_flags[f] & FF_NONEMPTY)
	  && buf[f][0] == '\0')
	break;
    if (f == nfields)
      return ACT_VALID;
    else
    {
      jump_to_field (f);
      Beep ();
    }
  }
  // [Esc]: Cancel
  else if (key == YK_ESC)
    return ACT_CANCEL;
  else
  {
reject_key :
    Beep ();
  }

  if (redraw)
    entry_drawn[field_no] = false;
  return ACT_NONE;
}


/*
 *	refresh
 *	Update the display
 */
void Entry2::refresh ()
{
  /* Draw the background (the part that never
     changes unless the window is obscured). */
  if (! background_drawn)
  {
    if (! geom_up_to_date)
      do_geom ();

    DrawScreenBox3D (win_x0, win_y0, win_x1, win_y1);
    push_colour (WINTITLE);
    DrawScreenString (title_x0, title_y0, title);
    set_colour (WINFG);
    for (size_t f = 0; f < nfields; f++)
    {
      size_t yofs = f * vstep;
      DrawScreenString (caption_x0, caption_y0 + yofs, caption[f]);
      DrawScreenBoxHollow (entry_box_x0,
			   entry_box_y0 + yofs,
	  		   entry_box_x0 + box_len[f] * FONTW + 2*entry_hofs - 1,
			   entry_box_y1 + yofs,
			   BLACK);
    }
    pop_colour ();
  }

  /* Draw the foreground (the part that might
     change as a result of the user actions). */
  for (size_t f = 0; f < nfields; f++)
  {
    if (! background_drawn || ! entry_drawn[f])
    {
      size_t l = strlen (buf[f]);
      int yofs = f * vstep;
      set_colour (BLACK);
      DrawScreenBox (entry_text_x0,
		     entry_text_y0 + yofs,
		     entry_text_x0 + box_len[f] * FONTW - 1,
		     entry_text_y1 + yofs);
      set_colour (WINFG);
      if (l >= box_len[f])
      {
	DrawScreenText (entry_text_x0, entry_text_y0 + yofs, "<%s",
	  buf[f] + l - (box_len[f] - 2));
      }
      else
	 DrawScreenString (entry_text_x0, entry_text_y0 + yofs, buf[f]);
      if (f != field_no)
        entry_drawn[f] = true;
    }
  }

  // Draw the cursor
  if (! background_drawn || ! entry_drawn[field_no])
  {
    push_colour (WINFG);
    int cur_pos = strlen (buf[field_no]);
    if (cur_pos >= box_len[field_no])
      cur_pos = box_len[field_no] - 1;
    int x = entry_text_x0 + cur_pos * FONTW;
    int y = entry_text_y0 + field_no * vstep;
    DrawScreenBoxwh (x, y, FONTW, FONTH);
    pop_colour ();
    entry_drawn[field_no] = true;
  }

  background_drawn = true;
}


/*
 *	PRIVATE METHODS
 */


/*
 *	count_widget
 *	Return the number of widgets found in <fmt>.
 */
int Entry2::count_widgets (const char *fmt)
{
  int nwidgets = 0;
  for (const char *p = fmt; *p; p++)
  {
    if (p[0] == '%' && p[1] != '%')
      nwidgets++;
  }
  return nwidgets;
}


/*
 *	fill_in_widgets_info
 *	Parse <fmt> and fill in the widgets info arrays.
 *	Return 0 on success, non-zero on error.
 */
int Entry2::fill_in_widgets_info (const char *fmt, va_list args)
{
  size_t f = 0;
  const char *last_literal = 0;
  const int F_PLUS     = 1;
  const int F_MINUS    = 2;
  const int F_ZERO     = 4;
  const int L_OMITTED  = 65535;
  const int L_ASTERISK = 65534;
  
  /* First step: parse <fmt> and fill in
     <box_len>, <buf_max_len>, <box_len>,
     <caption> <entry_flags>. */

  for (const char *p = fmt;;)
  {
    if (*p == '%')
    {
      p++;
      // Got %%
      if (*p == '%')
      {
	if (! last_literal)
	  last_literal = p - 1;
	p++;
	continue;
      }
      // Got a real %-field
      if (f >= nfields)
      {
	printf ("Internal error: at offset %d in widget fmt \"%s\": more than"
	    " %d fields.\nIgnoring excess field(s)\n",
	    (int) (p - fmt), fmt, (int) f);
	break;
      }
      int flags     = 0;
      int length    = L_OMITTED;
      int precision = L_OMITTED;
      char type      = 0;
      // Get the flag
      for (;;)
      {
	if (*p == '+')
	{
	  flags |= F_PLUS;
	  p++;
	}
	else if (*p == '-')
	{
	  flags |= F_MINUS;
	  p++;
	}
	else if (*p == '0')
	{
	  flags |= F_ZERO;
	  p++;
	}
	else
	  break;
      }
      // Get the optional length
      if (*p == '*')
      {
	length = L_ASTERISK;
	p++;
      }
      else if (isdigit ((unsigned char) *p))
      {
	length = 0;
	while (isdigit ((unsigned char) *p))
	  length = 10 * length + dectoi (*p++);
      }
      // Get the optional precision
      if (*p == '.')
      {
	p++;
	if (*p == '*')
	{
	  precision = L_ASTERISK;
	  p++;
	}
	else if (! isdigit ((unsigned char) *p))
	{
	  printf ("Internal error: at offset %d in widget fmt \"%s\": bad"
	      " precision spec \"%c\"\n", (int) (p - fmt), fmt, *p);
	  break;
	}
	else
	{
	  precision = 0;
	  while (isdigit ((unsigned char) *p))
	    precision = 10 * precision + dectoi (*p++);
	}
      }
      // Get the optional modifier (hh, h, l or Z)
      _field_flags_t size = FF_INT;
      if (*p == 'h')
      {
	if (p[1] == 'h')
	{
	  size = FF_CHAR;
	  p += 2;
	}
	else
	{
	  size = FF_SHORT;
	  p++;
	}
      }
      else if (*p == 'Z')
      {
	size = FF_SIZE_T;
	p++;
      }
      else if (*p == 'l')
      {
	size = FF_LONG;
	p++;
      }
      // Get the type (diousSxX)
      if (*p == 'd' || *p == 'i' || *p == 'o' || *p == 'x' || *p == 'X')
	type = 'd';
      else if (*p == 'u')
	type = 'u';
      else if (*p == 's')
	type = 's';
      else if (*p == 'S')
	type = 'S';
      else
      {
	printf ("Internal error: at offset %d in widget fmt \"%s\": bad"
	    " format \"%c\"\n", (int) (p - fmt), fmt, *p);
	continue;
      }
      p++;

      /* Fill in <entry_flags> and <caption> and
	 also <box_len> and <buf_max_len> if
	 they're explicit. */
      box_len[f] = precision;
      buf[f] = 0;
      buf_max_len[f] = length;
      if (type == 'd')
	entry_flags[f] = (_field_flags_t) (FF_ENTRY | FF_INTEGER | FF_SIGNED);
      else if (type == 'u')
	entry_flags[f] = (_field_flags_t) (FF_ENTRY | FF_INTEGER);
      else if (type == 's' || type == 'S')
      {
	entry_flags[f] = (_field_flags_t) (FF_ENTRY | FF_STRING);
	if (type == 'S')
	  entry_flags[f] = (_field_flags_t) (entry_flags[f] | FF_UPPERCASE);
	if (flags & F_PLUS)
	  entry_flags[f] = (_field_flags_t) (entry_flags[f] | FF_NONEMPTY);
      }
      else
	entry_flags[f] = (_field_flags_t) 0;  // Ouch !
      caption[f]     = last_literal;

      // On to the next field
      f++;
      last_literal = 0;
    }
    else
    {
      if (*p == '\0')
	break;
      if (! last_literal)
	last_literal = p;
      p++;
    }
  }

  /* Second phase: retrieve the arguments from
     the list and fill in <buf> and perhaps also
     <box_len> and <buf_max_len> if needed. */
  for (size_t f = 0; f < nfields; f++)
  {
    // "%*": retrieve the length
    if (buf_max_len[f] == L_ASTERISK)
      buf_max_len[f] = va_arg (args, size_t);

    // "%.*": retrieve the precision
    if (box_len[f] == L_ASTERISK)
      box_len[f] = va_arg (args, size_t);

    // Retrieve the pointer on the buffer
    if ((entry_flags[f] & FF_TYPE) == FF_ENTRY)
    {
      if ((entry_flags[f] & FF_SUBTYPE) == FF_STRING)
      {
        buf[f] = va_arg (args, char *);
        if (buf_max_len[f] == L_OMITTED)
	  buf_max_len[f] = strlen (buf[f]);  // Bletch !
	if (box_len[f] == L_OMITTED)
	  box_len[f] = y_min (buf_max_len[f], 30);
      }
      else if ((entry_flags[f] & FF_SUBTYPE) == FF_INTEGER)
      {
	switch (entry_flags[f] & FF_INTSIZE)
	{
	  case FF_CHAR :
	    buf[f] = (char *) va_arg (args, char *);
	    break;
	  case FF_SHORT :
	    buf[f] = (char *) va_arg (args, short *);
	    break;
	  case FF_INT :
	    buf[f] = (char *) va_arg (args, int *);
	    break;
	  case FF_SIZE_T :
	    buf[f] = (char *) va_arg (args, size_t *);
	    break;
	  case FF_LONG :
	    buf[f] = (char *) va_arg (args, long *);
	    break;
	  default :
	    ;  // FIXME
	}
      }
      else 
	;  // FIXME
    }
    else
      ;  // To be implemented later
  }
  return 0;
}

 
/*
 *	do_geom
 *	Do geometry computations
 */
void Entry2::do_geom ()
{
  // Compute the widths
  size_t title_len = strlen (title);	// Length of title
  size_t entry_len = 0;			// Length or longest entry
  for (size_t f = 0; f < nfields; f++)
  {
    size_t l = box_len[f];
    if (l > entry_len)
      entry_len = l;
  }
  size_t caption_len = 0;		// Length of longest caption
  for (size_t f = 0; f < nfields; f++)
  {
    size_t l = strlen (caption[f]);
    if (l > caption_len)
      caption_len = l;
  }
  size_t entry_width  = FONTW * entry_len + 2 * entry_hofs;
  size_t caption_width= FONTW * caption_len + WIDE_HSPACING;
  size_t inner_width  = y_max (entry_width + caption_width, FONTW * title_len);
  size_t outer_width  = inner_width  + 2 * win_hofs;

  // Compute the heights
  vstep = FONTH + 2 * entry_vofs + BOX_VSPACING;
  size_t inner_height = nfields * vstep - BOX_VSPACING + title_vspacing + FONTH;
  size_t outer_height = inner_height + 2 * win_vofs;

  // Compute the absolute coordinates
  if (win_x0 < 0)
     win_x0 = (ScrMaxX - outer_width) / 2;
  if (win_y0 < 0)
     win_y0 = (ScrMaxY - outer_height) / 2;
  win_x1        = win_x0 + outer_width - 1;
  win_y1        = win_y0 + outer_height - 1;
  title_x0      = win_x0 + win_hofs;
  title_y0      = win_y0 + win_vofs;
  entry_box_y0  = title_y0 + FONTH + title_vspacing;
  entry_text_y0 = entry_box_y0 + entry_vofs;
  entry_text_y1 = entry_text_y0 + FONTH - 1;
  entry_box_y1  = entry_text_y1 + entry_vofs;
  caption_y0    = entry_text_y0;
  caption_x0    = title_x0;
  entry_box_x0  = caption_x0 + caption_width;
  entry_text_x0 = entry_box_x0 + entry_hofs;
  entry_text_x1 = entry_text_x0 + entry_len * FONTW - 1;  // Unused
  entry_box_x1  = entry_text_x1 + entry_hofs;             // Unused

  geom_up_to_date = true;
}


/*
 *	jump_to_field
 *	Jump to a particular field
 */
void Entry2::jump_to_field (size_t field_no)
{
  if (field_no >= nfields)
  {
    printf ("Internal error: Entry2::jump_to_field %d (nf=%d)",
	(int) field_no, (int) nfields);
    return;
  }
  entry_drawn[this->field_no] = false;
  entry_drawn[field_no] = false;
  this->field_no = field_no;
  first_key = 1;
}


/*
 *	prev_field
 */
void Entry2::prev_field ()
{
  if (nfields < 1)
    return;
  size_t f = field_no;
  if (f < 1)
    f = nfields - 1;
  else
    f--;
  jump_to_field (f);
}


/*
 *	next_field
 */
void Entry2::next_field ()
{
  if (nfields < 1)
    return;
  size_t f = field_no + 1;
  if (f >= nfields)
    f = 0;
  jump_to_field (f);
}


/* TEST STUFF FOLLOWS */

