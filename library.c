#include <tcl.h>
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

unsigned long num = 0;

void Pcre2_Inst_Del_Cmd(ClientData cdata)
{
    pcre2_code *re = (pcre2_code *)cdata;

    pcre2_code_free(re);
}

int Pcre2_Inst_Cmd(cdata, interp, objc, objv)
    ClientData cdata;        /* Record describing procedure to be
				  * interpreted. */
register Tcl_Interp *interp; /* Interpreter in which procedure was
				  * invoked. */
int objc;                    /* Count of number of arguments to this
				  * procedure. */
Tcl_Obj *CONST objv[];       /* Argument value objects. */
{
    if (objc != 3 && objc != 2)
    {
        Tcl_WrongNumArgs(interp, 1, objv, "string ?cmdprefix?");
        return TCL_ERROR;
    }
    Tcl_Obj *result;
    Tcl_DString dresult; 
    Tcl_DStringInit(&dresult);
    pcre2_code *re = (pcre2_code *)cdata;
    pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(re, NULL);
    char *subject;
    int subject_length;
    PCRE2_SIZE *ovector = NULL;
    Tcl_Obj *cmdPrefix = NULL;
    if (objc == 3)
    {
        cmdPrefix = objv[2];
        result = Tcl_NewStringObj(NULL, 0);
    }
    else
    {
        result = Tcl_NewListObj(0, NULL);
    }
    int offset;
    subject = Tcl_GetStringFromObj(objv[1], &subject_length);
    for (;;)
    {
        if (ovector != 0)
        {
            offset = ovector[1];
        }
        else
        {
            offset = 0;
        }
        int rc = pcre2_match(
            re,             /* the compiled pattern */
            subject,        /* the subject string */
            subject_length, /* the length of the subject */
            offset,         /* start at offset 0 in the subject */
            0,              /* default options */
            match_data,     /* block for storing the result */
            NULL);          /* use default match context */

        if (rc < 0)
        {
            switch (rc)
            {
                PCRE2_UCHAR buffer[256];
            case PCRE2_ERROR_NOMATCH:
                if (cmdPrefix==NULL) {
                    Tcl_SetObjResult(interp, result);
                } else {
                    Tcl_DStringResult(interp, &dresult);
                }
                pcre2_match_data_free(match_data);
                Tcl_DStringFree(&dresult);
                return TCL_OK;
                /*
    Handle other special cases if you like
    */
            default:
                pcre2_match_data_free(match_data);
                Tcl_DStringFree(&dresult);
                pcre2_get_error_message(rc, buffer, sizeof(buffer));
                Tcl_SetObjResult(interp, Tcl_ObjPrintf("PCRE2 matching error %s", buffer));
                return TCL_ERROR;
            }
        }
        ovector = pcre2_get_ovector_pointer(match_data);

        Tcl_Obj *matchObj = Tcl_NewListObj(0, NULL);
        for (int i = 0; i < rc; i++)
        {
            PCRE2_SPTR substring_start = subject + ovector[2 * i];
            PCRE2_SIZE substring_length = ovector[2 * i + 1] - ovector[2 * i];
            if (Tcl_ListObjAppendElement(interp, matchObj, Tcl_NewStringObj(substring_start, substring_length)) != TCL_OK)
            {
                pcre2_match_data_free(match_data);
                Tcl_DStringFree(&dresult);
                return TCL_ERROR;
            };

            //   printf("%2d: %.*s\n", i, (int)substring_length, (char *)substring_start);
        }
        if (cmdPrefix == NULL) {
            Tcl_ListObjAppendElement(interp, result, matchObj);
        } else {
            Tcl_Obj * cmd = Tcl_NewListObj(0,NULL);
            Tcl_IncrRefCount(cmd);
            if (Tcl_ListObjAppendList(interp, cmd, cmdPrefix) != TCL_OK) {
                pcre2_match_data_free(match_data);
                Tcl_DStringFree(&dresult);
                return TCL_ERROR;
            }
            Tcl_ListObjAppendList(interp, cmd, matchObj);
             if (Tcl_EvalObjEx(interp, cmd, TCL_EVAL_DIRECT)!=TCL_OK) {
                pcre2_match_data_free(match_data);
                Tcl_DStringFree(&dresult);
                return TCL_ERROR;
             }
             Tcl_DStringAppend(&dresult,  Tcl_GetStringResult(interp),-1) ;
             Tcl_DecrRefCount(cmd);
        }
    }
    pcre2_match_data_free(match_data);
    Tcl_DStringFree(&dresult);
    return TCL_ERROR;
}

int Pcre2_Cmd(clientData, interp, objc, objv)
    ClientData clientData;   /* Record describing procedure to be
				  * interpreted. */
register Tcl_Interp *interp; /* Interpreter in which procedure was
				  * invoked. */
int objc;                    /* Count of number of arguments to this
				  * procedure. */
Tcl_Obj *CONST objv[];       /* Argument value objects. */
{
    int errornumber;
    PCRE2_SIZE erroroffset;
    pcre2_code *re;
    Tcl_Obj *pattern;
    Tcl_Obj *cmdname;
    if (objc != 2)
    {
        Tcl_WrongNumArgs(interp, 1, objv, "pattern");
        return TCL_ERROR;
    }
    pattern = objv[1];
    re = pcre2_compile(
        Tcl_GetString(pattern), /* the pattern */
        PCRE2_ZERO_TERMINATED,  /* indicates pattern is zero-terminated */
        0,                      /* default options */
        &errornumber,           /* for error number */
        &erroroffset,           /* for error offset */
        NULL);                  /* use default compile context */

    /* Compilation failed: print the error message and exit. */

    if (re == NULL)
    {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
        Tcl_SetObjResult(interp, Tcl_ObjPrintf("PCRE2 compilation failed at offset %d: %s", (int)erroroffset,
                                               buffer));
        return TCL_ERROR;
    }
    cmdname = Tcl_ObjPrintf("pcre%lu", num);
    Tcl_CreateObjCommand(interp, Tcl_GetString(cmdname), Pcre2_Inst_Cmd, re, Pcre2_Inst_Del_Cmd);
    Tcl_SetObjResult(interp, cmdname);
    num++;
    return TCL_OK;
}

DLLEXPORT int Aoclib_Init(Tcl_Interp *interp)
{
    Tcl_InitStubs(interp, "8.5", 0);
    Tcl_CreateObjCommand(interp, "pcre2", Pcre2_Cmd,
                         (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    return Tcl_PkgProvide(interp, "aoc", "0.1");
    return TCL_OK;
}