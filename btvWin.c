/* $Id: btvWin.c,v 1.103 1999/12/01 20:39:47 davewang Exp $ */

#include "btv.h"

/*
 * This is the basic data structure of the btvWin widget.
 * 
 * This code forms the backbone which allows one to take abstract
 * "resources" and combine them together to form a generic "cell".
 */

typedef struct {
	Tk_Window       tkwin;		/* Window that embodies the btvWin.  NULL
				 	* means window has been deleted but
				 	* widget record hasn't been cleaned up yet. */
	Display        *display;	/* X's token for the window's display. */
	Tcl_Interp     *interp;		/* Interpreter associated with widget. */
	Tcl_Command     widgetCmd;	/* Token for btvWin's widget command. */
	int             borderWidth;	/* Width of 3-D border around whole widget. */

	STATS		stats;

	TRACE		trace;

	int             updatePending;		/* Non-zero means a call to btvWinDisplay
					 	* has already been scheduled. */
	int		display_modes[DISPLAY_MODE_COUNT];
	int		color[COLOR_MODE_COUNT];
	GEOMETRY	geometry;
	Tk_3DBorder	Border_array[BTV_COLOR_PALLET_SIZE];
	GC		GC_array[BTV_COLOR_PALLET_SIZE];
	Tk_3DBorder	GRAY_Border_array[BTV_GRAY_PALLET_SIZE];
	GC		GRAY_GC_array[BTV_GRAY_PALLET_SIZE];
	char            font_name[10];          /* allow us to use different fonts */
	int             font_size;
	char            font_attribute[10];
	Tk_Font         font;
	char		*xScrollCmd;     /* Prefix of command to issue to update
                                         * horizontal scrollbar when view changes. */
	char		*yScrollCmd;     /* Prefix of command to issue to update
                                         * vertical scrollbar when view changes. */
	double 		xScrollFirst, xScrollLast;
                                        /* Most recent values reported to horizontal
                                         * scrollbar;  used to eliminate unnecessary
                                         * reports. */
	double		yScrollFirst, yScrollLast;
                                        /* Most recent values reported to vertical
                                         * scrollbar;  used to eliminate unnecessary
                                         * reports. */
} btvWin;

/*
 * Forward declarations for procedures defined later in this file:
 * these are the things that we use for the TK windows stuff.
 */

int btvWinCmd _ANSI_ARGS_((ClientData clientData, Tcl_Interp *, int, char **argv));
static void btvWinCmdDeletedProc _ANSI_ARGS_((ClientData clientData));
static int  btvWinConfigure _ANSI_ARGS_((Tcl_Interp *, btvWin *, int, char **argv, int));
static void btvWinDestroy _ANSI_ARGS_((char *memPtr));
static void btvWinDisplay _ANSI_ARGS_((ClientData clientData));
static void btvWinEventProc _ANSI_ARGS_((ClientData, XEvent *));
static int  btvWinWidgetCmd _ANSI_ARGS_((ClientData, Tcl_Interp *, int, char **argv));
static int  btvXviewCmd  _ANSI_ARGS_((ClientData *, Tcl_Interp *, int, char **argv));
static int  btvYviewCmd  _ANSI_ARGS_((ClientData *, Tcl_Interp *, int, char **argv));
static void btvMovYSBCmd _ANSI_ARGS_((Tcl_Interp *, ClientData *, int));
static void btvMovXSBCmd _ANSI_ARGS_((Tcl_Interp *, ClientData *, int));

/*
 * Information used for argv parsing.
 */

static Tk_ConfigSpec configSpecs[] =
{
	{TK_CONFIG_PIXELS, "-borderwidth", "borderWidth", "BorderWidth",
	 "2", Tk_Offset(btvWin, borderWidth), 0},
        {TK_CONFIG_STRING, "-xscrollcommand", "xScrollCommand", "ScrollCommand",
         DEF_XSCROLL_COMMAND, Tk_Offset(btvWin, xScrollCmd),
         TK_CONFIG_NULL_OK},
        {TK_CONFIG_STRING, "-yscrollcommand", "yScrollCommand", "ScrollCommand",
         DEF_YSCROLL_COMMAND, Tk_Offset(btvWin, yScrollCmd),
         TK_CONFIG_NULL_OK},
	{TK_CONFIG_END, (char *)NULL, (char *)NULL, (char *)NULL,
	 (char *)NULL, 0, 0}
};

/*--------------------------------------------------------------
 *
 * btvXviewCmd --
 *
 *      This procedure is invoked to process the "xview" option for
 *      the widget command for pttext widgets. See the user documentation
 *      for details on what it does.
 *
 * Results:
 *      A standard Tcl result.
 *
 * Side effects:
 *      See the user documentation.
 *
 *--------------------------------------------------------------
 */

