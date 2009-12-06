/*
 *	install - install files
 *	AYM 2001-04-10
 */


/*
This file is Copyright © 2001 André Majorel.

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


#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>


static void usage (void);
static int install_file (const char ifile[], const char ofile[], mode_t omode);
static void err (const char *fmt, ...);


int main (int argc, char *argv[])
{
  int rc = 0;
  const char *target_dir = NULL;
  mode_t mode = 0;
  int help = 0;

  /* Parse the command line */
  if (argc == 2 && strcmp (argv[1], "--help") == 0)
  {
    usage ();
    exit (0);
  }
  else
  {
    int g;
    while ((g = getopt (argc, argv, "d:m:")) != EOF)
    {
      if (g == 'd')
	target_dir = optarg;
      else if (g == 'm')
	mode = (mode_t) strtol (optarg, NULL, 8);
      else
      {
	err ("syntax error");
	exit (1);
      }
    }
  }
  if (target_dir == NULL && argc - optind != 2)
  {
    err ("syntax error");
    exit (1);
  }

  /* Copy all files */
  if (target_dir == NULL)
  {
    if (install_file (argv[optind], argv[optind + 1], mode) != 0)
      rc = 1;
  }
  else
  {
    int n;

    for (n = optind; n < argc; n++)
    {
      char *ofile = NULL;
      char *ibasename;

      ibasename = strrchr (argv[n], '/');  /* FIXME unixism */
      if (ibasename == NULL)
	ibasename = argv[n];
      ofile = malloc (strlen (target_dir) + strlen (ibasename) + 2);
      if (ofile == NULL)
      {
	err (strerror (ENOMEM));
	exit (1);
      }
      strcpy (ofile, target_dir);
      if (strlen (ofile) > 0 && ofile[strlen (ofile) - 1] != '/')  /* FIXME */
	strcat (ofile, "/");
      strcat (ofile, ibasename);
      if (install_file (argv[n], ofile, mode) != 0)
	rc = 1;
      free (ofile);
    }
  }

  return rc;
}


static void usage (void)
{
  puts ("Usage:");
  puts ("  install --help");
  puts ("  install [-m mode] ifile ofile");
  puts ("  install [-m mode] [-d dir] [file ...]");
}


static int install_file (const char ifile[], const char ofile[], mode_t omode)
{
  int rc       = 0;
  FILE *ifp    = NULL;
  FILE *ofp    = NULL;
  char *buf    = NULL;
  size_t bufsz = 0x4000;

  ifp = fopen (ifile, "rb");
  if (ifp == NULL)
  {
    err ("%s: %s", ifile, strerror (errno));
    rc = 1;
    goto byebye;
  }
  ofp = fopen (ofile, "wb");
  if (ofp == NULL)
  {
    err ("%s: %s", ofile, strerror (errno));
    rc = 1;
    goto byebye;
  }
  buf = malloc (bufsz);
  if (buf == NULL)
  {
    err (strerror (ENOMEM));
    rc = 1;
    goto byebye;
  }

  /* Copy the data */
  {
    size_t nbytes;

    while ((nbytes = fread (buf, 1, bufsz, ifp)) != 0)
    {
      if (fwrite (buf, 1, nbytes, ofp) != nbytes)
      {
	err ("%s: write error", ofile);
	rc = 1;
	goto byebye;
      }
    }
    if (ferror (ifp))
    {
      err ("%s: read error", ifile);
      rc = 1;
    }
  }

  /* Force the ownership to EUID:EGID. Useful if the targets
     already exist. */
  if (chown (ofile, geteuid (), getegid ()) != 0)
  {
    err ("%s: %s", ofile, strerror (errno));
    rc = 1;
    goto byebye;
  }

  /* Set the mode */
  if (chmod (ofile, omode) != 0)
  {
    err ("%s: %s", ofile, strerror (errno));
    rc = 1;
    goto byebye;
  }

  /* Copy the mtime */
  {
    struct stat s;
    struct utimbuf u;

    if (stat (ifile, &s) != 0)
    {
      err ("%s: %s", ifile, strerror (errno));
      rc = 1;
      goto byebye;
    }
    u.actime  = s.st_atime;
    u.modtime = s.st_mtime;
    if (utime (ofile, &u) != 0)
    {
      err ("%s: %s", ofile, strerror (errno));
      rc = 1;
      goto byebye;
    }
  }


byebye:
  if (ifp != NULL)
    fclose (ifp);
  if (ofp != NULL)
    if (fclose (ofp) != 0)
    {
      err ("%s: %s", ofile, strerror (errno));
      rc = 1;
    }
  if (buf != NULL)
    free (buf);
  return rc;
}


static void err (const char *fmt, ...)
{
  va_list argp;

  fflush (stdout);
  fputs ("install: ", stderr);
  va_start (argp, fmt);
  vfprintf (stderr, fmt, argp);
  va_end (argp);
  fputc ('\n', stderr);
}

