// ################################## begin ######################################################
// these are modifications to the tcl/tk 9.0b1 code to allow for expressions of various kinds w/o
// needing to use the [expr {}] command

#define mycode // all the code is conditional on this def


#ifdef mycode

#define Separate_ll_function
#include "expression.c" // this is best done inside tclGet.c since it's very small so won't conflict

int evaluate_ll_expression(char *,int64_t *); // prototypes
int evaluate_d_expression(char*, double*);
bugprintf(const char *,...);

#endif // mycode

// ##########################################end##############################################



/*
*----------------------------------------------------------------------
*
 * GetWideForIndex --
 *
 *	This function produces a wide integer value corresponding to the
 *	index value held in *objPtr. The parsing supports all values
 *	recognized as any size of integer, and the syntaxes end[-+]$integer
 *	and $integer[-+]$integer. The argument endValue is used to give
 *	the meaning of the literal index value "end". Index arithmetic
 *	on arguments outside the wide integer range are only accepted
 *	when interp is a working interpreter, not NULL.
 *
 * Results:
 *	When parsing of *objPtr successfully recognizes an index value,
 *	TCL_OK is returned, and the wide integer value corresponding to
 *	the recognized index value is written to *widePtr. When parsing
 *	fails, TCL_ERROR is returned and error information is written to
 *	interp, if non-NULL.
 *
 * Side effects:
 *	The type of *objPtr may change.
 *
 *----------------------------------------------------------------------
*/

static int
GetWideForIndex(
Tcl_Interp *interp,         /* Interpreter to use for error reporting. If
			     * NULL, then no error message is left after
			     * errors. */
Tcl_Obj *objPtr,            /* Points to the value to be parsed */
Tcl_WideInt endValue,       /* The value to be stored at *widePtr if
			     * objPtr holds "end".
			     * NOTE: this value may be TCL_INDEX_NONE. */
Tcl_WideInt *widePtr)       /* Location filled in with a wide integer
			     * representing an index. */
{
    int numType;
    void *cd;
    int code = Tcl_GetNumberFromObj(NULL, objPtr, &cd, &numType);
    
#ifdef mycode
    //-----------------------------------------------------------------------------------
    int thestatus = 0xbeef;
    long long answerll = 0xface;
    if ( code != TCL_OK && interp != NULL && objPtr != NULL )  {
	thestatus = evaluate_ll_expression(TclGetString(objPtr),&answerll);
	if ( thestatus == 0 ) {
	    *widePtr = answerll;
	    return TCL_OK;
	}
    }
    //-----------------------------------------------------------------------------------
#endif
    
    if (code == TCL_OK) {
	
	if (numType == TCL_NUMBER_INT) {
	    /* objPtr holds an integer in the signed wide range */
	    *widePtr = *(Tcl_WideInt *)cd;
	    if ((*widePtr < 0)) {
		*widePtr = (endValue == -1) ? WIDE_MIN : -1;
	    }
	    return TCL_OK;
	}
	if (numType == TCL_NUMBER_BIG) {
	    /* objPtr holds an integer outside the signed wide range */
	    /* Truncate to the signed wide range. */
	    *widePtr = ((mp_isneg((mp_int *)cd)) ? WIDE_MIN : WIDE_MAX);
	    return TCL_OK;
	}
    }
    /* objPtr does not hold a number, check the end+/- format... */
    return GetEndOffsetFromObj(interp, objPtr, endValue, widePtr);
}


/*
*----------------------------------------------------------------------
*
 * GetEndOffsetFromObj --
 *
 *	Look for a string of the form "end[+-]offset" or "offset[+-]offset" and
 *	convert it to an internal representation.
 *
 *	The internal representation (wideValue) uses the following encoding:
 *
 *	WIDE_MIN:   Index value TCL_INDEX_NONE (or -1)
 *	WIDE_MIN+1: Index value n, for any n < -1  (usually same effect as -1)
 *	-$n:        Index "end-[expr {$n-1}]"
 *	-2:         Index "end-1"
 *	-1:         Index "end"
 *	0:          Index "0"
 *	WIDE_MAX-1: Index "end+n", for any n > 1. Distinguish from end+1 for
 *                  commands like lset.
 *	WIDE_MAX:   Index "end+1"
 *
 * Results:
 *	Tcl return code.
 *
 * Side effects:
 *	May store a Tcl_ObjType.
 *
*----------------------------------------------------------------------
*/

