/* $Id: subject.c,v 1.3 2003/01/15 18:54:09 marilyndavis Exp $ */ 
/**********************************************************
 *  subject.c  Functions that deal with the subject line.
 *********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include<stdio.h>
#include<string.h>
#include"mailui.h"
#define IS_HEX(X) (  ( ((X) <= '9' && (X) >= '0') || \
   ((X) <= 'F' && (X) >= 'A') \
   || ((X) <= 'f' && (X) >= 'a')) ? 1 : 0)
/* for export */
/* char *subject is in ../../tools/filters/filter.c */
char *original_subject = "";
LANGUAGE the_language = DEFAULT_LANGUAGE;
char * good_strip;  /* if something good got stripped that was meant
		     to be in the subject line of a new poll, we
		     notice it  -- only exported to new_poll.c */
struct tag_def
{
  char * text;
  int len;
} prefix[]={
  {"for major", 0},   /* filled in with subject_prefix from list.config */
  { "[R]", 3},        /* For subject-line removal of petition signatures */
  { "Re:", 3},
  { "Poll:", 5},
  { "Confirm:", 8},
  { "Confirmez:", 10},
  { "Confirma:", 9},
  { "Closed:", 7},
  { "eVote Rcpt:", 11},
  { "eVote Recibo:", 13},
  { "eVote Ricevuta:", 14},
  { "Erreur:", 7},
  { "Petition:", 9},
  { "Errore:", 7},
  { "Error:", 6}};
struct tag_def suffix[] =
{
  {"(fwd)", 5}
};
static int chop(char *subject);
/************************************************************
 *   chops any spaces or tabs off the end of
 *   the subject line and returns the new length.
 **********************************************************/
int
chop(char * subject_line)
{
  int len = strlen(subject_line);
  if (len == 0)
    return 0;
  while (--len >= 0 
	&& (subject_line[len] == ' ' || subject_line[len] == '\t'))
    {
      subject_line[len] = '\0';
    }
  return len + 1;
}
/**********************************************************
 * Replaces a stripped down version of the subject
 * without Re:'s.
 **********************************************************/
OKorNOT
strip_subject(void)
{
  static char* p_s;
  YESorNO stripped_something;
  YESorNO strip_quote = NO;
  int len, i;
  char * get_config(char * looking_for);
  int no_prefixes = (sizeof prefix)/sizeof(struct tag_def);
  int no_suffixes = (sizeof suffix)/sizeof(struct tag_def);
  confirm_this.status = STARTING;
  if (petition == NO)
    {
      if ((prefix[0].text = get_config("subject_prefix")) == NULL)
	return NOT_OK;
      prefix[0].len = strlen(prefix[0].text);
    }
  p_s = subject;
  if (*p_s != '\0')
    {
      do
	{
	  stripped_something = NO;
	  while (*p_s == ' ' || *p_s == '"' || *p_s == '\t')
	    {
	      stripped_something = YES;
	      if (*p_s == '"')
		strip_quote = YES;
	      p_s++;
	    }
	  for (i = 0 ; i < no_prefixes; i++)
	    {
	      while (strNcmp(p_s, prefix[i].text, prefix[i].len) == 0)
		{
		  if (i > 2)
		    {
		      good_strip = prefix[i].text;
		    }
		  if (strcmp(prefix[i].text, "[R]") == 0)
		    remove_sig = YES;
		  if (samen(prefix[i].text, "Confirm", 7))
		    confirm_this.status = CHECK;
		  stripped_something = YES;
		  p_s += prefix[i].len;
		  while (*p_s == ' ' || *p_s == '\t')
		    p_s++;
		}
	    }
	  while (strNcmp(p_s, "Re", 2) == 0 
		&& p_s[2] <= '9' && p_s[2] >= '0')
	    {
	      stripped_something = YES;
	      while (*++p_s != ' ' && *p_s != ':')
		;
	      if (*p_s == ':')
		++p_s;
	      while (*p_s == ' ' || *p_s == '\t')
		p_s++;
	    }
	}
      while (stripped_something == YES && *p_s != '\0');
    }
  if (strip_quote == YES)
    while (subject[strlen(subject) -1] == '"')
      subject[strlen(subject) -1] = '\0';
  strcpy(subject, p_s);
  do
    {
      stripped_something = NO;
      len = chop(subject);
      if (petition)
	if (is_flag(&subject[len - 3], &the_language))
	  {
	    subject[len - 3] = '\0';
	    len = chop(subject);
	    forced_language = YES;
	    stripped_something = YES;
	  }
      for (i = 0; i < no_suffixes; i++)
	{
	  while (strNcmp(&subject[len - suffix[i].len], suffix[i].text,
			suffix[i].len) == 0)
	    {
	      stripped_something = YES;
	      subject[len - suffix[i].len] = '\0';
	      len = chop(subject);
	    }
	}
    }
  while (stripped_something == YES && subject[0] != '\0');
  if (strlen(subject) > TITLEN)
    {
      if (confirm_this.status != CHECK)
	subject[TITLEN] = '\0';
      else
	subject[TITLEN + CONFIRM_LEN + 1] = '\0';
    }
  return OK;
}
/************************************************************
 * Code added here to translate the Quoted Printable in subject lines.
 * This is because accentuated characters cannot be in a header (some
 * RFC obligation), so mailer agents do encode them in QP. But some
 * don't, and eVote cannot compare the subjects line from one with QP
 * and another one without QP. We have to translate QP then... Hope my
 * function will work everywhere, as the standard is quite tricky.
 *    Contributed by Laurent Chemla 
 *************************************************************/
void
unqp_subject(void)
{
  int i = 0, j = 0, k, flag;
  char * ptr;
  unsigned char cc;
  while (subject[i] != '\0') 
    {
      if (subject[i] == '=') 
	{
	  if (subject[i+1] == '?') 
	    {	/*skip QP header*/
	      ptr=strchr((char *)&subject[i+2],'?');
	      if (ptr != NULL && *(ptr+1) == 'Q' && *(ptr+2) == '?') 
		{
		  i=(int)(ptr-(char *)subject)+3;
		  flag=1;
		  continue;
		}
	    }
	  else 
	    {
	      if (IS_HEX(subject[i+1]) && IS_HEX(subject[i+2]))
		{
		  cc = (subject[i+1] <= '9' 
			&& subject[i+1] >= '0') ? 
		    (subject[i+1] - '0') : (subject[i+1] - 55);
		  cc *= 0x10;
		  cc += (subject[i+2] <= '9' && subject[i+2] >= '0') 
		    ? (subject[i+2] - '0') : (subject[i+2] - 55);
		  subject[i]=cc;
		  k=i+1;
		  while (subject[k+2] != '\0') 
		    {
		      subject[k]=subject[k+2];
		      k++;
		    }
		  subject[k]='\0';
		  continue;
		}
	    }
	} 
      else if (subject[i] == '?' && flag == 1 && subject[i+1] == '=') 
	{
	  flag=0;
	  i += 2;
	  if (subject[i] == ' ')
	    i++;
	  continue;
	}  
      else if (subject[i] == '_' && flag == 1) 
	{
	  subject[i]=' ';
	}
      subject[j++]=subject[i++];
    }
  subject[j]='\0';
}