int 
btvXviewCmd(clientData, interp, argc, argv)
	ClientData     *clientData;
	Tcl_Interp     *interp;
	int             argc;
	char          **argv;
{
	btvWin      *btvWinPtr = (btvWin *) clientData;
	Tk_Window       tkwin = btvWinPtr->tkwin;
	int             type;
	int             count;
	int		first_event;
	int		win_width;
	int		x_axis_scale;
	double		first_time;
	double          fraction;
	
	if (argc == 2) {
		btvMovXSBCmd(interp, clientData, 0);
		return TCL_OK;
	}
	type = Tk_GetScrollInfo(interp, argc, argv, &fraction, &count);
	win_width = Tk_Width(tkwin);
	x_axis_scale = btvWinPtr->geometry.x_axis_scale;

	switch (type) {
	case TK_SCROLL_ERROR:
		return TCL_ERROR;

	case TK_SCROLL_MOVETO:
		if (fraction > 1.0) {
			fraction = 1.0;
		}
		if (fraction < 0.0) {
			fraction = 0.0;
		}
		first_event 	= (btvWinPtr->trace.event_count) * fraction;
		first_time 		= btvWinPtr->trace.start_time +
					(btvWinPtr->trace.end_time - btvWinPtr->trace.start_time) * fraction;
		break;

	case TK_SCROLL_PAGES:
		first_event 	= (btvWinPtr->geometry.first_event) + count * x_axis_scale * win_width ;
		break;
	case TK_SCROLL_UNITS:
		first_event 	= (btvWinPtr->geometry.first_event) + count * 5 * x_axis_scale ;
		first_time 		= btvWinPtr->trace.start_time +
					(btvWinPtr->trace.end_time - btvWinPtr->trace.start_time) * fraction;
		break;
	}
	
	return TCL_OK;
}

/*--------------------------------------------------------------
 *
 * btvYviewCmd --
 *
 *
 *      This procedure is invoked to process the "yview" option for
 *      the widget command for pttext widgets. See the user documentation
 *      for details on what it does.
 *
 * Results:
 *      A standard Tcl result.
 *
 * Side effects:
 *      See the user documentation.
 *
 *-------------------------------------------------------------- */

int 
btvYviewCmd(clientData, interp, argc, argv)
	ClientData     *clientData;
	Tcl_Interp     *interp;
	int             argc;
	char          **argv;

{
	btvWin      *btvWinPtr = (btvWin *) clientData;
	Tk_Window       tkwin = btvWinPtr->tkwin;
	int             type;
	int             count,winheight;
	double          fraction;

	if (argc == 2) {
		btvMovYSBCmd(interp, clientData, 0);
		return TCL_OK;
	}

	type = Tk_GetScrollInfo(interp, argc, argv, &fraction, &count);

	switch (type) {
		case TK_SCROLL_ERROR:
			return TCL_ERROR;

		case TK_SCROLL_MOVETO:
			if (fraction > 1.0) {
				fraction = 1.0;
			}
			if (fraction < 0.0) {
				fraction = 0.0;
			}
			break;

		case TK_SCROLL_UNITS:
		case TK_SCROLL_PAGES:
	
			break;
	}

	return TCL_OK;
}


/*----------------------------------------------------------------------
 *
 * btvMovYSBCmd --
 *
 *      This procedure computes the fractions that indicate what's
 *      visible in a text window and, optionally, evaluates a
 *      Tcl script to report them to the text's associated scrollbar.
 *
 * Results:
 *      If report is zero, then interp->result is filled in with
 *      two real numbers separated by a space, giving the position of
 *      the top and bottom of the window as fractions from 0 to 1, where
 *      0 means the beginning of the text and 1 means the end.  If
 *      report is non-zero, then interp->result isn't modified directly,
 *      but a script is evaluated in interp to report the new scroll
 *      position to the scrollbar (if the scroll position hasn't changed
 *      then no script is invoked).
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

static void
btvMovYSBCmd(interp, clientData, report)
	Tcl_Interp     *interp;	/* If "report" is FALSE, string
				 * describing visible range gets
				 * stored in interp->result. */
	ClientData     *clientData;	/* Information about text widget. */
	int             report;	/* Non-zero means report info to
				 * scrollbar if it has changed. */
{
	btvWin      *btvWinPtr = (btvWin *) clientData;
	Tk_Window       tkwin = btvWinPtr->tkwin;
	int             result;
	char            buffer[200];
	double          first, last;

	first = 0.0;
		
	last = 1.0;

        if (!report) {
            sprintf(interp->result, "%g %g", first, last);
            return;
        }
        if ((first == btvWinPtr->yScrollFirst) && (last == btvWinPtr->yScrollLast)) {
            return;
        }
        btvWinPtr->yScrollFirst = first;
        btvWinPtr->yScrollLast = last;

	sprintf(buffer, " %g %g", first, last);

	result = Tcl_VarEval(interp, btvWinPtr->yScrollCmd, buffer, (char *)NULL);
	if (result != TCL_OK) {
		Tcl_AddErrorInfo(interp,
		     "\n    (vertical scrolling command executed by text)");
		Tcl_BackgroundError(interp);
	}
}


/*----------------------------------------------------------------------
 *
 * btvMovXSBCmd --
 *
 *      This procedure computes the fractions that indicate what's
 *      visible in a text window and, optionally, evaluates a
 *      Tcl script to report them to the text's associated scrollbar.
 *
 * Results:
 *      If report is zero, then interp->result is filled in with
 *      two real numbers separated by a space, giving the position of
 *      the top and bottom of the window as fractions from 0 to 1, where
 *      0 means the beginning of the text and 1 means the end.  If
 *      report is non-zero, then interp->result isn't modified directly,
 *      but a script is evaluated in interp to report the new scroll
 *      position to the scrollbar (if the scroll position hasn't changed
 *      then no script is invoked).
 *
 * Side effects:
 *      None.
 *
 *
 *----------------------------------------------------------------------
 */

