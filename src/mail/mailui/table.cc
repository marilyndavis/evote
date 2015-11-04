/* $Id: table.cc,v 1.4 2003/01/15 18:54:09 marilyndavis Exp $ */ 
/************************************************************
 *   table.cc  -- Keeps the translation table for petitions.
 *********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 *************************************************************
 *   To add a language, add an entry in the table of TONGUEs
 *   Keep your TONGUE table in the same order as your   |
 *   list of languages in the mailui.h file.            |
 *                                                      |
 *   In the table, give a translation for each of the   |
 *   words, "comment", "help", "info", "remove", and    |
 *   "end".  The first string is the name of your       |
 *   language and the last translate_xx where xx is     |
 *   the first two letters of the name of your          |
 *   language.                                          |
 *                                                      |          */
/*                                                      |          */
#include "mailui.h"                              /*     |          */
#include "table.h"                               /*     |          */
#include<sys/stat.h>                             /*     |          */
#include<unistd.h>                               /*     |          */
#include<errno.h>                                /*     |          */
TONGUE tongue[]=                                 /*     |          */
{                                                /*    \|/         */
  {"Nahuatl", "comment", "help", "info", "remove", "end", translate_en},
  {"Francais", "commentaire", "aide", "info", "retirer", "fin", translate_fr},
  {"Espanol", "comentario", "ayuda", "info", "borrar", "fin", translate_es},
  {"English", "comment", "help", "info", "remove", "end", translate_en},
  {"Deutsch", "comment", "help", "info", "remove", "end", translate_en},
  /*  {"Italiano", "commento", "aiuto", "info", "cancellare", "fine", translate_it}*/
  {"Italiano", "commento", "aiuto", "info", "cancellare", "fine", translate_en}
};
/* SORT is local only */
typedef struct
{
  char * name;
  YESorNO pub;
}SORT;
static SORT * sort;
/* exported only to pet_out_xx.c files */
int no_petitions = -1; 
int no_privates = -1;
int no_publics = -1;
/* more export */
LANGUAGE default_language = ENGLISH;
YESorNO forced_language = NO;
ANSWER answer_list[] ={
  {"Yes", 1},
  {"Si", 1},
  {"Oui", 1},
  {"S&iacute;", 1},
  {"Y", 1},
  {"1", 1},
  {"N", -1},
  {"No", -1},
  {"Non", -1},
  {"-1", -1},
  {"Dont Know", 0},
  {"No se", 0},
  {"No s&eacute;", 0},
  {"Je ne sais pas", 0},
  {"Non so", 0},
  {"Non saprei", 0},
  {"Yo no se", 0},
  {"I dont know", 0},
  {"Abstain", 0},
  {"maybe", 0},
  {"0", 0},
  {"?", 0}};
int no_of_answers = sizeof(answer_list)/sizeof(ANSWER);
int no_languages = sizeof(tongue)/sizeof(TONGUE);
static  Ulist<List> * dictionary;
static void collect_translations(void);
static int split_subject_line(char *str);
static void sort_list(void);
/***********************************************************
 *  Collects the translations of petitions to the_language
 *  and puts them in the sort table.
 *************************************************************/
void
collect_translations(void)
{
  List * the_list;
  Subject * the_subject;
  Translation * the_translation;
  int i=0;
  Iter<List> liter(*dictionary);
  if ((sort = (SORT *)malloc(no_petitions * sizeof(SORT)))
      == NULL)
    {
      sprintf(error_msg, "\nNo resources to sort petition list.\n");
      fprintf(stderr, "\nNo resources to sort petition list.\n");
      bounce_error(SENDER | ADMIN);
    }
  while ((the_list = liter()))
    {
      Iter<Subject> siter(the_list->subject_list);
      while ((the_subject = siter()))
	{
	  if (the_language == DEFAULT_LANGUAGE)
	    {
	      sort[i].name =  the_subject->string();
	    }
	  else
	    {
	      Iter<Translation> titer(the_subject->translation_list);
	      sort[i].name = NULL;
	      while ((the_translation = titer()))
		{
		  if (strcmp(the_translation->language_flag(), 
			     make_flag(tongue[the_language].name))
		      == 0)
		    {
		      sort[i].name= the_translation->string();
		      break;
		    }
		}
	      if (sort[i].name == NULL)
		sort[i].name =  the_subject->string();		  
	    }
	  sort[i++].pub = the_subject->is_public();
	}
    }
}
/************************************************************
 *    Finds and drops all the translations for the p_item in
 *    the list.
 *************************************************************/
