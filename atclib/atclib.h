/*
 *      atclib.h
 *	An excerpt of the header for Atclib
 */


/*
This file is part of Atclib.

Atclib is Copyright © 1995-1999 André Majorel.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307, USA.
*/


#if !defined(AL_AATCLIB_H)  /* To be immune from double inclusion */
#define AL_AATCLIB_H        /* To be immune from double inclusion */

/* WARNING
You should never ever define AL_AILLEGAL_ACCESS. This macro is used
to restrict access to information that is considered private,
undocumented and that can be changed without notice. Only Atclib
modules can define it, not application programs.
*/ 

#ifndef FILE
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	al_a*
 *	General
 */
/* This is meant to be an even more invalid pointer than NULL */
#define AL_AINVALIDPOINTER ((void *)-1)

/* To size_t what INT_MAX is to int. ANSI should have provided it! */
#define AL_ASIZE_T_MAX ((size_t)(((size_t)0)-1))

/* min and max are not available everywhere */
#define	al_amin(a,b) ((a) < (b) ? (a) : (b))
#define	al_amax(a,b) ((a) > (b) ? (a) : (b))

/* Common flags */
#define AL_AICASE  0x0001  /* Ignore case when matching strings */

typedef       char *al_as_t;   /* String */
typedef const char *al_acs_t;  /* Constant string */

/* Useful when you need a specific number of bits/bytes */
typedef unsigned char  al_au1_t;
typedef   signed char  al_as1_t;
typedef unsigned short al_au2_t;
typedef   signed short al_as2_t;
typedef unsigned long  al_au4_t;
typedef   signed long  al_as4_t;

extern int al_aerrno;
#define AL_ABADL      1  /* Not a valid list pointer */
#define AL_AEOL       2  /* Attempt to read at end of list or step past it */
#define AL_AINVAL     3  /* Invalid argument */
#define AL_ANOFIX     4  /* Not a fixed element length list */
#define AL_ANOMEM     5  /* Not enough memory */
#define AL_ANOVAR     6  /* Not a variable element length list */
#define AL_AOVERFLOW  7  /* Buffer overflow */
al_acs_t al_astrerror (int e);

extern const char al_adigits[36];  /* Contains 0-9A-Z */


/*
 *	al_f*
 *	Filesystem and disks
 */
/* FIXME how do you port this to UNIX ? */
#define AL_FDRV 2                       /* "<d>:" */
#define AL_FPATH 65                     /* <64_characters>"\" */
#define AL_FBASE 8                      /* "ABCDEFGH" */
#define AL_FEXT 4                       /* ".IJK" + 3 characters */
#define AL_FBE (AL_FBASE+AL_FEXT)
                                        /* complete spec (by definition) */
#define AL_FSPEC (AL_FDRV+AL_FPATH+AL_FBASE+AL_FEXT)
#define AL_FPCSD '\\'                   /* Path component separator for DOS */
#define AL_FPCSU '/'                    /* Path component separator for Unix */
#define al_fispcsd(c) ((c)=='\\'||(c)=='/') /* Is a path component separator? */
#define al_fispcsu(c) ((c)=='/')            /* Is a path component separator? */
#define AL_FPS '/'
#define al_fisps(c) ((c)=='/')
typedef char al_fdrv_t [AL_FDRV+1 ];
typedef char al_fpath_t[AL_FPATH+1];
typedef char al_fbase_t[AL_FBASE+1];
typedef char al_fext_t [AL_FEXT+1 ];
typedef char al_fbe_t  [AL_FBE+1  ];
typedef char al_fspec_t[AL_FSPEC+1];
void al_fana     (al_acs_t ispec, al_as_t odrv, al_as_t opath, al_as_t obase, al_as_t oext);
int al_fcanon    (al_acs_t strin, al_as_t strout);
int al_fchdir    (al_acs_t path);
int al_fnature   (al_acs_t spec);
int al_fmakepath (al_acs_t path);


/*
 *	al_l*
 *	Linked lists
 */
#ifdef AL_AILLEGAL_ACCESS
enum { AL_LLIST_MAGIC = 0x18a3 };  /* Magic number for al_llist_t */
#define al_lcheckmagic(list) \
do\
  if (list == NULL || list->magic != AL_LLIST_MAGIC)\
    { al_aerrno = AL_ABADL; return AL_ABADL; }\
while (0)
/* FIXME: this code assumes that (union *) and (void *) have the same
   size. I don't see why they wouldn't but I don't think this is
   warranted by the standard. */
typedef struct             /* One element of a fixed-length list */
  {
  void   *next;            /* Never used (overlaid by al_lelt_t.next) */
  char data[1];          /* First char of data buffer */
  } al_leltfix_t;

