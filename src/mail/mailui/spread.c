/* $Id: spread.c,v 1.3 2003/01/15 18:54:09 marilyndavis Exp $ */ 
/**********************************************************
 *  spread.c Functions that produce the spreadsheet for
 *           public groups.
 *********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include<stdio.h>
#include<sys/types.h>
#include<ctype.h>
#include"mailui.h"
#define TRUNC 21
/* should be in Clerk.h */
char *message(void);   /* ipc_msg.c */
/* private */
static int no_uids = 0;
static short choices;
static int cols;
static short id0;
static int cwidth;   /* This is for numeric polls */
static int get_cwidth(YESorNO *percent);
static int get_yn_cwidth(YESorNO * percent);
static int get_name_len(void);
static int get_num_cwidth(void);
static void printout(char *string);
static void print_line(int first, int last);
static OKorNOT store_spread(short id, unsigned long uid, 
			    const char vote_str[]);
#ifdef ROSA
static int total_reserves;
#endif
typedef struct spread_def
{
  short id;
  unsigned long uid;
  char vote_str[6];
} SPREAD;
static SPREAD *datum;
/**********************************************************
 *  OKorNOT fill_spread(void) driver to fill memory with the
 *          the spreadsheet values.
 **********************************************************/
OKorNOT
fill_spread(void)
{
  ITEM_INFO* p_item;
  int ret_code;
  if (p_item_copy->eVote.type != PLAIN
     || p_item_copy->eVote.priv_type != PUBLIC)
    {
      sprintf(error_msg,"\nProgrammer's error.  No spreadsheet for \"%s\".\n",
	      subject);
      return NOT_OK;
    }
  p_item = p_item_copy+1;
  do
    {
      switch (ret_code = who_voted(p_item, " >= !ACC"))
	{
	case FAILURE:  /* message written to stderr */
	  sprintf(error_msg,"\nThere is some difficulty at %s right now.  \nPlease try your \"eVote who\" command later.", whereami);
	  return NOT_OK;
	  break;
	case TOO_LONG:
	  sprintf(error_msg, "\nProgrammer error!\n");
	  return NOT_OK;
	  break;
	case STRING_OUT:
	  printf("%s", message());
	  break;
	case UID_LIST:
	case UID_LIST_MORE:
	  /* Only UID_LIST is left */
	  /* here we user uid_report() to iterate through
	     the list of voters the Clerk returned */
	  {
	    unsigned long uid;
	    char *vote_str;
	    while ((vote_str = uid_report(&uid)) != NULL)
	      {
		if (store_spread(p_item->dropping_id,
				uid, vote_str) == NOT_OK)
		  {
		    sprintf(error_msg, "\nOut of resources at %s.",
			    whereami);
		    return NOT_OK;
		  }
#ifdef ROSA
		if (same(subject,"Mexico March -eVote"))
		  total_reserves += atoi(vote_str);
#endif
	      }
	    if (uid == 0)
	      {
		failed_uid_report();
	      }  /* if uid == 0 */
	  }  /* short i */
	  break;  
	default:
	  fprintf(stderr, "Unexpected return in fill_spread = %d = %s.\n",
		  ret_code, get_rtype((RTYPE)ret_code));
	  return NOT_OK;
	} /* end of switch */
    }while (p_item++->eVote.more_to_come > 0);
  return OK;
}
/**********************************************************
 *  is_spread(void) determines if the token says "spreadsheet"
 *           or something close enough.
 **********************************************************/
YESorNO
is_spread(void)
{
  char spreadstr[] = "spreadsheet";
  if (token[0] != 's' && token[0] != 'S')
    return NO;
  if (strlen(token) < strlen(spreadstr))
    spreadstr[strlen(token)] = '\0';
  if (same(token, spreadstr))
    return YES;
  return NO;
}
/**********************************************************
 *  int get_cols(int *pcc)  determines the number of columns
 *           to generate for the spreadsheet.
 **********************************************************/
int
get_cols(int *pcc)
{
  int answer;
  if (is_option(pcc, "width", "number", &answer) == YES)
    {
      cols = answer;
      strcpy(token,"processed");
      return answer;
    }
  sprintf(error_msg, "\nCan't interpret column width in eVote who command.\n");
  return -1;
}
/**********************************************************
 *  Generates the stored spreadsheet on  stdout.
 **********************************************************/
