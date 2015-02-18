/*
 *    yadex.cc
 *    The main module.
 *    BW & RQ sometime in 1993 or 1994.
 */

/*
This file is part of Yadex.

Yadex incorporates code from DEU 5.21 that was put in the public domain in
1994 by Raphaël Quinet and Brendon Wyber.

The rest of Yadex is Copyright © 1997-2003 André Majorel and others.

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

#include <string>

#include "yadex.h"
#include <time.h>
#include "acolours.h"
#include "bench.h"
#include "cfgfile.h"
#include "decorate.h"
#include "disppic.h"  /* Because of "p" */
#include "editlev.h"
#include "endian.h"
#include "flats.h"
#include "game.h"
#include "gfx.h"
#include "gfx2.h"
#include "help1.h"
#include "levels.h"    /* Because of "viewtex" */
#include "lists.h"
#include "mkpalette.h"
#include "palview.h"
#include "patchdir.h"  /* Because of "p" */
#include "rgb.h"
#include "sanity.h"
#include "textures.h"
#include "x11.h"
#include "wadfile.h"
#include "wadlist.h"
#include "wadname.h"
#include "wadres.h"
#include "wads2.h"

using std::string;

/*
 *    Constants (declared in yadex.h)
 */
const char *const log_file       = "yadex.log";
const char *const msg_unexpected = "unexpected error";
const char *const msg_nomem      = "Not enough memory";

/*
 *    Not real variables -- just unique pointer values
 *    used by functions that return pointers to 
 */
char error_non_unique[1];  // Found more than one
char error_none[1];        // Found none
char error_invalid[1];     // Invalid parameter

/*
 *    Global variables
 */
const char *install_dir = 0;        // Where Yadex is installed
FILE *      logfile     = NULL;        // Filepointer to the error log
bool        Registered  = false;    // Registered or shareware game?
int         screen_lines = 24;      // Lines that our TTY can display
int         remind_to_build_nodes = 0;    // Remind user to build nodes
Wad_res     wad_res (&MasterDir);

// Set from command line and/or config file
bool      autoscroll             = 0;
unsigned long autoscroll_amp     = 10;
unsigned long autoscroll_edge    = 30;
const char *config_file          = NULL;
int       copy_linedef_reuse_sidedefs = 0;
int       cpu_big_endian         = 0;
bool      Debug                  = false;
int       default_ceiling_height = 128;
string    default_ceiling_texture = "CEIL3_5";
int       default_floor_height   = 0;
string    default_floor_texture  = "FLOOR4_8";
int       default_light_level    = 144;
string    default_lower_texture  = "STARTAN3";
string    default_middle_texture = "STARTAN3";
string    default_upper_texture  = "STARTAN3";
int       default_thing          = 3004;
int       double_click_timeout   = 200;
bool      Expert                 = false;
const char *Game                 = NULL;
int       grid_pixels_min        = 10;
int       GridMin                = 2;
int       GridMax                = 128;
int       idle_sleep_ms          = 50;
bool      InfoShown              = true;
int       zoom_default           = 0;  // 0 means fit
int       zoom_step              = 0;  // 0 means sqrt(2)
int       digit_zoom_base        = 100;
int       digit_zoom_step        = 0;  // 0 means sqrt(2)
confirm_t insert_vertex_split_linedef  = YC_ASK_ONCE;
confirm_t insert_vertex_merge_vertices = YC_ASK_ONCE;
bool      blindly_swap_sidedefs        = false;
const char *Iwad1   = NULL;
const char *Iwad2   = NULL;
const char *Iwad3   = NULL;
const char *Iwad4   = NULL;
const char *Iwad5   = NULL;
const char *Iwad6   = NULL;
const char *Iwad7   = NULL;
const char *Iwad8   = NULL;
const char *Iwad9   = NULL;
const char *Iwad10  = NULL;
const char *MainWad = NULL;
char **   PatchWads = NULL;
bool      Quiet     = false;
bool      Quieter   = false;
unsigned long scroll_less = 10;
unsigned long scroll_more = 90;
bool      Select0   = false;
int       show_help = 0;
int       sprite_scale = 100;
bool      SwapButtons  = false;
int       verbose      = 0;
int       welcome_message = 1;
const char *bench      = 0;

// Global variables declared in game.h
yglf_t yg_level_format   = YGLF__;
ygln_t yg_level_name     = YGLN__;
ygpf_t yg_picture_format = YGPF_NORMAL;
ygtf_t yg_texture_format = YGTF_NORMAL;
ygtl_t yg_texture_lumps  = YGTL_NORMAL;
al_llist_t *ldtdef       = NULL;
al_llist_t *ldtgroup     = NULL;
al_llist_t *stdef        = NULL;
al_llist_t *thingdef     = NULL;
al_llist_t *thinggroup   = NULL;
Wad_name sky_flat;

/*
 *    Prototypes of private functions
 */
static int   parse_environment_vars ();
static void  MainLoop ();
static void  print_error_message (const char *fmt, va_list args);
static void  add_base_colours ();
static const Wad_file *wad_by_name (const char *pathname);
static bool  wad_already_loaded (const char *pathname);

/*
 *    main
 *    Guess what.
 */
