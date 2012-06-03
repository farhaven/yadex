/*
 *	entry.cc
 *	Entry "widgets" (ahem).
 *	AYM 1998-11-30
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
#include "entry.h"
#include "gfx.h"

const char *strgetl (const char *& str, long& value);

/*
 *	InputInteger - display the integer input box
 *
 *	FIXME *valp, minv and maxv should be changed to long.
 */
int
InputInteger (int x0, int y0, int *valp, int minv, int maxv) {
	int key;
	int entry_out_x0;
	int entry_out_y0;
	int entry_out_x1;
	int entry_out_y1;
	int entry_text_x0;
	int entry_text_y0;
	int entry_text_x1;
	int entry_text_y1;
	const size_t boxlen    = 7;    // Visible width of entry box
	const size_t bufmaxlen = 50;     // Slack enough
	char *buf = new char[bufmaxlen + 1];

	entry_out_x0  = x0;
	entry_text_x0 = entry_out_x0 + HOLLOW_BORDER + NARROW_HSPACING;
	entry_text_x1 = entry_text_x0 + boxlen * FONTW - 1;
	entry_out_x1  = entry_text_x1 + HOLLOW_BORDER + NARROW_HSPACING;
	entry_out_y0  = y0;
	entry_text_y0 = entry_out_y0 + HOLLOW_BORDER + NARROW_VSPACING;
	entry_text_y1 = entry_text_y0 + FONTH - 1;
	entry_out_y1  = entry_text_y1 + HOLLOW_BORDER + NARROW_VSPACING;
	DrawScreenBoxHollow (entry_out_x0, entry_out_y0, entry_out_x1, entry_out_y1,
		BLACK);
	long val = *valp;
	snprintf (buf, bufmaxlen + 1, "%d", *valp);    // FIXME what if we were in hex ?
	for (bool firstkey = true; ; firstkey = false) {
		const char *checkp = buf;
		bool ok = strgetl (checkp, val) == 0
					&& checkp == buf + strlen (buf)
					&& val >= minv && val <= maxv;
		set_colour (BLACK);
		DrawScreenBox (entry_text_x0, entry_text_y0, entry_text_x1, entry_text_y1);
		if (ok)
			set_colour (WHITE);
		else
			set_colour (LIGHTRED);
		if (strlen (buf) > boxlen) {
			DrawScreenText (entry_text_x0, entry_text_y0, "<%s",
			buf + (strlen (buf) - boxlen + 1));
		} else
			DrawScreenString (entry_text_x0, entry_text_y0, buf);

		key = get_key ();
		if (key == YK_BACKSPACE && strlen (buf) > 0)
			buf[strlen (buf) - 1] = '\0';
		else if (key == YK_RETURN && ok) {
			*valp = (int) val;
			break;    // Return current value
		} else if (key == YK_LEFT || key == YK_RIGHT
			|| key == YK_UP   || key == YK_DOWN
			|| key == YK_TAB  || key == YK_BACKTAB) {
			*valp = (int) val;
			break;    // Return current value, even if not valid
		} else if (key == YK_ESC) {
			*valp = IIV_CANCEL;    // Return an out of range value
			break;
		} else if (is_ordinary (key) && strlen (buf) < bufmaxlen) {
			if (firstkey)
				if (key == ' ')    // Kludge : hit space to append to initial value
					continue;
			else
				*buf = '\0';
			al_sapc (buf, key, bufmaxlen);
		} else
			Beep ();
	}

	is.key = 0;    // Shouldn't have to do that but EditorLoop() is broken
	delete[] buf;
	return key;
}


/*
   ask for an integer value and check for minimum and maximum
*/
int
InputIntegerValue (int x0, int y0, int minv, int maxv, int defv) {
	int  val, key;
	char prompt[80];

	snprintf (prompt, sizeof prompt, "Enter a number between %d and %d:",
		minv, maxv);
	if (x0 < 0)
		x0 = (ScrMaxX - 25 - FONTW * strlen (prompt)) / 2;
	if (y0 < 0)
		y0 = (ScrMaxY - 55) / 2;
	DrawScreenBox3D (x0, y0, x0 + 25 + FONTW * strlen (prompt), y0 + 55);
	set_colour (WHITE);
	DrawScreenText (x0 + 10, y0 + 8, prompt);
	val = defv;
	while ((key = InputInteger (x0 + 10, y0 + 28, &val, minv, maxv)) != YK_RETURN
			&& key != YK_ESC)
		Beep ();
	return val;
}


