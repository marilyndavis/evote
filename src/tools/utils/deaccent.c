/* * eVote - Software for online consensus development.
 * Copyright (C) 2015 Marilyn Davis <marilyn@deliberate.com>
 *
 * This file is part of eVote.
 *
 * eVote is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * eVote is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with eVote.  If not, see <http://www.gnu.org/licenses/>.
 */

/* $Id: deaccent.c,v 1.3 2003/01/15 18:54:13 marilyndavis Exp $ */ 
#define PATH "/usr/local/majordomo/"    /*Place here the path to be prepended to the next name*/
/* #define TRACE "/tmp/deaccent.debug"   place here the name and path of file to save status */
/****************************************************************************
 *      Program to convert a file with accented letters to 7 bit text
 *            For command line arguments (all optionals)
 *             deaccent [command line args] [< input file] [> output file]
 *                The input file must exist, the output file if it exists
 *                is overwritten.
 *                The accents are placed after the letter, for cedillas  
 *                a comma is used, for a circle (above a or A) an asterisk
 *                is used. Ligated letters are simply untied, back conversion  
 *                is impossible for those.
 *       Contributed by John Jacq with help from Marilyn Davis. (ver 990323)
 *********************************************************
 **********************************************************/
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
typedef enum {FALSE, TRUE} boolean;
/*Global Variables*/
FILE* fp_out; /* = stdout;   /* Changes to stderr on error */
FILE* fp_error; /*  = stderr; /* Changes to fptrace on TRACE */
#ifdef TRACE
FILE * fptrace;
#endif
/* Functions available to all */
int deaccent(FILE *, FILE *);
void send(int argc, char ** argv, char ** env);
/***************************************************************************
 *      Main program: deaccent
 ***************************************************************************/
