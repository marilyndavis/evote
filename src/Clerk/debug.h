/* $Id: debug.h,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// debug.h -- little header for the vmem function for debugging
/*********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#ifndef _debug_h
#define _debug_h
#include"evotedef.h"
#include<stdio.h>
void vmem(char* msg, YESorNO free = NO, int size = 0);
extern int edebug;
extern char debugger[];
void doutput(char*);
extern ostream *dlogger; 
#endif