typedef struct             /* One element of a variable-length list */
  {
  void   *next;            /* Never used (overlaid by al_lelt_t.next) */
  size_t length;           /* Length of the element */
  char data[1];          /* First char of data buffer */
  } al_leltvar_t;

typedef union al_lelt_u    /* One element of any list */
  {
  union al_lelt_u *next;   /* Pointer to next element in the list or NULL */
  al_leltfix_t     f;      /* The fixed-length flavour */
  al_leltvar_t     v;      /* The variable-length flavour */
  } al_lelt_t;

struct al_llist_s          /* One instance of this per list */
  {
  unsigned   magic;        /* Magic number to validate the structure */
  size_t     length;       /* Size of an element in bytes or 0 */
  al_lelt_t *first;        /* First element of the list (or NULL) */
  al_lelt_t *current;      /* Current element */
  int        ateol;        /* Current is not current but previous */
  long       curno;        /* No. of current element */
  al_lelt_t *prev;         /* Previous element (or NULL) */
  long       total;        /* Total number of elements */
  };

struct al_lpos_s           /* Type used by al_lgetpos and al_lsetpos to store pointer position */
  {
  al_lelt_t *current;
  int        ateol;
  long       curno;
  al_lelt_t *prev;
  };
#endif

typedef struct al_llist_s al_llist_t;
typedef struct al_lpos_s  al_lpos_t;

int    al_leol   (al_llist_t *l);
long   al_lcount   (al_llist_t *l);
al_llist_t *al_lcreate (size_t eltsz);
int    al_ldelete  (al_llist_t *l);
int    al_ldiscard (al_llist_t *l);
int    al_lgetpos  (al_llist_t *l, al_lpos_t *pos);
int    al_linsert  (al_llist_t *l, const void *buf);
int    al_linsertl (al_llist_t *l, const void *buf, size_t length);
size_t al_llength  (al_llist_t *l);
int    al_lpeek    (al_llist_t *l, void *buf);
int    al_lpeekl   (al_llist_t *l, void *buf, size_t *length);
int    al_lpoke    (al_llist_t *l, const void *buf);
int    al_lpokel   (al_llist_t *l, const void *buf, size_t length);
void  *al_lptr     (al_llist_t *l);
int    al_lread    (al_llist_t *l, void *buf);
int    al_lreadl   (al_llist_t *l, void *buf, size_t *length);
int    al_lrewind  (al_llist_t *l);
int    al_lseek    (al_llist_t *l, long offset, int origin);
int    al_lsetpos  (al_llist_t *l, const al_lpos_t *pos);
int    al_lstep    (al_llist_t *l);
long   al_ltell    (al_llist_t *l);
int    al_lwrite   (al_llist_t *l, const void *buf);
int    al_lwritel  (al_llist_t *l, const void *buf, size_t length);


/*
 *	al_s*
 *	Strings
 */
int    al_sapc      (al_as_t dest, char   source, size_t maxlen);
int    al_saps      (al_as_t dest, al_acs_t source, size_t maxlen);
int    al_sapslower (al_as_t dest, al_acs_t source, size_t maxlen);
int    al_sapsupper (al_as_t dest, al_acs_t source, size_t maxlen);
int    al_scpc      (al_as_t dest, char   source, size_t maxlen);
int    al_scps      (al_as_t dest, al_acs_t source, size_t maxlen);
int    al_scpslower (al_as_t dest, al_acs_t source, size_t maxlen);
int    al_scpsupper (al_as_t dest, al_acs_t source, size_t maxlen);
int    al_sbegins   (al_acs_t mainstr, al_acs_t substr);
char  *al_sdup      (al_acs_t str);
int    al_sends     (al_acs_t mainstr, al_acs_t substr);
size_t al_sfirsts   (al_acs_t s1, al_acs_t s2, int flags);
size_t al_sfirstw   (al_acs_t s1, al_acs_t s2, int flags);
int    al_sisnum    (al_acs_t str);
int    al_strOLC    (al_acs_t str, char chr);
#define AL_SICASE 0x01  /* Ignore case when matching */
#define AL_SDOS   0x02  /* Dot is special and "\" is same as "/" */
#define AL_SLDOT  0x04  /* A leading dot "." is a special character */
#define AL_SSLASH 0x08  /* The slash "/" is a special character */
#define AL_SESC   0x10  /* The backslash "\" escapes "*" "?" and "[" */
int al_swcmatch (al_acs_t pattern, al_acs_t string, int flags);

#ifdef __cplusplus
}
#endif
#endif  /* To be immune from double inclusion */