static int
GetEndOffsetFromObj(
Tcl_Interp *interp,
Tcl_Obj *objPtr,            /* Pointer to the object to parse */
Tcl_WideInt endValue,       /* The value to be stored at "widePtr" if
			     * "objPtr" holds "end". */
Tcl_WideInt *widePtr)       /* Location filled in with an integer
			     * representing an index. */
{
    Tcl_ObjInternalRep *irPtr;
    Tcl_WideInt offset = -1;    /* Offset in the "end-offset" expression - 1 */
    void *cd;
    
    while ((irPtr = TclFetchInternalRep(objPtr, &endOffsetType)) == NULL) {
	Tcl_ObjInternalRep ir;
	Tcl_Size length;
	const char *bytes = Tcl_GetStringFromObj(objPtr, &length);
	
	if (*bytes != 'e') {
	    int numType;
	    const char *opPtr;
	    int t1 = 0, t2 = 0;
	    
	    /* Value doesn't start with "e" */
	    
	    /* If we reach here, the string rep of objPtr exists. */
	    
	    /*
	     * The valid index syntax does not include any value that is
	     * a list of more than one element. This is necessary so that
	     * lists of index values can be reliably distinguished from any
	     * single index value.
	     */
	    
	    /*
	    * Quick scan to see if multi-value list is even possible.
	    * This relies on TclGetString() returning a NUL-terminated string.
	    */
	    if ((TclMaxListLength(bytes, TCL_INDEX_NONE, NULL) > 1)
	    
	    /* If it's possible, do the full list parse. */
		&& (TCL_OK == TclListObjLengthM(NULL, objPtr, &length))
		&& (length > 1)) {
		goto parseError;
	    }
	    
	    /* Passed the list screen, so parse for index arithmetic expression */
	    if (TCL_OK == TclParseNumber(NULL, objPtr, NULL, NULL, TCL_INDEX_NONE, &opPtr,
	    TCL_PARSE_INTEGER_ONLY)) {
		Tcl_WideInt w1=0, w2=0;
		
		/* value starts with valid integer... */
		
		if ((*opPtr == '-') || (*opPtr == '+')) {
		    /* ... value continues with [-+] ... */
		    
		    /* Save first integer as wide if possible */
		    Tcl_GetNumberFromObj(NULL, objPtr, &cd, &t1);
		    if (t1 == TCL_NUMBER_INT) {
			w1 = (*(Tcl_WideInt *)cd);
		    }
		    
		    if (TCL_OK == TclParseNumber(NULL, objPtr, NULL, opPtr + 1,
		    TCL_INDEX_NONE, NULL, TCL_PARSE_INTEGER_ONLY)) {
			/* ... value concludes with second valid integer */
			
			/* Save second integer as wide if possible */
			Tcl_GetNumberFromObj(NULL, objPtr, &cd, &t2);
			if (t2 == TCL_NUMBER_INT) {
			    w2 = (*(Tcl_WideInt *)cd);
			}
		    }
		}
		/* Clear invalid internalreps left by TclParseNumber */
		TclFreeInternalRep(objPtr);
		
		if (t1 && t2) {
		    /* We have both integer values */
		    if ((t1 == TCL_NUMBER_INT) && (t2 == TCL_NUMBER_INT)) {
			/* Both are wide, do wide-integer math */
			if (*opPtr == '-') {
			    if (w2 == WIDE_MIN) {
				goto extreme;
			    }
			    w2 = -w2;
			}
			
			if ((w1 ^ w2) < 0) {
			    /* Different signs, sum cannot overflow */
			    offset = w1 + w2;
			} else if (w1 >= 0) {
			    if (w1 < WIDE_MAX - w2) {
				offset = w1 + w2;
			    } else {
				offset = WIDE_MAX;
			    }
			} else {
			    if (w1 > WIDE_MIN - w2) {
				offset = w1 + w2;
			    } else {
				offset = WIDE_MIN;
			    }
			}
		    } else {
			/*
			* At least one is big, do bignum math. Little reason to
			* value performance here. Re-use code.  Parse has verified
			* objPtr is an expression. Compute it.
			*/
			
			Tcl_Obj *sum;
			
			extreme:
			if (interp) {
			    Tcl_ExprObj(interp, objPtr, &sum);
			} else {
			    Tcl_Interp *compute = Tcl_CreateInterp();
			    Tcl_ExprObj(compute, objPtr, &sum);
			    Tcl_DeleteInterp(compute);
			}
			Tcl_GetNumberFromObj(NULL, sum, &cd, &numType);
			
			if (numType == TCL_NUMBER_INT) {
			    /* sum holds an integer in the signed wide range */
			    offset = *(Tcl_WideInt *)cd;
			} else {
			    /* sum holds an integer outside the signed wide range */
			    /* Truncate to the signed wide range. */
			    if (mp_isneg((mp_int *)cd)) {
				offset = WIDE_MIN;
			    } else {
				offset = WIDE_MAX;
			    }
			}
			Tcl_DecrRefCount(sum);
		    }
		    if (offset < 0) {
			offset = (offset == -1) ? WIDE_MIN : WIDE_MIN+1;
		    }
		    goto parseOK;
		}
	    }
	    goto parseError;
	}
	
	if ((length < 3) || (length == 4) || (strncmp(bytes, "end", 3) != 0)) {
	    /* Doesn't start with "end" */
	    goto parseError;
	}
	
	if (length > 4) {
	    int t;
	    
	    /* Parse for the "end-..." or "end+..." formats */
	    
	    if ((bytes[3] != '-') && (bytes[3] != '+')) {
		/* No operator where we need one */
		goto parseError;
	    }
	    if (TclIsSpaceProc(bytes[4])) {
		/* Space after + or - not permitted. */
		goto parseError;
	    }
	    
	    
	    
#ifdef mycode
	    //------------------------------------------------------------------------------
	    int thestatus = 0;
	    Tcl_WideInt answerll;
	    
	    thestatus = evaluate_ll_expression(bytes+4,&answerll);
	    if ( thestatus == 0 ) {
		offset = answerll;
		if (bytes[3] == '-') {
		    offset = (offset == WIDE_MIN) ? WIDE_MAX : -offset;
		}
		if (offset == 1) {
		    offset = WIDE_MAX; /* "end+1" */
		} else if (offset > 1) {
		    offset = WIDE_MAX - 1; /* "end+n", out of range */
		} else if (offset != WIDE_MIN) {
		    offset--;
		}
		ir.wideValue = offset;
		Tcl_StoreInternalRep(objPtr, &endOffsetType, &ir);
		goto tryit;
	    }
	    //------------------------------------------------------------------------------
#endif
	    
	    
	    /* Parse the integer offset */
	    if (TCL_OK != TclParseNumber(NULL, objPtr, NULL,
			bytes+4, length-4, NULL, TCL_PARSE_INTEGER_ONLY)) {
		/* Not a recognized integer format */
		goto parseError;
	    }


	    /* Got an integer offset; pull it from where parser left it. */
	    Tcl_GetNumberFromObj(NULL, objPtr, &cd, &t);
	    
	    
	    
	    if (t == TCL_NUMBER_BIG) {
		/* Truncate to the signed wide range. */
		if (mp_isneg((mp_int *)cd)) {
		    offset = (bytes[3] == '-') ? WIDE_MAX : WIDE_MIN;
		} else {
		    offset = (bytes[3] == '-') ? WIDE_MIN : WIDE_MAX;
		}
	    } else {
		/* assert (t == TCL_NUMBER_INT); */
		offset = (*(Tcl_WideInt *)cd);
		if (bytes[3] == '-') {
		    offset = (offset == WIDE_MIN) ? WIDE_MAX : -offset;
		}
		if (offset == 1) {
		    offset = WIDE_MAX; /* "end+1" */
		} else if (offset > 1) {
		    offset = WIDE_MAX - 1; /* "end+n", out of range */
		} else if (offset != WIDE_MIN) {
		    offset--;
		}
	    }
	}

    parseOK:
	/* Success. Store the new internal rep. */
	ir.wideValue = offset;
	Tcl_StoreInternalRep(objPtr, &endOffsetType, &ir);
    }
    
    offset = irPtr->wideValue;
    
#ifdef mycode
    tryit:
#endif // mycode
    
    if (offset == WIDE_MAX) {
	/*
	 * Encodes end+1. This is distinguished from end+n as noted
         * in function header.
	 * NOTE: this may wrap around if the caller passes (as lset does)
	 * listLen-1 as endValue and and listLen is 0. The -1 will be
	 * interpreted as FF...FF and adding 1 will result in 0 which
	 * is what we want. Callers like lset which pass in listLen-1 == -1
         * as endValue will have to adjust accordingly.
	 */
	*widePtr = (endValue == -1) ? WIDE_MAX : endValue + 1;
    } else if (offset == WIDE_MIN) {
	/* -1 - position before first */
	*widePtr = -1;
    } else if (offset < 0) {
	/* end-(n-1) - Different signs, sum cannot overflow */
	*widePtr = endValue + offset + 1;
    } else if (offset < WIDE_MAX) {
	/* 0:WIDE_MAX-1 - plain old index. */
	*widePtr = offset;
    } else {
	/* Huh, what case remains here? */
	*widePtr = WIDE_MAX;
    }
    return TCL_OK;

    /* Report a parse error. */
  parseError:
    if (interp != NULL) {
        char * bytes = TclGetString(objPtr);
        Tcl_SetObjResult(interp, Tcl_ObjPrintf(
                "bad index \"%s\": must be integer?[+-]integer? or"
                " end?[+-]integer?", bytes));
        if (!strncmp(bytes, "end-", 4)) {
            bytes += 4;
        }
        Tcl_SetErrorCode(interp, "TCL", "VALUE", "INDEX", (void *)NULL);
    }

    return TCL_ERROR;
}





