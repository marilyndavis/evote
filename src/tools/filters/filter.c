/* $Id: filter.c,v 1.9 2003/10/20 17:07:34 marilyndavis Exp $ */ 
/**********************************************************
 *  filter.c -- functions for filtering mail.
 *********************************************************
 **********************************************************/
#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include "filter.h"
char * filter_version = "1.0";
/* #define TRACE "/tmp/eVote.child.trace"  traces child process */
#ifdef TRACE
FILE * fptrace;
#endif
/***   HEADER DATA ***/
char * authen = NULL;
int authen_start = -1;
char * cc_address = NULL;
int cc_start = -1;
int date_start = -1;
char * for_address = NULL;
int from_start = -1;
char * from = NULL;
int message_id_start = -1;
int received_start = -1;
char * reply_to = NULL;
int reply_to_start = -1;
char * return_path = NULL;
int return_path_start = -1;
char * sender = NULL;
int sender_start = -1;
char * subject = NULL;
int subject_start = -1;
char * to_address = NULL;
int to_start = -1;
typedef	struct head_def 
{
  char * header;
  int * p_var;
  int len;
  char * * name;
  char * (*fn)(int, int*);
} HEAD;
HEAD header[] =
{
  {"From:", &from_start, 5, &from, get_guy},
  {"cc:", &cc_start, 3, &cc_address, get_guy},
  {"Date:", &date_start, 5, NULL, NULL},
  {"To:", &to_start, 3, &to_address, get_guy},
  {"Received:", &received_start, 9, NULL, NULL},
  {"Reply-To:", &reply_to_start, 9, &reply_to, get_guy},
  {"Resend-Sender:", &sender_start, 14, &sender, copy_line},
  {"Return-Path:", &return_path_start, 12, &return_path, get_guy},
  {"Sender:", &sender_start, 7, &sender, get_guy},
  {"Subject:", &subject_start, 8, &subject, copy_line},
  {"X-Sender:", &sender_start, 9, &sender, get_guy},
  {"X-Authentication-Warning:", &authen_start, 25, &authen, copy_line}
};
typedef struct
{
  char * name;
  int len;
  int at;
}GOOD_LIST;
GOOD_LIST * good_list;
int no_good_lists = -2;
typedef GOOD_LIST FORGET;
FORGET * forget;
int no_forgets = -2;
typedef struct
{
  char * name;
  int len;
}BAD_GUY;
BAD_GUY * bad_guy;
int no_bad_guys = -2;
int bad_guy_on = -1;
/* default paths to goodlists and badguys */
char * good_lists_fname = "/usr/local/majordomo/goodlists";
char * bad_guys_fname = "/usr/local/majordomo/badguys";
char * forgets_fname = "/usr/local/majordomo/forgets";
char * sendmail_command = "sendmail -t";
char * sendmail_path = "/usr/lib/sendmail";
char * whereami = "deliberate.com";
char * eVote_mail_to = "eVote-owner";
char * alert_address = "majordom";
char * wrapper_path = "/usr/local/majordomo";
char * log_fname = "/tmp/filters.log";
int log_level = 1;
/* local things */
#define LEN 10
#define LIST_BLOCK 10
#define BLOCK 200
static YESorNO capture_header(int start, YESorNO for_filter);
static YESorNO recognize(void);
static void get_good_lists(void);
static void get_bad_guys(void);
static YESorNO is_match(char *str, char *target);
static YESorNO is_us(char *str);
static void logit(char *msg, YESorNO log_error);
static YESorNO match_forgets(char * str);
static YESorNO match_good_list(char * str);
static void process_received(int start, YESorNO for_filter);
void process_signal(int signo);
static void set_up(int argc, char * argv[], char * env[]);
static YESorNO still_a_header(char *);
static void unsub_other_lists(char * for_whom);
static void unsub(char * list_name, char * where, char *from);
static void write_goodlists(char * list_name, char * where, char * from_str);
char * buffer;
int buff_up(void);
char * program;
int bytes;
char error_msg[MAX_ERROR + 1];
char * alias_address = NULL;
int msg_start = -1;
static char **env_copy;
YESorNO build = NO;  /* set in set_up and used by unsub */
char * first_token;
int time_out = 20;
/************************************************************
 *    Sends an emergency message.
 ************************************************************/
void
alert(char *msg, YESorNO log_error_msg)
{
  char from_address[200];
  FILE *fp;
  if (alert_address[0] == '\0')
    {
      fp = stderr;
    }
  else
    {
      fp = stdout;
      sprintf(from_address, "owner-%s@%s", program, whereami);
      mail_it(alert_address, from_address, msg);
    }
  fprintf(fp," %s: %s", program, error_msg);  
  dump_message(fp, NO, NO, YES);
  logit(msg, log_error_msg);
}
/************************************************************
 *    Reads from standard in to the buffer.
 *    Returns bytes read or -1 on error.
 *    Can be called repeatedly to read another file from stdin.
 *************************************************************/
int
buff_up(void)
{
  int i, ch;
  static int in_space = 0;
  authen_start = -1;  /* initialize or reinitialize globals */
  date_start = -1;    /* except for subject_start because we */
  from_start = -1;    /* want to keep the first subject */
  message_id_start = -1;
  received_start = -1;
  reply_to_start = -1;
  return_path_start = -1;
  sender_start = -1;
  to_start = -1;
  msg_start = -1;
  if (buffer == NULL 
      && (buffer = malloc((in_space = BLOCK) * sizeof(char)))
      == NULL)
    {
      fprintf(stderr,"\nCan't allocate space for buffer.\n");
      if (error_msg[0] == '\0')
	{
	  sprintf(error_msg, "\n%s is out of resources right now."
		  "\nIt can't find space to store a message for %s.\n",
		  whereami, program);
	}
      return -1;
    }
  for (i = 0; ch != EOF; i++)
    {
      buffer[i] = ((ch = getchar()) == EOF ? '\0' : ch);
      /* check for space */
      if (ch != EOF && i + 1 >= in_space)
	{
	  char * new_space;
	  if ((new_space = realloc(buffer, 
					  (in_space += BLOCK) * sizeof(char))) 
	      == NULL)
	    {
	      fprintf(stderr,"\nCan't allocate more buffer.\n");
	      strcat(error_msg, "\nWe're out of resources right now."
		     "\nPlease try your eVote command again later.\n");
	      return -1;
	    }
	  buffer = new_space;
	}
    }
  return i;
}
/************************************************************
 *  Tries to find the line in the header list.  Sets the
 *  appropriate variable at p_var to i;
 *  Special processing for Received: headers.  This collects
 *  each of them and processes each through process_received.
 *******************************************************************/