int main (int argc, char *argv[])
{
    int r;

    // Set <screen_lines>
    if (getenv ("LINES") != NULL)
       screen_lines = atoi (getenv ("LINES"));
    else
       screen_lines = 0;
    if (screen_lines == 0)
       screen_lines = 24;

    // First detect manually --help and --version
    // because parse_command_line_options() cannot.
    if (argc == 2 && strcmp (argv[1], "--help") == 0)
    {
       print_usage (stdout);
       if (fflush (stdout) != 0)
         fatal_error ("stdout: %s", strerror (errno));
       exit (0);
    }
    if (argc == 2 && strcmp (argv[1], "--version") == 0)
    {
       puts (what ());
       puts (config_file_magic);
       puts (ygd_file_magic);
       if (fflush (stdout) != 0)
         fatal_error ("stdout: %s", strerror (errno));
       exit (0);
    }

    // Second a quick pass through the command line
    // arguments to detect -?, -f and -help.
    r = parse_command_line_options (argc - 1, argv + 1, 1);
    if (r)
       goto syntax_error;

    if (show_help)
    {
       print_usage (stdout);
       exit (1);
    }

    printf ("%s\n", what ());

    // The config file provides some values.
    if (config_file != NULL)
        r = parse_config_file_user (config_file);
    else
        r = parse_config_file_default ();
    if (r == 0)
    {
       // Environment variables can override them.
       r = parse_environment_vars ();
       if (r == 0)
       {
          // And the command line argument can override both.
          r = parse_command_line_options (argc - 1, argv + 1, 2);
       }
    }
    if (r != 0)
    {
syntax_error :
       fprintf (stderr, "Try \"yadex --help\" or \"man yadex\".\n");
       exit (1);
    }

    if (Game != NULL && strcmp (Game, "doom") == 0)
    {
        if (Iwad1 == NULL)
        {
            err ("You have to tell me where doom.wad is.");
            fprintf (stderr,
                "Use \"-i1 <file>\" or put \"iwad1=<file>\" in yadex.cfg.\n");
            exit (1);
        }
        MainWad = Iwad1;
    }
    else if (Game != NULL && strcmp (Game, "doom2") == 0)
    {
        if (Iwad2 == NULL)
        {
            err ("You have to tell me where doom2.wad is.");
            fprintf (stderr,
                "Use \"-i2 <file>\" or put \"iwad2=<file>\" in yadex.cfg.\n");
            exit (1);
        }
        MainWad = Iwad2;
    }
    else if (Game != NULL && strcmp (Game, "zdoom") == 0)
    {
        if (Iwad2 == NULL)
        {
            err("You have to tell me where doom2.wad is");
            fprintf (stderr,
                "Use \"-i2 <file>\" or put \"iwad2=<file>\" in yadex.cfg.\n");
            exit (1);
        }
        MainWad = Iwad2;
    }
    else if (Game != NULL && strcmp (Game, "heretic") == 0)
    {
        if (Iwad3 == NULL)
        {
            err ("You have to tell me where heretic.wad is.");
            fprintf (stderr,
                "Use \"-i3 <file>\" or put \"iwad3=<file>\" in yadex.cfg.\n");
            exit (1);
        }
        MainWad = Iwad3;
    }
    else if (Game != NULL && strcmp (Game, "hexen") == 0)
    {
        if (Iwad4 == NULL)
        {
            err ("You have to tell me where hexen.wad is.");
            fprintf (stderr,
                "Use \"-i4 <file>\" or put \"iwad4=<file>\" in yadex.cfg.\n");
            exit (1);
        }
        MainWad = Iwad4;
    }
    else if (Game != NULL && strcmp (Game, "strife") == 0)
    {
        if (Iwad5 == NULL)
        {
            err ("You have to tell me where strife1.wad is.");
            fprintf (stderr,
                "Use \"-i5 <file>\" or put \"iwad5=<file>\" in yadex.cfg.\n");
            exit (1);
        }
        MainWad = Iwad5;
    }
    else if (Game != NULL && strcmp (Game, "doom02") == 0)
    {
        if (Iwad6 == NULL)
        {
            err ("You have to tell me where the Doom alpha 0.2 iwad is.");
            fprintf (stderr,
            "Use \"-i6 <file>\" or put \"iwad6=<file>\" in yadex.cfg.\n");
            exit (1);
        }
        MainWad = Iwad6;
    }
    else if (Game != NULL && strcmp (Game, "doom04") == 0)
    {
        if (Iwad7 == NULL)
        {
            err ("You have to tell me where the Doom alpha 0.4 iwad is.");
            fprintf (stderr,
                "Use \"-i7 <file>\" or put \"iwad7=<file>\" in yadex.cfg.\n");
            exit (1);
        }
        MainWad = Iwad7;
    }
    else if (Game != NULL && strcmp (Game, "doom05") == 0)
    {
        if (Iwad8 == NULL)
        {
            err ("You have to tell me where the Doom alpha 0.5 iwad is.");
            fprintf (stderr,
                "Use \"-i8 <file>\" or put \"iwad8=<file>\" in yadex.cfg.\n");
            exit (1);
        }
        MainWad = Iwad8;
    }
    else if (Game != NULL && strcmp (Game, "doompr") == 0)
    {
        if (Iwad9 == NULL)
        {
            err ("You have to tell me where the Doom press release iwad is.");
            fprintf (stderr,
                "Use \"-i9 <file>\" or put \"iwad9=<file>\" in yadex.cfg.\n");
            exit (1);
        }
        MainWad = Iwad9;
    }
    else if (Game != NULL && strcmp (Game, "strife10") == 0)
    {
        if (Iwad10 == NULL)
        {
            err ("You have to tell me where strife1.wad is.");
            fprintf (stderr,
                "Use \"-i10 <file>\" or put \"iwad10=<file>\" in yadex.cfg.\n");
            exit (1);
        }
        MainWad = Iwad10;
    }
    else
    {
        if (Game == NULL)
            err ("You didn't say for which game you want to edit.");
        else
            err ("Unknown game \"%s\"", Game);
        fprintf (stderr,
            "Use \"-g <game>\" on the command line or put \"game=<game>\" in yadex.cfg\n"
            "where <game> is one of \"doom\", \"doom02\", \"doom04\", \"doom05\","
            " \"doom2\",\"zdoom\"\n\"doompr\", \"heretic\", \"hexen\", \"strife\" and "
            "\"strife10\".\n");
        exit (1);
    }
    if (Debug)
    {
        logfile = fopen (log_file, "a");
        if (logfile == NULL)
            warn ("can't open log file \"%s\" (%s)", log_file, strerror (errno));
        LogMessage (": Welcome to Yadex!\n");
    }
    if (Quieter)
        Quiet = true;

    // Sanity checks (useful when porting).
    check_types ();
    check_charset ();

    // Misc. things done only once.
    cpu_big_endian = native_endianness ();
    add_base_colours ();

    // Load game definitions (*.ygd).
    InitGameDefs ();
    LoadGameDefs (Game);

    // Load the iwad and the pwads.
    if (OpenMainWad (MainWad))
        fatal_error ("If you don't give me an iwad, I'll quit. I'm serious.");
    if (PatchWads)
    {
        const char * const *pwad_name;
        for (pwad_name = PatchWads; *pwad_name; pwad_name++)
            OpenPatchWad (*pwad_name);
    }

    /* sanity check */
    CloseUnusedWadFiles ();
    
    // if we use zdoom, now parse the decorate lump
    if (Game != NULL && strcmp (Game, "zdoom") == 0)
        read_decorate();

    // BRANCH 1 : benchmarking (-b)
    if (bench != 0)
    {
        benchmark (bench);
        return 0;  // Exit successfully
    }

    // BRANCH 2 : normal use ("yadex:" prompt)
    else
    {
        if (welcome_message)
           print_welcome (stdout);

        if (strcmp (Game, "hexen") == 0)
            printf (
            "WARNING: Hexen mode is experimental. Don't expect to be able to do any\n"
            "real Hexen editing with it. You can edit levels but you can't save them.\n"
            "And there might be other bugs... BE CAREFUL !\n\n");

        if (strcmp (Game, "strife") == 0)
            printf (
            "WARNING: Strife mode is experimental. Many thing types, linedef types,\n"
            "etc. are missing or wrong. And be careful, there might be bugs.\n\n");

        /* all systems go! */
        MainLoop ();
    }

    /* that's all, folks! */
    CloseWadFiles ();
    FreeGameDefs ();
    LogMessage (": The end!\n\n\n");
    if (logfile != NULL)
        fclose (logfile);
    if (remind_to_build_nodes)
        printf ("\n"
            "** You have made changes to one or more wads. Don't forget to pass\n"
            "** them through a nodes builder (E.G. BSP) before running them.\n"
            "** Like this: \"ybsp foo.wad -o tmp.wad; doom -file tmp.wad\"\n\n");

    return 0;
}


