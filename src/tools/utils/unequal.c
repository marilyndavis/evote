/* $Id: unequal.c,v 1.3 2003/01/15 18:54:13 marilyndavis Exp $ */ 
#define PATH "/usr/local/bin/"   /* Insert default path here if required */
//#define TRACE "/tmp/unequal.dbg"  /* Insert debug file name & path here if required*/
/*********************************** RCS header ******************************
$Id: unequal.c,v 1.3 2003/01/15 18:54:13 marilyndavis Exp $
$Header: /cvsroot/evote/eVote/src/tools/utils/unequal.c,v 1.3 2003/01/15 18:54:13 marilyndavis Exp $
$Author: marilyndavis $
****************************************************************************
 *      Program to convert an email with quoted printables to 8 bit text
 *      It waits for the headers to be finished before conversion starts.
 *            For command line arguments the program to pipe into is next, 
 *            followed by its command line arguments.
 *                unequal [command line arguments]
 *********************************************************
 **********************************************************/
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
/* Macro Functions */
#define tryhex(x) (x >='0'&& x <='9' ? x -'0':\
                  (x >= 'A' && x <= 'F' ? x-'A'+10 :\
                  (x >= 'a' && x <= 'f' ? x-'a'+10: -1)))
typedef enum {FALSE, TRUE} boolean;
/*Global Variables*/
FILE* fp_out;   /* Changes to stderr on error */
FILE* fp_error; /* Changes to fptrace on TRACE */
#ifdef TRACE
FILE * fptrace;
#endif
/* subroutines */
void send(int, char **, char ** );
int qptotxt(FILE*, FILE*);
int parse_header(FILE*, FILE*);
/****************************************************************************
 *      Program to convert a file with quoted printables to 8 bit text
 *       uses qptotxt and send as functions.
 ****************************************************************************/