YESorNO
capture_header(int start, YESorNO for_filter)
{
  static int no_headers = sizeof(header)/sizeof(HEAD);
  int i, dummy;
  static int last_start = -5;
  if (start == last_start)
    return NO;
  for (i = 0; i < no_headers; i++)
    {
      if (samen(header[i].header, &buffer[start], header[i].len))
	{
	  if (header[i].p_var != NULL 
	      && (*header[i].p_var == -1 
		  || samen(header[i].header, "Received:", 9)))
	    {
	      *header[i].p_var = start + header[i].len;
	      if (header[i].name)
		{
		  *header[i].name = header[i].fn(start + header[i].len, 
						 &dummy);
		}
	      if (!samen(header[i].header, "Received:", 9))
		return YES;
	      process_received(received_start, for_filter);
	      return YES;
	    }
	}
    }
  return NO;
}
/************************************************************
 *          returns malloced space with copy in it.
 *          Starts copying at buffer[start].
 *          Copies to the '\n' but leaves space to tack on a
 *          '\n' if needed by the caller.
 *************************************************************/
char *
copy_line(int start, int * ends)
{
  int i = start;
  int j;
  int fspace = LEN;
  char * new_fspace;
  char * line;
  if ((line = malloc(LEN)) == NULL)
    {
      fprintf(stderr, "\nCan't allocate space for reading header.\n");
      return NULL;
    }
  while (buffer[i] == ' ')
    i++;
  start = i;
  *ends = start;
  for (j = 0; buffer[i]; i++, j++)
    {
      if (((buffer[i] == '\n' || buffer[i] == '\r')
	   && buffer[i+1] != ' ' && buffer[i+1] != '\t')
	  || buffer[i] == '\0')
	break;
      if (j + 2 >= fspace)
	{
	  if ((new_fspace = realloc(line, fspace += LEN)) 
	      == NULL)
	    {
	      fprintf(stderr, "\nCan't allocate enough in_space for 'line'\n");
	      return NULL;
	    }
	  line = new_fspace;
	}
      line[j] = buffer[i];
    }
  line[j] = '\0';
  *ends = i;
  return line;
}
/**********************************************************
 *     Manufactures an email-style date.  If when == 0
 *     now is returned.
 **********************************************************/
char*
date_str(time_t when)
{
  static char where[80] = "";
  struct tm *time_struct;
  static time_t now;
  if (now != 0 && when == 0)  /* asking for now and already have it */
    return where;
  if (when == 0)
    {
      now = time(NULL);
      time_struct = localtime(&now);
    }
  else
    {
      now = 0;
      time_struct = localtime(&when);
    }
  strftime(where, 80, "%a, %e %b %Y %T %z\n", time_struct);
  return where;
}
/************************************************************
 *  Expects there to be a message in error_msg
 ************************************************************/
void
dump_bad(void)
{
  char msg[400];
  sprintf(msg,"Dropping junk for %s.", for_address ? for_address : "no one");
  alert(msg, YES);
}
/************************************************************
 *   Dumps the incoming message.
 *************************************************************/
void
dump_message(FILE * fp, YESorNO skip_headers, 
	     YESorNO skip_first_token, YESorNO provide_intro)
{
  int i, ch;
  YESorNO started = NO;
  if (provide_intro)
    {
      fprintf(fp,"  --- \n");
      fprintf(fp,"  Incoming Message \n");
      fprintf(fp,"  --- \n\n>");
    } 
  if (buffer == NULL)
    {
      while ((ch = getchar()) != EOF)
	{
	  fputc(ch, fp);
	}
      return;
    }
  i = (skip_headers ? msg_start : 0) ;
  if (skip_first_token)
    {
      for (i = msg_start ; buffer[i] ; i++)
	{
	  if (!started && (buffer[i] == ' ' || buffer[i] == '\n'
			   || buffer[i] == '\t'))
	    continue;
	  started = YES;
	  if (buffer[i] == ' ' || buffer[i] == '\n' 
	      || buffer[i] == '\t')
	    break;
	}
    }
  for (; buffer[i]; i++)
    {
      fputc(buffer[i], fp);
    }
}
/************************************************************
 *     Forks a child and makes the stdout of the child come
 *     into the stdin of the parent.  It returns YES if the
 *     process is the parent, NO if it is the child.
 *************************************************************/
YESorNO
fork_is_parent(void)
{
  int pfd[2];
  int send_pid;
  FILE * fperr = stderr;
  if (pipe(pfd) == -1)
    {
      fprintf(stderr,"%s", error_msg);
      fprintf(stderr, "\nCan't open pipe.  Message lost.\n");
      dump_message(stderr, NO, NO, YES);
      exit(1);
    }
  switch (send_pid = fork())
    {
    case -1:
      fprintf(stderr,"%s", error_msg);
      fprintf(stderr, "\nCan't fork process.  Message lost.\n");
      dump_message(stderr, NO, NO, YES);
      exit(1);
    case 0:  /* child process */
#ifdef TRACE
      if ((fptrace = fopen(TRACE, "a")) == NULL)
	{
	  fprintf(fperr,"\nCan't open trace file for child: %s\n",
		  TRACE);
	}
      else
	{
	  fperr = fptrace; 
	  fprintf(fperr,"\nTrace started, close(0) next\n");
	  fflush(fptrace);
	}
#endif
      if (close(0) == -1)   /* close stdin */
	{
	  fprintf(stderr,"\nAlready closed.\n");
	  fprintf(fperr,"%s", error_msg);
	  fprintf(fperr, "\nChild: Can't close stdin.  Message lost.\n");
	  dump_message(fperr, NO, NO, YES);
	  fflush(fperr);
	  exit(2); 
	}
#ifdef TRACE
      fprintf(fperr, "\nStdin closed. About to dup(pfd[0])\n");
      fflush(fperr);
#endif
      if (dup(pfd[0]) != 0)  /* pipe input is stdin */
	{
	  fprintf(fperr,"%s", error_msg);
	  fflush(fperr);
	  fprintf(fperr, "\nChild: Can't dup pipe reading to stdin.  Errno = %d. Message lost.\n", errno);
	  fprintf(fperr, "\nIf errno == 9,  don't close(0) before calling fork_is_parent()\n");
	  dump_message(fperr, NO, NO, YES);
	  fflush(fperr);
	  exit(2);
	}
#ifdef TRACE
      fprintf(fperr, "\ndup succeeded. Next close(pfd[0]) and pfd[1])\n");
      fflush(fperr);
#endif
      if (close(pfd[0]) == -1 || close(pfd[1]) == -1)
	{
	  fprintf(fperr, "\nChild: Can't close pipe's extra descriptors.  Continuing.\n");
	}
#ifdef TRACE
      fprintf(fperr,"\nAll done with fork_is_parent. Returning child.\n");
      fflush(fperr);
      fclose(fperr);
#endif      
      return NO;
      break;
    default:  /* parent */
      break;
    }
  /* now we're the parent */
  if (close(1) == -1)   /* close stdout */
    {
      fprintf(stderr,"%s", error_msg);
      fprintf(stderr, "\nParent: Can't close stdout.  Message lost.\n");
      dump_message(stderr, NO, NO, YES);
      exit(3);
    }
  if (dup(pfd[1]) != 1)  /* pipe output is stdout */
    {
      fprintf(stderr,"%s", error_msg);
      fprintf(stderr, "\nParent: Can't dup pipe writing to stdout.  Message lost.\n");
      dump_message(stderr, NO, NO, YES);
      exit(3);
    }
  if (close(pfd[0]) == -1 || close(pfd[1]) == -1)
    {
      fprintf(stderr,"%s", error_msg);
      fprintf(stderr, "\nParent: Can't close pipe's extra descriptors.  Continuing.\n");
    }
  return YES;
}
/************************************************************
 *    Reads the list of bad guys.
 *************************************************************/