/*
 *    parse_environment_vars
 *    Check certain environment variables.
 *    Returns 0 on success, <>0 on error.
 */
static int parse_environment_vars ()
{
    char *value;

    value = getenv ("YADEX_GAME");
    if (value != NULL)
        Game = value;
    return 0;
}


/*
   play a fascinating tune
*/

void Beep ()
{
    if (! Quieter)
        {
            putchar ('\a');
            fflush (stdout);
        }
}

/*
 *    fatal_error
 *    Print an error message and terminate the program with code 2.
 */
void fatal_error (const char *fmt, ...)
{
    va_list args;
    va_start (args, fmt);
    print_error_message (fmt, args);

    // ... on the other hand, with X, we don't have to
    // call TermGfx() before printing so we do it last so
    // that a segfault occuring in TermGfx() does not
    // prevent us from seeing the stderr message.
    if (GfxMode)
        TermGfx ();  // Don't need to sleep (1) either.

    // Clean up things and free swap space
    ForgetLevelData ();
    ForgetWTextureNames ();
    ForgetFTextureNames ();
    CloseWadFiles ();
    exit (2);
}

/*
 *    err
 *    Print an error message but do not terminate the program.
 */
void err (const char *fmt, ...)
{
    va_list args;

    va_start (args, fmt);
    print_error_message (fmt, args);
}


/*
 *    print_error_message
 *    Print an error message to stderr.
 */
