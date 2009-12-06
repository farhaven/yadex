/*
 *	ftime.c
 *	Print the access, change or modification time of files
 *	AYM 1999-08-31
 */


/*
ftime is Copyright © 1999 André Majorel.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307, USA.
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


static void err (const char *fmt, ...);


static int verbose = 0;
static int print_names = 1;
static char which_time = 'm';
static const char format_dateonly[] = "%Y-%m-%d";
static const char format_nosecs[]   = "%Y-%m-%d %H:%M";
static const char format_default[]  = "%Y-%m-%d %H:%M:%S";
static const char *time_format = format_default;


int main (int argc, char *argv[])
{
  int n;
  int nfiles = 0;
  const char **file_list = NULL;
  time_t time = 0;
  time_t newtime;
  struct tm *tm;

  /* Parse the command line. Put the names of the reference
     files in <file_list>. Put the name of the output file in
     <oname>. */
  for (n = 1; n < argc; n++)
  {
    if (! strcmp (argv[n], "-a"))
    {
      which_time = 'a';
    }
    else if (! strcmp (argv[n], "-c"))
    {
      which_time = 'c';
    }
    else if (! strcmp (argv[n], "-d"))
    {
      time_format = format_dateonly;
    }
    else if (! strncmp (argv[n], "-f", 2))
    {
      if (argv[n][2])
	time_format = argv[n] + 2;
      else
      {
	n++;
	if (n >= argc)
	{
	  err ("-f requires an argument");
	  exit (1);
	}
	time_format = argv[n];
      }
    }
    else if (! strcmp (argv[n], "-m"))
    {
      which_time = 'm';
    }
    else if (! strcmp (argv[n], "-s"))
    {
      time_format = format_nosecs;
    }
    else if (! strcmp (argv[n], "-v"))
    {
      verbose = 1;
    }
    else
    {
      file_list = realloc (file_list, (nfiles + 1) * sizeof *file_list);
      if (file_list == NULL)
      {
	err ("Not enough memory");
	exit (2);
      }
      file_list[nfiles] = argv[n];
      nfiles++;
    }
  }
  if (nfiles == 1)
  {
    print_names = 0;
  }
  else
  {
    print_names = 1;
  }

  /* Print one line for each file */
  for (n = 0; n < nfiles; n++)
  {
    struct stat sbuf;
    time_t      time;
    char        tbuf[101];

    if (stat (file_list[n], &sbuf))
    {
      err ("Warning: can't stat \"%s\" (%s)", file_list[n], strerror (errno));
      continue;
    }
    if (which_time == 'a')
      time = sbuf.st_atime;
    else if (which_time == 'c')
      time = sbuf.st_ctime;
    else if (which_time == 'm')
      time = sbuf.st_mtime;
    else
      ;  /* Ugh ? */

    tm = localtime (&time);
    strftime (tbuf, sizeof tbuf, time_format, tm);
    fputs (tbuf, stdout);
    if (print_names)
    {
      putchar (' ');
      fputs (file_list[n], stdout);
    }
    putchar ('\n');
  }

  if (file_list != NULL)
    free (file_list);
  exit (0);
}


static void err (const char *fmt, ...)
{
  va_list list;
  fflush (stdout);
  fputs ("ftime: ", stderr);
  va_start (list, fmt);
  vfprintf (stderr, fmt, list);
  fputc ('\n', stderr);
}