void
get_bad_guys(void)
{
  FILE *fp;
  int no = -1, space = 0;
  char line[200];
  if (no_bad_guys > -2)
    return;
  no_bad_guys = -1;
  if ((fp = fopen(bad_guys_fname, "r")) == NULL)
    return;
  if ((bad_guy = malloc((space = LIST_BLOCK) * sizeof(BAD_GUY)))
      == NULL)
    return;
  while (fgets(line, 200, fp) != NULL)
    {
      if (line[0] == '\n' || line[0] == '#')
	continue;
      if (no+1 >= space)
	{
	  if ((bad_guy = realloc(bad_guy,
					   (space += LIST_BLOCK) * sizeof(BAD_GUY)) )
	      == NULL)
	    return;
	}
      bad_guy[++no].len = strlen(line) - 1;
      bad_guy[no].name = malloc(bad_guy[no].len + 1);
      line[bad_guy[no].len ] = '\0';  /* lop off '\n' */
      strcpy(bad_guy[no].name, line);
    }			
  no_bad_guys = no +1;
}
/************************************************************/
char *
get_first_token(void)
{
  int i;
  int dummy;
  for (i = msg_start; buffer[i]; i++)
    {
      if (buffer[i] != ' ' && buffer[i] != '\t' 
	  && buffer[i] != '\n')
	break;
    }
  first_token = copy_line(i, &dummy);
  for (i = 0; first_token[i]; i++)
    {
      if (first_token[i] == ' ' || first_token[i] == '\t')
	{
	  first_token[i] = '\0';
	  break;
	}
    }
  return first_token;
}
/************************************************************
 *    Reads the list of forget addresses
 *************************************************************/
void
get_forgets(void)
{
  FILE *fp;
  int no = -1, space = 0, i;
  char line[200];
  if (no_forgets > -2)
    return;
  no_forgets = -1;
  if ((fp = fopen(forgets_fname, "r")) == NULL)
    {
      sprintf(error_msg,"\nCan't open %s.  Error no: %d.\n", forgets_fname,
	      errno);
      return;
    }
  if ((forget = malloc((space = LIST_BLOCK) * sizeof(FORGET)))
      == NULL)
    return;
  while (fgets(line, 200, fp) != NULL)
    {
      if (line[0] == '\n' || line[0] == '#')
	continue;
      if (no+1 >= space)
	{
	  if ((forget = realloc(forget, (space += LIST_BLOCK) 
					 * sizeof(FORGET)))
	      == NULL)
	    return;
	}
      forget[++no].len = strlen(line) - 1;
      line[forget[no].len] = '\0'; /* lop off '\n' */
      forget[no].name = malloc(forget[no].len + 1);
      strcpy(forget[no].name, line);
      for (i = 0; i < forget[no].len; i++)
	{
	  if (forget[no].name[i] == '@')
	    break;
	}
      if (forget[no].name[i] != '@')
	{
	  fprintf(stderr,"\nTrouble in %s in %s.\nSkipping.\n",
		  forget[no].name, forgets_fname);
	  free(forget[no].name);
	  no--;
	  continue;
	}
      forget[no].at = i;
    }			
  no_forgets = no + 1;
}
/************************************************************
 *    Reads the list of good lists from the file.
 *************************************************************/
void
get_good_lists(void)
{
  FILE *fp;
  int no = -1, space = 0, i;
  char line[200];
  if (no_good_lists > -2)
    return;
  no_good_lists = -1;
  if ((fp = fopen(good_lists_fname, "r")) == NULL)
    {
      sprintf(error_msg,"\nCan't open %s.  Error no: %d.\n", good_lists_fname,
	      errno);
      return;
    }
  if ((good_list = malloc((space = LIST_BLOCK) * sizeof(GOOD_LIST)))
      == NULL)
    return;
  while (fgets(line, 200, fp) != NULL)
    {
      if (line[0] == '\n' || line[0] == '#')
	continue;
      if (no+1 >= space)
	{
	  if ((good_list =realloc(good_list,
					      (space += LIST_BLOCK) 
					      * sizeof(GOOD_LIST)))
	      == NULL)
	    return;
	}
      good_list[++no].len = strlen(line) - 1;
      line[good_list[no].len] = '\0'; /* lop off '\n' */
      good_list[no].name = malloc(good_list[no].len + 1);
      strcpy(good_list[no].name, line);
      for (i = 0; i < good_list[no].len; i++)
	{
	  if (good_list[no].name[i] == '@')
	    break;
	}
      if (good_list[no].name[i] != '@')
	{
	  fprintf(stderr,"\nTrouble in %s in %s.\nSkipping.\n",
		  good_list[no].name, good_lists_fname);
	  free(good_list[no].name);
	  no--;
	  continue;
	}
      good_list[no].at = i;
    }			
  no_good_lists = no + 1;
}
/************************************************************
 *      Finds the email address on the line.
 *************************************************************/