void 
drop_translations(char *list, ITEM_INFO *p_item)
{
  char *fname;
  List * the_list;
  Subject * the_subject;
  Translation * p_translation;
  read_translations();

  the_list = dictionary->get(&List(list));
  the_subject = the_list->get(&Subject(p_item->eVote.title));
  Iter<Translation> titer(the_subject->translation_list);
  while ((p_translation = titer()))
    {
      fname = pet_fname("text/", p_item);
      strcat(fname, whole_name(p_translation->language_flag()));
      chmod(fname, 00660);
      if (unlink(fname) != 0 && errno != ENOENT)
	{
	  sprintf(&error_msg[strlen(error_msg)], 
		  "\nCan't drop poll text for %s", 
		  p_item->eVote.title); 
	  perror(error_msg);
	}
      the_subject->drop(p_translation);
      titer.reset();
    }
  the_list->drop(the_subject);
  write_translations();
  return;
}
/************************************************************
 *    Tries to find the subject as a petition in any language
 *    and on any petition list.  If so, it switches to the
 *    correct language and the correct petition list.
 *************************************************************/
OKorNOT
find_and_switch (int argc, char* argv[], YESorNO *do_confirm)
{
  char line[MAX_LINE + 1];
  char eVote_subject[MAX_LINE + 1];
  char the_list[MAX_LINE + 1];
  YESorNO found = NO;
  int k, j;
  FILE *fp;
  *do_confirm = NO;
  if ((fp = open_trans_file("r")) == NULL)
    return NOT_OK;
  while (fgetsn(line, MAX_LINE, fp) != NULL)
    {
      if (strncmp(line, LISTER, LISLEN) == 0)
	{
	  strcpy(the_list, &line[LISLEN]);
	  continue;
	}
      /* SUBLEN is 11 */
      if (strncmp(line, SUBJECTER, SUBLEN) == 0)
	{
	  *do_confirm = NO;
	  j = split_subject_line(line);
	  /* Now line[j] == flag & line[j-1] == '\0'*/
	  strcpy(eVote_subject, &line[SUBLEN + 17]); /* 5 = PUB: */
	  *do_confirm =(line[SUBLEN+5] == 'D' ? YES:NO);
	  if (same(subject, eVote_subject))
	    {
	      if (!forced_language)
		{
		  is_flag(&line[j], &the_language);
		}
	      found = YES;
	      break;
	    }
	  continue;
	}
      if (strncmp(line, TRANSER, TRALEN) == 0)
	{
	  if (same(subject, &line[TRALEN+3]))
	    {
	      subject = make_string(eVote_subject);
	      if (!forced_language)  /* forced with a flag on the
					subject line */
		{
		  is_flag(&line[TRALEN - 1], &the_language);
		}
	      found = YES;
	      break;
	    }
	}
    }
  fclose(fp);
  if (!found)
    {
      if (!forced_language)
	the_language = ENGLISH;
      return OK;
    }
  list = make_string(the_list);
  make_lclist();
  translate(the_language, SET_COMMENT, NULL, MAYBE, 
	    MAYBE, MAYBE, NULL, NULL, MAYBE);
  for (k = 0; k < argc; k++)
    {
      if (samen(argv[k], "petition", 8))
	{
	  char *space;
	  space = (char*)malloc(strlen(argv[k]) + strlen(list));
	  strcpy(space, list);
	  strcat(space, &argv[k][8]);
	  argv[k] = space;
	}
    }
  return OK;
}
/************************************************************
 *     Looks for the subject to be in the translation
 *     table and retrieves the translation of it to the language.
 ************************************************************/
char *
get_trans(char * subject, LANGUAGE lang, YESorNO force)
{
  List * the_list;
  Subject * the_subject;
  char * answer;
  read_translations();
  the_list = dictionary->get(&List(list));
  the_subject = the_list->get(&Subject(subject));
  answer= the_subject->pull(tongue[lang]);
  return (force ? (answer ? answer : subject) : answer);
}
/************************************************************
 *   determines if str is a language flag and if so, reports
 *   the language in lang.
 ************************************************************/
