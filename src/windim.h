/*
 *	windim.h
 *	Win_dim class - store the width or height of a window
 *	AYM 2000-05-29
 */


#ifndef YH_WINDIM  /* DO NOT INSERT ANYTHING BEFORE THIS LINE */
#define YH_WINDIM


struct Win_dim_priv;

class Win_dim
{
   public:
     Win_dim ();
     Win_dim (const char *string);
     ~Win_dim ();
     int set (const char *string);
     int pixels (int ref_pixels);
     void string (char *buf, size_t buf_size);

   private:
     Win_dim (const Win_dim&);			// Too lazy to implement it
     Win_dim& operator= (const Win_dim&);	// Too lazy to implement it
     struct Win_dim_priv* p;
};

#endif  /* DO NOT ADD ANYTHING AFTER THIS LINE */