char *
get_guy(int start, int * ends)
{
  int i = start;
  int end;
  int j;
  int fspace = LEN;
  char * new_fspace;
  char * person;
  YESorNO quoting = NO;
  YESorNO ended = NO;
  *ends = start;
  if ((person = malloc(LEN)) == NULL)
    {
      fprintf(stderr, "\nCan't allocate in_space for 'person' header.\n");
      return NULL;
    }
  while (buffer[i] == ' ')
    i++;
  start = i;
  for (; buffer[i]; i++)
    {
      switch (buffer[i])
	{
	case '\0':
	  break;
	case '\n':
	case '\r':
	  if (buffer[i+1] == ' ' || buffer[i+1] == '\t')
	    continue;
	  ended = YES;
	  break;
	case '"':
	  quoting = (quoting == YES ? NO : YES);
	  break;
	case '<':
	  if (quoting)
	    break;
	  j = 0;
	  while (buffer[++i] != '>')
	    {
	      if (buffer[i] == '\"' ||
		  buffer[i] == '\n' || buffer[i] == '\r' ||
		  buffer[i] == '\0'|| buffer[i] == '\0')
		{
		  fprintf(stderr, "\nCan't figure out 'Person'.\n");
		  return NULL;
		}
	      if (j + 1 >= fspace)
		{
		  if ((new_fspace = realloc(person, fspace += LEN)) 
		      == NULL)
		    {
		      fprintf(stderr, "\nCan't allocate enough in_space for 'person'\n");
		      return NULL;
		    }
		  person = new_fspace;
		}
	      person[j++] = buffer[i];
	    }
	  person[j] = '\0';
	  *ends = i;
	  return person;
	case '@':
	  if (quoting)
	    break;
	  j = 0;
	  while (buffer[i] != ' ' && buffer[i] != '\n' 
		 && buffer[i] != '\t' && i >= start)
	    i--;
	  if (i == 0)
	    {
	      fprintf(stderr, "\nCan't figure out 'Person'.\n");
	      return NULL;
	    }
	  for (++i; buffer[i]; i++)
	    {
	      if (buffer[i] == ' ' || buffer[i] == '\t'|| buffer[i] == ','
		  || buffer[i] == '\n' || buffer[i] == '\r' || buffer[i] == ';')
		break;
	      if (buffer[i] == '\'' || buffer[i] == '"')
		{
		  fprintf(stderr, "\nCan't figure out 'Person'.\n");
		  return NULL;
		}
	      if (j + 1 >= fspace)
		{
		  if ((new_fspace = realloc(person, fspace += LEN)) 
		      == NULL)
		    {
		      fprintf(stderr, "\nCan't allocate enough in_space for 'person'.\n");
		      return NULL;
		    }
		  person = new_fspace;
		}
	      person[j++] = buffer[i]; 
	    }
	  person[j] = '\0';
	  *ends = i;
	  return person;
	case '\\':
	  if (buffer[i+1] == '"')
	    i++;
	  break;
	default:
	  break;
	}
      if (ended)
	break;
    }
  /* start over */
  end = i;
  j = 0;
  i = start - 1;
  while (++i < end && buffer[i] != ' ' && buffer[i] != ';')
    {
      if (j + 1 >= fspace)
	{
	  if ((new_fspace = realloc(person, fspace += LEN)) 
	      == NULL)
	    {
	      fprintf(stderr, "\nCan't allocate enough in_space for 'person'.\n");
	      return NULL;
	    }
	  person = new_fspace;
	}
      if (buffer[i] == '\'' || buffer[i] == '"')
	{
	  fprintf(stderr, "\nCan't figure 'person'.\n");
	  return NULL;
	}
      person[j++] = buffer[i];
    }
  person[j] = '\0';
  *ends = i;
  if (j == 0)
    return NULL;
  return person;
}
/************************************************************
 *     This is called repeatedly to get all the addresses,
 *     one at a time, from a header.
 ************************************************************/
char *
get_multi_address(int start, YESorNO init)
{
  static int i;
  if (init)
    {
      i = start;
    }
  for (; buffer[i]; i++)
    {
      switch (buffer[i])
	{
	case '\t':
	case ',':
	case ' ':
	  continue;
	  break;
	case '\n':
	  if (buffer[i+1] == '\t'
	      || buffer[i+1] == ' ')
	    continue;
	  return NULL;
	  break;
	default:
	  return get_guy(i, &i);
	  break;
	}
    }
  return NULL;
}
/************************************************************/
YESorNO
is_match(char *str, char *target)
{
  int i, k, l;
  int at;
  int len;
  if (!str || !target)
    return NO;
  len = strlen(target);
  for (at = 0; target[at] != '@' && target[at] ; at++)
    ;
  if (target[at] != '@')
    return NO;
  for (i = 0; str[i]; i++)
    {
      if (str[i] != '@')
	continue;
      if (samen(&str[i+1], &target[at + 1], len - at - 2))
	{ /* possible -- same site */
	  for (k = i-1, l = at - 1; l >= 0; l--, k--)
	    {
	      if (!SAME_LETTER(str[k], target[l]))
		{
		  break;
		}
	    }
	  return YES;
	}
    }
  return NO;
}
/************************************************************
 *  Checks if whereami is in the string.  whereami is initialized
 *  in set_up(...)
 *************************************************************/
YESorNO
is_us(char *str)
{
  int i;
  int len = strlen(whereami);
  for (i = 0; str[i]; i++)
    {
      if (str[i] != '@')
	continue;
      if (samen(&str[i+1], whereami, len))
	return YES;
    }
  return NO;
}
/**********************************************************
 *    Mallocs some space for a copy of string, copies
 *    and returns a pointer to the copy.
 *********************************************************/
char *
keep_string(char *str)
{
  char *where;
  if ((where = malloc(strlen(str)+1)) == NULL)
    return NULL;
  strcpy(where, str);
  return where;
}
/************************************************************/
void
logit(char *msg, YESorNO log_error)
{
  FILE *fp;
  char buf[400];
  static YESorNO done = NO;
  if (done)
    exit(0);
  done = YES;
  if ((fp = fopen(log_fname,"a")) != NULL)
    {
      fprintf(fp,"%s: %s %s\n", program, date_str(0), msg);
      if (log_error && error_msg)
	fprintf(fp,"%s",	error_msg);
      fclose(fp);
      exit(0);
    }
  sprintf(buf,"\nCan't open log file: %s.\n", log_fname);
  alert(buf, NO);
}
/*************************************************************
 *       Driver for fence and angel.  Only returns if the
 *       message looks good.
 *************************************************************/
