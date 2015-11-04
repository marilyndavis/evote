/* $Id: unhex.c,v 1.3 2003/01/15 18:54:13 marilyndavis Exp $ */ 
#define TRACE "/tmp/unhex.debug"
#define PATH "/usr/local/majordomo/"
/****************************************************************
 *  unhex.c
 *   This program reads stdin and looks for hex code that comes
 *   from a mailto.  It replaces the hex code with ascii characters
 *   and puts the result on stdout.
 *   It reads the command line and assumes that argv[1] is an
 *   executable program in the same directory and it pipes stdout
 *   into that program copying all the rest of the command line
 *   arguments into that program's command line.
 *   If there is no argv[1], it just pipes to stdout.
 *   If it encounters an error, it reports the error to stderr
 *   and it places the unhexed version of the message onto 
 *   stderr.
 *   NOTE:  This program also unhexes the Subject: line.
 *   Compile:   cc -o unhex unhex.c
 *****************************************************************/ 
#include<stdio.h>
#include<errno.h>
#define KEY "hex="   /* These are the first letters in the mail
			message if the message is to be unhexed.
			Otherwise, the message is sent along
			untouched -- except the subject is still
			unhexed.
		     */
FILE* fp_out = stdout;  /* Changes to stderr on error */
FILE* fp_error = stderr; /* Changes to fptrace on TRACE */
#ifdef TRACE
FILE * fptrace;
#endif
typedef enum {NO, YES} YESorNO;
/* function declarations -- same order as definitions */
void pass_headers(void);
void pass_it(char *this, int last);
int same(char *str1, char* str2, int len);
void send(int argc, char ** argv, char ** env);
void unhex_it(void);
void unhex_string(char * input);
/*********************************************************
 *      main function for unhex.c
 **********************************************************/
int
main(int argc, char *argv[], char *env[])
{
  char check_space[sizeof(KEY) + 1];
  char key[sizeof(KEY) + 1];
  int key_len;
  int i;
#ifdef TRACE
  int cc;
  fptrace = fopen(TRACE, "w");
  fp_error = fptrace;
  fprintf(fptrace,"\n\nunhex called as:\n");
  for (cc = 0; cc < argc ; cc++)
    fprintf(fptrace," %s", argv[cc]);
  fprintf(fptrace,"\nEnvironment is:\n");
  for (cc = 0; env[cc] != NULL; cc++)
    fprintf(fptrace, "%s ", env[cc]);
  fprintf(fptrace,"\n\n");
#endif
  if (argc > 1)
    send(argc, argv, env);   /* open pipe to argv[1] */
  pass_headers();   /* pipe headers along, unhexing the subject */
  key_len = strlen(KEY);
  strncpy(key, KEY, key_len);
  for (i = 0; i < key_len; i++)
    {
      if ((check_space[i] = getchar()) == EOF 
	  || check_space[i] != key[i])
	{
	  pass_it(check_space, i);    /* doesn't need unhexing */
	}
    }
  unhex_it();
  exit(0);
}
/************************************************************
 *     Passes the characters in the headers from stdin to
 *     fp_out.
 *     It notices the Subject: field and unhexes it if necessary.
 ************************************************************/
#define LINELEN 200
#define SUBJECT "Subject:"
void
pass_headers(void)
{
  int len;
  char line[LINELEN + 1];
  int subject_len = strlen(SUBJECT);
  YESorNO subject_done = NO;
  YESorNO last_was_whole = YES;
  while ((len = mygets(line, LINELEN, stdin)) != 0)
    {
      if (!subject_done && same(line, SUBJECT, subject_len))
	{
	  subject_done = YES;
	  unhex_string(&line[subject_len]);
	}
      fputs(line, fp_out);
      if (line[0] == '\n' && last_was_whole == YES)
	break;
      if (line[len - 1] == '\n')
	last_was_whole = YES;
      else
	last_was_whole = NO;
    }
  return;
}
/*************************************************************
 *         Passes back the number of chars read so that
 *         you don't have to strlen on the result to find
 *         the end.
 ************************************************************/
