/* 
 * btvaInit.c 
 *
 * $Id: btvaInit.c,v 1.8 1999/12/28 05:43:31 davewang Exp $
 */

#include <tk.h>

/*
 * The following variable is a special hack that is needed in order for
 * Sun shared libraries to be used for Tcl.
 */

extern int      matherr();
int            *tclDummyMathPtr = (int *)matherr;

/*
 * Declaration for the various widget's class command procedure:
 */

extern int btvUtilsCmd _ANSI_ARGS_((ClientData clientData,
			      Tcl_Interp * interp, int argc, char *argv[]));

extern int btvWinCmd _ANSI_ARGS_((ClientData clientData,
			      Tcl_Interp * interp, int argc, char *argv[]));

/*
 *----------------------------------------------------------------------
 *
 * main --
 *
 *      This is the main program for the application.
 *
 * Results:
 *      None: Tk_Main never returns here, so this procedure never
 *      returns either.
 *
 * Side effects:
 *      Whatever the application does.
 *
 *----------------------------------------------------------------------
 */

int
main(argc, argv)
	int             argc;	/* Number of command-line arguments. */
	char          **argv;	/* Values of command-line arguments. */
{
	Tcl_FindExecutable(argv[0]);
	Tk_Main(argc, argv, Tcl_AppInit);
	return 0;		/* Needed only to prevent compiler warning. */
}
/*
 *----------------------------------------------------------------------
 *
 * Tcl_AppInit --
 *
 *      This procedure performs application-specific initialization.
 *      Most applications, especially those that incorporate additional
 *      packages, will have their own version of this procedure.
 *
 * Results:
 *      Returns a standard Tcl completion code, and leaves an error
 *      message in interp->result if an error occurs.
 *
 * Side effects:
 *      Depends on the startup script.
 *
 *----------------------------------------------------------------------
 */

int
Tcl_AppInit(interp)
	Tcl_Interp     *interp;	/* Interpreter for application. */
{
/*
	char DIRECTORY[256];
	DIRECTORY = "";
	interp = Tcl_CreateInterp();
	Tcl_SetVar(interp, "tcl_library",DIRECTORY, TCL_GLOBAL_ONLY);
*/
	if (Tcl_Init(interp) == TCL_ERROR) {
		return TCL_ERROR;
	}
	if (Tk_Init(interp) == TCL_ERROR) {
		return TCL_ERROR;
	}
	Tcl_StaticPackage(interp, "Tk", Tk_Init, (Tcl_PackageInitProc *) NULL);

        Tcl_CreateCommand(interp, "btvWin", btvWinCmd,
            (ClientData) Tk_MainWindow(interp), (Tcl_CmdDeleteProc *) NULL);

        Tcl_CreateCommand(interp, "btvUtils", btvUtilsCmd,
            (ClientData) Tk_MainWindow(interp), (Tcl_CmdDeleteProc *) NULL);

	return TCL_OK;
}