int
main(int argc, char *argv[], char ** env)
{
  int err;
#ifdef TRACE
  int cc;
  fptrace = fopen(TRACE, "w");
  fp_error = fptrace;
  fprintf(fptrace,"\n\nunequal called as:\n");
  for (cc = 0; cc < argc ; cc++)
    fprintf(fptrace," %s", argv[cc]);
  fprintf(fptrace,"\nEnvironment is:\n");
  for (cc = 0; env[cc] != NULL; cc++)
    fprintf(fptrace, "%s ", env[cc]);
  fprintf(fptrace,"\n\n");
  fflush(fptrace);
#else
  fp_error = stderr;
#endif
  fp_out = stdout;
  /* We send the rest of the arguments to the next program and launch it*/
  if (argc > 1 )  /* Now we check the rest of the command line */
    send (argc, argv, env);    /* open pipe to argv[1] */
  /* First we pass the headers unchanged, then we decode the body */
  if ((err=parse_header(stdin, fp_out)) == 0)
     err = qptotxt(stdin, fp_out); /* let's do the real work */
  if (err != 0 )
    {
      fprintf(fp_error,"End of file encountered while writing to ouput device!\nOperation cut short--");
      exit(1);   /* problems ! */
    }
  exit(0);       /* 0 means no error to me */
}
/***************************************************************************
* Function qptotxt, converts QP's to ascii. 
* QP's are of the form =XX where X = {0:9}+{A:F}
* We search for "=" then test the next 2 characters to check if
* they belong to the set (we ignore the \n and \r) if any.
* The program is a 2 state machine, switching from NORMAL state to QP state
* when a "=" is encountered.
* One function is used:
*       tryhex(int) = compares the ascii value of the integer to the valid
*                     hex characters and returns the decimal value of this
*                     character if it is a hex, and -1 if it isn't.
****************************************************************************/ 
int
qptotxt(FILE *ifp, FILE *ofp)
{
  typedef enum {NORMAL, QP} STATE;
  int letter, k, hexval, temp, store[4];
  int empty (int*, int, FILE *);
  STATE mode = NORMAL;
  boolean ret, isblank;
  while ((letter = getc(ifp)) != EOF)
    {
      switch (mode)
	{
	case NORMAL:          /* We print unless we find an = in which
				 case we switch to QP mode. No break is
				 provided at the end of this case, if the
				 case remains normal, a continue sends you
				 back to the while loop. Otherwise you fall
				 directly into the QP case. */
	  switch (letter)
	    {
	    case '=':         /* could be a QP */
	      mode = QP;      /* switch to QP mode for a while */
	      k=0;
	      break;          /* and go directly to QP NOW!*/
	    default:          /* Not one :)    */
	      if ((putc(letter, ofp)) == EOF) return -1; /* Lack of space
						       returns -1 */
	      continue;       /* go back to while loop */ 
	    }
	  /* CAUTION, NO BREAK PROVIDED, ONLY THE CONTINUE WILL SAVE YOU*/
	case QP:               /* We are now in QP mode  */
	  store[k]=letter;
	  switch (k++)
	    {
	    case 0:            /* first arrived, loop back */
	      ret = FALSE;
	      isblank = FALSE;
	      break;
	    case 1:            /* First letter after = */
	      if (letter == '=')   /* Another "=" */
		{
		  if (putc('=', ofp) == EOF)
		    return -1;
		  k=1;     /* restart new QP run*/
		  break;
		}
	      else if (letter == '\n')  /* case of unix QP return */
		mode = NORMAL;   /* we pretend we saw nothing :) */
	      else if (letter == '\r')
		ret = TRUE;      /* we flag the next loop should flush
                                      out a \n (DOS QP return) */
	      else if (letter == ' ') /* character is blank */
		isblank = TRUE; /* Set the flag */ 
	      else if ((temp = tryhex(letter)) >= 0) /* character is hex */
		hexval = 16*temp; /* first hex is times 16 */ 
	      else      /* This is not a QP, we print the buffer */
		{
		  if (empty(store, k, ofp) < 0)
		      return -1; /* Lack of space returns an error */
		  mode = NORMAL;
		}
	      break;
	    case 2:         /* 
			       2nd letter after "=" We check first for
			       the 2nd half of a DOS return. Then we check for
			       the case of a space followed by either \n or 
			       \r\n. Finally if neither of those, we check for
			       the least significant nibble of an ascii.
			     */
	      if (letter == '=')   /* Another "=" print & loop to QP*/
		{
		  if (! ret) /* Unless a return was found, dump store */
		    {
		      if (empty(store, k-1, ofp) < 0)
			return -1; /* lack of space returns an error */
		    }
		  k=1;                /* restart new QP run at 1 */
		  isblank = FALSE;    /* reset the flags         */
		  ret = FALSE;        /* but stay in QP mode     */
		  break;
		}
	      else if (ret)     /* a return was the first letter */
		{
		  if (letter != '\n')   /*Impossible to happen but who knows*/
		    {
		      if (putc(letter, ofp) == EOF) /*print the last letter read*/
			return -1; /*Lack of space returns an error */
		    }
		  mode = NORMAL;    /* In any case, we ignore the QP return*/
		}
	      else if (isblank) /* First letter was a blank */
		{
		  if (letter == '\n') /* 'Twas a QP return, ignore!*/
		    {
		      mode = NORMAL;
		    }
		  else if (letter == '\r') /* Set the expected new line flag */
		    ret = TRUE;           /* and remain in QP mode */
		  else       /* Else case of mistaken identity, print*/
		    {
		      if (empty(store, k, ofp) < 0)
		          return -1; /* Lack of space returns an error */
		      mode = NORMAL;
		    }
		}
	      else if ((temp = tryhex(letter)) >= 0) /* Second letter is hex */
		{
		  hexval += temp;
		  if ((putc(hexval, ofp)) == EOF) /* We write the QP as ASCII*/
		    return -1; /* Lack of space returns an error */
		  mode = NORMAL; /*We reset the mode to normal and loop back*/
		}
	      else      /* This is not a QP, we print the buffer */
		{
		  if (empty(store, k, ofp) < 0)
		    return -1; /* Lack of space returns an error */
		  mode = NORMAL;
		}
	      break;
	    case 3:  /* We can only arrive here if previous letter was '\r'.
			This case deals with when an equal is followed by a
			space then a DOS return (\r\n). It seems to be a
			relatively frequent  QP which is to be deleted */
	      if (letter == '=')   /* Another "=" */
		{
		  k=1;     /* restart new QP run, ignore the return*/
		  isblank = FALSE;    /* reset the flags         */
		  ret = FALSE;        /* but stay in QP mode     */
		  break;
		}
	      if (letter != '\n')   /* Impossible to happen but who knows*/
		{                   /* Ignore the "= \r" but output this
				       letter*/
		  if (putc(letter, ofp) == EOF) /*print the last letter read*/
		    return -1; /*Lack of space returns an error */
		}
	      mode = NORMAL;    /* return to normal mode and quit */
	      break; 
	    } /* end of "k" switch */
	} /* end of "mode" switch */
    } /* end of while */
  return 0; /* No errors, we return 0 */
}
/*******************************************************************
 * empty(store,k,ofp).  Sends k characters from buffer store to ofp
 *******************************************************************/