static void
btvMovXSBCmd(interp, clientData, report)
	Tcl_Interp     *interp;	/* If "report" is FALSE, string
				 * describing visible range gets
				 * stored in interp->result. */
	ClientData     *clientData;	/* Information about text widget. */
	int             report;	/* Non-zero means report info to
				 * scrollbar if it has changed. */
{
	btvWin      *btvWinPtr = (btvWin *) clientData;
	Tk_Window       tkwin = btvWinPtr->tkwin;
	int             result;
	char            buffer[200];
	double          first, last;

	first = 0.0;

	last = 1.0;

        if (!report) {
            sprintf(interp->result, "%g %g", first, last);
            return;
        }
        if ((first == btvWinPtr->xScrollFirst) && (last == btvWinPtr->xScrollLast)) {
            return;
        }
        btvWinPtr->xScrollFirst = first;
        btvWinPtr->xScrollLast = last;

	sprintf(buffer, " %g %g", first, last);

	result = Tcl_VarEval(interp, btvWinPtr->xScrollCmd, buffer, (char *)NULL);
	if (result != TCL_OK) {
		Tcl_AddErrorInfo(interp,
		     "\n    (vertical scrolling command executed by text)");
		Tcl_BackgroundError(interp);
	}
}

/*
 *--------------------------------------------------------------
 *
 * btvWinCmd --
 *
 *      This procedure is invoked to process the "btvWin" Tcl
 *      command.  It creates a new "btvWin" widget.
 *
 * Results:
 *      A standard Tcl result.
 *
 * Side effects:
 *      A new widget is created and configured.
 *
 *--------------------------------------------------------------
 */

int
btvWinCmd(clientData, interp, argc, argv)
	ClientData      clientData;	/* Main window associated with
					 * interpreter. */
	Tcl_Interp     *interp;	/* Current interpreter. */
	int             argc;	/* Number of arguments. */
	char          **argv;	/* Argument strings. */
{
	Tk_Window       main = (Tk_Window) clientData;
	btvWin      	*btvWinPtr;
	Tk_Window       tkwin;
	int		i,type;
	char		font_description[100];
	FILE		*fin;

	if (argc < 2) {
		Tcl_AppendResult(interp, "wrong # args: should be \"",
			    argv[0], " pathName ?options?\"", (char *)NULL);
		return TCL_ERROR;
	}
	tkwin = Tk_CreateWindowFromPath(interp, main, argv[1], (char *)NULL);
	if (tkwin == NULL) {
		return TCL_ERROR;
	}
	Tk_SetClass(tkwin, "btvWin");

	/*
	 * Allocate and initialize the widget record.
	 */

	btvWinPtr = (btvWin *) calloc(1,sizeof(btvWin));
	btvWinPtr->tkwin 				= tkwin;
	btvWinPtr->display 				= Tk_Display(tkwin);
	btvWinPtr->interp 				= interp;
	btvWinPtr->color[HIGHLIGHT] 			= BTV_Red;
	btvWinPtr->color[BACKGROUND] 			= BTV_White;
	btvWinPtr->color[GRID] 				= BTV_Black;
	btvWinPtr->color[FETCH] 			= BTV_Green;
	btvWinPtr->color[LOCK_RD] 			= BTV_Black;
	btvWinPtr->color[LOCK_WR] 			= BTV_Black;
	btvWinPtr->color[MEM_RD] 			= BTV_Blue;
	btvWinPtr->color[MEM_WR] 			= BTV_Red;
	btvWinPtr->color[IO_RD] 			= BTV_Yellow;
	btvWinPtr->color[IO_WR] 			= BTV_Magenta;
	btvWinPtr->color[INT_ACK] 			= BTV_Black;
	btvWinPtr->color[BOFF] 				= BTV_Brown;
	btvWinPtr->color[UNKNOWN] 			= BTV_Black;
	btvWinPtr->geometry.x_axis_scale		= 10000;
	btvWinPtr->geometry.y_axis_scale		= 5;
	btvWinPtr->yScrollCmd				= NULL;
	btvWinPtr->xScrollCmd				= NULL;
	btvWinPtr->geometry.origin_x			= 0;
	btvWinPtr->geometry.origin_y			= 0;
	btvWinPtr->display_modes[MAIN_MODE]		= HISTOGRAM_MODE;
	btvWinPtr->display_modes[SHOW_LEGENDS] 		= TRUE;
	btvWinPtr->display_modes[SHOW_LABELS] 		= TRUE;
	btvWinPtr->display_modes[SHOW_THREAD_NUMBER]	= ALL_THREADS;
	btvWinPtr->display_modes[HASH_MODE]		= FLOAT;
	btvWinPtr->display_modes[COLOR_SCHEME]		= COLOR_BY_ACCESS_TYPE;
	btvWinPtr->display_modes[SHOW_BW_LIMIT]		= FALSE;
	btvWinPtr->geometry.offset_x			= 60;
	btvWinPtr->geometry.offset_y			= 60;
	btvWinPtr->trace.trace_type			= NO_TRACE;
	
	for(i =0 ;i< MAX_SEGMENT_COUNT;i++){
		btvWinPtr->trace.segment[i] 		= NULL;
	}
        sprintf(btvWinPtr->font_name,"Times");
        btvWinPtr->font_size 				= 12;
	sprintf(btvWinPtr->font_attribute,"bold");
	sprintf(font_description,"%s %d %s",
		btvWinPtr->font_name,
		btvWinPtr->font_size,
		btvWinPtr->font_attribute);
	btvWinPtr->font = Tk_GetFont(interp, tkwin, font_description);

	btvWinPtr->widgetCmd = Tcl_CreateCommand(interp,
		       Tk_PathName(btvWinPtr->tkwin), btvWinWidgetCmd,
			(ClientData) btvWinPtr, btvWinCmdDeletedProc);

	btvWinPtr->updatePending = 0;

	Tk_CreateEventHandler(btvWinPtr->tkwin, ExposureMask | StructureNotifyMask,
			      btvWinEventProc, (ClientData) btvWinPtr);

	if (btvWinConfigure(interp, btvWinPtr, argc - 2, argv + 2, 0) != TCL_OK) {
		Tk_DestroyWindow(btvWinPtr->tkwin);
		return TCL_ERROR;
	}

	interp->result = Tk_PathName(btvWinPtr->tkwin);
	return TCL_OK;
}