static void print_error_message (const char *fmt, va_list args)
{
    fflush (stdout);
    fputs ("Error: ", stderr);
    vfprintf (stderr, fmt, args);
    fputc ('\n', stderr);
    fflush (stderr);

    if (Debug && logfile != NULL)
    {
        fputs ("Error: ", logfile);
        vfprintf (logfile, fmt, args);
        fputc ('\n', logfile);
        fflush (logfile);
    }
}

/*
 *    nf_bug
 *    Report about a non-fatal bug to stderr. The message
 *    should not expand to more than 80 characters.
 */
void nf_bug (const char *fmt, ...)
{
    static bool first_time = 1;
    static int repeats = 0;
    static char msg_prev[81];
    char msg[81];

    va_list args;
    va_start (args, fmt);
    vsnprintf (msg, sizeof msg, fmt, args);
    if (first_time || strncmp (msg, msg_prev, sizeof msg))
    {
        fflush (stdout);
        if (repeats)
        {
            fprintf (stderr, "Bug: Previous message repeated %d times\n",
            repeats);
            repeats = 0;
        }

        fprintf (stderr, "Bug: %s\n", msg);
        fflush (stderr);
        if (first_time)
        {
            fputs ("REPORT ALL \"Bug:\" MESSAGES TO THE MAINTAINER !\n", stderr);
            first_time = 0;
        }
        strncpy (msg_prev, msg, sizeof msg_prev);
    }else
    {
        repeats++;  // Same message as above
        if (repeats == 10)
        {
            fflush (stdout);
            fprintf (stderr, "Bug: Previous message repeated %d times\n",
            repeats);
            fflush (stderr);
            repeats = 0;
        }
    }
}

/*
   write a message in the log file
*/
void LogMessage (const char *logstr, ...)
{
    va_list  args;
    time_t   tval;
    char    *tstr;

    if (Debug && logfile != NULL)
    {
        va_start (args, logstr);
        /* if the message begins with ":", output the current date & time first */
        if (logstr[0] == ':')
        {
            time (&tval);
            tstr = ctime (&tval);
            tstr[strlen (tstr) - 1] = '\0';
            fprintf (logfile, "%s", tstr);
        }
        vfprintf (logfile, logstr, args);
        fflush (logfile);  /* AYM 19971031 */
    }
}