YESorNO
is_flag(char *str, LANGUAGE *lang)
{
  int i;
  if (str[0] == '\0' || str[0] != '-')
    return NO;
  for (i = 0; i < no_languages; i++)
    {
      if (samen(&str[1], tongue[i].name, 2))
	{
	  *lang = (LANGUAGE)i;
	  return YES;
	}
    }
  return NO;
}
/************************************************************
 *   Opens up the file and returns the FILE *.
 *************************************************************/
FILE *
open_trans_file(char * mode)
{
  static char fname[FNAME + 1];
  FILE * fp;
  int i;
  char com[FNAME + 10] = "";
  if (fname[0] == '\0')
    {
      strcpy(fname, listdir);
      i = strlen(fname) + 1;
      do
	{
	  while (fname[--i] != '/')
	    ;
	}
      while (strncmp(&fname[i], "/lists", 6) != 0);
      sprintf(&fname[i], "%s", TRANSLATION_DATA);
    }
  while ((fp = fopen(fname, mode)) == NULL)
    {
      if (com[0] == '\0')
	{
	  sprintf(com, "touch %s", fname);
	  system(com);
	  continue;
	}
      sprintf(error_msg,"Can't open %s to read petition translations.\n",
	      fname);
      perror(error_msg);
      return NULL;
    }
  return fp;
}
/************************************************************
 *    If want_public == MAYBE, prints them all.
 ************************************************************/
void
print_petitions(YESorNO want_public= MAYBE)
{
  int i = 0;
  if (no_petitions == -1)
    read_translations();
  if (sort == NULL)
    {
      collect_translations();
      sort_list();
    }
  for (i = 0; i < no_petitions; i++)
    {
      if (want_public == MAYBE || want_public == sort[i].pub)
	printf("%s\n", sort[i].name);
    }
}
/************************************************************
 *     Sorts the list of petitions.
 ************************************************************/
void
sort_list(void)
{
  SORT keep;
  int i, j;
  for (i = 1; i < no_petitions; i++)
    {
      keep = sort[i];
      for (j = i - 1; j >= 0; j--)
	{
	  if (strCmp(sort[j].name, sort[i].name) > 0)
	    sort[j+1] = sort[j];
	  else
	    break;
	}
      sort[j+1] = keep;
    }
  return;
}
/************************************************************
 *  Opens and reads the translations file into the table
 *  in memory.
 ************************************************************/
void 
read_translations(void)
{
  FILE *fp;
  char line[MAX_LINE + 1];
  static Translation* this_translation;
  static List *this_list;
  static Subject * this_subject;
  int j;
  YESorNO do_confirm = NO;
  if (dictionary != NULL)
    return;
  dictionary = new Ulist<List> ;
  no_petitions = 0;
  no_publics = 0;
  no_privates = 0;
  if ((fp = open_trans_file("r")) == NULL)
    return;
  while (fgetsn(line, MAX_LINE, fp) != NULL)
    {
      if (strncmp(line, LISTER, LISLEN) == 0)
	{
	  this_list = dictionary->get(&List(&line[LISLEN]));
	  continue;
	}
      if (strncmp(line, SUBJECTER, SUBLEN) == 0)
	{
      	  j = split_subject_line(line); /* line[j] == flag */
	  do_confirm = NO;
	  /* PRI: = 5  DO_CONFIRM: = 12*/
	  if (strncmp(&line[SUBLEN + 6], "O_CONFIRM:", 10) == 0)
	    {
	      do_confirm = (line[SUBLEN + 5] == 'D'? YES : NO);
	      this_subject = &Subject(&line[SUBLEN + 5 + 12], YES,
				      do_confirm, &line[j]);
	    }
	  else
	    {
	      this_subject = &Subject(&line[SUBLEN + 5], YES, NO, &line[j]);
	    }
	  if (line[SUBLEN + 1] == 'R') /* PRI: in line */
	    {
	      this_subject->make_private();
	      no_privates++;
	    }
	  else
	    {
	      no_publics++;
	    }
	  if (do_confirm)
	    {
	      this_subject->make_confirm();
	    }
	  no_petitions++;
	  this_subject = this_list->get(this_subject);
	  continue;
	}
      if (strncmp(line, TRANSER, TRALEN) == 0)
	{
	  this_translation = this_subject->get(&Translation(&line[TRALEN-1]));
	}
    }
  if (fp != NULL)
    fclose(fp);
  return;
}
/************************************************************
 *    Hand it the subject line, as read from the translation.data
 *    file, and it will put a '\0' to separate the default
 *    language flag for the petition.
 *    It returns the indext into str of the '-' of the language
 *    flag.
 *************************************************************/
