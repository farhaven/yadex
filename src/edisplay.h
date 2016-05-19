/*
 *	edisplay.h
 *	AYM 1998-09-16
 */


class highlight_c;
class infobar_c;
class objinfo_c;


class edisplay_c
   {
   public :
      edisplay_c (edit_t *e);
      ~edisplay_c ();
      void refresh ();
      void need_refresh ();
      void highlight_object (Objid& obj);
      void forget_highlight ();

   private :
      edit_t *e;		/* Parent editing window */
      int pointer_scnx;		/* Physical position of the pointer */
      int pointer_scny;
      int refresh_needed;	/* If true, the display needs to be refreshed */
      highlight_c *highlight;
      objinfo_c *objinfo;
      infobar_c *infobar;
   };



