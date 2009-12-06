/*
 *	entry2.h
 *	A string entry box class
 *	AYM 1999-04-12
 */


#ifndef YH_ENTRY2  /* To prevent multiple inclusion */
#define YH_ENTRY2  /* To prevent multiple inclusion */


typedef enum
{
  ACT_NONE,
  ACT_VALID,
  ACT_CANCEL
} Entry2_action_t;


class Entry2
{
  typedef enum  // Strictly private to the class
  {
    FF_TYPE      = 0xf000,  // Type mask
      FF_ENTRY   = 0x0000,  //   Entry
      FF_CHECK   = 0x1000,  //   Check button (unimplemented)
      FF_RADIO   = 0x2000,  //   Radio button (unimplemented)
      FF_BUTTON  = 0x3000,  //   Button (unimplemented)
    FF_SUBTYPE   = 0x0f00,  // Subtype mask
      FF_STRING  = 0x0000,  //   Entry: String entry
      FF_INTEGER = 0x0100,  //   Entry: Integer entry
    FF_UPPERCASE = 0x0001,  // String entry: upper-case everything
    FF_NONEMPTY  = 0x0002,  // String entry: can't be empty
    FF_SIGNED    = 0x0001,  // Integer entry: it can be signed (unimplemented)
    FF_INTSIZE   = 0x000e,  // Integer entry: size mask
      FF_CHAR    = 0x0000,  //   char (signed or unsigned)
      FF_SHORT   = 0x0002,  //   short (signed or unsigned)
      FF_INT     = 0x0004,  //   int  (signed or unsigned)
      FF_SIZE_T  = 0x0006,  //   size_t
      FF_LONG    = 0x0008,  //   long (signed or unsigned)
    FF__
  } _field_flags_t;

  public :

    Entry2 (const char *title, const char *fmt, ...);
    ~Entry2 ();
    int loop ();
    Entry2_action_t process_event (const input_status_t &is);
    void refresh ();

  private :

    int count_widgets (const char *fmt);
    int fill_in_widgets_info (const char *fmt, va_list args);
    void do_geom ();
    void jump_to_field (size_t field_no);
    void prev_field ();
    void next_field ();

    // Per-field data
    unsigned short  *box_len;
    char           **buf;
    unsigned short  *buf_max_len;
    const char     **caption;
    bool            *entry_drawn;
    _field_flags_t *entry_flags;

    // Input
    bool first_key;
    size_t field_no;
    size_t nfields;

    // General display stuff
    const char *title;
    bool background_drawn;

    // Geometry constants
    int entry_hofs;
    int entry_vofs;
    int win_hofs;
    int win_vofs;
    int title_vspacing;

    // Geometry stuff
    bool geom_up_to_date;
    int outer_width;
    int outer_height;
    int vstep;
    int win_x0;
    int win_y0;
    int win_x1;
    int win_y1;
    int title_x0;
    int title_y0;
    int caption_x0;
    int caption_y0;
    int entry_box_x0;
    int entry_box_y0;
    int entry_box_x1;
    int entry_box_y1;
    int entry_text_x0;
    int entry_text_y0;
    int entry_text_x1;
    int entry_text_y1;

    // Convenience functions.
    bool is_integer_entry (_field_flags_t flags)
    {
      return (flags & FF_TYPE) == FF_ENTRY
	&& (flags & FF_SUBTYPE) == FF_INTEGER;
    }

    bool is_string_entry (_field_flags_t flags)
    {
      return (flags & FF_TYPE) == FF_ENTRY
	&& (flags & FF_SUBTYPE) == FF_STRING;
    }
};


#endif  /* DO NOT ADD ANYTHING AFTER THIS LINE */
