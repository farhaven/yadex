/*
 *	dependcy.h
 *	Dependency class
 *	AYM 2000-04-09
 */


/* This class, along with the Serial_num class, allow one to
   express a relationship of dependency (think makefiles)
   between two objects. For the sake of discussion, let's assume
   that class Target depends on class Source.

   Make Source use a Serial_num object and make it call
   Serial_num::bump() whenever it changes. Make Target use a
   Dependency object. The Dependency object should be
   constructed with a pointer to the Serial_num member of Source.

   In all public methods of Target, start by checking whether
   Source has been modified while we were out by calling
   Dependency::outdated(). If the latter returns true, Target
   shall update itself based on the new state of Source and then
   call Dependency::update(). */


#ifndef YH_DEPENDCY  /* DO NOT INSERT ANYTHING BEFORE THIS LINE */
#define YH_DEPENDCY


class Serial_num;			// Defined in serialnum.h
typedef unsigned long serial_num_t;	// Copied from serialnum.h


class Dependency
{
  public :
    Dependency (Serial_num *sn);
    bool outdated ();
    void update ();

  private :
    Serial_num   *serial_num;
    serial_num_t  token;
    bool          token_valid;
};


#endif  /* DO NOT ADD ANYTHING AFTER THIS LINE */