void
look_for_trouble(int argc, char *argv[], char *env[])
{
  char msg[400];
  int i;
  /* extra args for fence and angel */
  if (!same(program, "fence") && !same(program, "angel"))
    {
      alert("Look for trouble called by the wrong program", NO);
    }
  while (argv[1] && argv[1][0] == '-')
    {
      switch (argv[1][1])
	{
	case 'b':
	case 'B':
	  build = YES;
	  break;
	case 'a':
	case 'A':
	  if ((alias_address = keep_string(argv[2])) == NULL)
	    {
	      alert("No space to keep alias", NO);
	    }
	  for (i = 1; i < argc; i++)
	    {
	      argv[i] = argv[i+1];
	    }
	  argc--;
	  break;
	default:
	  alert("Unknown argument", NO);
	  break;
	}
      for (i = 1; i < argc; i++)
	{
	  argv[i] = argv[i+1];
	}
      argc--;
    }  
  if (same(program, "fence") && alias_address == NULL)
    {
      alert("-a alias_address is needed by fence", NO);
    }
  read_message(argc, argv, env);
  if (for_address)
    {
      if (match_forgets(for_address))
	exit(0);
    }
  if (sender)
    {
      if ((to_address && same(sender, to_address))
	  || (for_address && same(sender, for_address)))
	{
	  sprintf(error_msg, 
		  "sender is the same as the to address: %s", to_address);
	  dump_bad();
	}
      else
	{
	  if(match_good_list(sender) || is_us(sender))
	    return;
	  if(match_forgets(sender))
	    {
	      sprintf(msg,"%s forgotten for %s", sender, for_address);
	      logit(msg, NO);  /* exits */
	    }
	}
    }
  if (from)
    {
      if ((to_address && same(from, to_address))
	  || (for_address && same(from, for_address)))
	{
	  sprintf(error_msg, 
		  "from is the same as the to address: %s", to_address);
	  dump_bad();
	}
      else
	{
	  if (match_good_list(from))
	    return;
	  if (match_forgets(from))
	    {
	      sprintf(msg, "%s forgotten for %s", from, for_address);
	      logit(msg, NO);  /* exits */
	    }
	}
    }
  if (subject)
    {
      if (samen(subject, "Confirm subscribe", 17))
	{
	  alert("Confirm ignored.", NO);
	}
      if (samen(subject, "Majordomo results:", 18))
	{
	  alert("Majordomo results ignored", NO);
	}
    }
  unsub_other_lists((same(program,"fence") ? alias_address : for_address));
  if (recognize())  /* looks for a familiar address in the
		       To: Cc: and Sender: fields */
    {
      return;
    }
  dump_bad();
}
/**********************************************************
 *    This forks and execs a process to sendmail as given
 *        in the makefile.
 *        The parent's stdout is attached to the child's
 *        stdin.
 ***********************************************************/
#define MAX_ARGS 20
void
mail_it(char * mailto, char * from_address, char * subject)
{
  int i, k;
  char *sendarg[MAX_ARGS];
  char *command_copy;
  if ((command_copy = malloc(strlen(sendmail_command) + 1))
      == NULL)
    {
      fprintf(stderr, "\nCan't malloc space in mail_to:\nMessage lost:\n");
      dump_message(stderr, NO, NO, YES);
      exit(5);
    }
  strcpy(command_copy, sendmail_command);
  sendarg[0] = command_copy;
  for (k = 0, i= 0; command_copy[i]; i++)
    {
      if (IS_WHITE(command_copy[i]))
	{
	  command_copy[i] = '\0';
	  while (command_copy[++i] 
		 && IS_WHITE(command_copy[i]))
	    {}
	  if (command_copy[i])  /* found another arg */
	    {
	      sendarg[++k] = &command_copy[i];
	    }
	  else  /* it ended */
	    {
	      break;
	    }
	}
    }
  sendarg[++k] = NULL;
  if (same(mailto, "test") || fork_is_parent())
    {
      printf("From: %s", from_address);
      printf("\nReply-To: %s", from_address);
      printf("\nTo: %s", mailto);
      printf("\nSubject: %s", subject);
      printf("\n\n");
      return;
    }  
  /* child does this */
  execve(sendmail_path, sendarg, env_copy);  
  fprintf(stderr,"\nERROR = %d\n", errno);
  fprintf(stderr,"%s", error_msg);
  fprintf(stderr,"\nExec failed. Message lost.\n");
  fprintf(stderr,"\nCall was \"%s, %s\".\n", 
	  sendmail_path, command_copy);
  /*  for (i = 0; argv_copy[i] != NULL; i++)
      fprintf(stderr," %s", argv_copy[i]); */
  fprintf(stderr,"\n");
  dump_message(stderr, NO, NO, YES);
  exit(4);
}
/************************************************************
 *   Checks the string to see if it matches any of the good
 *   lists in the good_lists file.
 *************************************************************/
YESorNO
match_good_list(char * str)
{
  int i, j, k, l;
  if (no_good_lists == -2)
    get_good_lists();
  for (i = 0; str[i]; i++)
    {
      if (str[i] != '@')
	continue;
      for (j = 0; j < no_good_lists; j++)
	{
	  if (samen(&str[i+1], &good_list[j].name[good_list[j].at + 1],
		    good_list[j].len - good_list[j].at - 1))
	    { /* possible -- same site */
	      for (k = i-1, l = good_list[j].at - 1; l >= 0; l--, k--)
		{
		  if (!SAME_LETTER(str[k], good_list[j].name[l]))
		    {
		      break;
		    }
		}
	      if (l < 0)  /* match */
		return YES;
	    }
	}
    }
  return NO;
}
/************************************************************
 *   Checks the string to see if it matches any of the good
 *   lists in the forgets file.
 *************************************************************/
YESorNO
match_forgets(char * str)
{
  int  j;
  if (no_forgets == -2)
    get_forgets();
  for (j = 0; j < no_forgets; j++)
    {
      if (is_match(forget[j].name, str))
	return YES;
    }
  return NO;
}
/*************************************************************
 *      The starts of the significant headers are noted.
 *      The "Received:" headers are checked as they come in.
 *      buffer[msg_start] is the beginning of the real
 *      message -- just past the headers.
 ************************************************************/