int
empty(int * store, int k, FILE * ofp)
{
  while (k-->0)
    if (putc(*store++, ofp) == EOF)
      return -1;
  return 0;
}
/********************************************************************
 * returns 1 on read error or EOF, 2 on write error, 0 = OK
 ********************************************************************/
int
parse_header(FILE* input, FILE* output)
{
  int previous=0, letter;
  int CR='\r';
  int LF='\n';
  while ((letter=fgetc(input)) != EOF)
    {
      if (putc(letter, output) == EOF)  // first: output the character
	return 2;
      if (letter == CR)   // If character is CR, we ignore it in the buffer
	continue;
      /* We step the buffer along and test for 2 line feeds */
      if (previous == LF && letter == LF)
	return 0;
      previous=letter;   // We save this letter for the next round
    }
  return 1;  // If we got here it's because of EOF or error
}
/**********************************************************
*    This forks and execs a process:
*        It execs argv[1] and copies the other arguments
*        to argv[1]' command line.
*        The parent's stdout is attached to the child's
*        stdin.
***********************************************************/
void
send(int argc, char ** argv, char ** env)
{
  int pfd[2];
  int i;
  char *path;
  char next_arg[100];
  char error_msg[1000];
  void send_err(char * program, char * error_msg);
  if (pipe(pfd) == -1)
    {
      send_err(argv[1], "\nCan't open pipe.\n");
    }
  switch (fork())
    {
    case -1:
      send_err(argv[1], "\nCan't fork a new process.\n");
      break;
    case 0:  /* child process */
      if (close(0) == -1)   /* close stdin */
	{
	  send_err(argv[1],  "\nChild: Can't close stdin.\n");
	  break;
	}
      if (dup(pfd[0]) != 0)  /* pipe input is stdin */
	{
	  send_err(argv[1],  "\nChild: Can't dup pipe reading to stdin.\n");
	  break;
	}
      if (close(pfd[0]) == -1 || close(pfd[1]) == -1)
	{
	  fprintf(fp_error, "\nunequal: Child: Can't close pipe's extra descriptors.  Continuing.\n");
	}
      if ((path=malloc(strlen(PATH)+strlen(argv[1])+1)) == NULL)
	{
	  send_err(argv[1],  "\nCan't malloc space for path.\n");
	}
      strcpy(path, PATH);
      strcat(path, argv[1]);
#ifdef TRACE
      fprintf(fptrace,"\nexecve called on %s, args are :\n", path);
      for (i = 0; (argv+1)[i] != NULL; i++)
	fprintf(fptrace," %s", (argv+2)[i]);    
      fprintf(fptrace,"\nEnv passed is:\n");
      for (i=0; (env)[i] != NULL; i++)
	fprintf(fptrace, " %s", env[i]);
      fflush(fptrace);
#endif
      execve(path, argv+1, env);  
      sprintf(error_msg,"\nExec Failed, ERROR = %d\n\tCall was \"%s\".  Args were:", errno, path);
      for (i = 2; argv[i] != NULL; i++)
	{
	  sprintf(next_arg, " %s", argv[i]);
	  strcat(error_msg, next_arg);
	}
      strcat(error_msg,"\n");
      send_err(argv[1], error_msg);
      break;
    default:  /* parent */
      break;
    }
/* now we're the parent */
  if (close(1) == -1)   /* close stdout */
    {
      send_err(argv[1], "\nParent: Can't close stdout.\n");
    }
  if (dup(pfd[1]) != 1)  /* pipe output is stdout */
    {
      send_err(argv[1], "\nParent: Can't dup pipe writing to stdout.\n");
    }
  if (close(pfd[0]) == -1 || close(pfd[1]) == -1)
    {
      fprintf(fp_error, "\nParent: Can't close pipe's extra descriptors.  Continuing.\n");
    }
}
/**********************************************************************
*    Upon error in send(), this changes the stdout to stderr
*    and prints an error message.
**********************************************************************/
void
send_err(char * program, char * error_msg)
{
  fp_out = fp_error;
  fprintf(fp_error,"\nunequal: Failed to pipe to %s.\n", program);
  fprintf(fp_error, error_msg);
  fprintf(fp_error,"\nLost message follows:\n");
}