void
gen_spread(void)
{
  int name_len;
  int allowed;
  int trunc_allowed;
  YESorNO percent = NO;
  char text[MAX_LINE + 1];
  YESorNO trunc = NO;
  int sheet, sheets;
  int first, last, left;
  char format[30];
  int start, end, index;
  int choice;
  unsigned long readers, voters, yeses;
  char result[10];
  char vote_str[10];
  int len;
  int i, j, k;
  voters = get_mail_voters(id0);
  cwidth = get_cwidth(&percent);
  if (cols == 0)
    {
      cols = 80;
    }
  if (cols < TRUNC + 1 + 2 * cwidth)
    cols = TRUNC + 1 + 2 * cwidth;
  /* get the longest string that could be on the right */
  if ((name_len = get_name_len()) >= cols - 1 - 2* cwidth)
    name_len = cols -1 - 2*cwidth;
  /* how many choices can fit on a sheet? */
  allowed = (cols - 1 - name_len)/cwidth;
  /* what if we truncate the string on the right? */
  trunc_allowed = (cols - 1 - TRUNC)/cwidth;
  /* calculate the number of sheets necessary */
  sheets = 0;
  while (++sheets)
    {
      /* maybe we don't need to truncate */
      if (sheets * allowed >= choices)
	{
	  trunc = NO;
	  break;
	}
      /* maybe we do */
      if (sheets * trunc_allowed >= choices)
	{
	  trunc = YES;
	  allowed = trunc_allowed;
	  break;
	}
    }
  for (sheet = 1; sheet <= sheets; sheet++)
    {
      /* we calculate the first and last choice for
	 this sheet */
      first = (sheet - 1) * allowed + 1;
      last = sheet * allowed;
      /* make a format string for the choice titles */
      sprintf(text,"%d", last);
      len = strlen(text);
      sprintf(format," %%%dd %%s", len);
      if (last > choices)
	last = choices;
      /* how much space is left for the text string on 
	 each line ? */
      left = cols - (last - first + 1) * cwidth - 2;
      /* make the headings for the sheet */
      for (i = 0, choice = first; choice <= last; choice++, i++)
	{
	  printf("\n");
	  for (j = 0; j < (cwidth - 1)/2 + 1; j++)
	    printf(" ");
	  for (j = 0; j < i ; j++)
	    {
	      printf("|");
	      for (k = 0; k < cwidth - 1; k++)
		printf(" ");
	    }
	  printf(",");
	  for (j = (i+1) * cwidth; j < (last - first + 1) * cwidth + 2; j++)
	    printf("-");
	  sprintf(text, format, choice, 
		  item_info[datum[choice-1].id].eVote.title);
	  if (trunc == YES)
	    text[left] = '\0';
	  printf("%s", text);
	}
      printf("\n");
      /* Then the line of numbers */
      for (choice = first; choice <= last; choice++)
	{
	  if (last < 10)
	    {
	      for (j = 0; j < (cwidth +1)/2; j++)
		printf(" ");
	      printf("%d", choice);
	      for (j++; j < cwidth; j++)
		printf(" ");
	    }
	  else
	    {
	      if (cwidth > 5)
		printf(" ");
	      printf("%4d", choice);
	      if (cwidth > 4)
		printf(" ");
	      if (cwidth > 6)
		printf(" ");
	    }
	}
      printf("\n");
      /* a separating line */
      print_line(first, last);
      /* now, for each user, give her votes */
      for (i = 0; i < no_uids; i++)
	{
	  /* Which entries in the datum array pertain
	     to this user and this set of choices? */
	  start = i*choices + first - 1;
	  end = i*choices + last - 1;
	  /* print each vote */
	  for (index = start; index <= end; index++)
	    {
	      printout(datum[index].vote_str);
	    }
	  /* print the email address */
	  sprintf(text,"| %s", who_is(datum[start].uid));
	  if (trunc == YES)
	    text[left + 1] = '\0';
	  printf("%s\n", text);
	}
      /* line to separate the totals */
      print_line(first, last);
      if (item_info[id0].eVote.type == GROUPEDN 
	 || item_info[id0].eVote.type == GROUPEDn)
	{
	  for (choice = first + id0 - 1; choice <= last + id0 - 1; choice++)
	    {
	      sprintf(result,"%hd", item_info[choice].eVote.min);
	      printout(result);
	    }
	  printf("|    %s\n", "MINIMUM VOTE");	
	  print_line(first, last);
	  for (choice = first + id0 - 1; choice <= last + id0 - 1; choice++)
	    {
	      sprintf(result,"%hd", item_info[choice].eVote.max);
	      printout(result);
	    }
	  printf("|    %s\n", "MAXIMUM VOTE");	
	  print_line(first, last);
	}
      if (item_info[id0].eVote.vstatus == UNSEEN)
	continue;
      for (choice = first + id0 - 1; choice <= last + id0 - 1; choice++)
	{
	  get_mail_stats(choice, &readers, vote_str, result);
	  if (item_info[id0].eVote.vstatus == UNSEEN)
	    strcpy(result,"?");
	  else
	    if (percent == YES)
	      {
		yeses = atoul(result);
		sprintf(result,"%4.1f", (float)yeses/voters);
	      }
	  printout(result);
	}
      if (item_info[id0].eVote.type == GROUPEDN 
	 || item_info[id0].eVote.type == GROUPEDn
	 || item_info[id0].eVote.type == TALLIEDN )
	{
	  printf("|    %s\n", "AVERAGE");
#ifdef ROSA
	  if (same(subject,"Mexico March -eVote"))
	    {
	      print_line(first, last);
	      printf("|%4d |    TOTAL RESERVATIONS\n", total_reserves);
	    }
#endif
	  continue;
	}
      /* yes/no only now */
      printf("|    %s\n", (percent == YES? "PERCENT YES" : "YES"));
      for (choice = first + id0 - 1; choice <= last + id0 - 1; choice++)
	{
	  get_mail_stats(choice, &readers, vote_str, result);
	  if (item_info[id0].eVote.vstatus == UNSEEN)
	    strcpy(result,"?");
	  else
	    {
	      yeses = atoul(result);
	      if (percent == YES)
		sprintf(result,"%4.1f", (float)(voters-yeses)/voters);
	      else
		sprintf(result,"%4ld", voters-yeses);
	    }
	  printout(result);
	}
      printf("|    %s\n", (percent == YES? "PERCENT NO" : "NO"));
    }
}
/**********************************************************
 *  int get_cwidth(YESorNO *percent) returns the width of
 *          each column.  If percent == YES, there are
 *          so many voters that a percent is given instead
 *          of a count.
 **********************************************************/