void
parse_message(YESorNO for_filter)
{
  int line_start = 0;
  YESorNO done = NO;
  YESorNO blank_so_far = NO;
  YESorNO msg_started = NO;
  int i;
  for (msg_start = 0, i = 0; !done && buffer[i]; i++)
    {
      switch (buffer[i])
	{
	case ':':
	  if (msg_start > 0)
	    break;
	  /* if it's a Received header, it collects the "for"
	     and checks for bad guys */
	  capture_header(line_start, for_filter);
	  break;
	case '\n':
	case '\r':
	  /* look for a continued line */
	  if (buffer[++i]) /*(ch = getchar()) != EOF)*/
	    {
	      i--;
	      if (buffer[i] == ' ' || buffer[i] == '\t')
		break;
	    }
	  line_start = i + 1;
	  if (blank_so_far == YES)  /* last line was blank */
	    {
	      msg_start = line_start;
	      while (buffer[msg_start] == '\0')
		msg_start--;
	    }
	  blank_so_far = YES;
	  if (msg_start > 0 && msg_started == YES)
	    {
	      if (still_a_header(&buffer[msg_start]))
		{
		  msg_started = NO;
		  msg_start = 0;
		  break;
		}
	      done = YES;
	    }
	  break;
	case EOF:
	  done = YES;
	  break;
	case '\t':
	case ' ':
	case '-':
	  break;
	default:
	  blank_so_far = NO;
	  if (msg_start > 0)
	    {
	      msg_start = line_start;
	      msg_started = YES;
	    }
	  break;
	}
    }
}	
/************************************************************
 *   looks through the header for the for_address.  If
 *   it's for_filter, it also looks for
 *   any strings that match the bad_guys listed in the 
 *   bad_guys file.
 *************************************************************/
void
process_received(int start, YESorNO for_filter)
{
  int i = start;
  int j;
  int dummy;
  for (i = start; buffer[i]; i++)
    {
      if (((buffer[i] == '\n' || buffer[i] == '\r')
	   && (buffer[i+1] != ' ' && buffer[i+1] != '\t'))
	  || buffer[i] == '\0')
	break;
      if (program && strncmp(&buffer[i], "for ", 4) == 0)
	{
	  if ((for_address = get_guy(i + 4, &dummy)) != NULL
	      && strncmp(for_address, program, strlen(program)) == 0)
	    {
	      for_address = NULL;
	      break;
	    }
	  break;
	}
    }
  if (!for_filter)
    return;
  if (no_bad_guys == -2)
    get_bad_guys();
  for (i = start; buffer[i]; i++)
    {
      if (((buffer[i] == '\n' || buffer[i] == '\r')
	   && (buffer[i+1] != ' ' && buffer[i+1] != '\t'))
	  || buffer[i] == '\0')
	return;
      for (j = 0 ; j < no_bad_guys; j++)
	{
	  if (strncmp(bad_guy[j].name, &buffer[i], bad_guy[j].len) == 0)
	    {
	      char msg[400];
	      sprintf(msg, "%s attacking %s", 
		      bad_guy[bad_guy_on].name, for_address);
	      if (match_forgets(bad_guy[j].name))
		logit(msg, NO);
	      alert(msg, NO);
	    }
	}
    }
  return;
}
/********************************************************
 *     Caught signals come here.
 *************************************************************/
void
process_signal(int signo)
{
  static int count = 0;
  char *signame;
  char buf[500];
  if (++count > 1)
    exit(1);
  signal(signo, process_signal);
  signame = set_signal_name(signo);
  sprintf(error_msg, "Signal %d:%s received.\n", signo, signame);
  fprintf(stderr, "Signal %d:%s received.\n", signo, signame);
  dump_message(stderr, NO, NO, NO);
  sprintf(buf,"%s: Signal %d:%s received.", program, signo, signame);
  alert(buf, NO);
  exit(1);
}
/************************************************************
 *  void read_message(...)    Called by shelter and forward and pick
 *                            -- does not
 *                            look for trouble.
 ************************************************************/
void
read_message(int argc, char *argv[], char *env[])
{
  set_up(argc, argv, env);
  parse_message(NO);
}
/************************************************************
 *    Checks that an address on the To: or CC: lines have
 *    the for address, or a recognized list, or that the
 *    Sender or X-Sender is a recognized address.
 *    Otherwiser it is suspicious and it is marked for
 *    sending an alert.
 *************************************************************/
YESorNO
recognize(void)
{
  YESorNO init;
  int i = 0, j;
  char * who;
  int starts[2] ={0, 0};
  char * get_multi_address(int start, YESorNO init);
  if (to_start > 0)
    {
      starts[i++] = to_start;
    }
  if (cc_start > 0)
    starts[i++] = cc_start;
  for (j = 0; j < i; j++)
    {
      init = YES;
      while ((who = get_multi_address(starts[j], init)) 
	     != NULL)
	{
	  init = NO;
	  if (is_match(for_address, who)
	      || is_us(who) || match_good_list(who))
	    return YES;
	}
    }
  if (is_match(sender, for_address))
    return YES;
  sprintf(error_msg, "%s%sCan't find for=%s on To:, Cc: lines.",
	  alias_address ? alias_address : "", 
	  alias_address ? "-> " : "", for_address);
  return NO;
}
/**********************************************************
 *  very sloppy string compare.
 **********************************************************/
YESorNO
same(const char* x, const char* y)
{
  int i = -1;
  int j = 0;
  while (x[++i] != '\0' && y[j] != '\0')
    {
      if (TOUPPER(x[i]) != TOUPPER(y[j]))
	{
	  while (x[i] == ' ' || x[i] == '"' || x[i] == '\'')
	    i++;
	  while (y[j] == ' ' || y[j] == '"' || y[j] == '\'')
	    j++;
	  if (TOUPPER(x[i]) != TOUPPER(y[j]))
	    return NO;
	}
      j++;
    }
  while (x[i] == ' ' || x[i] == '"' || x[i] == '\'')
    i++;
  while (y[j] == ' ' || y[j] == '"' || y[j] == '\'')
    j++;
  if (x[i] != '\0' || y[j] != '\0')
    return NO;
  return YES;
}
/************************************************************/
YESorNO
same_address(const char *one, const char *two)
{
  return same(one, two);
}
/**********************************************************
 *  very sloppy string compare for n chars.
 **********************************************************/
YESorNO
samen(const char* x, const char* y, int n)
{
  int i = -1;
  if (x == NULL || y == NULL)
    {
      if (x == NULL && y == NULL)
	return YES;
      return NO;
    }
  while (x[++i] != '\0' && y[i] != '\0' && i < n )
    {
      if (TOUPPER(x[i]) != TOUPPER(y[i]))
	{
	  return NO;
	}
    }
  if (i >= n)
    return YES;
  return NO;
}
/************************************************************
 *    Called from eVote/src/mail/mailui/signal.c:react_to_signal
 *    as well as in filters.c:process_signal
 *************************************************************/
