/*
 *	editzoom.h
 *	AYM 1998-11-09
 */


/* zoom factors for the digit keys
 */
extern double digit_zoom_factors[10];


/*
 *      edit_zoom_init - initialise
 */
void edit_zoom_init (void);


/*
 *	edit_zoom_in - zoom_in
 *
 *	If zooming in would result in a zoom factor higher than
 *	10.0, do nothing.
 *
 *	Return 0 on success, non-zero on failure.
 */
int edit_zoom_in (edit_t *e);


/*
 *	edit_zoom_out - zoom_out
 *
 *	If zooming out would result in a zoom factor lesser than
 *	0.05, do nothing.
 *
 *	Return 0 on success, non-zero on failure.
 */
int edit_zoom_out (edit_t *e);


/*
 *	edit_set_zoom - set zoom factor
 *
 *	If the new zoom factor is less than 0.05 or more than
 *	10.0, do nothing.
 *
 *	Return 0 on success, non-zero on failure.
 */
int edit_set_zoom (edit_t *e, double zoom_factor);