int
split_subject_line(char *str)
{
  int j;
  for (j = strlen(str); j > 0; j--)
    {
      if (str[j] == '-' && str[j-1] == ' ')
	{
	  str[j-1] = '\0';
	  return j;
	}
    }
  return -1;
}
/************************************************************
 *    Adds the new subject and other info to the translation
 *    table.  Check first that ! table_exists.
 *************************************************************/
void
table_add(char *list, char *subject, char *trans, char * flag,
	  PRIV_TYPE ptype, YESorNO confirm_petition)
{
  List * the_list;
  Subject * the_subject;
  Translation * the_translation;
  the_list = &List(list);
  the_list = dictionary->get(the_list);
  the_subject = the_list->get(&Subject(subject, 
				       ptype == PUBLIC ? YES : NO, 
				       confirm_petition, flag));
  the_translation = the_subject->put(&Translation(trans, flag));
}
/************************************************************
 *   Reports if the subject already exists in any language on any
 *   list.
 *************************************************************/
YESorNO
table_exists(char *subject, char *list, char *default_subject,
	     char * language_flag)
{
  List * the_list;
  Subject * the_subject;
  Translation * the_translation;
  Subject starget(subject);
  Translation ttarget(subject, "-df");
  YESorNO found = NO;
  read_translations();
  Iter<List> liter(*dictionary);
  while ((the_list = liter()))
    {
      Iter<Subject> siter(the_list->subject_list);
      while ((the_subject = siter()))
	{
	  if (*the_subject == starget)
	    {
	      strcpy(language_flag, the_subject->get_flag());
	      found = YES;
	      break;
	    }
	  Iter<Translation> titer(the_subject->translation_list);
	  while ((the_translation = titer()))
	    {
	      if (same(the_translation->string(), ttarget.string()))
		{
		  strcpy(language_flag, the_translation->language_flag());
		  found = YES;
		  break;
		}
	    }
	  if (found)
	    {
	      break;
	    }
	}
      if (found)
	{
	  break;
	}
    }
  if (found)
    {
      strcpy(default_subject, the_subject->string());
      strcpy(list, the_list->string());
      return YES;
    }
  return NO;
}
/************************************************************/
char *
whole_name(char *flag)
{
  int i;
  for (i = 0; i < no_languages; i++)
    {
      if (flag[0] == '-' && samen(&flag[1], tongue[i].name, 2))
	{
	  return tongue[i].name;
	}
    }
  if (same(flag,"-df"))
    return tongue[DEFAULT_LANGUAGE].name;
  return NULL;
}
/************************************************************
 *    Writes the translation tables to disk
 *************************************************************/
OKorNOT
write_translations(void)
{
  FILE *fp;
  List * the_list;
  Subject * the_subject;
  Translation * the_translation;
  if ((fp = open_trans_file("w")) == NULL)
    return NOT_OK;
  Iter<List> liter(*dictionary);
  while ((the_list = liter()))
    {
      if (the_list->subject_list.is_empty())
	continue;
      fprintf(fp,"%s%s\n", LISTER, the_list->string());
      Iter<Subject> siter(the_list->subject_list);
      while ((the_subject = siter()))
	{
	  fprintf(fp,"%s%s%s%s %s\n", SUBJECTER, 
		  (the_subject->is_public() ? "PUB: " : "PRI: " ),
		  (the_subject->needs_confirm() ? 
		   "DO_CONFIRM: " : "NO_CONFIRM: "),
		  the_subject->string(), the_subject->get_flag());
	  Iter<Translation> titer(the_subject->translation_list);
	  while ((the_translation = titer()))
	    {
	      fprintf(fp,"%s%s %s\n", TRANSER, 
		      the_translation->language_flag() + 1,
		      the_translation->string());
	    }
	}
    }
  fclose(fp);
  return OK;
}