/*
   the main program menu loop
*/
static void MainLoop ()
{
    char input[120];
    char *com, *out;
    FILE *file, *raw;

    for (;;)
    {
        /* get the input */
        printf ("yadex: ");
        if (! fgets (input, sizeof input, stdin))
        {
            puts ("q");
            break;
        }

        /* Strip the trailing '\n' */
        if (strlen (input) > 0 && input[strlen (input) - 1] == '\n')
            input[strlen (input) - 1] = '\0';

        /* eat the white space and get the first command word */
        com = strtok (input, " ");

        /* user just hit return */
        if (com == NULL)
            printf ("Please enter a command or ? for help.\n");
        /* user inputting for help */
        else if (strcmp (com, "?") == 0
                    || strcmp (com, "h") == 0
                    || strcmp (com, "help") == 0)
        {
            printf ("? | h | help                      --"
                    " display this text\n");
            printf ("b[uild] <WadFile>                 --"
                    " build a new iwad\n");
            printf ("c[reate] <levelname>              --"
                    " create and edit a new (empty) level\n");
            printf ("d[ump] <DirEntry> [outfile]       --"
                    " dump a directory entry in hex\n");
            printf ("e[dit] <levelname>                --"
                    " edit a game level saving results to\n");
            printf ("                                          a patch wad file\n");
            printf ("g[roup] <WadFile>                 --"
                    " group all patch wads in a file\n");
            printf ("i[nsert] <RawFile> <DirEntry>     --"
                    " insert a raw file in a patch wad file\n");
            printf ("l[ist] <WadFile> [outfile]        --"
                    " list the directory of a wadfile\n");
            printf ("m[aster] [outfile]                --"
                    " list the master directory\n");
            printf ("make_gimp_palette <outfile>       --"
                    " generate a gimp palette file from\n"
                    "                                    "
                    " entry 0 of lump PLAYPAL.\n");
            printf ("make_palette_ppm <outfile>        --"
                    " generate a palette image from\n"
                    "                                    "
                    " entry 0 of lump PLAYPAL.\n");
            printf ("q[uit]                            --"
                    " quit\n");
            printf ("r[ead] <WadFile>                  --"
                    " read a new wad patch file\n");
            printf ("s[ave] <DirEntry> <WadFile>       --"
                    " save one object to a separate file\n");
            printf ("set                               --"
                    " list all options and their values\n");
            printf ("v[iew] [<spritename>]             --"
                    " display the sprites\n");
            printf ("viewflat [<flatname>]             --"
                    " flat viewer\n");
            printf ("viewpal                           --"
                    " palette viewer\n");
            printf ("viewpat [<patchname>]             --"
                    " patch viewer\n");
            printf ("viewtex [<texname>]               --"
                    " texture viewer\n");
            printf ("w[ads]                            --"
                    " display the open wads\n");
            printf ("x[tract] <DirEntry> <RawFile>     --"
                    " save (extract) one object to a raw file\n");
        }
        /* user asked for list of open wad files */
        else if (strcmp (com, "wads") == 0 || strcmp (com, "w") == 0)
        {
            const Wad_file *wf;
            wad_list.rewind ();
            if (wad_list.get (wf))
                printf ("%-40s  Iwad\n", wf->pathname ());
            while (wad_list.get (wf))
                printf ("%-40s  Pwad (%.*s)\n",
            wf->pathname (), (int) WAD_NAME, wf->what ());
        }
        /* user asked to quit */
        else if (strcmp (com, "quit") == 0 || strcmp (com, "q") == 0)
        {
            if (! Registered)
                printf ("Remember to register your copy of the game !\n");
            break;
        }
        /* user asked to edit a level */
        else if (strcmp (com, "edit") == 0 || strcmp (com, "e") == 0
                    || strcmp (com, "create") == 0 || strcmp (com, "c") == 0)
        {
            const int newlevel = strcmp (com, "create") == 0
                || strcmp (com, "c") == 0;
            char *level_name = 0;
            com = strtok (NULL, " ");
            if (com == 0)
                if (! newlevel)
                {
                    printf ("Which level ?\n");
                    continue;
                }else
                    level_name = 0;
            else
            {
                level_name = find_level (com);
                if (level_name == error_invalid)
                {
                    printf ("\"%s\" is not a valid level name.\n", com);
                    continue;
                }else if (level_name == error_none)
                {
                    printf ("Neither E%dM%d nor MAP%02d exist.\n",
                    atoi (com) / 10, atoi (com) % 10, atoi (com));
                    continue;
                }else if (level_name == error_non_unique)
                {
                    printf ("Both E%dM%d and MAP%02d exist. Use an unambiguous name.\n",
                    atoi (com) / 10, atoi (com) % 10, atoi (com));
                    continue;
                }else if (level_name == NULL)
                {
                    printf ("Level %s not found.", com);
                    // Hint absent-minded users
                    if ((tolower (*com) == 'e' && yg_level_name == YGLN_MAP01)
                        || (tolower (*com) == 'm' && yg_level_name == YGLN_E1M1))
                        printf (" You are in %s mode.", Game);
                    else if (tolower (*com) == 'e' && com[1] > '1' && ! Registered)
                        printf (" You have the shareware iwad.");
                    putchar ('\n');
                    continue;
                }
            }
            EditLevel (level_name, newlevel);
            if (level_name)
                free (level_name);
        }
        /* user asked to build a new main wad file */
        else if (strcmp (com, "build") == 0 || strcmp (com, "b") == 0)
        {
            com = strtok (NULL, " ");
            if (com == NULL)
            {
                printf ("Wad file name argument missing.\n");
                continue;
            }
            if (wad_already_loaded (com))
            {
                printf ("%s: in use, close it first\n", com);
                continue;
            }
            BuildNewMainWad (com, 0);
        }
        /* user asked to build a compound patch wad file */
        else if (strcmp (com, "group") == 0 || strcmp (com, "g") == 0)
        {
            wad_list.rewind ();
            const Wad_file *wf;
            if (! wad_list.get (wf) || ! wad_list.get (wf))
            {
                printf ("You need at least two open wad files "
                        "if you want to group them.\n");
                continue;
            }
            com = strtok (NULL, " ");
            if (com == NULL)
            {
                printf ("Wad file name argument missing.\n");
                continue;
            }
            if (wad_already_loaded (com))
            {
                printf ("%s: in use, close it first\n", com);
                continue;
            }
            BuildNewMainWad (com, 1);
        }
        /* user ask for a listing of a wad file */
        else if (strcmp (com, "list") == 0 || strcmp (com, "l") == 0)
        {
            com = strtok (NULL, " ");
            if (com == NULL)
            {
                printf ("Wad file name argument missing.\n");
                continue;
            }
            const Wad_file *wf = wad_by_name (com);
            if (wf == 0)
            {
                printf ("%s: not open\n", com);
                continue;
            }
            out = strtok (NULL, " ");
            if (out)
            {
                printf ("Outputting directory of \"%s\" to \"%s\".\n",
                wf->pathname (), out);

                if ((file = fopen (out, "w")) == NULL)
                    fatal_error ("error opening output file \"%s\"", com);

                fprintf (file, "%s\n", what ());
                ListFileDirectory (file, wf);
                fprintf (file, "\nEnd of file.\n");
                fclose (file);
            }
            else
                ListFileDirectory (stdout, wf);
        }
        /* user asked for the list of the master directory */
        else if (strcmp (com, "master") == 0 || strcmp (com, "m") == 0)
        {
            out = strtok (NULL, " ");
            if (out)
            {
                printf ("Outputting master directory to \"%s\".\n", out);

                if ((file = fopen (out, "w")) == NULL)
                    fatal_error ("error opening output file \"%s\"", com);

                fprintf (file, "%s\n", what ());
                ListMasterDirectory (file);
                fprintf (file, "\nEnd of file.\n");
                fclose (file);
            }
            else
                ListMasterDirectory (stdout);
        }
        // make_gimp_palette
        else if (strcmp (com, "make_gimp_palette") == 0)
        {
            out = strtok (NULL, " ");
            if (out == NULL)
            {
                printf ("Output file name argument missing.\n");
                continue;
            }
            make_gimp_palette (0, out);
        }
        // make_palette_ppm
        else if (strcmp (com, "make_palette_ppm") == 0)
        {
            out = strtok (NULL, "");
            if (out == NULL)
            {
                printf ("Output file name argument missing.\n");
                continue;
            }
            make_palette_ppm (0, out);
        }
        // make_palette_ppm
        else if (strcmp (com, "mp2") == 0)
        {
            out = strtok (NULL, "");
            if (out == NULL)
            {
                printf ("Output file name argument missing.\n");
                continue;
            }
            make_palette_ppm_2 (0, out);
        }
        /* user asked to list all options and their values */
        else if (strcmp (com, "set") == 0)
        {
            dump_parameters (stdout);
        }
        /* user asked to read a new patch wad file */
        else if (strcmp (com, "read") == 0 || strcmp (com, "r") == 0)
        {
            com = strtok (NULL, " ");
            if (com == NULL)
            {
                printf ("Wad file name argument missing.\n");
                continue;
            }
            out = strtok (NULL, " ");
            if (out)
                *out = '\0';
            out = (char *) malloc(strlen (com) + 1);
            strncpy (out, com, strlen(com));
            OpenPatchWad (out);
            CloseUnusedWadFiles ();
        }
        /* user asked to dump the contents of a wad file */
        else if (strcmp (com, "dump") == 0 || strcmp (com, "d") == 0)
        {
            com = strtok (NULL, " ");
            if (com == NULL)
            {
                printf ("Object name argument missing.\n");
                continue;
            }
            out = strtok (NULL, " ");
            if (out)
            {
                printf ("Outputting directory entry data to \"%s\".\n", out);

                if ((file = fopen (out, "w")) == NULL)
                    fatal_error ("error opening output file \"%s\"", com);

                fprintf (file, "%s\n", what ());
                DumpDirectoryEntry (file, com);
                fprintf (file, "\nEnd of file.\n");
                fclose (file);
            }
            else
                DumpDirectoryEntry (stdout, com);
        }
        // "v"/"view" - view the sprites
        else if (strcmp (com, "view") == 0 || strcmp (com, "v") == 0) {
			  if (InitGfx()) {
				  init_input_status ();
				  do {
					  get_input_status ();
				  } while (is.key != YE_EXPOSE);
				  force_window_not_pixmap ();  // FIXME quick hack
				  const char *sprite = strtok (NULL, " ");
				  vector<string> lumps = wad_res.sprites.list();
				  char buf[WAD_PIC_NAME + 1];
				  *buf = '\0';
				  if (sprite != 0) {
					  strncat (buf, sprite, sizeof buf - 1);
					  for (char *p = buf; *p != '\0'; p++)
						  *p = toupper (*p);
				  }
				  InputNameFromListWithFunc (0, 0, "Sprite viewer", lumps, 10, string(buf), 320, 200, display_pic,
						  HOOK_DISP_SIZE | HOOK_SPRITE);
				  TermGfx ();
			  }
        }
		  // "viewflat" - view the flats
		  else if (strcmp (com, "viewflat") == 0) {
			  if (InitGfx ()) {
				  init_input_status ();
				  do {
					  get_input_status ();
				  } while (is.key != YE_EXPOSE);
				  com = strtok (NULL, " ");
				  force_window_not_pixmap ();  // FIXME quick hack
				  string buf;
				  if (com != NULL)
					  buf = string(com);
				  ReadFTextureNames ();
				  vector<string> flat_names;
				  for (size_t n = 0; n < NumFTexture; n++)
					  flat_names.push_back(string(flat_list[n].name));
				  ChooseFloorTexture (0, 0, "Flat viewer", flat_names, buf);
				  ForgetFTextureNames ();
				  TermGfx ();
			  }
		  }
		  // "viewpal" - view the palette (PLAYPAL and COLORMAP)
		  else if (strcmp (com, "viewpal") == 0) {
			  if (InitGfx ()) {
				  init_input_status ();
				  do {
					  get_input_status ();
				  } while (is.key != YE_EXPOSE);
				  force_window_not_pixmap ();  // FIXME quick hack
				  Palette_viewer pv;
				  pv.run ();
				  TermGfx ();
			  }
		  }
        // "viewpat" - view the patches
		  else if (strcmp (com, "viewpat") == 0) {
			  if (InitGfx()) {
				  init_input_status ();
				  do {
					  get_input_status ();
				  } while (is.key != YE_EXPOSE);
				  com = strtok (NULL, " ");
				  force_window_not_pixmap ();  // FIXME quick hack
				  patch_dir.refresh (MasterDir);
				  string buf;
				  if (com != NULL)
					  buf = string(com);
				  InputNameFromListWithFunc (0, 0, "Patch viewer",
						  patch_dir.list(), 10, buf, 256, 256, display_pic,
						  HOOK_DISP_SIZE | HOOK_PATCH);
				  TermGfx ();
			  }
		  }
		  // "viewtex" - view the textures
		  else if (strcmp (com, "viewtex") == 0) {
			  if (InitGfx ()) {
				  init_input_status ();
				  do {
					  get_input_status ();
				  } while (is.key != YE_EXPOSE);
				  com = strtok (NULL, " ");
				  force_window_not_pixmap ();  // FIXME quick hack
				  patch_dir.refresh (MasterDir);
				  string buf;
				  if (com != NULL)
					  buf = string(com);
				  ReadWTextureNames ();
				  ChooseWallTexture(0, 0, "Texture viewer", WTexture, buf);
				  ForgetWTextureNames ();
				  TermGfx ();
			  }
		  }
		  /* user asked to save an object to a separate pwad file */
		  else if (strcmp (com, "save") == 0 || strcmp (com, "s") == 0) {
			  com = strtok (NULL, " ");
			  if (com == NULL) {
				  printf ("Object name argument missing.\n");
				  continue;
			  }
			  if (strlen (com) > WAD_NAME || strchr (com, '.') != NULL) {
				  printf ("Invalid object name.\n");
				  continue;
			  }
			  out = strtok (NULL, " ");
			  if (out == NULL) {
				  printf ("Wad file name argument missing.\n");
				  continue;
			  }
			  if (wad_already_loaded (com)) {
				  printf ("%s: in use, close it first\n", com);
				  continue;
			  }
			  printf ("Saving directory entry data to \"%s\".\n", out);
			  if ((file = fopen (out, "wb")) == NULL)
				  fatal_error ("error opening output file \"%s\"", out);
			  SaveDirectoryEntry (file, com);
			  fclose (file);
		  }
		  /* user asked to encapsulate a raw file in a pwad file */
		  else if (strncmp (com, "insert", sizeof("insert")) == 0 || strncmp (com, "i", sizeof("i")) == 0) {
			  com = strtok (NULL, " ");
			  if (com == NULL) {
				  printf ("Raw file name argument missing.\n");
				  continue;
			  }
			  out = strtok (NULL, " ");
			  if (out == NULL) {
				  printf ("Object name argument missing.\n");
				  continue;
			  }
			  if (strlen (out) > WAD_NAME || strchr (out, '.') != NULL) {
				  printf ("Invalid object name.\n");
				  continue;
			  }
			  if ((raw = fopen (com, "rb")) == NULL)
				  fatal_error ("error opening input file \"%s\"", com);
			  /* kluge */
			  strcpy (input, out);
			  strcat (input, ".wad");
			  if (wad_already_loaded (input)) {
				  printf ("%s: in use, close it first\n", input);
				  continue;
			  }
			  printf ("Including new object %s in \"%s\".\n", out, input);
			  if ((file = fopen (input, "wb")) == NULL)
				  fatal_error ("error opening output file \"%s\"", input);
			  SaveEntryFromRawFile (file, raw, out);
			  fclose (raw);
			  fclose (file);
		  }
		  /* user asked to extract an object to a raw binary file */
		  else if (strcmp (com, "xtract") == 0
				  || strcmp (com, "extract") == 0
				  || strcmp (com, "x") == 0) {
			  com = strtok (NULL, " ");
			  if (com == NULL) {
				  printf ("Object name argument missing.\n");
				  continue;
			  }
			  if (strlen (com) > WAD_NAME || strchr (com, '.') != NULL) {
				  printf ("Invalid object name.\n");
				  continue;
			  }
			  out = strtok (NULL, " ");
			  if (out == NULL) {
				  printf ("Raw file name argument missing.\n");
				  continue;
			  }
			  if (wad_already_loaded (com)) {
				  printf ("%s: in use, close it first\n", com);
				  printf ("Besides do you really want to overwrite a wad file with"
						  " raw data ?\n");
				  continue;
			  }
			  printf ("Saving directory entry data to \"%s\".\n", out);
			  if ((file = fopen (out, "wb")) == NULL)
				  fatal_error ("error opening output file \"%s\"", out);
			  SaveEntryToRawFile (file, com);
			  fclose (file);
		  }
		  /* unknown command */
		  else
			  printf ("Unknown command \"%s\"!\n", com);
	 }
}

