/* $Id: explain.c,v 1.3 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/***********************************************************
 *  ../eVoteui/explain.c  - eVote explanations for the demo.
 ************************************************************
 **********************************************************/
#include "../eVote.h"
static YESorNO do_teacher = NO;
#define NO_HELPS 50
#define KEY_LEN 10
#include "explain.txt"
void
explain(char *inp)
{
  YESorNO go_back = YES;
  char * input = inp;
  char * new_input;
  char prompt[250];
  char * show(char*);
  if (strcmp(input,"x teacher") == 0)
    do_teacher = YES;
  sprintf(prompt,"\nExplain what?                                 (q to quit explanations)%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8);
  input++;
  while (1)
    {
      if (*input == '\0')
	{
	  input = GetArg(prompt);
	  go_back = NO;
	}
      switch (*input)
	{
	case ' ':
	  input++;
	  break;
	case 'q':
	  return;
	  break;
	case 'x':
	case 'X':
	  input++;
	  break;
	case '\0':
	  printf("\nWhenever you don't know what to do, enter '?'\n\n");
	  break;
	default:
	  if ((new_input = show(input)) != NULL)
	    input = new_input;
	  else
	    {
	      if (go_back == YES)
		return;
	      *input = '\0';
	    }
	  break;
	}
    }
}
/************************************************************/
void
fix_up(char *subject)
{
  short i;

  if ((i=(short)strlen(subject)) < 3)
    {
      while (i++ < 3)
	strcat(subject," ");
    }
  if (strncmp(subject, "contents", 4) == 0
     ||
     strncmp(subject,"Contents", 4) == 0
     || strncmp(subject, "CONTENTS", 4) == 0)
    strcpy(subject,"cotents");
  if (strncmp(subject,"*?*", 3) == 0)
    strcpy(subject,"UNSEEN");
  if (strncmp(subject,"*", 1) == 0)
    strcpy(subject,"OPEN");
  if (strcmp(subject,"P  ") == 0)
    strcpy(subject,"PUBLIC");
  if (strcmp(subject,"p  ") == 0)
    strcpy(subject,"PRIVATE");
  if (strcmp(subject,"I  ") == 0 || strcmp(subject,"i  ") == 0)
    strcpy(subject,"IF-VOTED");
  if (strcmp(subject,"t  ") == 0
     || strcmp(subject,"T  ") == 0)
    {
      strcpy(subject,"teacher");
    }  
}
/************************************************/
char*
show(char* subject)
{
  static short i;
  static YESorNO found = NO;
  YESorNO backing = NO;
  char buf[KEY_LEN+1];
  char * input;
  short line;
  void fix_up(char *subject);
  strcpy(buf, subject);
  do
    {
      if (backing == YES)
	{
	  if (i > 1)
	    i--;
	  else
	    return NULL;
	}
      else if (found == NO)
	{
	  fix_up(subject);
	  for (i = 0; i < NO_HELPS && help[i].key[0] != '\0'
		 && help[i].xscreen[0][0] != '.' 
		 && help[i].key != NULL; i++)
	    {
	      /*fix_case could change case even if it doesn't match */
	      if (fix_case(subject, help[i].key, 3) == OK)
		break;
	    }
	  /* if we couldn't find the topic */
	  if (help[i].xscreen[0][0] == '.'
	      || help[i].key == NULL || i == NO_HELPS 
	      || help[i].key[0] == '\0')
	    {
	      printf("\n'%s' is not an available topic.\n", buf);
	      printf("\nEnter '?' to see what is available.\n");
	      return NULL;
	    }
	}
      do  /* loop for multiple screen topics */
	{
	  /* print the screen */
	  for (line = 0; line < NO_LINES; line++)
	    {
	      printf("\n");
	      if (help[i].xscreen[line][0] == '.')
		{
		  break;
		}
	      printf("%s", help[i].xscreen[line]);
	    }
	  if (help[i].more == YES )
	    {
	      input = GetArg("\n < ENTER > to continue ... or enter a new topic, 'b'ack, or 'q':  ");
	      switch (*input)
		{
		case 'b':
		  if (i > 1)
		    {
		      i--;
		      found = YES;
		    }
		  else
		    {
		      if (do_teacher == YES)
			found = YES;
		      return NULL;
		    }
		  continue;
		  break;
		case '?':
		  do_teacher = NO;
		  i = 0;
		  continue;
		  break;
		case '\0':
		  i++;
		  continue;
		  break;
		case 'q':
		  if (do_teacher == YES)
		    {
		      do_teacher = NO;
		      found = YES;
		    }
		  return input;
		  break;
		case 't':
		  if (do_teacher == YES)
		    {
		      i++;
		      break;
		    }
		  do_teacher = YES;
		  printf("\n\nDo you want to start the teacher:\n\n  1. at the beginning?\n\n  2. at this topic?\n");
		  switch (eVote_asknum(NO_Q, 1, 2, NO))
		    {
		    case 1:
		      i = 0;
		      break;
		    case 2:
		      i++;
		      break;
		    case EQUIT:
		      do_teacher = NO;
		      return NULL;
		      break;
		    case EQUESTION:
		    case ENOREPLY:
		      do_teacher = NO;
		      break;
		    }
		  break;
		case 'x':
		case 'X':
		  input++;
		default:
		  return input;
		  break;
		}
	    }
	  else break;
	}
      while (1);  /* finish of same subject */
      if (do_teacher == YES)
	{
	  backing = NO;
	  if (i > 1)
	    input =  GetArg("\n < ENTER > to continue teacher .. or enter a new topic, 'b'ack or 'q':  ");
	  else
	    input = GetArg("\n < ENTER > to continue teacher .. or enter a new topic, or 'q':  ");
	  switch (*input)
	    {
	    case 'b':
	      backing = YES;
	      break;
	    case 'q':
	      found = YES;
	      do_teacher = NO;
	      return NULL;
	    case '?':
	      do_teacher = NO;
	      strcpy(subject,"?  ");
	      continue;
	      break;
	    case '\0':
	      if (strcmp(help[i].next,"END") == 0)
		{
		  printf("\nThat's the end of the teacher for now.\n");
		  do_teacher = NO;
		  found = NO;
		  i = 0;
		  return NULL;
		}
	      else
		{
		  found = YES;
		  i++;
		  continue;
		}
	      break;
	    default:
	      do_teacher = NO;
	      return input;
	      break;
	    }
	}
      else
	return NULL;
    }
  while (1);
  return NULL;
}