/*
 *--------------------------------------------------------------
 *
 * btvWinWidgetCmd --
 *
 *      This procedure is invoked to process the Tcl command
 *      that corresponds to a widget managed by this module.
 *      See the user documentation for details on what it does.
 *
 * Results:
 *      A standard Tcl result.
 *
 * Side effects:
 *      See the user documentation.
 *
 *--------------------------------------------------------------
 */

static int
btvWinWidgetCmd(clientData, interp, argc, argv)
	ClientData      clientData;	/* Information about btvWin widget. */
	Tcl_Interp     *interp;	/* Current interpreter. */
	int             argc;	/* Number of arguments. */
	char          **argv;	/* Argument strings. */

{
	btvWin      *btvWinPtr = (btvWin *) clientData;
	Tk_Window       tkwin = btvWinPtr->tkwin;
	int             i,result = TCL_OK;
	int 		temp_x;
	int		temp_y;
	size_t         	length;
	char            c;
	double		delta_x,delta_y;
	double		timestamp;
	int		start_no,end_no;
	int		old_x_axis_scale;

	if (argc < 2) {
		Tcl_AppendResult(interp, "wrong # args: should be \"",
			  argv[0], " option ?arg arg ...?\"", (char *)NULL);
		return TCL_ERROR;
	}
	Tcl_Preserve((ClientData) btvWinPtr);
	c = argv[1][0];
	length = strlen(argv[1]);

	if ((c == 'c') && (strncmp(argv[1], "configure", length) == 0) && (length >= 2)) {
		if (argc == 2) {
			result = Tk_ConfigureInfo(	interp, 
							tkwin, 
							configSpecs, 
							(char *)btvWinPtr, 
							(char *)NULL, 
							0);
		} else if (argc == 3) {
			result = Tk_ConfigureInfo(	interp, 
							tkwin, 
							configSpecs, 
							(char *)btvWinPtr, 
							argv[2], 
							0);
		} else {
			result = btvWinConfigure(	interp, 
								btvWinPtr, 
								argc - 2, 
								argv + 2,
						    		TK_CONFIG_ARGV_ONLY);
		}
	} else if((c == 't') && (strncmp(argv[1], "trace_read", length)== 0) && (argc == 4)) {
		FILE *fin;
		c = argv[2][0];
		length = strlen(argv[2]);
		if((c == 'k') && (strncmp(argv[2], "k6", length)== 0)) {
			if((fin = fopen(argv[3],"r")) == NULL){
				printf("File open failed\n");
			} else {
				if(read_k6_trace(fin,&(btvWinPtr->trace)) != BTV_OK){
					printf("file read failed!\n");
				} else {
					printf("size[%d] starttime[%lf] endtime[%lf]\n",
						btvWinPtr->trace.event_count,
						btvWinPtr->trace.start_time,
						btvWinPtr->trace.end_time);
					btvWinPtr->trace.trace_type	= K6_TRACE;
					btvWinPtr->geometry.first_time 	= btvWinPtr->trace.start_time;
				}
				fclose(fin);
			}
		} else if((c == 'm') && (strncmp(argv[2], "mase", length)== 0)) {
			if((fin = fopen(argv[3],"r")) == NULL){
				printf("File open failed\n");
			} else {
				if(read_mase_trace(fin,&(btvWinPtr->trace)) != BTV_OK){
					printf("file read failed!\n");
				} else {
					printf("size[%d] starttime[%lf] endtime[%lf]\n",
						btvWinPtr->trace.event_count,
						btvWinPtr->trace.start_time,
						btvWinPtr->trace.end_time);
					btvWinPtr->trace.trace_type	= MASE_TRACE;
					btvWinPtr->geometry.first_time 	= btvWinPtr->trace.start_time;
				}
				fclose(fin);
			}
		} else if((c == 't') && (strncmp(argv[2], "tm", length)== 0)) {
			if((fin = fopen(argv[3],"r")) == NULL){
				printf("File open failed\n");
			} else {
				if(read_tm_trace(fin,&(btvWinPtr->trace)) != BTV_OK){
					printf("file read failed!\n");
				} else {
					printf("size[%d] starttime[%lf] endtime[%lf]\n",
						btvWinPtr->trace.event_count,
						btvWinPtr->trace.start_time,
						btvWinPtr->trace.end_time);
					btvWinPtr->trace.trace_type	= TM_TRACE;
					btvWinPtr->geometry.first_time 	= btvWinPtr->trace.start_time;
				}
				fclose(fin);
			}
		} else if((c == 's') && (strncmp(argv[2], "ss", length)== 0)) {
			if((fin = fopen(argv[3],"r")) == NULL){
				printf("File open failed\n");
			} else {
				if(read_ss_trace(fin,&(btvWinPtr->trace)) != BTV_OK){
					printf("file read failed!\n");
				} else {
					printf("size[%d] starttime[%lf] endtime[%lf]\n",
						btvWinPtr->trace.event_count,
						btvWinPtr->trace.start_time,
						btvWinPtr->trace.end_time);
					btvWinPtr->trace.trace_type	= SS_TRACE;
					btvWinPtr->geometry.first_time 	= 0.0;
				}
				fclose(fin);
			}
		}
		btvWinPtr->updatePending = 0;
	} else if((c == 's') && (strncmp(argv[1], "set", length)== 0)) {
		c = argv[2][0];
		length = strlen(argv[2]);
		if((c == 'x') && (strncmp(argv[2], "x_axis_scale", length)== 0) && (argc == 4)) {
			old_x_axis_scale = btvWinPtr->geometry.x_axis_scale;
			sscanf(argv[3],"%d", &(btvWinPtr->geometry.x_axis_scale));
			btvWinPtr->geometry.origin_x = (int)((btvWinPtr->geometry.origin_x)  
					* (double)old_x_axis_scale / (double)btvWinPtr->geometry.x_axis_scale) ;
		} else if ((c == 'y') && (strncmp(argv[2], "y_axis_scale", length)== 0) && (argc == 4)) { 
			sscanf(argv[3],"%d", &(btvWinPtr->geometry.y_axis_scale));
		} else if ((c == 'B') && (strncmp(argv[2], "BG_color", length)== 0) && (argc == 4)) { 
			sscanf(argv[3],"%d", &(btvWinPtr->color[BACKGROUND]));
		} else if ((c == 'I') && (strncmp(argv[2], "IO_RD_color", length)== 0) && (argc == 4)) { 
			sscanf(argv[3],"%d", &(btvWinPtr->color[IO_RD]));
		} else if ((c == 'I') && (strncmp(argv[2], "IO_WR_color", length)== 0) && (argc == 4)) { 
			sscanf(argv[3],"%d", &(btvWinPtr->color[IO_WR]));
		} else if ((c == 'M') && (strncmp(argv[2], "MEM_RD_color", length)== 0) && (argc == 4)) { 
			sscanf(argv[3],"%d", &(btvWinPtr->color[MEM_RD]));
		} else if ((c == 'M') && (strncmp(argv[2], "MEM_WR_color", length)== 0) && (argc == 4)) { 
			sscanf(argv[3],"%d", &(btvWinPtr->color[MEM_WR]));
		} else if ((c == 'B') && (strncmp(argv[2], "BOFF_color", length)== 0) && (argc == 4)) { 
			sscanf(argv[3],"%d", &(btvWinPtr->color[BOFF]));
		} else if ((c == 'U') && (strncmp(argv[2], "UNKNOWN_color", length)== 0) && (argc == 4)) { 
			sscanf(argv[3],"%d", &(btvWinPtr->color[UNKNOWN]));
		} else if ((c == 'F') && (strncmp(argv[2], "FETCH_color", length)== 0) && (argc == 4)) { 
			sscanf(argv[3],"%d", &(btvWinPtr->color[FETCH]));
		} else if ((c == 'G') && (strncmp(argv[2], "GRID_color", length)== 0) && (argc == 4)) { 
			sscanf(argv[3],"%d", &(btvWinPtr->color[GRID]));
		} else if ((c == 's') && (strncmp(argv[2], "show_thread", length)== 0) && (argc == 4)) { 
			sscanf(argv[3],"%d", &(btvWinPtr->display_modes[SHOW_THREAD_NUMBER]));
		} else if ((c == 'h') && (strncmp(argv[2], "hash_mode", length)== 0) && (argc == 4)) { 
			sscanf(argv[3],"%d", &(btvWinPtr->display_modes[HASH_MODE]));
		} else if ((c == 'c') && (strncmp(argv[2], "color_scheme", length)== 0) && (argc == 4)) { 
			sscanf(argv[3],"%d", &(btvWinPtr->display_modes[COLOR_SCHEME]));
		} else if ((c == 's') && (strncmp(argv[2], "showlimit", length)== 0) && (argc == 4)) { 
			sscanf(argv[3],"%d", &(btvWinPtr->display_modes[SHOW_BW_LIMIT]));
		} else if ((c == 's') && (strncmp(argv[2], "showlegends", length)== 0) && (argc == 4)) { 
			sscanf(argv[3],"%d", &(btvWinPtr->display_modes[SHOW_LEGENDS]));
		} else if ((c == 's') && (strncmp(argv[2], "showlabels", length)== 0) && (argc == 4)) { 
			sscanf(argv[3],"%d", &(btvWinPtr->display_modes[SHOW_LABELS]));
		}
		btvWinPtr->updatePending = 0;
	} else if((c == 's') && (strncmp(argv[1], "scan", length)== 0) && (argc == 5)) {
		c = argv[2][0];
		length = strlen(argv[2]);
		if((c == 'm') && (strncmp(argv[2], "mark", length)== 0))  {
			sscanf(argv[3],"%d",&(btvWinPtr->geometry.mark_x));
			sscanf(argv[4],"%d",&(btvWinPtr->geometry.mark_y));
		} else if((c == 'm') && (strncmp(argv[2], "move", length)== 0)) {
			/* x and y have opposite signs in our thinking, due to the fact that
			   in the Xwindows system, origin is at the upper left hand corner of a 
			   screen, while we want it on the bottom left hand corner */
			sscanf(argv[3],"%d",&(temp_x));
			sscanf(argv[4],"%d",&(temp_y));
			btvWinPtr->geometry.origin_x += temp_x - btvWinPtr->geometry.mark_x ; 
			btvWinPtr->geometry.mark_x = temp_x;
			btvWinPtr->geometry.origin_x = MIN(0,btvWinPtr->geometry.origin_x);
			/*
			btvWinPtr->geometry.origin_y += btvWinPtr->geometry.mark_y - temp_y ; 
			btvWinPtr->geometry.mark_y = temp_y;
			btvWinPtr->geometry.origin_y = MIN(0,btvWinPtr->geometry.origin_y);
			*/
		} else if((c == 's') && (strncmp(argv[2], "stat_start", length)== 0)) {
			sscanf(argv[3],"%d",&(temp_x));
			timestamp = btv_find_timestamp(&(btvWinPtr->geometry), 
						btvWinPtr->display_modes, 
						&(btvWinPtr->trace),
						temp_x);
			start_no =  btv_find_event_no(&(btvWinPtr->geometry), 
						&(btvWinPtr->trace),
						timestamp);
			btvWinPtr->stats.start_no = start_no;
		} else if((c == 's') && (strncmp(argv[2], "stat_end", length)== 0)) {
			sscanf(argv[3],"%d",&(temp_x));
			timestamp = btv_find_timestamp(&(btvWinPtr->geometry), 
						btvWinPtr->display_modes, 
						&(btvWinPtr->trace),
						temp_x);
			end_no =  btv_find_event_no(&(btvWinPtr->geometry), 
						&(btvWinPtr->trace),
						timestamp);
			btvWinPtr->stats.end_no = end_no;
			btv_gather_basic_stats(&(btvWinPtr->stats), &(btvWinPtr->trace));
			btv_print_basic_stats(&(btvWinPtr->stats));
		}
		btvWinPtr->updatePending = 0;
	} else if((c == 'g') && (strncmp(argv[1], "get", length)== 0)) {
		c = argv[2][0];
		length = strlen(argv[2]);
		if((c == 'B') && (strncmp(argv[2], "BG_color", length)== 0))  {
			sprintf(interp->result, "%s", color_map[btvWinPtr->color[BACKGROUND]]);
		} else if ((c == 'I') && (strncmp(argv[2], "IO_RD_color", length)== 0)){ 
			sprintf(interp->result, "%s", color_map[btvWinPtr->color[BACKGROUND]]);
		} else if ((c == 'I') && (strncmp(argv[2], "IO_WR_color", length)== 0)){
			sprintf(interp->result, "%s", color_map[btvWinPtr->color[BACKGROUND]]);
		} else if ((c == 'M') && (strncmp(argv[2], "MEM_RD_color", length)== 0)){ 
			sprintf(interp->result, "%s", color_map[btvWinPtr->color[BACKGROUND]]);
		} else if ((c == 'M') && (strncmp(argv[2], "MEM_WR_color", length)== 0)){ 
			sprintf(interp->result, "%s", color_map[btvWinPtr->color[BACKGROUND]]);
		} else if ((c == 'B') && (strncmp(argv[2], "BOFF_color", length)== 0)){ 
			sprintf(interp->result, "%s", color_map[btvWinPtr->color[BACKGROUND]]);
		} else if ((c == 'U') && (strncmp(argv[2], "UNKNOWN_color", length)== 0)){ 
			sprintf(interp->result, "%s", color_map[btvWinPtr->color[BACKGROUND]]);
		} else if ((c == 'F') && (strncmp(argv[2], "FETCH_color", length)== 0)){
			sprintf(interp->result, "%s", color_map[btvWinPtr->color[BACKGROUND]]);
		} else if ((c == 't') && (strncmp(argv[2], "timestamp", length)== 0)){
			sscanf(argv[3],"%d",&(temp_x));
			timestamp = btv_find_timestamp(&(btvWinPtr->geometry), 
						btvWinPtr->display_modes, 
						&(btvWinPtr->trace),
						temp_x);
			start_no =  btv_find_event_no(&(btvWinPtr->geometry), 
						&(btvWinPtr->trace),
						timestamp);
			printf("event_no[%d] timestamp[%lf]\n",start_no,timestamp);
		}
	} else if((c== 'z') && (strncmp(argv[1], "zoom", length)== 0) && (argc == 3)) {
		c = argv[2][0];
		length = strlen(argv[2]);
		if((c == 'i') && (strncmp(argv[2], "in", length)== 0))  {
			old_x_axis_scale = btvWinPtr->geometry.x_axis_scale;
			btvWinPtr->geometry.x_axis_scale = MAX(1,old_x_axis_scale/2);
			btvWinPtr->geometry.origin_x = (int)((btvWinPtr->geometry.origin_x)  
					* (double)old_x_axis_scale / (double)btvWinPtr->geometry.x_axis_scale) ;
			if(btvWinPtr->geometry.x_axis_scale != old_x_axis_scale){
				btvWinPtr->geometry.y_axis_scale = MAX(1,btvWinPtr->geometry.y_axis_scale/2);
			}
		} else if ((c == 'o') && (strncmp(argv[2], "out", length)== 0)){ 
			old_x_axis_scale = btvWinPtr->geometry.x_axis_scale;
			btvWinPtr->geometry.x_axis_scale *= 2;
			btvWinPtr->geometry.origin_x = (int)((btvWinPtr->geometry.origin_x)  
					* (double)old_x_axis_scale / (double)btvWinPtr->geometry.x_axis_scale) ;
			btvWinPtr->geometry.y_axis_scale *= 2;
		}
	}
	if (!btvWinPtr->updatePending) {
		Tcl_DoWhenIdle(btvWinDisplay, (ClientData) btvWinPtr);
		btvWinPtr->updatePending = 1;
	}
	Tcl_Release((ClientData) btvWinPtr);
	return result;

      	error:
		Tcl_AppendResult(	interp, 
					"bad option \"", 
					argv[1], 
					"\": must be configure, or IDK", 
					(char *)NULL);
		Tcl_Release((ClientData) btvWinPtr);
		return TCL_ERROR;
}