/*
 *    add_base_colours
 *    Add the NCOLOURS base colours to the list of
 *    application colours.
 */
static void add_base_colours ()
{
    for (size_t n = 0; n < NCOLOURS; n++)
    {
        rgb_c c;

        // The first 16 are the standard IRGB VGA colours.
        // FIXME they're to be removed and replaced by
        // "logical" colours.
#ifdef WHITE_BACKGROUND
        if (n == 0)
            irgb2rgb (15, &c);
        else if (n == 15)
            irgb2rgb (0, &c);
        else
#endif
        if (n < 16)
            irgb2rgb (n, &c);

        // Then there are the colours used to draw the
        // windows and the map. The colours used to draw
        // the things are parametrized in the .ygd ; they
        // are added by load_game().
        // FIXME they should not be hard-coded, of course !

        /* WINBG* is for window backgrounds. Use the _HL variant is
        for highlighted parts of windows (E.G. the current line in
        a menu). _LIGHT and _DARK are for window borders and
        grooves. There is no _HL flavour of these because I didn't
        feel the need. */
#ifdef WHITE_BACKGROUND
        else if (n == WINBG)        c.set (0xe2, 0xdc, 0xd6);
        else if (n == WINBG_LIGHT)  c.set (0xee, 0xe8, 0xe2);
        else if (n == WINBG_DARK)   c.set (0xc3, 0xbe, 0xb9);
        else if (n == WINBG_HL)     c.set (0xf4, 0xee, 0xe7);
#else
        else if (n == WINBG)        c.set (0x2a, 0x24, 0x18);
        else if (n == WINBG_LIGHT)  c.set (0x48, 0x42, 0x3c);
        else if (n == WINBG_DARK)   c.set (0x20, 0x1b, 0x12);
        else if (n == WINBG_HL)     c.set (0x58, 0x50, 0x48);
#endif

        /* WINFG* is for regular text. _DIM is for greyed out text
        (for disabled options or text that is not applicable). */
#ifdef WHITE_BACKGROUND
        else if (n == WINFG)        c.set (0x60, 0x60, 0x60);
        else if (n == WINFG_HL)     c.set (0x30, 0x30, 0x30);
        else if (n == WINFG_DIM)    c.set (0xB8, 0xB8, 0xB8);
        else if (n == WINFG_DIM_HL) c.set (0x90, 0x90, 0x90);
#else
        else if (n == WINFG)        c.set (0xa0, 0xa0, 0xa0);
        else if (n == WINFG_HL)     c.set (0xd0, 0xd0, 0xd0);
        else if (n == WINFG_DIM)    c.set (0x48, 0x48, 0x48);
        else if (n == WINFG_DIM_HL) c.set (0x70, 0x70, 0x70);
#endif

        /* WINLABEL is for text of lesser importance. For example,
        the brackets around key binding are displayed in WINLABEL,
        while what's between them is displayed in WINFG. The
        difference with WINFG is not very noticeable but it does
        improve readability. The static text in the object info
        windows should be displayed in WINLABEL. */
#ifdef WHITE_BACKGROUND
        else if (n == WINLABEL)        c.set (0x88, 0x88, 0x88);
        else if (n == WINLABEL_HL)     c.set (0x60, 0x60, 0x60);
        else if (n == WINLABEL_DIM)    c.set (0xc8, 0xc8, 0xc8);
        else if (n == WINLABEL_DIM_HL) c.set (0xb0, 0xb0, 0xb0);
#else
        else if (n == WINLABEL)        c.set (0x78, 0x78, 0x78);
        else if (n == WINLABEL_HL)     c.set (0xa0, 0xa0, 0xa0);
        else if (n == WINLABEL_DIM)    c.set (0x38, 0x38, 0x38);
        else if (n == WINLABEL_DIM_HL) c.set (0x50, 0x50, 0x50);
#endif

#ifdef WHITE_BACKGROUND
        else if (n == GRID1)  c.set (0x80, 0x80, 0xff);
        else if (n == GRID2H) c.set (0xf0, 0xf0, 0xff);
        else if (n == GRID2V) c.set (0xf0, 0xf0, 0xff);
        else if (n == GRID3H) c.set (0xd0, 0xd0, 0xff);
        else if (n == GRID3V) c.set (0xd0, 0xd0, 0xff);
        else if (n == GRID4H) c.set (0xb0, 0xb0, 0xff);
        else if (n == GRID4V) c.set (0xb0, 0xb0, 0xff);
#else
        else if (n == GRID1)  c.set (0, 0, 0xc0);
        else if (n == GRID2H) c.set (0, 0, 0x30);
        else if (n == GRID2V) c.set (0, 0, 0x40);
        else if (n == GRID3H) c.set (0, 0, 0x50);
        else if (n == GRID3V) c.set (0, 0, 0x70);
        else if (n == GRID4H) c.set (0, 0, 0x80);
        else if (n == GRID4V) c.set (0, 0, 0xc0);
#endif

        else if (n == LINEDEF_NO) c.set (0x40, 0xd0, 0xf0);
        else if (n == SECTOR_NO)  c.set (0x40, 0xd0, 0xf0);
        else if (n == THING_NO)   c.set (0x40, 0xd0, 0xf0);
        else if (n == VERTEX_NO)  c.set (0x40, 0xd0, 0xf0);
        else if (n == CLR_ERROR)  c.set (0xff, 0,    0);
        else if (n == THING_REM)  c.set (0x40, 0x40, 0x40);

        else if (n == SECTOR_TAG)     c.set (0x00, 0xff, 0x00);
        else if (n == SECTOR_TAGTYPE) c.set (0x00, 0xe0, 0xe0);
        else if (n == SECTOR_TYPE)    c.set (0x00, 0x80, 0xff);

#ifdef WHITE_BACKGROUND
        else if (n == WINTITLE) c.set (0xb0, 0x80, 0x00);
#else
        else if (n == WINTITLE) c.set (0xff, 0xff, 0x00);
#endif

        else
            fatal_error ("Wrong acn %d", n);

        acolour_t acn = add_app_colour (c);
        if (acn != n)
            fatal_error ("add_base_colours: got %d for %d\n", acn, n);
    }
}


static const Wad_file *wad_by_name (const char *pathname)
{
    const Wad_file *wf;

    for (wad_list.rewind (); wad_list.get (wf);)
        if (strcmp(pathname, wf->pathname ()) == 0)
            return wf;
    return 0;
}

static bool wad_already_loaded (const char *pathname)
{
    return wad_by_name (pathname) != 0;
}