int
mygets(char *buf, int max, FILE* input)
{
  int ch;
  int i = -1;
  while (++i < max)
    {
      if ((ch = getchar()) == EOF)
	{
	  if (i == 0)
	    return 0;
	  break;
	}
      buf[i] = (char)ch;
      if (buf[i] == '\n')
	{
	  break;
	}
    }
  buf[++i] = '\0';
  return i;
}
/*************************************************************
 *        Passes the message along untouched.
 ************************************************************/
void
pass_it(char *this, int last)
{
  int i;
  int ch;
  for (i = 0; i <= last; i++)
    {
      fputc(this[i], fp_out);
    }
  while ((ch = getchar()) != EOF)
    fputc(ch, fp_out);
  exit(0);
}
/************************************************************
 *       Case insensitive string compare for len chars.
 ************************************************************/
#define TOUPPER(X) (((X) <= 'z'&& (X) >= 'a') ? (X) + 'A' - 'a' : (X))
int
same(char *str1, char* str2, int len)
{
  int i;
  for (i = 0; i < len; i++)
    {
      if (TOUPPER(str1[i]) != TOUPPER(str2[i]))
	return NO;
    }
  return YES;
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
      send_err(argv[1], "\nCan fork a new process.\n");
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
	  fprintf(fp_error, "\nUnhex: Child: Can't close pipe's extra descriptors.  Continuing.\n");
	}
      if ((path = malloc(strlen(PATH) + strlen(argv[1]) + 2))
	  == NULL)
	{
	  send_err(argv[1],  "\nCan't malloc space for path.\n");
	}
      strcpy(path, PATH);
      if (path[strlen(PATH)] -1 != '/')
	strcat(path,"/");
      strcat(path, argv[1]);
#ifdef TRACE
      fprintf(fptrace,"\nexecve called on %s, args are :\n", 
	      path);
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
  fprintf(fp_error,"\nUnhex: Failed to pipe to %s.\n", program);
  fprintf(fp_error, error_msg);
  fprintf(fp_error,"\nLost message follows:\n");
}
/*************************************************************
 *     Reads stdin and places it on fp_out converting hex to
 *     ascii as it goes.
 ************************************************************/
#define IS_HEX(X) (  ( ((X) <= '9' && (X) >= '0') || \
     ((X) <= 'F' && (X) >= 'A') \
   || ((X) <= 'f' && (X) >= 'a')) ? 1 : 0)
#define HEXNUM(X)( ((X) <= '9' && (X) >= '0') ? ((X) - '0') \
	   : ((X) <= 'F' && (X) >= 'A') ? ((X) - 'A' + 10) \
	   : ((X) <= 'f' && (X) >= 'a') ? ((X) - 'a' + 10): -1 )
void
unhex_it(void)
{
  int input;
  char new_char[3];
  int ch[3];
  while ((input = getchar()) != EOF)
    {
      switch (input)
	{
	case '+':
	  fputc(' ', fp_out);
	  break;
	case '%':
	  ch[0] = getchar();
	  ch[1] = getchar();
	  new_char[0] = HEXNUM(ch[0]);
	  new_char[1] = HEXNUM(ch[1]);
	  new_char[2] = new_char[0] * 16 + new_char[1];
	  if (new_char[2] != '\r')
	    fputc(new_char[2], fp_out);
	  break;
	default:
	  fputc(input, fp_out);
	  break;
	}
    }
}
/************************************************************
 *         Looks for hex sequences in the string and
 *         replaces them with regular ascii.
 ************************************************************/
void
unhex_string(char * input)
{
  int in_len = strlen(input);
  int i, j;
  YESorNO do_it = NO;
  int check;
  if (in_len == 0)
    return;
  for (i = 0; input[i+2] != '\0' ; i++)
    {
      if (input[i] == '%' && IS_HEX(input[i + 1]) && IS_HEX(input[i+2]))
	{
	  do_it = YES;
	  break;
	}
    }
  if (do_it == NO)
    return;
  for (i = 0, j= 0; input[i]; i++, j++)
    {
      if (input[i] != '%'
	  || (input[i+1] != '\0' && !IS_HEX(input[i+1]) 
	      && input[i+2] != '\0' && !IS_HEX(input[i+2])))
	{
	  input[j] = input[i];
	  continue;
	}
      check = HEXNUM(input[i+1]) * 16 + HEXNUM(input[i+2]);
      input[j] = check;
      i += 2;
    }
  input[j] = '\0';
  return;
}