// This is the code for canvas coordinates AND other configuration numbers:
// from tclObj.c



/*
 *----------------------------------------------------------------------
 *
 * Tcl_GetDoubleFromObj --
 *
 *	Attempt to return a double from the Tcl object "objPtr". If the object
 *	is not already a double, an attempt will be made to convert it to one.
 *----------------------------------------------------------------------
 */

int
Tcl_GetDoubleFromObj(
    Tcl_Interp *interp,         /* Used for error reporting if not NULL. */
    Tcl_Obj *objPtr,	/* The object from which to get a double. */
    double *dblPtr)	/* Place to store resulting double. */
{

    do {
	if (objPtr->typePtr == &tclDoubleType) {
	    if (isnan(objPtr->internalRep.doubleValue)) {
		if (interp != NULL) {
		    Tcl_SetObjResult(interp, Tcl_NewStringObj(
			    "floating point value is Not a Number", -1));
                    Tcl_SetErrorCode(interp, "TCL", "VALUE", "DOUBLE", "NAN",
                            (void *)NULL);
		}
		return TCL_ERROR;
	    }
	    *dblPtr = (double) objPtr->internalRep.doubleValue;
	    return TCL_OK;
	}
	if (objPtr->typePtr == &tclIntType) {
	    *dblPtr = (double) objPtr->internalRep.wideValue;
	    return TCL_OK;
	}
	if (objPtr->typePtr == &tclBignumType) {
	    mp_int big;

	    TclUnpackBignum(objPtr, big);
	    *dblPtr = TclBignumToDouble(&big);
	    return TCL_OK;
	}
    } while (SetDoubleFromAny(interp, objPtr) == TCL_OK);

#define mycode
#ifdef mycode 
int evaluate_ll_expression(char *,int64_t *); // prototypes
int evaluate_d_expression(char*, double*);
//------------------------------------------------------------------------   
    double answerd; int thestatus=1;
	thestatus = evaluate_d_expression(TclGetString(objPtr),&answerd);
	if ( thestatus == 0 ) {
		*dblPtr = answerd;
		return TCL_OK;
	
	}
 //------------------------------------------------------------------------   
#endif  //mycode
    return TCL_ERROR;
}



