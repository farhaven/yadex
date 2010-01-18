/*
 *	wads.h
 *	AYM 1998-11-25
 */


#ifndef YH_WADS  /* DO NOT INSERT ANYTHING BEFORE THIS LINE */
#define YH_WADS


int  file_read_i16    (FILE *,  i16 *buf, long count = 1);
int  file_read_i32    (FILE *,  i32 *buf, long count = 1);
long file_read_vbytes (FILE *, void *buf, long count);
int  file_read_bytes  (FILE *, void *buf, long count);
void file_write_i16   (FILE *,  i16 buf);
void file_write_i32   (FILE *,  i32 buf, long count = 1);
void file_write_name  (FILE *, const char *name);
void WriteBytes       (FILE *, const void *, long);
int  copy_bytes       (FILE *dest, FILE *source, long size);

#endif  /* DO NOT ADD ANYTHING AFTER THIS LINE */
