/* $Id: traceRead.c,v 1.103 1999/12/01 20:39:47 davewang Exp $ */

#include "tcl.h"
#include "btv.h"

int traceReadCmd _ANSI_ARGS_((ClientData clientData, Tcl_Interp *, int, char **argv));

/*
 *--------------------------------------------------------------
 *
 * traceReadCmd --
 *
 *      This procedure is invoked to process the "traceRead" Tcl
 *      command.  It creates a new "traceRead" widget.
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
traceReadCmd(clientData, interp, argc, argv)
	ClientData      clientData;	/* Main window associated with
					 * interpreter. */
	Tcl_Interp     *interp;	/* Current interpreter. */
	int             argc;	/* Number of arguments. */
	char          **argv;	/* Argument strings. */
{
	int		i,type;
	FILE		*fin;
	TRACE		*trace;
	char 		c;
	key_t		length;

	if (argc < 2) {
		Tcl_AppendResult(interp, "wrong # args: should be \"",
			    argv[0], " pathName ?options?\"", (char *)NULL);
		return TCL_ERROR;
	} else if((c== 'o') && (strncmp(argv[1], "open", length)== 0) && (argc == 4)) {
		FILE *fin;
		c = argv[2][0];
		length = strlen(argv[2]);
		if((c == 'k') && (strncmp(argv[2], "k6", length)== 0)) {
			if((fin = fopen(argv[3],"r")) == NULL){
				printf("File open failed\n");
			} else {
				if(read_k6_trace(fin,trace) != BTV_OK){
					printf("file read failed!\n");
				} else {
					printf("size[%d] starttime[%lf] endtime[%lf]\n",
						trace->event_count,
						trace->start_time,
						trace->end_time);
					trace->trace_type     = K6_TRACE;
				}
				fclose(fin);
			}
		} else if((c == 'm') && (strncmp(argv[2], "mase", length)== 0)) {
			if((fin = fopen(argv[3],"r")) == NULL){
				printf("File open failed\n");
			} else {
				if(read_mase_trace(fin,trace) != BTV_OK){
					printf("file read failed!\n");
				} else {
					printf("size[%d] starttime[%lf] endtime[%lf]\n",
						trace->event_count,
						trace->start_time,
						trace->end_time);
					trace->trace_type     = MASE_TRACE;
				}
				fclose(fin);
			}
		}
	} else if((c== 'c') && (strncmp(argv[1], "close", length)== 0) && (argc == 3)) {
		sscanf(argv[2],"%x",trace);
	}
	return TCL_OK;
}

