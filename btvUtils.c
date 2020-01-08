#include "btv.h"

int btvUtilsCmd _ANSI_ARGS_((ClientData clientData, Tcl_Interp *,
					int, char **argv));
/*
 *--------------------------------------------------------------
 *
 * btvUtilsCmd --
 *
 *      This procedure is invoked to process the "btvUtils" Tcl
 *      command.  It creates a new "btvUtils" widget.
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
btvUtilsCmd(clientData, interp, argc, argv)
	ClientData      clientData;	/* Main window associated with
					 * interpreter. */
	Tcl_Interp     *interp;	/* Current interpreter. */
	int             argc;	/* Number of arguments. */
	char          **argv;	/* Argument strings. */
{
        size_t          length;
        char            c;
	int		color_id,resource_id;
	TRACE		*trace;

	length = strlen(argv[1]);
	c = argv[1][0];

	if (argc < 2) {
		Tcl_AppendResult(interp, "wrong # args: should be \"",
			    argv[0], " pathName ?options?\"", (char *)NULL);
		return TCL_ERROR;
	} else if((c== 'g') && (strncmp(argv[1], "get_color_name", length)== 0) && (argc == 3)) {
		sscanf(argv[2],"%d",&color_id);
 		sprintf(interp->result, "%s", color_map[color_id]);
	} else if((c== 'o') && (strncmp(argv[1], "open_trace", length)== 0) && (argc == 4)) {
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
	} else if((c== 'c') && (strncmp(argv[1], "close_trace", length)== 0) && (argc == 3)) {
		sscanf(argv[2],"%x",trace);
	}
	return TCL_OK;
}