// from tclGet.c  get ints and doubles, called by the above or those it calls



/*
 *----------------------------------------------------------------------
 *
 * Tcl_GetInt --
 *
 *	Given a string, produce the corresponding integer value.
 *----------------------------------------------------------------------
 */


int evaluate_ll_expression(char *,int64_t *); // prototypes
int evaluate_d_expression(char*, double*);
int bugprintf(const char *,...);

int
Tcl_GetInt(
    Tcl_Interp *interp,		/* Interpreter to use for error reporting. */
    const char *src,		/* String containing a (possibly signed)
				 * integer in a form acceptable to
				 * Tcl_GetIntFromObj(). */
    int *intPtr)		/* Place to store converted result. */
{
    Tcl_Obj obj;
    int code;

    obj.refCount = 1;
    obj.bytes = (char *) src;
    obj.length = strlen(src);
    obj.typePtr = NULL;
	
#define mycode
#ifdef mycode
#define Separate_ll_function
#include "expression.c"
// ----------------------------------------------
    long long answerll; int thestatus;
	thestatus = evaluate_ll_expression(src,&answerll);
	if ( thestatus == 0 ) {
		*intPtr = (int)answerll;
		code = TCL_OK;
		goto outahere;
	}
	goto here2;
        if       ( thestatus > 0x1000 ) {
            Tcl_SetObjResult(interp, Tcl_ObjPrintf(
                "\"%s\": bad number or mini-expression \n"
                "%*s^^^^  code %x", src,thestatus& 0xff, "" ,thestatus));

        } else if ( thestatus == 2  ) {
            Tcl_SetObjResult(interp, Tcl_ObjPrintf(
                "\"%s\": unbalanced parens \n" , src));
        } else if ( thestatus == 3 ) {
            Tcl_SetObjResult(interp, Tcl_ObjPrintf(
                "\"%s\": exceded max paren depth \n", src));
        }
        Tcl_SetErrorCode(interp, "TCL", "VALUE", "NUMBER", (void *)NULL);
    return TCL_ERROR;
// ----------------------------------------------
#endif// mycode
    
#ifdef mycode
 here2:
#endif // mycode

    code = Tcl_GetIntFromObj(interp, &obj, intPtr);
    
#ifdef mycode
 outahere:   
#endif // mycode

    if (obj.refCount > 1) {
	Tcl_Panic("invalid sharing of Tcl_Obj on C stack");
    }
    TclFreeInternalRep(&obj);
    return code;
}

