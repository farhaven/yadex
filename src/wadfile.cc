/*
 *	wadfile.cc - Wad_file class
 *	AYM 2001-09-18
 */


/*
This file is part of Yadex.

Yadex incorporates code from DEU 5.21 that was put in the public domain in
1994 by Raphaël Quinet and Brendon Wyber.

The rest of Yadex is Copyright © 1997-2003 André Majorel and others.and others.

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
#include "wadfile.h"


/*
 *	Wad_file::~Wad_file - dtor
 */
Wad_file::~Wad_file () {
	if (directory != 0) {
		free (directory);
		directory = 0;			// Catch bugs
	}
	if (fp != 0) {
		fclose (fp);
		fp = 0;				// Catch bugs
	}
}

/*
 *	Wad_file::where - return file(offset) string
 *
 *	Return pointer to a per-Wad_file buffer.
 */
const char
*Wad_file::where () const
{
	const unsigned long offset       = ftell (fp);
	const size_t        offset_len   =  + 3;
	const size_t        name_len_max = sizeof where_ - 1 - offset_len;

	if (name_len_max >= filename.size())
		snprintf (where_, sizeof(where_), "%s(%lXh)", filename.c_str(), offset);
	else {
		const char  *ellipsis = "...";
		const size_t total    = name_len_max - strlen (ellipsis);
		const int left     = total / 2;
		const int right    = total - left;
		snprintf (where_, sizeof(where_), "%*s%s%*s(%lXh)",
			left, filename.c_str(),
			ellipsis,
			right, filename.c_str() + total,
			offset);
	}

	return where_;
}


/*
 *	Wad_file::read_vbytes - read bytes from a wad file
 *
 *	Read up to <count> bytes and store them into buffer
 *	<buf>. <count> is _not_ limited to size_t. If an I/O
 *	error occurs, set the error flag. EOF is not considered
 *	an error.
 *
 *	Return the number of bytes read.
 */
long Wad_file::read_vbytes (void *buf, long count) const
{
  long bytes_read_total;
  size_t bytes_read;
  size_t bytes_to_read;

  bytes_read_total = 0;
  bytes_to_read    = 0x8000;
  while (count > 0)
  {
    if (count <= 0x8000)
      bytes_to_read = (size_t) count;
    bytes_read = fread (buf, 1, bytes_to_read, fp);
    bytes_read_total += bytes_read;
    if (bytes_read != bytes_to_read)
      break;
    buf = (char *) buf + bytes_read;
    count -= bytes_read;
  }
  if (ferror (fp))
  {
    if (! error_)
      err ("%s: read error", where ());
    error_ = true;
  }
  return bytes_read_total;
}

/*
 *	Wad_file::pathname - return the pathname of the file
 */
const string Wad_file::pathname () const {
  return filename;
}

/*
 *	Wad_file::pic_format - return the pic_format of the wad
 */
ygpf_t Wad_file::pic_format () const {
  return pic_format_;
}

/*
 *	Wad_file::error - tell whether any errors occurred
 *
 *	Reset the error indicator and call clearerr() on the
 *	underlying stdio stream. Thus calling Wad_file::error()
 *	again immediately after always returns false. Calling
 *	this function is the only way to clear the error flag of
 *	a Wad_file.
 *
 *	So short that it's a good candidate for inlining.
 *
 *	Return true if an error occurred, false otherwise.
 */
bool Wad_file::error () const {
	if (! error_)
		return false;

	clearerr (fp);
	error_ = false;
	return true;
}

/*
 *	Wad_file::seek - move the file pointer
 *
 *	If an error occurs, set the error flag.
 */
void Wad_file::seek (long offset) const {
	if (fseek (fp, offset, 0) != 0) {
		if (! error_)
			err ("%s: can't seek to %lXh", filename.c_str(), offset);
		error_ = true;
	}
}

/*
 *	Wad_file::read_uint8_t - read a byte
 *
 *	If an error occurs, set the error flag and the return
 *	value is undefined.
 */
uint8_t Wad_file::read_uint8_t () const {
	uint8_t v = uint8_t (getc (fp));

	if (feof (fp) || ferror (fp)) {
		if (! error_)
			err ("%s: read error", where ());
		error_ = true;
	}
	return v;
}

/*
 *	Wad_file::read_uint8_t - read a byte
 *
 *	If an error occurs, set the error flag and the contents
 *	of buf is undefined.
 */
void Wad_file::read_uint8_t (uint8_t& buf) const {
	buf = getc (fp);

	if (feof (fp) || ferror (fp)) {
		if (! error_)
			err ("%s: read error", where ());
		error_ = true;
	}
}

/*
 *	Wad_file::read_int16_t - read a little-endian 16-bit signed integer
 *
 *	If an error occurs, set the error flag and the return
 *	value is undefined.
 */
int16_t Wad_file::read_int16_t () const {
	const size_t nbytes = 2;
	uint8_t buf[nbytes];

	if (fread (buf, 1, nbytes, fp) != nbytes) {
		if (! error_)
			err ("%s: read error", where ());
		error_ = true;
		return EOF;  // Whatever
	}
	return buf[0] | buf[1] << 8;
}


/*
 *	Wad_file::read_int16_t - read a little-endian 16-bit signed integer
 *
 *	The value read is stored in *buf. If an error occurs,
 *	set the error flag and the contents of *buf is undefined.
 */
void Wad_file::read_int16_t (int16_t *buf) const {
	uint8_t c1 = getc(fp);
	uint8_t c2 = getc(fp);
	*buf = c1 | (c2 << 8);

	if (feof (fp) || ferror (fp)) {
		if (!error_)
			err ("%s: read error", where ());
		error_ = true;
	}
}


/*
 *	Wad_file::read_int32_t - read little-endian 32-bit signed integers
 *
 *	Read <count> little-endian 32-bit signed integers from
 *	wad file <wadfile> into *buf. If an error occurs, set
 *	error_ and the contents of *buf is undefined.
 */
void Wad_file::read_int32_t (int32_t *buf, long count) const {
	while (count-- > 0) {
		uint8_t c[4];
		*buf = 0;
		for (uint i = 0; i < sizeof(c); i++) {
			c[i] = getc(fp);
			*buf |= (c[i] << (8 * i));
		}
		buf++;
	}

	if (feof (fp) || ferror (fp)) {
		if (! error_)
			err ("%s: read error", where ());
		error_ = true;
	}
}


/*
 *	Wad_file::read_bytes - read bytes from a wad file
 *
 *	Read <count> bytes and store them into buffer <buf>.
 *	<count> is _not_ limited to size_t. If an I/O error
 *	occurs or EOF is reached before the requested number of
 *	bytes is read, set the error flag.
 */
void Wad_file::read_bytes (void *buf, long count) const {
	long bytes_read;

	bytes_read = read_vbytes (buf, count);
	if (bytes_read != count) {
		if (! error_)
			err ("%s: read error", where ());
		error_ = true;
	}
}


/*
 *	Wad_file::what - what a wad contains
 *
 *	Written for the sake of the "w" command. Return the
 *	name of the first lump in the wad, which gives an idea
 *	of what it contains. The string is *not* NUL-terminated.
 */
const char *Wad_file::what () const {
	if (directory == 0)
		return "(nodir)";
	if (dirsize < 1)
		return "(empty)";
	return directory[0].name;
}