/*
 *----------------------------------------------------------------------
 *
 * btvWinConfigure --
 *
 *      This procedure is called to process an argv/argc list in
 *      conjunction with the Tk option database to configure (or
 *      reconfigure) a btvWin widget.
 *
 * Results:
 *      The return value is a standard Tcl result.  If TCL_ERROR is
 *      returned, then interp->result contains an error message.
 *
 * Side effects:
 *      Configuration information, such as colors, border width,
 *      etc. get set for btvWinPtr;  old resources get freed,
 *      if there were any.
 *
 *----------------------------------------------------------------------
 */

static int
btvWinConfigure(interp, btvWinPtr, argc, argv, flags)
	Tcl_Interp     	*interp;	/* Used for error reporting. */
	btvWin      	*btvWinPtr;	/* Information about widget. */
	int             argc;	/* Number of valid entries in argv. */
	char          	**argv;	/* Arguments. */
	int             flags;	/* Flags to pass to
				 * Tk_ConfigureWidget. */
{
	XGCValues       gcValues;
	int		i;
        XColor          *xcolor; 

	if (Tk_ConfigureWidget(interp, btvWinPtr->tkwin, configSpecs,
		       argc, argv, (char *)btvWinPtr, flags) != TCL_OK) {
		return TCL_ERROR;
	}

        gcValues.function = GXcopy;
        gcValues.graphics_exposures = False;

	for(i=0;i<BTV_COLOR_PALLET_SIZE;i++){
		xcolor = Tk_GetColor(interp,btvWinPtr->tkwin,Tk_GetUid(color_map[i]));	
		gcValues.foreground = xcolor->pixel;
		btvWinPtr->Border_array[i] = 
			Tk_Get3DBorder(interp, btvWinPtr->tkwin, Tk_GetUid(color_map[i]));
		btvWinPtr->GC_array[i] = Tk_GetGC(btvWinPtr->tkwin,
				GCFunction | GCGraphicsExposures | GCForeground, &gcValues);
	}

	for(i=0;i<BTV_GRAY_PALLET_SIZE;i++){
		xcolor = Tk_GetColor(interp,btvWinPtr->tkwin,Tk_GetUid(gray_map[i]));	
		gcValues.foreground = xcolor->pixel;
		btvWinPtr->GRAY_Border_array[i] = 
			Tk_Get3DBorder(interp, btvWinPtr->tkwin, Tk_GetUid(gray_map[i]));
		btvWinPtr->GRAY_GC_array[i] = Tk_GetGC(btvWinPtr->tkwin,
				GCFunction | GCGraphicsExposures | GCForeground, &gcValues);
	}

	/*
	 * Register the desired geometry for the window.  Then arrange for
	 * the window to be displayed.
	 */

	Tk_GeometryRequest(btvWinPtr->tkwin, 800,400);
	Tk_SetInternalBorder(btvWinPtr->tkwin, btvWinPtr->borderWidth);
	if (!btvWinPtr->updatePending) {
		Tcl_DoWhenIdle(btvWinDisplay, (ClientData) btvWinPtr);
		btvWinPtr->updatePending = 1;
	}
	return TCL_OK;
}