/*
   ask for a filename
*/
void
InputFileName (int x0, int y0, const char *prompt, size_t maxlen, char *filename) {
	int   key;
	size_t l;
	size_t boxlen;
	bool  firstkey;
	int width;
	int title_y0;
	int entry_out_x0, entry_out_y0;
	int entry_out_x1, entry_out_y1;
	int entry_text_x0, entry_text_y0;
	int entry_text_x1, entry_text_y1;

	for (l = strlen (filename) + 1; l <= maxlen; l++)
		filename [l] = '\0';
	/* compute the width of the input box */
	if (maxlen > 20)
		boxlen = 20;
	else
		boxlen = maxlen;
	/* compute the width of the dialog box */
	if (strlen (prompt) > boxlen)
		l = strlen (prompt);
	else
		l = boxlen;

	width = 2 * HOLLOW_BORDER + 2 * NARROW_HSPACING + boxlen * FONTW;
	if ((int) (strlen (prompt) * FONTW) > width)
		width = strlen (prompt) * FONTW;
	width += 2 * BOX_BORDER + 2 * WIDE_HSPACING;

	if (x0 < 0)
		x0 = (ScrMaxX - width) / 2;
	if (y0 < 0)
		y0 = (ScrMaxY - 2 * BOX_BORDER - 2 * WIDE_VSPACING
			- (int) (2.5 * FONTH) - 2 * HOLLOW_BORDER
			- 2 * NARROW_VSPACING) / 2;
	/* draw the dialog box */
	entry_out_x0  = x0 + BOX_BORDER + WIDE_HSPACING;
	entry_text_x0 = entry_out_x0  + HOLLOW_BORDER + NARROW_HSPACING;
	entry_text_x1 = entry_text_x0 + boxlen * FONTW - 1;
	entry_out_x1  = entry_text_x1 + NARROW_HSPACING + HOLLOW_BORDER;
	title_y0      = y0 + BOX_BORDER + WIDE_VSPACING;
	entry_out_y0  = title_y0 + (int) (1.5 * FONTH);
	entry_text_y0 = entry_out_y0  + HOLLOW_BORDER + NARROW_VSPACING;
	entry_text_y1 = entry_text_y0 + FONTH - 1;
	entry_out_y1  = entry_text_y1 + NARROW_VSPACING + HOLLOW_BORDER;

	DrawScreenBox3D (x0, y0, x0 + width - 1, entry_out_y1 + WIDE_VSPACING);
	DrawScreenBoxHollow (entry_out_x0, entry_out_y0, entry_out_x1, entry_out_y1, BLACK);
	set_colour (WINTITLE);
	DrawScreenString (entry_out_x0, title_y0, prompt);
	firstkey = true;
	for (;;) {
		l = strlen (filename);
		set_colour (BLACK);
		DrawScreenBox (entry_text_x0, entry_text_y0, entry_text_x1, entry_text_y1);
		set_colour (WHITE);
		if (l > boxlen) {
			DrawScreenText (entry_text_x0, entry_text_y0, "<%s",
			filename + (l - boxlen + 1));
		} else
			DrawScreenString (entry_text_x0, entry_text_y0, filename);
		key = get_key ();
		if (firstkey && is_ordinary (key)) {
			for (l = 0; l <= maxlen; l++)
				filename[l] = '\0';
			l = 0;
		}
		firstkey = false;
		if (l < maxlen && is_ordinary (key)) {
			filename[l] = key;
			filename[l + 1] = '\0';
		} else if (l > 0 && key == YK_BACKSPACE)
			filename[l-1] = '\0';
		else if (key == YK_RETURN)
			break;  /* return "filename" */
		else if (key == YK_ESC) {
			filename[0] = '\0'; /* return an empty string */
			break;
		} else
			Beep ();
	}
	is.key = 0;  // Shouldn't have to do that but EditorLoop() is broken
}


/*
 *      strgetl - parse a C-style signed long integer
 *
 *	Parse anything that would be a legal C signed long
 *	integer literal (L and U suffixes are not allowed).
 *	After the function returns, <str> points on the first
 *	character that could not be parsed. If what was parsed
 *	constitutes a valid integer, <value> contains its value
 *	and the return value is a null pointer. Otherwise,
 *	<value> is undefined and the return value is a static
 *	string describing the error.
 */
const char
*strgetl (const char *& str, long& value) {
  int base = 10;
  int sign = 1;

  // Leading + or -
  if (*str == '-') {
    sign = -1;
    str++;
  } else if (*str == '+')
    str++;

  // 0- or 0x- prefix
  if (*str == '0' && (str[1] == 'x' || str[1] == 'X')) {
    base = 16;
    str += 2;
  } else if (*str == '0')
    base = 8;    // Don't advance str, so that "0" passes the next test

  // Check that there is at least one digit
  if (hextoi (*str) < 0 || hextoi (*str) >= base) {
    if (base == 8)
      return "expected an octal digit";
    else if (base == 10)
      return "expected a decimal digit";
    else
      return "expected a hex digit";
  }

  // Swallow all non-prefix digits
  for (value = 0; *str != '\0'; str++) {
    int digitval = hextoi (*str);
    if (digitval < 0 || digitval >= base)
      return 0;
    value = base * value + sign * digitval;
  }
  return 0;
}

