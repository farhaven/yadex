/* Compatibility wrappers for OpenBSD's strl{cat,cpy}, to be used if your libc doesn't have them */
/* 
This file is part of Yadex.

Copyright 2016 Gregor Best <gbe@unobtanium.de>

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* These are implemented in terms of snprintf/asprintf because strn{cpy,cat} are just
 * too weird
 */

size_t
strlcpy(char *dst, const char *src, size_t dstsize) {
	return snprintf(dst, dstsize, "%s", src);
}

size_t
strlcat(char *dst, const char *src, size_t dstsize) {
	char *tmp = NULL;
	int len = 0;

	len = asprintf(&tmp, "%s%s", dst, src);
	if (len < 0) {
		free(tmp);
		return 0;
	}

	/* +1 because asprintf doesn't count the final \0 in its return value */
	if (dstsize > len + 1)
		dstsize = len + 1;

	memmove(dst, tmp, dstsize);

	free(tmp);
}