/*
 *--------------------------------------------------------------
 *
 * btvWinEventProc --
 *
 *      This procedure is invoked by the Tk dispatcher for various
 *      events on btvWins.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      When the window gets deleted, internal structures get
 *      cleaned up.  When it gets exposed, it is redisplayed.
 *
 *--------------------------------------------------------------
 */

static void
btvWinEventProc(clientData, eventPtr)
	ClientData      clientData;	/* Information about window. */
	XEvent         *eventPtr;	/* Information about event. */
{
	btvWin      *btvWinPtr = (btvWin *) clientData;

	if (eventPtr->type == Expose) {
		if (!btvWinPtr->updatePending) {
			Tcl_DoWhenIdle(btvWinDisplay, (ClientData) btvWinPtr);
			btvWinPtr->updatePending = 1;
		}
	} else if (eventPtr->type == ConfigureNotify) {
		if (!btvWinPtr->updatePending) {
			Tcl_DoWhenIdle(btvWinDisplay, (ClientData) btvWinPtr);
			btvWinPtr->updatePending = 1;
		}
	} else if (eventPtr->type == DestroyNotify) {
		if (btvWinPtr->tkwin != NULL) {
			btvWinPtr->tkwin = NULL;
			Tcl_DeleteCommand(btvWinPtr->interp,
				    Tcl_GetCommandName(btvWinPtr->interp,
						  btvWinPtr->widgetCmd));
		}
		if (btvWinPtr->updatePending) {
			Tcl_CancelIdleCall(btvWinDisplay, (ClientData) btvWinPtr);
		} 
		if(btvWinPtr->font != NULL){
			Tk_FreeFont(btvWinPtr->font);
		}
		Tcl_EventuallyFree((ClientData) btvWinPtr, btvWinDestroy);
	}
}

