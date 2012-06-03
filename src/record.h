/*
 *	record.h - record events for later playback
 *	AYM 2000-05-17
 */


#ifndef YH_RECORD  /* DO NOT INSERT ANYTHING BEFORE THIS LINE */
#define YH_RECORD


class Recording 
{
  public :
    read ();
    write ();

  private :
    class priv;
    priv *p;
};


#endif  /* DO NOT ADD ANYTHING AFTER THIS LINE */