int
get_cwidth(YESorNO *percent)
{
  int	cwidth = 7;   /* This is for numeric polls */
  switch (item_info[id0].eVote.type)
    {
    case GROUPEDY:
      cwidth = get_yn_cwidth(percent);
      break;
    case GROUPEDN:
    case GROUPEDn:
      cwidth = get_num_cwidth();
      break;
    default:
      /* impossible */
      break;
    }
  return cwidth;
}
/**********************************************************
 * Helper function to get the width of a cell
 **********************************************************/
int
get_num_cwidth(void)
{
  int choice, i;
  short max = 0;
  short min = 0;
  short len = 0;
  short len_max = 0;
  char buf[10];
  char result[8];
  char vote_str[8];
  unsigned long readers;
  for (choice = id0; choice <= choices + id0; choice++)
    {
      if (item_info[choice].eVote.max > max)
	max = item_info[choice].eVote.max;
      if (item_info[choice].eVote.min > min)
	min = item_info[choice].eVote.min;
      if (item_info[id0].eVote.vstatus == UNSEEN)
	continue;
      get_mail_stats(choice, &readers, vote_str, result);
      for (i=0, len=0; result[i]; i++)
	if (result[i] != ' ')
	  len++;
      if (len > len_max)
	len_max = len;
    }
  max = sprintf(buf, "%hd", max);
  min = sprintf(buf, "%hd", min);
  if (min > max)
    max = min;
  if (len_max > max)
    max = len_max;
  max += 3;  /* surrounding blanks and one | */
  return max;
}
/**********************************************************
 * Helps with get_cwidth for y/n votes.
 **********************************************************/
