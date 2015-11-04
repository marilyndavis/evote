/* $Id: forward.c,v 1.5 2003/10/17 23:07:50 marilyndavis Exp $ */ 
/*********************************************************
 *   forward.c   -   Can be called by wrapper from the
 *                  sendmail aliases file:
 *    in_address: "|/usr/local/wrapper forward -f from_address -t to_address"
 *       -t return will return the message to the sender.  You can add
 *       replacement triplets to the command line to change the subject:
 *        -s "En;" "xx" will replace all "En;"'s in the subject
 *       line with "xx".  -s "*En;" "xx" will only replace the
 *       "En;" string if it's the start of the subject.
 *       You can put up to MAX=10 -s "this" "that" triplets on the
 *       command line.
 ************************************************************
 *   To compile, alter the makefile and type:
 *   make
 *   Put the resulting executable in with your majordomo
 *   scripts and you're ready.
 *********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include "filter.h"
#define MAX_REPLACERS 10
char * replacer[MAX_REPLACERS][2];
char * forward_to_address;
char * from_address;
void arg_err(void);
int read_command_line(int argc, char *argv[]);
char * replace_subject(int pairs);
int
main(int argc, char *argv[], char *env[])
{
  int pairs;
  program = "forward";
  read_message(argc, argv, env);
  pairs = read_command_line(argc, argv);  /* also gets addresses */
  mail_it(forward_to_address, from_address, replace_subject(pairs));
  dump_message(stdout, YES, NO, NO);
  exit(0);
}
/************************************************************
 *     Returns the number of replacement pairs and the
 *     external forward_to_address is gathered.
 *     Generates errors to stderr.
 *************************************************************/
int
read_command_line(int argc, char *argv[])
{
  int k, i;
  for (k = 0, i = 1 ; i < argc ; i++)
    {
      if (same(argv[i], "-s"))
	{
	  if (argc - i < (forward_to_address ? 4 : 5))
	    {
	      arg_err();
	    }
	  replacer[k][0] = argv[++i];
	  replacer[k++][1] = argv[++i];
	  if (k >= MAX_REPLACERS)
	    {
	      fprintf(stderr, 
		      "\nforward: Only %d replacement pairs allowed.\n",
		      MAX_REPLACERS);
	      exit(2);
	    }
	  continue;
	}
      if (same(argv[i], "-f"))
	{
	  from_address = argv[++i];
	  continue;
	}
      if (same(argv[i], "-t"))
	{
	  forward_to_address = argv[++i];
	  if (same(forward_to_address, "return"))
	    {
	      forward_to_address = from;
	    }
	  continue;
	}
    }
  if (!forward_to_address || !from_address)
    {
      arg_err();
    }
  return k;
}
/************************************************************/
void
arg_err(void)
{
  fprintf(stderr,
	  "\nforward [-s s1 s2]...-f from_address -t forward_to_address\n"
	  "If -t return, then it returns to sender.");
  exit(1);
}
/************************************************************
 *         Passes back the rewritten subject.
 *************************************************************/
char *replace(char *str, char *this_str, char *with_this);
char *
replace_subject(int pairs)
{
  int i;
  char *new_subject = subject;
  for (i = 0; i < pairs; i++)
    {
      if (replacer[i][0][0] == '*' 
	 && samen(&replacer[i][0][1], subject, strlen(&replacer[i][0][1])))
	{
	  return replace(subject, &replacer[i][0][1], replacer[i][1]);
	}
      if ((new_subject = replace(subject, replacer[i][1], replacer[i][0]))
	 != subject) /* something was replaced */
	return new_subject;
    }
  return new_subject;
}
/************************************************************
 *          Replaces the this_str string within the str
 *          with with_this.
 ************************************************************/
char
*replace(char *str, char *this_str, char *with_this)
{
  int i;
  int len_this = strlen(this_str);
  int len_str = strlen(str);
  int len_with = strlen(with_this);
  char *new_str;
  for (i = 0; i <= len_str - len_this ; i++)
    {
      if (samen(&str[i], this_str, len_this))
	{   /* hit */
	  if ((new_str = malloc(len_str + len_with - len_this + 1))
	     == NULL)
	    {
	      sprintf(error_msg,"\nCan't allocate space for new subject.\n");
	      return NULL;
	    }
	  new_str[0] = '\0';
	  strncpy(new_str, str, i);
	  new_str[i] = '\0';
	  strcat(new_str, with_this);
	  strcat(new_str, &str[i + len_this]);
	  return new_str;
	}
    }
  return str;
}