// also in tclGet.c

/*
 *----------------------------------------------------------------------
 *
 * Tcl_GetDouble --
 *
 *	Given a string, produce the corresponding double-precision
 *	floating-point value.
 *----------------------------------------------------------------------
 */

int
Tcl_GetDouble(
    Tcl_Interp *interp,		/* Interpreter used for error reporting. */
    const char *src,		/* String containing a floating-point number
				 * in a form acceptable to
				 * Tcl_GetDoubleFromObj(). */
    double *doublePtr)		/* Place to store converted result. */
{
    Tcl_Obj obj;
    int code;

    obj.refCount = 1;
    obj.bytes = (char *) src;
    obj.length = strlen(src);
    obj.typePtr = NULL;
    
#ifdef mycode  
 //------------------------------------------------------------------------   
    double answerd; int thestatus=1;
	thestatus = evaluate_d_expression(src,&answerd);
	if ( thestatus == 0 ) {
		*doublePtr = answerd;
		code = TCL_OK;
		goto outahere2;
	} else {
	
	}
 //------------------------------------------------------------------------   
#endif  //mycode
      
    code = Tcl_GetDoubleFromObj(interp, &obj, doublePtr);
#ifdef mycode
 outahere2:   
#endif // mycode
    if (obj.refCount > 1) {
	Tcl_Panic("invalid sharing of Tcl_Obj on C stack");
    }
    TclFreeInternalRep(&obj);
    return code;
}