char *
set_signal_name(int signo)
{
  switch (signo)
    {
    case SIGALRM:
      return "SIGALARM";
      break;
    case SIGILL:
      return "SIGILL";
      break;
    case SIGTRAP:
      return "SIGTRAP";
      break;
    case SIGABRT:
      return "SIGABRT";
      break;
#ifdef __FreeBSD__
    case SIGEMT:
      return "SIGEMT";
      break;
#endif
    case SIGFPE:
      return "SIGFPE";
      break;
    case SIGBUS:
      return "SIGBUS";
      break;
    case SIGSEGV:
      return "SIGSEGV";
      break;
#ifdef FREEBSD
    case SIGSYS:
      return "SIGSYS";
      break;
#endif
    case SIGTERM:
      return "SIGTERM";
      break;
#ifdef LINUX
    case SIGPOLL:
      return "SIGPOLL";
      break;
#endif
    case SIGHUP:
      return "SIGHUP";
      break;
    case SIGINT:
      return "SIGINT";
      break;
    case SIGQUIT:
      return "SIGQUIT";
      break;
#ifdef LINUX
    case SIGPWR:
      return "SIGPWR";
      break;
#endif
#ifdef FREEBSD
    case SIGKILL:
      return "SIGKILL";
      break;
    case SIGPIPE:
      return "SIGPIPE";
      break;
    case SIGURG	:
      return "SIGURG	";
      break;
    case SIGTSTP:
      return "SIGTSTP";
      break;
    case SIGCONT:
      return "SIGCONT";
      break;
    case SIGCHLD:
      return "SIGCHLD";
      break;
    case SIGTTIN:
      return "SIGTTIN";
      break;
    case SIGTTOU:
      return "SIGTTOU";
      break;
    case SIGIO:
      return "SIGIO";
      break;
    case SIGXCPU:
      return "SIGXCPU";
      break;
    case SIGXFSZ:
      return "SIGXFSZ";
      break;
    case SIGVTALRM:
      return "SIGVTALRM";
      break;
    case SIGPROF:
      return "SIGPROF";
      break;
    case SIGUSR1:
      return "SIGUSR1";
      break;
    case SIGUSR2:
      return "SIGUSR2";
      break;
#endif
    }
  return "UNKNOWN";
}
/************************************************************
 * Checks if the line is the start of another group of headers.
 *************************************************************/
YESorNO
still_a_header(char *str)
{
  static char *head_start[] = {"This is a multi-part message in MIME format.",
			       "This message is in MIME format."};
  static int no = sizeof(head_start)/sizeof(char*);
  int i, len, headlen;
  for (len= 0; str[len] && str[len] != '\n' ; len++)
    ;
  if (*str == '-' && *(str+1) == '-' && *(str+2) != ' '
      && *(str+2) != '\n' && *(str+2) != '\0')
    {
      return YES;
    }
  for (i = 0; i < no; i++)
    {
      headlen = strlen(head_start[i]);
      if (samen(head_start[i], str,
		(headlen > len? len : headlen)))
	{
	  return YES;
	}
    }
  return NO;
}
/*******************************************************
 *      Sets every signal in my book to go to the function.
 *************************************************************/     
void
set_signals(void(* fn_to_call)(int))
{
  signal(SIGALRM, fn_to_call);
  if (signal(SIGINT, SIG_IGN) != SIG_IGN)
    signal(SIGINT, fn_to_call);
#ifdef LINUX
  if (signal(SIGPWR, SIG_IGN) != SIG_IGN)
    signal(SIGPWR, fn_to_call);
#endif
  if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
    signal(SIGQUIT, fn_to_call);
  if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
    signal(SIGHUP, fn_to_call);
#ifdef LINUX
  if (signal(SIGPOLL, SIG_IGN) != SIG_IGN)
    signal(SIGPOLL, fn_to_call);
#endif
  if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
    signal(SIGTERM, fn_to_call);
#ifdef FREEBSD
  if (signal(SIGSYS, SIG_IGN) != SIG_IGN)
    signal(SIGSYS, fn_to_call);
#endif
  if (signal(SIGSEGV, SIG_IGN) != SIG_IGN)
    signal(SIGSEGV, fn_to_call);
  if (signal(SIGBUS, SIG_IGN) != SIG_IGN)
    signal(SIGBUS, fn_to_call);
  if (signal(SIGFPE, SIG_IGN) != SIG_IGN)
    signal(SIGFPE, fn_to_call);
#ifdef __FreeBSD__
  if (signal(SIGEMT, SIG_IGN) != SIG_IGN)
    signal(SIGEMT, fn_to_call);
  if (signal(SIGKILL, SIG_IGN) != SIG_IGN)
    signal(SIGKILL, fn_to_call);
  if (signal(SIGPIPE, SIG_IGN) != SIG_IGN)
    signal(SIGPIPE, fn_to_call);
  if (signal(SIGURG, SIG_IGN) != SIG_IGN)
    signal(SIGURG, fn_to_call);
  if (signal(SIGTSTP, SIG_IGN) != SIG_IGN)
    signal(SIGTSTP, fn_to_call);
  if (signal(SIGCONT, SIG_IGN) != SIG_IGN)
    signal(SIGCONT, fn_to_call);
  if (signal(SIGCHLD, SIG_IGN) != SIG_IGN)
    signal(SIGCHLD, fn_to_call);
  if (signal(SIGTTIN, SIG_IGN) != SIG_IGN)
    signal(SIGTTIN, fn_to_call);
  if (signal(SIGTTOU, SIG_IGN) != SIG_IGN)
    signal(SIGTTOU, fn_to_call);
  if (signal(SIGIO, SIG_IGN) != SIG_IGN)
    signal(SIGIO, fn_to_call);
  if (signal(SIGXCPU, SIG_IGN) != SIG_IGN)
    signal(SIGXCPU, fn_to_call);
  if (signal(SIGXFSZ, SIG_IGN) != SIG_IGN)
    signal(SIGXFSZ, fn_to_call);
  if (signal(SIGVTALRM, SIG_IGN) != SIG_IGN)
    signal(SIGVTALRM, fn_to_call);
  if (signal(SIGPROF, SIG_IGN) != SIG_IGN)
    signal(SIGPROF, fn_to_call);
  if (signal(SIGUSR1, SIG_IGN) != SIG_IGN)
    signal(SIGUSR1, fn_to_call);
  if (signal(SIGUSR2, SIG_IGN) != SIG_IGN)
    signal(SIGUSR2, fn_to_call);
#endif
  if (signal(SIGABRT, SIG_IGN) != SIG_IGN)
    signal(SIGABRT, fn_to_call);
  if (signal(SIGTRAP, SIG_IGN) != SIG_IGN)
    signal(SIGTRAP, fn_to_call);
  if (signal(SIGILL, SIG_IGN) != SIG_IGN)
    signal(SIGILL, fn_to_call);
  alarm(time_out);
}
/************************************************************/
void
set_up(int argc, char* argv[], char *env[])
{
  int i;
  env_copy = env;
#ifdef GOODLISTS_PATH
  good_lists_fname = GOODLISTS_PATH;
#endif
#ifdef SENDMAIL_COMMAND
  sendmail_command = SENDMAIL_COMMAND;
#endif
#ifdef SENDMAIL_PATH
  sendmail_path = SENDMAIL_PATH;
#endif
#ifdef FORGETS_PATH
  forgets_fname = FORGETS_PATH;
#endif
#ifdef LOG_FNAME
  log_fname = LOG_FNAME;
#endif
#ifdef DOMAIN
  whereami = DOMAIN;
#endif
#ifdef BADGUYS
  bad_guys_fname = BADGUYS;
#endif
#ifdef ALERT_ADDRESS
  alert_address = ALERT_ADDRESS;
#endif
#ifdef TIME_OUT
  time_out = TIME_OUT;
#endif
#ifdef WRAPPER_PATH
  wrapper_path = WRAPPER_PATH;
#endif
#ifdef LOG_PATH
  log_fname = LOG_PATH;
#endif
#ifdef LOG_LEVEL
  log_level = LOG_LEVEL;
#endif
  i = strlen(argv[0]);
  for (i--; i > 0; i--)
    {
      if (argv[0][i] == '/')
	{
	  i++;
	  break;
	}
    }
  if (program == NULL && ((program = keep_string(&argv[0][i])) == NULL))
    {
      alert("No resources to store program name", NO);
    }
  set_signals(process_signal);
  if ((bytes = buff_up()) == -1)
    alert("No resources", NO);
  if (!same(program, "forward") && !same(program, "pick")
      && !same(program, "puppet") && !same(program, "shelter"))
    close(0);  /* to release mailer process */
}
/************************************************************
 *   list_name = list
 *   site = domain.com
 *   Fiddles the arguments to call match_good_list
 *   and then puts the arguments back as they were.
 *************************************************************/