/*
 *----------------------------------------------------------------------
 *
 * btvWinCmdDeletedProc --
 *
 *      This procedure is invoked when a widget command is deleted.  If
 *      the widget isn't already in the process of being destroyed,
 *      this command destroys it.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      The widget is destroyed.
 *
 *----------------------------------------------------------------------
 */

static void
btvWinCmdDeletedProc(clientData)
	ClientData      clientData;	/* Pointer to widget record for widget. */
{
	btvWin		*btvWinPtr 	= (btvWin *) clientData;
	Tk_Window	tkwin 		= btvWinPtr->tkwin;

	/*
	 * This procedure could be invoked either because the window was
	 * destroyed and the command was then deleted (in which case tkwin
	 * is NULL) or because the command was deleted, and then this procedure
	 * destroys the widget.
	 */

	if (tkwin != NULL) {
		btvWinPtr->tkwin = NULL;
		Tk_DestroyWindow(tkwin);
	}
}

/*
 *--------------------------------------------------------------
 *
 * btvWinDisplay --
 *
 *      This procedure redraws the contents of a btvWin window.
 *      It is invoked as a do-when-idle handler, so it only runs
 *      when there's nothing else for the application to do.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Information appears on the screen.
 *
 *--------------------------------------------------------------
 */

static void
btvWinDisplay(clientData)
	ClientData      clientData;	/* Information about window. */
{
	btvWin      	*btvWinPtr = (btvWin *) clientData;
	Tk_Window       tkwin = btvWinPtr->tkwin;
	Pixmap          pm = None;
	Drawable        d;
	GC              gc;
	int		win_width,win_height;

	btvWinPtr->updatePending = 0;
	if (!Tk_IsMapped(tkwin)) {
		return;
	}

	win_width = Tk_Width(tkwin);
	win_height = Tk_Height(tkwin);

	if(win_height < 300 || win_width < 300 || (btvWinPtr->trace.trace_type == NO_TRACE)){
		return;
	}
	
	btvWinPtr->geometry.win_width  = win_width;
	btvWinPtr->geometry.win_height = win_height;

	/*
	 * Create a pixmap for double-buffering
	 */

	pm = XCreatePixmap(Tk_Display(tkwin), Tk_WindowId(tkwin),
		      (unsigned)Tk_Width(tkwin), (unsigned)Tk_Height(tkwin),
			   (unsigned)DefaultDepthOfScreen(Tk_Screen(tkwin)));
	d = pm;

	/*
	 * Redraw the widget's background and border.
	 */

	Tk_Fill3DRectangle(	tkwin, 
				d, 
				btvWinPtr->Border_array[btvWinPtr->color[BACKGROUND]],
				0, 
				0, 
				win_width,
				win_height,
	 		 	0,
				TK_RELIEF_SOLID);

	if(btvWinPtr->display_modes[SHOW_LABELS]){
		btv_draw_labels(tkwin,
				d,
				btvWinPtr->Border_array,
				btvWinPtr->GC_array,
				&(btvWinPtr->geometry),
				btvWinPtr->font,
				btvWinPtr->display_modes,
				btvWinPtr->color,
				&(btvWinPtr->trace));
	}

	if(btvWinPtr->display_modes[MAIN_MODE] == HISTOGRAM_MODE){
		btv_draw_histogram(tkwin,
				d,
				btvWinPtr->Border_array,
				btvWinPtr->GC_array,
				&(btvWinPtr->geometry),
				btvWinPtr->font,
				btvWinPtr->display_modes,
				btvWinPtr->color,
				&(btvWinPtr->trace));
	}

	if(btvWinPtr->display_modes[SHOW_LEGENDS]){
		btv_draw_legends(tkwin,
				d,
				btvWinPtr->Border_array,
				btvWinPtr->GC_array,
				&(btvWinPtr->geometry),
				btvWinPtr->font,
				btvWinPtr->display_modes,
				btvWinPtr->color,
				&(btvWinPtr->trace));
	}
	
	/*
	 *  we're done. Double buffer it.
 	 */

	gc = btvWinPtr->GC_array[btvWinPtr->color[BACKGROUND]];
	XCopyArea(Tk_Display(tkwin), pm, Tk_WindowId(tkwin), gc,
		0, 0, (unsigned)win_width, (unsigned)win_height,
		  0, 0);
	XFreePixmap(Tk_Display(tkwin), pm);
}

/*
 *----------------------------------------------------------------------
 *
 * btvWinDestroy --
 *
 *      This procedure is invoked by Tcl_EventuallyFree or Tcl_Release
 *      to clean up the internal structure of a btvWin at a safe time
 *      (when no-one is using it anymore).
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Everything associated with the btvWin is freed up.
 *
 *----------------------------------------------------------------------
 */

static void
btvWinDestroy(memPtr)
	char           *memPtr;	/* Info about btvWin widget. */
{
	btvWin      *btvWinPtr = (btvWin *) memPtr;

	Tk_FreeOptions(configSpecs, (char *)btvWinPtr, btvWinPtr->display, 0);
	ckfree((char *)btvWinPtr);
}

