/*
 *	events.c
 *	Half baked internal event handling.
 *	AYM 1998-11-09
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


/*
This module sucks TOTALLY !
Issues :
- should functions that take an event type as a criterion
  limit themselves to the last event or look the entire
  queue ?
- should probably group all YK_* events under the type
  YE_KEYPRESS and put the exact key number in a struct
  field,
- and more...
*/


#include "yadex.h"
#include "events.h"


// Number of cells in the event queue (actually a circular
// buffer). The queue can actually hold only that number
// minus one events.
#define Y_EVENT_QUEUE 100
static int event_queue[Y_EVENT_QUEUE];
// Index of the head and tail of the circular buffer.
// At any moment, the next event to get is in event_queue[head]
// and the next put will be put in event_queue[tail]
static int head = 0;
static int events_in_queue = 0;



/*
 *
 */
void init_event ()
{
  head = 0;
  events_in_queue = 0;
}


/*
 *
 */
void send_event (int event)
{
  if (events_in_queue == Y_EVENT_QUEUE)
    fatal_error ("Event buffer full");
  event_queue[(head + events_in_queue) % Y_EVENT_QUEUE] = event;
  events_in_queue++;
}


/*
 *	has_event
 *	Is there any event at all ?
 */
int has_event ()
{
  return events_in_queue != 0;
}


/*
 *	has_event
 *	Is there an event of that type ?
 */
int has_event (int event)
{
  return events_in_queue != 0
    && event_queue[head] == event;
}


/*
 *	has_key_press_event
 *	Is there an YK_* event ?
 *	FIXME should create YE_KEY_PRESS and delete this function.
 */
int has_key_press_event ()
{
  return events_in_queue != 0
    && event_queue[head]
    && (event_queue[head] & ~ (YK_ALT | YK_CTRL | YK_SHIFT)) < YK__LAST;
}


/*
 *	get_event
 *	Get the next event
 */
int get_event ()
{
  if (events_in_queue == 0)  // The buffer is empty
    return 0;

  int e = event_queue[head];
  events_in_queue--;
  head++;
  if (head == Y_EVENT_QUEUE)
    head = 0;
  return e;
}