int
get_yn_cwidth(YESorNO * percent)
{
  int choice; 
  unsigned long readers, voters,  yeses, nos;
  char result[10];
  char vote_str[10];
  int cwidth = 4;
  unsigned long biggest = 0L;
  readers = voters = yeses = nos = 0L;
  voters = get_mail_voters(id0);
  for (choice = id0; choice <= choices + id0; choice++)
    {
      get_mail_stats(choice, &readers, vote_str, result);
      if ((yeses = atoul(result)) > biggest)
	biggest = yeses;
      if ((nos = voters - yeses) > biggest)
	biggest = nos;
    }
  if (biggest > 9999L)
    *percent = YES;
  else if (biggest > 999L)
    cwidth = 7;
  else if (biggest > 99L)
    cwidth = 6;
  else if (biggest > 9L)
    cwidth = 5;
  return cwidth;
}
/**********************************************************
 *  int get_name_len(void)  reports the length of the longest
 *      email address in the group.
 **********************************************************/
int
get_name_len(void)
{
  char check[9];
  int len = 0, add;
  int maybe, i, j;
  for (i = 0, j= 0 ; j < no_uids ; j++, i += choices)
    {
      if ((maybe = strlen(who_is(datum[i].uid))) > len)
	len = maybe;
    }
  sprintf(check,"%d ", choices);
  add = strlen(check);
  for (i = id0; i <= choices + id0; i++)
    {
      if ((maybe = add + strlen(item_info[i].eVote.title)) > len)
	len = maybe;
    }
  return len;
}
/**********************************************************
 *  void printout(char *string)  Prints a cell to stdout.
 *       The value for the cell is in string.
 **********************************************************/
void
printout(char *string)
{
  int len, i;
  while (*string == ' ')
    string++;
  len = strlen(string);
  while (string[len-1] == ' ')
    string[--len] = '\0';
  if (strncmp(string, "!ACC", 4) == 0 || strncmp(string,"ACC", 3) == 0)
    strcpy(string, " ");
  if (same(string, "Voted") || same(string, "yes") || same(string, "no"))
    {
      string[1] = '\0';
      if (islower(string[0]))
	string[0] = toupper(string[0]);
    }
  len = strlen(string);
  printf("| ");
  for (i = 0; i < (cwidth -len - 3); i++)
    printf(" ");
  printf("%s ", string);
}
/**********************************************************
 *  void print_line(int first, int last)
 *         Prints lines on top of the cells.      
 **********************************************************/
void
print_line(int first, int last)
{
  int choice, i;
  for (choice = first; choice <= last; choice++)
    {
      printf(" ");
      for (i = 0; i < cwidth-1; i++)
	printf("-");
    }
  printf("\n");
}
/**********************************************************
 *  process_spread(int cc)  passing the spread processing
 *           onto process_who(void)
 **********************************************************/
void
process_spread(int cc)
{
  process_who(cc);
}
/**********************************************************
 *      Stores the vote_str for the item id and the uid
 *      in memory.
 *      NOT_OK returned means memory allocation failed.
 **********************************************************/
OKorNOT
store_spread(short id, unsigned long uid, const char vote_str[])
{
  static int index;
  static YESorNO set = NO;
  static unsigned long first_uid = 0;
  if (set == NO)
    {
      if (first_uid == 0)
	{
	  choices = item_info[id].eVote.no_in_group;
	  if ((datum = (SPREAD *) malloc(choices * sizeof(SPREAD)))
	     == NULL)
	    return NOT_OK;
	  first_uid = uid;
	  id0 = id;
	  no_uids++;
	}
      else
	{
	  if (uid == first_uid)
	    set = YES;
	  else
	    {
	      if ((datum = realloc(datum, choices * sizeof(SPREAD) 
					    * ++no_uids))
		 == NULL)
		return NOT_OK;
	    }
	}
      index = (no_uids-1) * choices;
    }
  if (set == YES)
    {
      if (uid == first_uid)
	{
	  index = id - datum[0].id;
	}
      else
	index += choices;
    }
  datum[index].id = id;
  datum[index].uid = uid;
  strcpy(datum[index].vote_str, vote_str); 
  return OK;
}
