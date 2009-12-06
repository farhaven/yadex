/*
 *	events.h
 *	AYM 1998-11-09
 */


/*
The idea here is to have the application generate
events internally (as opposed to X event passed
by get_input_status()).
It's still _only_ an idea ; this API is bogus.
*/

void init_event ();
void send_event (int);
int has_event ();
int has_event (int);
int has_key_press_event ();
int get_event ();