YESorNO
try_list_match(char * list_name, char * site)
{
  int len;
  YESorNO ret;
  len = strlen(list_name);
  strcat(list_name, site);
  ret = match_good_list(list_name);
  list_name[len] = '\0';
  return ret;
}
/************************************************************
 * After it is determined that the message does not
 * come from any good_lists, it is sent here to determine
 * if it is from a majordomo list, and if so, an unsub
 * message is sent.
 *************************************************************/
void
unsub_other_lists(char * for_whom)
{
  int i, j;
  char list_name[200] = "";
  char maj_name[200];
  YESorNO subscribe_found = NO, succeed_found = NO;
  char who[200];
  if (sender != NULL && strncmp(sender, "owner-", 6) == 0)
    {
      for (j= 0, i = 6; sender[i] != '@' ; i++, j++)
	{
	  list_name[j] = sender[i];
	}
      list_name[j] = '\0';
      sprintf(maj_name, "majordomo@%s", &sender[i+1]);
      unsub(list_name, maj_name, for_whom);
    }
  if (samen("Majordomo", from, 9))
    {
      for (i = msg_start ; buffer[i]; i++)
	{
	  if (samen(&buffer[i], "subscribe", 9) 
	      && !samen(&buffer[i-2], "un", 2))
	    {
	      sscanf(&buffer[i+10], " %s ", list_name);
	      subscribe_found = YES;
	      i += 9 + strlen(list_name);
	      continue;
	    }
	  if (subscribe_found && samen(&buffer[i], "succeeded", 9))
	    {
	      succeed_found = YES;
	      break;
	    }
	}
      if (try_list_match(list_name, &from[11]))
	return;
      if (!subscribe_found || !succeed_found)
	return;
      unsub(list_name, from, for_whom);
    }
  if (!authen)
    return;
  for (i = 0 ; authen[i]; i++)
    {
      if (samen(&authen[i], "set sender to owner-", 20))
	{
	  strcpy(list_name, &authen[i+20]);
	  break;
	}
    }
  if (list_name[0] == '\0')  /* didn't find it */
    return;
  if (match_good_list(list_name))
    return;
  for (i = 0; list_name[i] ; i++)
    {
      if (list_name[i] == '@')
	{
	  list_name[i] = '\0';
	  sprintf(who, "majordomo@%s", &list_name[i+1]);
	  for (i = 0; who[i]; i++)
	    {
	      if (who[i] == ' ')
		{
		  who[i] = '\0';
		  unsub(list_name, who, for_whom);
		}
	    }
	}
    }
}
/************************************************************
 *      Sends an unsub message:
 *      To: where = majordomo@somewhere.com
 *      From: from_str = forwarded_from_address
 *      Msg:  unsubscribe list_name
 *************************************************************/
void
unsub(char * list_name, char * where, char * from_str)
{
  char msg[300];
  if (build) /* set in setup */
    {
      write_goodlists(list_name, where, from_str);
    }
  mail_it(where, from_str, "Defense");
  printf("unsubscribe %s\nend\n", list_name);
  printf("\n%s.%s from %s\n\n", program, filter_version, whereami);
  printf("\nis sending this message for %s.\n", from_str);
  printf("\nAny troubles, write to %s.\n", alert_address);
  fflush(stdout);
  sprintf(msg, "unsub:%s@%s for %30s",
	  list_name, &where[10], from_str);
  sprintf(error_msg, "unsubbing");
  alert(msg, NO);
}
/************************************************************
 *      Adds the list to the goodlists.  It makes also adds
 *      a comment of from_str, the address we are protecting.
 *      To: where = majordomo@somewhere.com
 *      From: from_str = forwarded_from_address
 *      Msg:  unsubscribe list_name
 *************************************************************/
void
write_goodlists(char * list_name, char * where, char * from_str)
{
  FILE *fp;
  char msg[300];
  if ((fp = fopen(good_lists_fname, "a")) == NULL)
    {
      sprintf(error_msg,"\nCan't open %s.  Error no: %d.\n", good_lists_fname,
	      errno);
      return;
    }
  fprintf(fp,"# %s\n%s@%s\n", from_str, list_name, &where[10]);
  fclose(fp);
  sprintf(msg, "\nAdded:%s@%s for %30s",
	  list_name, &where[11], from_str);
  sprintf(error_msg, "\nAdded %s@%s to %s for %s",
	  list_name, &where[11], good_lists_fname, from_str);
  alert(msg, YES);
}
