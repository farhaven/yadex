/*
 *	gamedef.h - Game_def class
 *	AYM 2000-08-30
 */


#ifndef YH_GAMEDEF  /* DO NOT INSERT ANYTHING BEFORE THIS LINE */
#define YH_GAMEDEF


class Game_def_priv;


typedef struct
{
  char ldtgroup;
  const char *shortdesc;
  const char *longdesc;
  const char *argument1; // these describe a linedefs arguments
  const char *argument2; // this only works for hexen maps or
  const char *argument3; // doom in hexen format maps
  const char *argument4;
  const char *argument5;
} linedef_type_t;


typedef char linedef_type_group_id_t;


typedef struct
{
  const char *desc;
} linedef_type_group_t;


typedef struct
{
  const char *shortdesc;
  const char *longdesc;
} sector_type_t;


typedef char thing_type_group_id_t;


typedef struct
{
  char flags;		// Flags
  int radius;		// Radius of thing
  const char *desc;	// Short description of thing
  const char *sprite;	// Root of name of sprite for thing
} thing_type_t;


const char THINGDEF_SPECTRAL = 0x01;


typedef struct
{
  char thinggroup;	// Thing group
  rgb_c rgb;		// RGB colour
  acolour_t acn;	// Application colour#
  const char *desc;	// Description of thing group
} thing_type_group_t;


typedef enum { YGLF__, YGLF_ALPHA, YGLF_DOOM, YGLF_HEXEN } yglf_t;
typedef enum { YGLN__, YGLN_E1M10, YGLN_E1M1, YGLN_MAP01 } ygln_t;
typedef enum { YGPF_NORMAL, YGPF_ALPHA, YGPF_PR } ygpf_t;
typedef enum { YGTF_NORMAL, YGTF_NAMELESS, YGTF_STRIFE11 } ygtf_t;
typedef enum { YGTL_NORMAL, YGTL_TEXTURES, YGTL_NONE } ygtl_t;


/* shorthands to make program more readable */

#define LDT_FREE    '\0'  /* KLUDGE: bogus ldt group   (see game.c) */
#define ST_FREE     '\0'  /* KLUDGE: bogus sector type (see game.c) */
#define THING_FREE  '\0'  /* KLUDGE: bogus thing group (see game.c) */


/*
 *	Game_def - contain all the definitions relative to a game
 */
class Game_def
{
  public :
    Game_def ();
    ~Game_def ();
    const linedef_type_t& linedef_type (wad_ldtype_t type) const;
    const sector_type_t&  sector_type  (wad_stype_t  type) const;
    const thing_type_t&   thing_type   (wad_ttype_t  type) const;
    void linedef_type     (wad_ldtype_t type, linedef_type_t& data);
    void sector_type      (wad_stype_t type,  sector_type_t&  data);
    void thing_type       (wad_ttype_t type,  thing_type_t&   data);
    void del_linedef_type (wad_ldtype_t type);
    void del_sector_type  (wad_stype_t  type);
    void del_thing_type   (wad_ttype_t  type);
    int level_format      (yglf_t level_format);
    int level_name        (yglf_t level_name);
    int picture_format    (ygpf_t picture_format);
    int texture_format    (ygtf_t texture_format);
    int texture_lumps     (ygtl_t texture_lumps);
    yglf_t level_format   () const;
    ygln_t level_name     () const;
    ygpf_t picture_format () const;
    ygtf_t texture_format () const;
    ygtl_t texture_lumps  () const;

  private :
    Game_def            (const Game_def&);	// Too lazy to implement it
    Game_def& operator= (const Game_def&);	// Too lazy to implement it
    Game_def_priv *priv;
};


#endif  /* DO NOT ADD ANYTHING AFTER THIS LINE */
