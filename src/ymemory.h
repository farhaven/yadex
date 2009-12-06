/*
 *	ymemory.h
 *	Memory allocation functions.
 *	AYM 1999-03-24
 */


void *GetMemory (unsigned long size);
void *ResizeMemory (void *, unsigned long size);
void FreeMemory (void *);
void huge *GetFarMemory (unsigned long size);
void huge *ResizeFarMemory (void huge *old, unsigned long size);
void FreeFarMemory (void huge *);