int
main(int argc, char *argv[], char *env[])
{
  int errnum;
  fp_out = stdout;
  fp_error = stderr;
#ifdef TRACE
  int cc;
  fptrace = fopen(TRACE, "w");
  fp_error = fptrace;
  fprintf(fptrace,"\n\ndeaccent called as:\n");
  for (cc = 0; cc < argc ; cc++)
    fprintf(fptrace," %s", argv[cc]);
  fprintf(fptrace,"\nEnvironment is:\n");
  for (cc = 0; env[cc] != NULL; cc++)
    fprintf(fptrace, "%s ", env[cc]);
  fprintf(fptrace,"\n\n");
#endif
  /* We send the rest of the arguments to the next program */
  if (argc > 1 )  /* Now we check the rest of the command line */
    send (argc, argv, env);    /* open pipe to argv[1] */
  /* Now we do our stuff, like stripping or re-accenting */
  errnum = deaccent(stdin, fp_out);
  if (errnum != 0 )
    fprintf(fp_error,"End of file encontered while writing to ouput device!\n\
Operation cut short--");
  exit(0);
}
/***************************************************************************
* Function deaccent, converts accented letters to 7 bit ascii pairs. 
* accented letters are printed with the de-accented letter followed by the
* accent.
****************************************************************************/ 
int
deaccent(FILE *ifp, FILE *ofp)
{
  int letter;
  int old=0;
  int i;
  int ascii[128][2];
  char vowels[]="AEIOUYaeiouy";
  boolean old_vowel=FALSE ;
  /*           We initialise the array ascii                     */
  ascii[  0][0]=128; ascii[  0][1]='\0';    /* No replacement    */ 
  ascii[  1][0]=129; ascii[  1][1]='\0';    /* No replacement    */ 
  ascii[  2][0]=130; ascii[  2][1]='\0';    /* No replacement    */ 
  ascii[  3][0]=131; ascii[  3][1]='\0';    /* No replacement    */ 
  ascii[  4][0]=132; ascii[  4][1]='\0';    /* No replacement    */ 
  ascii[  5][0]=133; ascii[  5][1]='\0';    /* No replacement    */ 
  ascii[  6][0]=134; ascii[  6][1]='\0';    /* No replacement    */ 
  ascii[  7][0]=135; ascii[  7][1]='\0';    /* No replacement    */ 
  ascii[  8][0]=136; ascii[  8][1]='\0';    /* No replacement    */ 
  ascii[  9][0]=137; ascii[  9][1]='\0';    /* No replacement    */ 
  ascii[ 10][0]='S'; ascii[ 10][1]='^';     /* S^ with "v" accent*/
  ascii[ 11][0]=139; ascii[ 11][1]='\0';    /* No replacement    */ 
  ascii[ 12][0]='O'; ascii[ 12][1]='E';     /* OE ligated        */
  ascii[ 13][0]=141; ascii[ 13][1]='\0';    /* No replacement    */ 
  ascii[ 14][0]=142; ascii[ 14][1]='\0';    /* No replacement    */ 
  ascii[ 15][0]=143; ascii[ 15][1]='\0';    /* No replacement    */ 
  ascii[ 16][0]=144; ascii[ 16][1]='\0';    /* No replacement    */ 
  ascii[ 17][0]='\''; ascii[ 17][1]='\0';    /* Left Single Quote */ 
  ascii[ 18][0]='\''; ascii[ 18][1]='\0';    /* Right Single Quote*/ 
  ascii[ 19][0]='\"'; ascii[ 19][1]='\0';    /* Left Double Quote */ 
  ascii[ 20][0]='\"'; ascii[ 20][1]='\0';    /* Right Double Quote*/ 
  ascii[ 21][0]=149; ascii[ 21][1]='\0';    /* No replacement    */ 
  ascii[ 22][0]=150; ascii[ 22][1]='\0';    /* No replacement    */ 
  ascii[ 23][0]=' '; ascii[ 23][1]='\0';    /* Special Space     */ 
  ascii[ 24][0]=152; ascii[ 24][1]='\0';    /* No replacement    */ 
  ascii[ 25][0]=153; ascii[ 25][1]='\0';    /* No replacement    */ 
  ascii[ 26][0]='s'; ascii[ 26][1]='^';     /* s^ with "v" accent*/
  ascii[ 27][0]=155; ascii[ 27][1]='\0';    /* No replacement    */ 
  ascii[ 28][0]='o'; ascii[ 28][1]='e';     /* oe ligated        */
  ascii[ 29][0]=157; ascii[ 29][1]='\0';    /* No replacement    */ 
  ascii[ 30][0]=158; ascii[ 30][1]='\0';    /* No replacement    */ 
  ascii[ 31][0]='Y'; ascii[ 31][1]=':';     /* Y:                */
  ascii[ 32][0]=160; ascii[ 32][1]='\0';    /* No replacement    */ 
  ascii[ 33][0]=' '; ascii[ 33][1]='!';     /* Topsy turvy !     */
  ascii[ 34][0]='c'; ascii[ 34][1]='\0';    /* US cents          */ 
  ascii[ 35][0]='L'; ascii[ 35][1]='b';     /* British Pound     */ 
  ascii[ 36][0]=164; ascii[ 36][1]='\0';    /* No replacement    */ 
  ascii[ 37][0]='Y'; ascii[ 37][1]='#';     /* Japanese Yen      */ 
  ascii[ 38][0]='|'; ascii[ 38][1]='\0';    /* Vertical line     */ 
  ascii[ 39][0]=167; ascii[ 39][1]='\0';    /* S with circle     */ 
  ascii[ 40][0]=168; ascii[ 40][1]='\0';    /* No replacement    */ 
  ascii[ 41][0]='C'; ascii[ 41][1]='R';     /* Copyright symbol  */
  /* I have replaced the copyright symbol by CR. I'm opened to
     other suggestions as I don't particularly like it. One rule:
     It can't be more than 2 characters long! */
  ascii[ 42][0]=170; ascii[ 42][1]='\0';    /* No replacement    */ 
  ascii[ 43][0]='<'; ascii[ 43][1]='<';     /* French left quote */ 
  ascii[ 44][0]=172; ascii[ 44][1]='\0';    /* No replacement    */ 
  ascii[ 45][0]='-'; ascii[ 45][1]='\0';    /* Dash              */ 
  ascii[ 46][0]=174; ascii[ 46][1]='\0';    /* No replacement    */ 
  ascii[ 47][0]=175; ascii[ 47][1]='\0';    /* No replacement    */ 
  ascii[ 48][0]=176; ascii[ 48][1]='\0';    /* Degree symbol     */ 
  ascii[ 49][0]='+'; ascii[ 49][1]='-';     /* +/-               */ 
  ascii[ 50][0]=178; ascii[ 50][1]='\0';    /* Power 2           */ 
  ascii[ 51][0]=179; ascii[ 51][1]='\0';    /* No replacement    */ 
  ascii[ 52][0]='\''; ascii[ 52][1]='\0';    /* Acute accent      */
  ascii[ 53][0]='u'; ascii[ 53][1]='\0';    /* Greek mu          */ 
  ascii[ 54][0]=182; ascii[ 54][1]='\0';    /* No replacement    */ 
  ascii[ 55][0]='.'; ascii[ 55][1]='\0';    /* DOT               */ 
  ascii[ 56][0]=184; ascii[ 56][1]='\0';    /* No replacement    */ 
  ascii[ 57][0]=185; ascii[ 57][1]='\0';    /* No replacement    */ 
  ascii[ 58][0]=186; ascii[ 58][1]='\0';    /* No replacement    */ 
  ascii[ 59][0]='>'; ascii[ 59][1]='>';     /* French right quote*/ 
  ascii[ 60][0]=188; ascii[ 60][1]='\0';    /* No replacement    */ 
  ascii[ 61][0]=189; ascii[ 61][1]='\0';    /* No replacement    */ 
  ascii[ 62][0]=190; ascii[ 62][1]='\0';    /* No replacement    */ 
  ascii[ 63][0]=' '; ascii[ 63][1]='?';     /* Topsy turvy ?     */
  ascii[ 64][0]='A'; ascii[ 64][1]='`';     /* A`                */
  ascii[ 65][0]='A'; ascii[ 65][1]='\'';    /* A'                */
  ascii[ 66][0]='A'; ascii[ 66][1]='^';     /* A^ (MS)           */
  ascii[ 67][0]='A'; ascii[ 67][1]='~';     /* A~ (MS)           */
  ascii[ 68][0]='A'; ascii[ 68][1]=':';     /* A:                */
  ascii[ 69][0]='A'; ascii[ 69][1]='*';     /* A* circle accent  */
  ascii[ 70][0]='A'; ascii[ 70][1]='E';     /* AE ligated        */
  ascii[ 71][0]='C'; ascii[ 71][1]=',';     /* C, cedilla        */
  ascii[ 72][0]='E'; ascii[ 72][1]='`';     /* E`                */
  ascii[ 73][0]='E'; ascii[ 73][1]='\'';    /* E'                */
  ascii[ 74][0]='E'; ascii[ 74][1]='^';     /* E^                */
  ascii[ 75][0]='E'; ascii[ 75][1]=':';     /* E:                */
  ascii[ 76][0]='I'; ascii[ 76][1]='`';     /* I`                */
  ascii[ 77][0]='I'; ascii[ 77][1]='\'';    /* I'                */
  ascii[ 78][0]='I'; ascii[ 78][1]='^';     /* I^                */
  ascii[ 79][0]='I'; ascii[ 79][1]=':';     /* I:                */
  ascii[ 80][0]=208; ascii[ 80][1]='\0';    /* No replacement    */ 
  ascii[ 81][0]='~'; ascii[ 81][1]='\0';    /* N~                */
  ascii[ 82][0]='O'; ascii[ 82][1]='`';     /* O`                */
  ascii[ 83][0]='O'; ascii[ 83][1]='\'';    /* O'                */
  ascii[ 84][0]='O'; ascii[ 84][1]='^';     /* O^                */
  ascii[ 85][0]='O'; ascii[ 85][1]='~';     /* O~ (MS)           */
  ascii[ 86][0]='O'; ascii[ 86][1]=':';     /* O:                */
  ascii[ 87][0]=215; ascii[ 87][1]='\0';    /* No replacement    */ 
  ascii[ 88][0]=216; ascii[ 88][1]='\0';    /* No replacement    */ 
  ascii[ 89][0]='U'; ascii[ 89][1]='`';     /* U`                */
  ascii[ 90][0]='U'; ascii[ 90][1]='\'';    /* U'                */
  ascii[ 91][0]='U'; ascii[ 91][1]='^';     /* U^                */
  ascii[ 92][0]='U'; ascii[ 92][1]=':';     /* U:                */
  ascii[ 93][0]='Y'; ascii[ 93][1]='\'';    /* Y'                */
  ascii[ 94][0]=222; ascii[ 94][1]='\0';    /* No replacement    */ 
  ascii[ 95][0]='s'; ascii[ 95][1]='s';     /* ss (German Beta)  */
  ascii[ 96][0]='a'; ascii[ 96][1]='`';     /* a`                */
  ascii[ 97][0]='a'; ascii[ 97][1]='\'';    /* a'                */
  ascii[ 98][0]='a'; ascii[ 98][1]='^';     /* a^                */
  ascii[ 99][0]='a'; ascii[ 99][1]='~';     /* a~ (MS)           */
  ascii[100][0]='a'; ascii[100][1]=':';     /* a:                */
  ascii[101][0]='a'; ascii[101][1]='*';     /* a* circle accent  */
  ascii[102][0]='a'; ascii[102][1]='e';     /* ae ligated        */
  ascii[103][0]='c'; ascii[103][1]=',';     /* c, cedilla        */
  ascii[104][0]='e'; ascii[104][1]='`';     /* e`                */
  ascii[105][0]='e'; ascii[105][1]='\'';    /* e'                */
  ascii[106][0]='e'; ascii[106][1]='^';     /* e^                */
  ascii[107][0]='e'; ascii[107][1]=':';     /* e:                */
  ascii[108][0]='i'; ascii[108][1]='`';     /* i`                */
  ascii[109][0]='i'; ascii[109][1]='\'';    /* i'                */
  ascii[110][0]='i'; ascii[110][1]='^';     /* i^                */
  ascii[111][0]='i'; ascii[111][1]=':';     /* i:                */
  ascii[112][0]=240; ascii[112][1]='\0';    /* No replacement    */ 
  ascii[113][0]='~'; ascii[113][1]='\0';    /* n~                */
  ascii[114][0]='o'; ascii[114][1]='`';     /* o`                */
  ascii[115][0]='o'; ascii[115][1]='\'';    /* o'                */
  ascii[116][0]='o'; ascii[116][1]='^';     /* o^                */
  ascii[117][0]='o'; ascii[117][1]='~';     /* o~ (MS)           */
  ascii[118][0]='o'; ascii[118][1]=':';     /* o:                */
  ascii[119][0]=247; ascii[119][1]='\0';    /* No replacement    */
  ascii[120][0]=248; ascii[120][1]='\0';    /* No replacement    */ 
  ascii[121][0]='u'; ascii[121][1]='`';     /* u`                */
  ascii[122][0]='u'; ascii[122][1]='\'';    /* u'                */ 
  ascii[123][0]='u'; ascii[123][1]='^';     /* u^                */
  ascii[124][0]='u'; ascii[124][1]=':';     /* u:                */
  ascii[125][0]='y'; ascii[125][1]='\'';    /* y'                */
  ascii[126][0]=254; ascii[126][1]='\0';    /* No replacement    */ 
  ascii[127][0]='y'; ascii[127][1]=':';     /* y:                */
  /* We read the input and replace by 2 characters if necessary */
  while ((letter = getc(ifp)) != EOF)
    {
      /*      NORMAL ROUTINE STARTS HERE, (after the special cases) 
	      If the ascii value goes to 8 bit, we replace by the 2
	      character code.                                           */
      if (letter >= 128)
	{
	  if (putc(ascii[letter-128][0], ofp) == EOF)
	    return -1;                        /*Write the first part     */
	  if (ascii[letter-128][1] != '\0')    /* only print the 2nd when */
	    {                                 /* non zero                */
	      if (putc(ascii[letter-128][1], ofp) == EOF)
		return -1; /* Write the second part */
	    }
	}
      else
	{
	  if (putc(letter, ofp) == EOF) return -1; /* write it */
	}
    }  /* end of while */
  return 0; 
}
/**********************************************************
*    This forks and execs a process:
*        It execs argv[1] and copies the other arguments
*        to argv[1]' command line.
*        The parent's stdout is attached to the child's
*        stdin.
*        Marilyn, I got rid of the default "/" which was
*        inserted if the last character of PATH is not "/"
*        Without this, a NULL PATH is possible and one can refer
*        to the next program by "~/bin/whatever" or even just
*        "whatever" if the program is in the same directory
*        as deaccent. 
***********************************************************/
void
send(int argc, char ** argv, char ** env)
{
  int ch;
  int pfd[2];
  int i, len;
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
	  fprintf(fp_error, "\ndeaccent: Child: Can't close pipe's extra descriptors.  Continuing.\n");
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
  fprintf(fp_error,"\ndeaccent: Failed to pipe to %s.\n", program);
  fprintf(fp_error, error_msg);
  fprintf(fp_error,"\nLost message follows:\n");
}
