/* $Id: trans.c,v 1.4 2003/01/15 18:54:13 marilyndavis Exp $ */ 
/* trans.c -- for translating code strings to another language
*********************************************************
*    Copyright (c) 1994...2015 Deliberate.com Patented.
*    by Marilyn Davis
*  This program is free software; you can redistribute it and/or
*  modify it under the terms of the eVote(R)/Clerk License as
*  published by Deliberate.Com.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  eVote(R)/Clerk License for more details.
*
*  You should have received a copy of the eVote(R)/Clerk License
*  along with this program in EVOTE_HOME_DIR/eVote/doc/eVote.doc. 
*  If not, write to Deliberate.Com 2555 W. Middlefield Rd. #150,
*  Mountain View, CA 94043 USA or office@deliberate.com.
************************************************************
*             gcc -o trans trans.c -g -Wall
*    trans.c  This translates c code.
*             Call like:
*             trans English French filename(s)
*              or
*             trans en fr filename(s)
*             Expects an en_fr translation table to be in
*             the same directory with the filename(s)
*             and with this program.  fr_en is fine too.
*             If there isn't one, that's ok.
*             It translates the best it can according to 
*             the current table and leaves in the English
*             if it has no French.
*             Output file is text_fr.c if the input file is
*             text.c.  It's text_fr_fr.c if text_fr.c is the
*             input file.  It's ok if French and English are
*             mixed in the input file.
*             It outputs an en_fr.new that has the untrans-
*             lated strings.
*             Add new translations to the end of en_fr and
*             run it again.
************************************************************/
#include"trans.h"
int
main(int argc, char *argv[])
{
  int i;
  i = command_args(argc, argv);
  collect_skips();
  fill_tables();
  for (; i < argc; i++)
    {
      if (process_file(argv[i]) == OK)
	sort_tables(NO);
    }
  print_table();
  exit(0);
}
/************************************************************
 *         returns the number of chars in the < argument >
 *                 -1 if there's trouble.
 ************************************************************/
int
arglen(char *str)
{
  int i;
  int brackets;
  for (i = 0, brackets = 0; str[i]; i++)
    {
      if (str[i] == '>' && i > 0
	  && str[i-1] != '-')
	{
	  if (--brackets == 0)
	    return i;
	  continue;
	}
      if (str[i] == '<' && str[i+1] == '%')
	brackets++;
    }
  if (str[i] == '\0')
    return -1;
  return 0;
}
/************************************************************/
void
arg_err(void)
{
  printf("\nusage:\ntrans <-d > language_in language_out file_name(s)\n");
  exit(1);
}
/************************************************************
 *          String compare according to the TOSORT #define.
 *          Returns YES if str1 is before str2
 *                   NO if str1 is after str2
 *                MAYBE if they are the same.
 *          Also, it ignores the string's arguments that
 *          are wrapped in <>'s -- unless there's a string
 *          in the argument.
 *************************************************************/	
YESorNO
before(char *str1, char *str2)
{
  int i, j;
  for (i = 0, j = 0; str1[i] && str2[j] ; i++, j++)
    {
      if (TOSORT(str1[i]) < TOSORT(str2[j]))
	return YES;
      if (TOSORT(str1[i]) > TOSORT(str2[j]))
	return NO;
      if (str1[i] == '<' && str1[i+1] == '%')
	i += skip_arg(&str1[i]);
      if (str2[j] == '<' && str2[j+1] == '%')
	j += skip_arg(&str2[j]);
    }
  if (str1[i] == '\0' && str2[j] == '\0')
    return MAYBE;
  if (str1[i] == '\0')
    return YES;
  return NO;
}
/************************************************************
 *   This takes the white space  and '.', ':', ';', ',' off
 *     the end of str and counts the characters stripped.
 *     It's special because it counts '\n' etc as two characters
 *     because it's within a string in the file.
 *************************************************************/
int
big_trunc(char *str)
{
  int len, len2;
  int i = 0;
  len2 = len = strlen(str);
  while (--len >= 0)
    {
      if (str[len] == ' ' || str[len] == '.' || str[len] == ','
	  || str[len] == ';' || str[len] == ':')
	{
	  str[len] = '\0';
	  i++;
	}
      else if (str[len] == '\n' || str[len] == '\t')
	{
	  i += 2;  /* it's two chars in the file buffer */
	  str[len] = '\0';
	}
      else
	break;
    }
  return i;
}
/************************************************************/
void
collect_skips(void)
{
  FILE *fp;
  char * name = "trans.skips";
  char line[STRINGLEN +1];
  long end;
  char * where;
  char * space;
  YESorNO ifd;

  if ((fp = fopen(name, "r")) == NULL)
    {
      fprintf(stderr,
	      "\nCan't open %s to read phrases to leave untranslated.\n",
	      name);
      fprintf(stderr,"Proceeding anyway.\n");
      return;
    }
  no_skips = 0;
  no_ifdefs = 0;
  /* 	count the  number of skips */
  while (fgets(line, STRINGLEN, fp) != NULL)
    {
      if (line[0] == '#' || line[0] == '\n')
	continue;
      if (strncmp(line, "ifdef-", 6) == 0)
	{
	  no_ifdefs++;
	  continue;
	}
      no_skips++;
    }
  end = ftell(fp);
  if ((skip = malloc(sizeof(char*)*no_skips)) == NULL)
    {
      fprintf(stderr,"\ncollect_skips: Unsuccessful memory allocation.\n");
      exit(1);
    }
  if ((ifdef = malloc(sizeof(char*)*no_ifdefs)) == NULL)
    {
      fprintf(stderr,"\ncollect_skips: Unsuccessful memory allocation.\n");
      exit(1);
    }
  fseek(fp, 0L, SEEK_SET);
  no_skips = 0;	
  no_ifdefs = 0;
  while (fgets(line, STRINGLEN, fp) != NULL)
    {
      if (line[0] == '#' || line[0] == '\n')
	continue;
      ifd = (strncmp(line, "ifdef-", 6) == 0) ? YES : NO;
      where = ifd ? line + 6 : line;
      if ((space = malloc(strlen(where))) == NULL)
	{
	  fprintf(stderr,"\ncollect_skips: Unsuccessful memory allocation.\n");
	  exit(1);
	}
      where[strlen(where)-1] = '\0';
      strcpy(space, where);
      (ifd ? ifdef[no_ifdefs++] : skip[no_skips++]) = space;
    }
  if (fp != NULL)
    fclose(fp);
}
/************************************************************
 *       Interprets the languages and debug option from
 *       the command line.
 *       Returns the i for argv[i] == first file to process.
 *************************************************************/
int
command_args(int argc, char **argv)
{
  int i=0;
  void arg_err();
  int string[3];
  int j = -1;
  while (++i < argc)
    {
      /* save the first three strings */
      if (*argv[i] != '-')
	{
	  if (j >= 2)
	    {
	      continue;
	    }
	  j++;
	  string[j] = i;
	  continue;
	}
      if (j >= 2)  /* option beyond file names */
	arg_err();
      /* Now look for command line options */
      switch (*(argv[i] + 1))
	{
	case 'd':
	  debug = 1;
	  break;
	default:
	  arg_err();
	}
    }
  if (j != 2)
    arg_err();
  find_languages(argv[string[0]], argv[string[1]]);
  return string[2];
}
/************************************************************
 *        This checks that the arguments in the table
 *        entries match exactly.
 *        returns OK if it finds all matching arguments
 *                NOT_OK if it finds the same # of entries
 *                       but they don't match.
 *                UNDECIDED if it finds a different number
 *                       of arguments and the one with fewer
 *                       all matches.
 *************************************************************/
OKorNOT
check_args(char * from_str, char *to_str)
{
  int i, j;
  int from_args;
  int to_args;
  int missing = 0;
  from_args = count_args(from_str);
  to_args = count_args(to_str);
  for (i = 0; from_str[i]; i++)
    {         
      if (from_str[i] == '<' && from_str[i+1] == '%') 
	/* found argument */
	{
	  j = 0;
	  for (; to_str[j]; j++)
	    {
	      if (to_str[j] == '<' && to_str[j+1] == '%')
		{
		  if (match_args(&from_str[i], &to_str[j]))
		    {
		      i += arglen(&from_str[i]);
		      break;
		    }
		  else
		    {
		      j += arglen(&to_str[j]);
		    }
		}
	    }
	  if (to_str[j] == '\0')  /* didn't find this entry */
	    {
	      if (from_args > to_args)  /* may be ok */
		{
		  missing++;
		  i += arglen(&from_str[i]);
		}
	      else
		return NOT_OK;
	    }
	}
    }
  if (from_args == to_args)
    {
      if (missing != 0)
	return NOT_OK;
      return OK;
    }
  if (from_args > to_args)
    {
      if (missing == from_args - to_args)
	return UNDECIDED;
      return NOT_OK;
    }
  if (to_args > from_args)
    {
      if (missing == 0)
	return UNDECIDED;
      return NOT_OK;
    }
  return NOT_OK; // impossible
}
/************************************************************
 *          Looks at the word that starts a str and
 *          returns:
 *          NONE     if it's all l.c.
 *          STARTS   if the first char is u.c. and the rest 
 *                   is l.c.
 *          ALL      if it's all u.c.
 *          MIX      otherwise
 ***********************************************************/
CAPS
check_caps(char *str)
{
  int uc = 0;
  int lc = 0;
  int i;
  YESorNO first_char_cap = NO;
  for (i = 0; str[i] && !IS_WHITE(str[i]); i++)
    {
      if (islower(str[i]))
	lc++;
      else if (isupper(str[i]))
	{
	  if (i == 0)
	    first_char_cap = YES;
	  uc++;
	}
    }
  if (lc == 0 && uc > 0)
    return ALL;
  if (uc == 0 && lc > 0)
    return NONE;
  if (uc == 1 && first_char_cap && lc > 0)
    return STARTS;
  return MIX;
}
/************************************************************
 *  When this is called, file[length] has the last character
 *  read, which was a '\n'.  This checks if this is a #ifdef XYZ
 *  where XYZ was given in the trans.skips file as ifdef-XYZ.
 *  If so, it eats up the file until the appropriate #endif
 *  and it decrements length so that the line was skipped.
 *  This is called by collect_file only.
 ************************************************************/
YESorNO
is_ifdef(long *pl, FILE * fp)
{
  int nest = 1;
  int line_start, key, j;
  YESorNO hit = NO;
  char buffer[LEN];
  /* find where the line started */
  for( line_start = *pl - 1; line_start >= 0; line_start--)
    {
      if (file[line_start] == '\n')
	{
	  break;
	}
    }
  line_start++;
  /* is it an #ifdef line? */
  if (strncmp(&file[line_start], "#ifdef ", 7) != 0)
    {
      return NO;
    }
  key = line_start + 7;
  for (j = 0; j < no_ifdefs; j++)
    {
      if (strncmp(&file[key], ifdef[j], strlen(ifdef[j])) == 0)
	{  /* hit */
	  hit = YES;
	  break;
	}
    }
  if (!hit)
    {
      return NO;
    }
  /* read the file to the matching #endif and throw it away */
  while(fgets(buffer, LEN, fp) != NULL)
    {
      if (strncmp(buffer, "#endif", 6) == 0)
	{
	  if (--nest == 0)
	    {
	      *pl = line_start;
	      return YES;
	    }
	}
      if (strncmp(buffer, "#ifdef", 6) == 0)
	{
	  nest++;
	}
    }
  return MAYBE;
}
/************************************************************
 *    Collects a text entry from the fp file (en_fr or such)
 *    and places it in where.  It quits when is recognizes
 *    quitter, STARS or DASHES.
 ************************************************************/
void
collect(char *where, FILE * fp, char *quitter)
{
  YESorNO started = NO;
  int i;
  char line[LEN +1];
  where[0] = '\0';
  while (fgets(line, LEN, fp) != NULL 
	 && strncmp(line, quitter, 15) != 0 
	 && strncmp(line, quitter+1, 15) != 0)
    {
      i = 0;
      if (!started)
	{
	  for (i = 0; line[i]; i++)
	    {
	      if (!IS_WHITE(line[i]))
		{
		  started = YES;
		  break;
		}
	    }
	}
      strcat(where, &line[i]);
    }
  trunc(where);
}
/************************************************************
 *          Collects the open file into the file[] array.
 *************************************************************/	
OKorNOT
collect_file(FILE *fp)
{
  int ch;

  length = 0;
  while ((ch = fgetc(fp)) != EOF)
    {
      file[length] = ch;
      if (ch == '\n')
	{
	  if (is_ifdef(&length, fp))
	  {
	  /* length is right on the # */
	    length--;
	  }
	}

      if (++length >= FILELEN)
	{
	  fprintf(stderr, "FILELEN is %d needs to be bigger for this file.\n",
		  FILELEN);
	  exit(1);
	}
    }
  if (length == 0)
    return NOT_OK;
  return OK;
}
/************************************************************
 *        Makes space for the new phrase and copies
 *        the source phrase into it and returns the 
 *        new phrase by reference into *dest
 ************************************************************/
void
copy_phrase(PHRASE *dest, PHRASE *source)
{
  int i;
  dest->string =
    malloc(strlen(source->string) +1);
  strcpy(dest->string, source->string);
  dest->no_args = source->no_args;
  for (i = 0; i < dest->no_args; i++)
    {
      strcpy(dest->arg[i].conv, source->arg[i].conv);
      strcpy(dest->arg[i].whites, source->arg[i].whites);
      strcpy(dest->arg[i].variable, source->arg[i].variable);
    }
}
/************************************************************/
void
copy_table(TABLE *dest, TABLE* source)
{
  copy_phrase(&dest->from, &source->from);
  copy_phrase(&dest->to, &source->to);
}
/************************************************************/
int
count_args(char * str)
{
  int i;
  int args = 0;
  for (i = 0; str[i]; i++)
    {         
      if (str[i] == '<' &&  str[i+1] == '%') 
	/* found argument */
	{
	  args++;
	}
    }
  return args;
}
/************************************************************
 *     reads the translation table from disk and makes both
 *     the from_table and to_table.
 ************************************************************/
void
fill_tables(void)
{
  FILE *fp;
  char name[LEN + 1];
  TABLE new_table;
  int i = -1;
  int trouble= 0;
  OKorNOT cc;
  sprintf(name,"%s_%s", from.flag, to.flag);
  if ((fp = fopen(name, "r")) == NULL)
    {
      sprintf(name,"%s_%s", to.flag, from.flag);
      if ((fp = fopen(name, "r")) == NULL)
	{
	  sprintf(name,"%s_%s", from.flag, to.flag);
	  printf("\nNo %s translation table found.\n",
		 name);
	  return;
	}
    }
  table_exists = YES;
  if (debug)
    printf("\nReading table:\n");
  while ((new_table = next_table(fp)).from.string[0] != '\0')
    {
      if (debug)
	{
	  printf("\n%s:%s\n%s:%s\n", from.flag, new_table.from.string,
		 to.flag, new_table.to.string);
	}
      if (++i >= TABLELEN)
	{
	  fprintf(stderr,"\ntrans: fill_tables: Out of table space.\n");
	  exit(1);
	}
      copy_table(&from_table[i], &new_table);
      if (starts_same(new_table.to.string, UNKNOWN))
	{
	  continue;
	}
      if ((cc = check_args(new_table.from.string, new_table.to.string)) 
	  == NOT_OK)
	{
	  fprintf(stderr,"\n >>> ERROR: The arguments aren't right for: \n");
	  print_entry(stderr, &new_table);
	  fprintf(stderr, STARS);
	  trouble++;
	}
      else if (cc == UNDECIDED)
	{
	  /*fprintf(stderr,"\n >>> WARNING: The number of arguments is different for:\n");
	    print_entry(stderr, &new_table);
	    fprintf(stderr, STARS); */
	}
    }
  fclose(fp);
  if (trouble)
    {
      fprintf(stderr,"\nErrors with %d %s in %s.\n",
	      trouble, (trouble > 1 ? "entries" : "entry"),
	      name);
      exit(0);
    }
  no_in_table = i+1;
  new_no_in_table = i+1;
  sort_tables(YES);  /* also makes to_table */
}
/************************************************************/
void
find_languages(char *is_from, char * is_to)
{
  int i;
  to.name = "";
  from.name = "";
  no_languages = sizeof(language)/sizeof(LANGUAGE);
  for (i = 0; i < no_languages; i++)
    {
      if (before(is_from, language[i].flag) == MAYBE
	  || before (is_from, language[i].name) == MAYBE)
	from = language[i];
      if (before(is_to, language[i].flag) == MAYBE
	  || before (is_to, language[i].name) == MAYBE)
	to = language[i];
    }
  if (to.name[0] == '\0')
    {
      fprintf(stderr,"\nCan't recognize %s as a language.\n",
	      is_to);
      exit(1);
    }
  if (from.name[0] == '\0')
    {
      fprintf(stderr,"\nCan't recognize %s as a language.\n",
	      is_to);
      exit(1);
    }
  if (to.tongue == from.tongue)
    {
      fprintf(stderr,"\n%s to %s done!\n", is_from, is_to);
    }
}
/************************************************************
 *       phrase is the from_phrase.
 *       index is the translation in the from_table
 *       Returns a version of the from_table[index].to
 *       phrase that is case compatible with the phrase.
 ***********************************************************/
PHRASE *
fix_phrase(PHRASE *phrase, int index)
{
  CAPS word[LEN];
  int j, i;
  static PHRASE answer;
  static char string[STRINGLEN + 1];
  int last_in_word;
  answer.string = string;
  answer.no_args = from_table[index].to.no_args;
  strcpy(string, from_table[index].to.string);
  if (answer.no_args != phrase->no_args)  /* don't mess with it */
    {
      /* copy the args from the table's to phrase 
	 but the whites from the input phrase */
      for (i = 0; i < answer.no_args; i++)
	{
	  strcpy(answer.arg[i].variable, from_table[index].to.arg[i].variable);
	  strcpy(answer.arg[i].conv, from_table[index].to.arg[i].conv);
	  strcpy(answer.arg[i].whites,
		 phrase->arg[i < phrase->no_args ?
			    i : phrase->no_args - 1].whites);
	}
      return &answer;
    }
  /* match args and shuffle if necessary */
  for (i = 0; i < answer.no_args; i++)
    {
      for (j = 0; j < answer.no_args; j++)
	{
	  if (strcmp(from_table[index].to.arg[i].variable,
		     from_table[index].from.arg[j].variable) == 0)
	    {
	      strcpy(answer.arg[i].variable, phrase->arg[j].variable);
	      strcpy(answer.arg[i].conv, phrase->arg[j].conv);
	      strcpy(answer.arg[i].whites,
		     phrase->arg[j < phrase->no_args ?
				j : phrase->no_args - 1].whites);
	      break;
	    }
	  /* if they're the same except for a string in the args */
	  if (samevar(from_table[index].to.arg[i].variable,
		      from_table[index].from.arg[j].variable))
	    {
	      strcpy(answer.arg[i].variable, 
		     from_table[index].to.arg[j].variable);
	      strcpy(answer.arg[i].conv, from_table[index].to.arg[j].conv);
	      strcpy(answer.arg[i].whites,
		     phrase->arg[j < phrase->no_args ?
				j : phrase->no_args - 1].whites);
	      break;
	    }
	}
      if (j == answer.no_args)
	{
	  fprintf(stderr,"\nVariables don't match up on the table entry for:\n");
	  print_entry(stderr, &from_table[index]);
	}
    }
  /* if the phrase is an exact match: */
  if (!strsame(phrase->string, from_table[index].from.string))
    {
      /* check the case on the words in the phrase */
      for (j = -1, i=0; phrase->string[i]; i++)
	{
	  if (IS_WHITE(phrase->string[i]))
	    continue;
	  /* skip arguments */
	  if (phrase->string[i] == '<' && phrase->string[i+1] == '%')
	    {
	      i += arglen(&phrase->string[i]);
	      continue;
	    }
	  if (++j >= LEN)  /* only check LEN words */
	    {
	      fprintf(stderr,"\ntrans: fix_caps: Only fixing caps on %d "
		      "words in %s.\n", j, phrase->string);
	      break;
	    }
	  word[j] = check_caps(&phrase->string[i]);
	  while (phrase->string[i] && !IS_WHITE(phrase->string[i]))
	    i++;
	  i--;
	}
      last_in_word = j ;
      for (j = -1, i = 0; string[i]; i++)
	{
	  if (IS_WHITE(string[i]))
	    continue;
	  /* skip arguments */
	  if (string[i] == '<' && string[i+1] == '%')
	    {
	      i += arglen(&string[i]);
	      continue;
	    }
	  make_caps(&string[i], 
		    word[(++j > last_in_word ? last_in_word : j)]);
	  while (string[i] && !IS_WHITE(string[i]))
	    i++;
	  i--;
	}
    }
  return &answer;
}
/************************************************************
 *    It starts at file[i] and looks for the next argument
 *    and passes back the string that represents the argument
 *    in the table.  end_args is the position in the file[]
 *    where the arguments end so that the caller won't reparse
 *    the argument. *p_arg is where the argument details should
 *    land. 
 *************************************************************/	
char *
get_arg(int i, ARG * p_arg, int * end_args, YESorNO new_file)
{
  static int sofar;
  static char arg[500];
  YESorNO cat = NO;
  int k = -1;
  int w = -1, v = -1;
  int parens = 0;
  YESorNO quotes = NO;
  YESorNO started = NO;
  if (new_file)
    sofar = 0;
  i--;
  do
    {
      cat = NO;
      while (++i < length)  /* find end of the string */
	{
	  if (file[i] == '"' && file[i-1] != '\\')
	    break;
	}
      while (++i < length)
	{  /* concatinating strings */
	  if (file[i] == '"')
	    {
	      cat = YES;
	      break;
	    }
	  if (file[i] == ',')
	    break;
	}
    }
  while (cat == YES);
  if (i < sofar)  /* Don't do the same argument twice */
    i = sofar;
  while (++i < length)
    {
      if (file[i] == '(')  /* keep track of parens and quotes */
	{
	  parens++;
	}
      else if (file[i] == '\\' && file[i+1] == '"')
	{
	  if (quotes == YES)
	    quotes = YES;
	  else
	    quotes = NO;
	}
      else if (file[i] == ')' && parens > 0)
	{
	  parens--;
	}
      else if (file[i] == ',' || file[i] == ')')
	{
	  if (parens == 0)
	    {
	      p_arg->variable[++v] = '\0';
	      arg[++k] = '\0';
	      sofar = i;  /* for next time */
	      *end_args = i;
	      return arg;
	    }
	}
      if (!started
	  && IS_WHITE(file[i]))
	{
	  p_arg->whites[++w] = file[i];
	  continue;
	}
      if (!started)
	{
	  /*arg[++k] = ':';  */
	  p_arg->whites[++w] = '\0';
	  started = YES;
	}
      p_arg->variable[++v] = file[i];
      arg[++k] = file[i];
    }
  return NULL;  // impossible
}
/************************************************************
 *     Reads the next line of the file and expects it to match
 *     one of the language flags.
 *     Passes back which language.
 *************************************************************/
LANGUAGE
get_language(FILE *fp)
{
  int i;
  char line[LEN + 1];
  static LANGUAGE the_language;
  char * the_line;
  YESorNO good_line;
  the_language.name = "";
  while ((the_line = fgets(line, LEN, fp)) != NULL)
    {
      if (line[0] == '#')
	continue;
      good_line = NO;
      for (i=0; line[i]; i++)
	{
	  if (!IS_WHITE(line[i]))
	    good_line = YES;
	  break;
	}
      if (good_line)
	break;
    }
  if (the_line == NULL)
    return the_language;
  /* find the language from the language line */
  for (i = 0; i < no_languages; i++)
    {
      if (starts_same(language[i].name, line))
	break;
    }
  if (i == no_languages)
    {
      for (i = 0; i < no_languages; i++)
	{
	  if (starts_same(language[i].flag, line))
	    break;
	}
      if (i == no_languages)
	{
	  fprintf(stderr, "\nDon't recognize %s as a language\n", 
		  line);
	  exit(1);
	}
    }
  if ((the_language = language[i]).tongue != to.tongue
      && the_language.tongue != from.tongue)
    {
      fprintf(stderr,"\nThere is %s in the table file for %s to %s.\n",
	      the_language.name, from.name, to.name);
      exit(1);
    }
  return the_language;
}
/************************************************************
 *      Returns YES if the phrase is something we want
 *      to translate.  No if not.
 *************************************************************/
YESorNO
is_something(char *str, int * starts)
{
  YESorNO something = MAYBE;
  int brackets, j, parens, quotes;
  /* if it's just an argument, never mind */
  something = NO;
  brackets = 0;
  parens = 0;
  quotes = 0;
  for (j = 0; str[j]; j++)
    {
      if (str[j] == '(' && brackets)
	parens++;
      if (str[j] == ')' && brackets)
	parens--;
      if (str[j] == '>')  /* maybe something */
	{
	  if (parens)  /*  a ' >= ' etc. */
	    continue;
	  if (str[j-1] != '-')
	    brackets--;
	  continue;
	}
      if (str[j] == '<' && str[j+1] == '%')
	{
	  brackets++;
	  continue;
	}
      /* Look for strings that are in conditional arguments like:
	 printf("\n%d. %s", i+1, (pet_vote[i] == 0 ? "No" : 
	 (pet_vote[i] == 1 ? "Yes" : "Don't know"))); */
      if (brackets && str[j] == '"')
	{
	  quotes += quotes? -1: 1;
	}
      if ((brackets == 0 || (brackets && quotes))
	  && ((str[j] >= 'a' && str[j] <= 'z')
	      || (str[j] >= 'A' && str[j] <= 'Z')))
	{
	  something = YES;
	  break;
	}
    }
  if (something == NO)   /* whole phrase is arguments */
    return NO;
  /* strip the eVote out */
  if (strncmp("eVote ", str, 6) == 0)
    {
      for (j = 0; str[j]; j++)
	{
	  str[j] = str[j+6];
	}
      *starts += 6;
      str[j] = '\0';
    }
  /* check for skippers */
  for (j = 0; j < no_skips; j++)
    {
      if (strcmp(str, skip[j]) == 0)
	{
	  return NO;
	}
    }
  /* last check to be sure that there is
     some real language in the string */
  for (j = 0; str[j]; j++)
    {
      if ((str[j] >= 'a' && str[j] <= 'z')
	  || (str[j] >= 'A' && str[j] <= 'Z'))
	{
	  if ((str[j] == 'n' || str[j] == 't')
	      && j > 0 && str[j-1] == '\\')
	    continue;
	  return YES;
	}
    }
  return NO;
}
/************************************************************
 * It makes str conform to the convention that how specifies.
 *************************************************************/
void
make_caps(char *str, CAPS how)
{
  int i;
  for (i = 0; str[i] && !IS_WHITE(str[i]); i++)
    {
      switch (how)
	{
	case STARTS:
	  if (i != 0)
	    break;
	  how = NONE;  /* i == 0, next time no caps */
	case ALL:
	  if (islower(str[i]))
	    str[i] = toupper(str[i]);
	  break;
	case NONE:
	  if (isupper(str[i]))
	    str[i] = tolower(str[i]);
	  break;
	case MIX: // never happens
	  break;
	}
    }
}
/************************************************************
 * Resorts the from_table into to_table, sorted by the to language.
 *************************************************************/
void
make_to_table(void)
{
  int i, j;
  to_table[0] = from_table[0];
  for (i=1; i < no_in_table; i++)
    {
      /*printf("\n%d = %s", i, from_table[i].to.string); */
      for (j = i-1; j >= 0; j--)
	{
	  if (before(from_table[i].to.string, to_table[j].to.string) == YES)
	    {
	      to_table[j+1] = to_table[j];
	      /*printf("\nmoved %s down to %d", to_table[j+1].to.string,
		j+1); */
	    }
	  else
	    {
	      /*printf("\n%s goes before %s", to_table[j].to.string,
		from_table[i].to.string); */
	      break;
	    }
	}
      to_table[j+1] = from_table[i];
    }
  /*	for (k = 0; k < no_in_table; k++)
	{
	printf(" %.8s", to_table[k].to.string);
	}  */
}
/************************************************************
 *    str1 and str2 are argument strings <%hd: ntt : name>
 *                                    or <%hd:name>
 *************************************************************/
YESorNO
match_args(char *str1, char *str2)
{
  int i, j;
  int brackets = 0;
  for (i = 0, j = 0; str1[i] && str2[j] ; i++, j++)
    {
      if (str1[i] != str2[j])
	return NO;
      if (str1[i] == '(')
	{
	  i += parenlen(&str1[i]) -1;
	  j += parenlen(&str2[j]) -1;
	  continue;
	}
      if (str1[i] == '<' && str1[i+1] == '%')
	brackets++;
      if (str1[i] == '>' && i > 0 && str1[i-1] != '-')
	{
	  if (--brackets)
	    continue;
	  return YES;
	}
    }
  return MAYBE;
}
/*************************************************************
 *      Returns a pointer to the next phrase in the file.
 *      Copy the phrase because the next phrase will wipe
 *      out the previous phrase.
 *      Fills in the arg array.
 *  *starts = first char in the string that is not white space
 *  *string_ends = last char in the string that is not white space
 *                 or punctuation ',', ':', '.', ':'
 *  *ends = first character beyond the phrase, i.e.
 something  ");
 ^
 something", arg);
 ^
 something";
 ^
*************************************************************/	
char buffer[STRINGLEN + 1];
PHRASE *
next_phrase(int *starts, int * string_ends, int * ends)
{
  static PHRASE phrase;
  static int i = 0;
  int k = -1;
  int j, l;
  YESorNO inquote = NO;
  YESorNO done;
  int end_args = 0;
  YESorNO new_file = YES;
  int arg_index = -1;
  /*	for (j = 0; j < phrase.no_args; j++)
	{
	phrase.arg[j].conv[0] = '\0';
	phrase.arg[j].whites[0] = '\0';
	phrase.arg[j].variable[0] = '\0';
	} */
  *starts = 0;
  *ends = 0;
  *string_ends = 0;
  phrase.string = buffer;
  phrase.no_args = 0;
  for (; i < length; i++)
    {
      /* look for character constants to translate */
      if (k == -1  && file[i] == '\'' && file[i+2] == '\''
	  && isalpha(file[i+1]))
	{
	  for (j = i+3; j < length  && file[j] != '\n' ; j++)
	    {
	      if (file[j] == '/' && file[j+1] == '*')
		{  /* found comment */
		  j += 2;
		  while (IS_WHITE(file[j]))
		    {
		      j++;
		    }
		  j--;
		  while (file[++j] != '*' || file[j+1] != '/')
		    {
		      buffer[++k] = file[j];
		    }
		  if (k == -1)
		    break;
		  while (IS_WHITE(buffer[k] ))
		    k--;
		  buffer[++k] = '\0';
		  *starts = i+1;
		  *string_ends = i+1;
		  *ends = j+2;
		  i = j+2;
		  return &phrase;
		}
	    } /* found '\n' before comment starts, falls out */
	}  /* character constant code */
      /* skip comments */
      if (file[i] == '/' && file[i+1] == '*')
	{
	  for (; i < length; i++)
	    {
	      if (file[i] == '*' && file[i+1] == '/')
		break;
	    }
	  i++;
	  continue;
	}
      if (file[i] == '"' && file[i-1] != '\\') /* found quote */
	{
	  if (inquote)
	    {
	      j = i;
	      while (++j < length)  /* look for more quote to cat */
		{
		  if (file[j] ==  '"')
		    {
		      i = j + 1;  /* skip to concatonate */
		      *string_ends -= 2;  /* if the next string
					     is just white space,
					     subtract the two "'s */
		      /*concat = YES;  */
		      j = length;
		      break;
		    }
		  else if (file[j] == ',' || file[j] == ')' || file[j] == ':'
			   || file[j] == ';' || file[j] == '#'
			   || (file[j] >= 'a' && file[j] <= 'z')
			   || (file[j] >= 'A' && file[j] <= 'Z'))
		    /* end of quote for sure */
		    {
		      buffer[++k] = '\0';
		      /*if (concat)
		       *string_ends += i-1;
		       else */
		      *string_ends = i-1;
		      *ends = j;
		      if (end_args > *ends)
			*ends = end_args;
		      i = *ends + 1;  /* start past quote */
		      /* next time */
		      (*string_ends) -= big_trunc(buffer);
		      if (is_something(buffer, starts))  
			return &phrase;
		      /* skip this one and start over */
		      k = -1;
		      inquote = NO;
		      phrase.no_args = 0;
		      arg_index = -1;
		      *starts = 0;
		      *string_ends = 0;
		      *ends = 0;
		      break;  /* start again */
		    }
		  else  /* white space after string ends */
		    {
		      (*string_ends)--;
		      continue;
		    }
		} /* while looking for end of statement */
	    } /* inquote  and found end of quote */
	  else  /* not in quote and found start of quote */
	    {   /* #include"something" not translated */
	      j = i;
	      while (--j > 0 && file[j] != '\n')
		;
	      if (file[j] == '\n')
		j++;
	      if (strncmp(&file[j],"#include", 7) == 0)
		{  /* dump this line */
		  while (++i < length && file[i] != '\n')
		    ;
		}
	      else
		{
		  if (file[i+1] != '\"'
		      && (i > 0 
			  && (file[i-1] != '\'' || file[i+1] != '\'')))
		    {
		      inquote = YES;
		    }
		  i++;
		}
	    }
	}
      if (inquote)
	{
	  done = NO;
	  if (k == -1)  /* haven't started yet */
	    {
	      if (file[i] == '.' || file[i] == ',' || file[i] == ':'
		  || file[i] == ';')
		continue;
	      if (file[i] == '\\'
		  && (file[i+1] == 't' || file[i+1] == 'n'
		      || file[i+1] == '\\'))
		{
		  i++;
		  continue;
		}
	      if (strncmp(&file[i], "eVote", 5) == 0)
		{
		  i += 4;
		  continue;
		}
	    }
	  if (file[i] == '\\' 
	      && ((file[i+1] == 't' || file[i+1] == 'n'
		   || file[i+1] == '\\' || file[i+1] == '"')))
	    {
	      switch (file[i+1])
		{
		case 't':
		  buffer[++k] = '\t';
		  break;
		case 'n':
		  buffer[++k] = '\n';
		  break;
		case '"':
		  if (k == -1)
		    *starts = i;
		  buffer[++k] = '\"';
		  break;
		case '\\':
		  if (k == -1)
		    *starts = i;
		  buffer[++k] = '\\';
		  break;
		}
	      i++;
	      continue;
	    }
	  if (file[i] == '%')
	    {
	      if (file[i+1] == '%')
		{
		  i++;  /* skip one */
		  continue;
		}
	      /* manufacture the argument */
	      if (k == -1)
		*starts = i;
	      buffer[++k] = '<';
	      buffer[++k] = '%';
	      l = -1;
	      arg_index++;
	      phrase.no_args++;
	      phrase.arg[arg_index].conv[++l] = '%';
	      while (++i < length)
		{
		  buffer[++k] = file[i];
		  phrase.arg[arg_index].conv[++l] = file[i];
		  /* recognize the end of the conversion */
		  if (IS_CONV_CHAR(file[i]))
		    {
		      done = YES;
		      phrase.arg[arg_index].conv[++l] = '\0';
		      buffer[++k] = ':';
		      strcpy(&buffer[++k], get_arg(i, &phrase.arg[arg_index], 
						   &end_args, new_file));
		      new_file = NO;
		      while (buffer[++k] != '\0')
			;
		      buffer[k] = '>';
		      break;
		    }
		}
	    }  /* % done */
	  if (!done)
	    {
	      if (k == -1 && IS_WHITE(file[i]))
		continue;
	      if (k == -1 && file[i] == '%')
		*starts = i-1;
	      else if (k == -1)
		*starts = i;
	      *string_ends = 0;
	      buffer[++k] = file[i];
	      if (k > STRINGLEN - 100)
		{
		  fprintf(stderr,
			  "\ntrans: next_phrase: Out of buffer space.\n");
		  exit(1);
		}
	    }
	}
    }
  if (k == -1)
    {
      i = 0;
      return NULL;
    }
  return NULL;
}
/************************************************************
 *        Returns the next entry from the translation table
 ************************************************************/
TABLE
next_table(FILE *fp)
{
  static TABLE answer;
  static char to_buffer[STRINGLEN + 1];
  static char from_buffer[STRINGLEN + 1];
  char line[LEN + 1];
  LANGUAGE lang1, lang2;
  if (answer.to.string == NULL)
    {
      answer.to.string = to_buffer;
      answer.from.string = from_buffer;
      rewind(fp);
      /* find first row of stars -- allows for documentation
	 at the top */
      while (fgets(line, LEN, fp) != '\0')
	{
	  if (strcmp(line, STARS) == 0)
	    break;
	}
    }
  answer.to.no_args = 0;
  answer.from.no_args = 0;
  answer.to.string[0] = '\0';
  answer.from.string[0] = '\0';
  if ((lang1 = get_language(fp)).name[0] == '\0')
    return answer;
  collect((lang1.tongue == to.tongue ? to_buffer: from_buffer),
	  fp, DASHES);
  lang2 = (lang1.tongue == to.tongue ? from : to);
  if (lang2.tongue != get_language(fp).tongue)
    {
      fprintf(stderr, 
	      "\nLanguages are messed up in the %s to %s table.\n",
	      from.name, to.name);
      exit(1);
    }
  collect((lang2.tongue == to.tongue ? to_buffer: from_buffer), fp, STARS);
  parse_args(&answer.to);
  parse_args(&answer.from);
  return answer;
}
/************************************************************/
int
parenlen(char *str)
{
  int i;
  int parens;
  for (i = 0, parens = 0; str[i]; i++)
    {
      if (str[i] == ')')
	{
	  if (--parens == 0)
	    return i + 1;
	  continue;
	}
      if (str[i] == '(')
	parens++;
    }
  if (str[i] == '\0')
    return -1;
  return 0;
}
/************************************************************
 *        reads the phrase.string for the arguments and
 *        fills the arg array and no_args.
 *        Note that whites will be blank.
 ************************************************************/
void
parse_args(PHRASE * phrase)
{
  int i, brackets, j;
  char * str = phrase->string;
  phrase->no_args = 0;
  for (i = 0; str[i] ; i++)
    {
      if (str[i] != '<' || str[i+1] != '%')
	continue;
      /* collect conversion characters */
      j = -1;
      while (str[++i] && !IS_CONV_CHAR(str[i]))
	phrase->arg[phrase->no_args].conv[++j] = str[i];
      phrase->arg[phrase->no_args].conv[++j] = str[i];
      phrase->arg[phrase->no_args].conv[++j] = '\0';
      i += 2; /* skip ':' */
      for (brackets = 1, j = -1; str[i]; i++)
	{
	  if (str[i] == '>' 
	      && str[i-1] != '-')
	    {
	      if (--brackets == 0)
		break;
	    }
	  if (str[i] == '<')
	    brackets++;
	  phrase->arg[phrase->no_args].variable[++j] = str[i];
	}
      phrase->arg[phrase->no_args++].variable[++j] = '\0';
    }
  return;
}
/************************************************************
 *      Prints the args for the phrase
 *************************************************************/
void
print_args(FILE* fp, PHRASE *p_phrase)
{
  int i;
  if (p_phrase->no_args == 0)
    return;
  for (i = 0; i < p_phrase->no_args; i++)
    {
      if (i > 0)
	fprintf(fp,",");
      fprintf(fp,"%s%s", p_phrase->arg[i].whites, 
	      p_phrase->arg[i].variable);
    }
}
/************************************************************/
void
print_entry(FILE *fp, TABLE *entry)
{
  fprintf(fp,"%s", STARS);
  fprintf(fp,"%s:\n", from.flag);
  fprintf(fp,"%s\n", entry->from.string);
  fprintf(fp,"%s", DASHES);
  fprintf(fp,"%s:\n", to.flag);
  if (starts_same(entry->to.string, UNKNOWN))
    fprintf(fp,"%s", UNKNOWN);  /* controlling '\n' */
  else
    fprintf(fp,"%s\n", entry->to.string);
}
/************************************************************/
void
print_table(void)
{
  int i;
  FILE *fp;
  char file_name[LEN + 1];
  char command[2*LEN +1];
  char new_file_name[LEN + 1];
  FILE * new_fp;
  int knowns = 0;
  int unknowns = 0;
  sprintf(new_file_name,"%s_%s.new", from.flag, to.flag);
  if ((new_fp = fopen(new_file_name,"w")) == NULL)
    {
      fprintf(stderr,"\nCan't open %s.\n", new_file_name);
    }
  if (table_exists)
    {
      sprintf(file_name, "%s_%s", from.flag, to.flag);
      sprintf(command,"cp %s %s.back", file_name, file_name);
      system(command);
      if ((fp = fopen(file_name, "w")) == NULL)
	{
	  fprintf(stderr,"\nCan't open %s.\n", file_name);
	  return;
	}
    }
  for (i = 0; i < no_in_table; i++)
    {
      if (starts_same(from_table[i].to.string, UNKNOWN))
	{
	  print_entry(new_fp, &from_table[i]);
	  unknowns++;
	}
      else if (table_exists)
	{
	  knowns++;
	  print_entry(fp, &from_table[i]);
	}
    }
  if (table_exists && knowns)
    {
      fprintf(fp,"%s\n", STARS);
      fclose(fp);
    }
  if (unknowns)
    fprintf(new_fp,"%s\n", STARS);
  fclose(new_fp);
}
/************************************************************/
OKorNOT
process_file(char * name)
{
  FILE *fp;
  if ((fp = fopen(name,"r")) == NULL)
    {
      fprintf(stderr,"\nCan't find %s.\n", name);
      return NOT_OK;
    }
  /* puts entire file in memory in the file[] array */
  collect_file(fp);
  fclose(fp);
  write_code(name);
  return OK;
}
/************************************************************
 *   Returns YES if they are the same variable up to a string.
 *   Blanks don't count.
 ************************************************************/
YESorNO
samevar(char *str1, char* str2)
{
  int i, j;
  for (i = 0, j = 0; str1[i] && str2[j] ; i++, j++)
    {
      while (str1[i] == ' ')
	i++;
      while (str2[j] == ' ')
	j++;
      if (str1[i] != str2[j])
	return NO;
      if (str1[i] == '"')
	return YES;
    }
  if (str1[i] == '\0' && str2[j] == '\0')
    return YES;
  return NO;
}
/************************************************************
 *         returns the number of chars in the < argument >
 *                 -1 if there's trouble.
 *                  0 if there's a string in the argument.
 ************************************************************/
int
skip_arg(char *str)
{
  int i;
  int brackets;
  for (i = 0, brackets = 0; str[i]; i++)
    {
      if (str[i] == '"')
	return 0;
      if (str[i] == '>' && i > 0
	  && str[i-1] != '-')
	{
	  if (--brackets == 0)
	    return i; /* + 1; */
	  continue;
	}
      if (str[i] == '<')
	brackets++;
    }
  if (str[i] == '\0')
    return -1;
  return 0;
}
/*************************************************************
 *  Sorts the table and removes the duplicates.
 *  Frees the space that had duplicates and returns the new
 *  number in the array.  Also calls sort on the to_table.
 ************************************************************/
void
sort_tables(YESorNO print_dups)
{
  int i, j;
  TABLE holder;
  YESorNO before_this;
  /* classic insertion sort */
  no_in_table = new_no_in_table;
  for (i = 1; i < no_in_table; i++)
    {
      holder = from_table[i];
      for (j = i - 1; j >= 0; j--)
	{
	  if ((before_this = before(holder.from.string, 
				    from_table[j].from.string)) == YES)
	    {
	      from_table[j+1] = from_table[j];
	    }
	  else
	    {
	      break;
	    }
	}
      from_table[j+1] = holder;
    }
  /* get out the duplicates */
  for (i = 0; i < no_in_table - 1; i++)
    {
      if (before(from_table[i].from.string,
		 from_table[i+1].from.string) == MAYBE)  /* same*/
	{
	  if (!starts_same(from_table[i].to.string, UNKNOWN) 
	      && !starts_same(from_table[i+1].to.string, UNKNOWN)
	      && before(from_table[i].to.string, 
			from_table[i+1].to.string) != MAYBE)
	    {
	      if (print_dups)
		{
		  fprintf(stderr, "\nThere are two different translations for \n\t%s:\n\n%s\n\tand\n%s\n",
			  from_table[i].from.string, from_table[i].to.string, 
			  from_table[i+1].to.string);
		}
	      continue;  /* leaving both in for now */
	    }
	  /* if the first one has an answer, keep it */
	  if (!starts_same(from_table[i].to.string, UNKNOWN)
	      && starts_same(from_table[i+1].to.string, UNKNOWN))
	    from_table[i+1] = from_table[i];
	  /* take out the duplicate */
	  if (print_dups)
	    {
	      fprintf(stderr,"\nRemoving this duplicate:\n");
	      print_entry(stderr, &from_table[i]);
	    }
	  free(from_table[i].from.string);
	  if (!starts_same(from_table[i].to.string, UNKNOWN))
	    free(from_table[i].to.string);
	  for (j = i; j < no_in_table - 1; j++)
	    {
	      from_table[j] = from_table[j+1];
	    }
	  from_table[j].from.string = NULL;
	  from_table[j].to.string = NULL;
	  no_in_table--;
	  i--;
	}
    }
  new_no_in_table = no_in_table;
  make_to_table();
}
/************************************************************
 *     Returns YES if they are the same string according to
 *     the TO_SORT for as long as the shortest string.
 *************************************************************/
YESorNO
starts_same(char* str1, char * str2)
{
  int i;
  for (i = 0; str1[i] && str2[i] && str1[i] != '\n' && str2[i] != '\n' ; i++)
    {
      if (TOSORT(str1[i]) != TOSORT(str2[i]))
	{
	  return NO;
	}
    }
  return YES;
}
/************************************************************
 *          Includes case.
 *          Returns YES if they are the same
 *                   NO if they are not.
 *          It ignores the string's arguments that
 *          are wrapped in <>'s -- unless there's a string
 *          in the argument.
 *************************************************************/	
YESorNO
strsame(char *str1, char *str2)
{
  int i, j;
  for (i = 0, j = 0; str1[i] && str2[j] ; i++, j++)
    {
      if (str1[i] != str2[j])
	return NO;
      if (str1[i] == '<' && str1[i+1] == '%')
	i += skip_arg(&str1[i]);
      if (str2[j] == '<' && str2[j+1] == '%')
	j += skip_arg(&str2[j]);
    }
  if (str1[i] == '\0' && str2[j] == '\0')
    return YES;
  return NO;
}
/************************************************************
 *      Truncates the white space from the end of the string
 *      and returns the number of characters stripped.
 *************************************************************/
int
trunc(char *str)
{
  int len;
  int i = 0;
  len = strlen(str);
  while (--len >= 0)
    {
      if (IS_WHITE(str[len]))
	{
	  str[len] = '\0';
	  i++;
	}
      else
	break;
    }
  return i;
}
/************************************************************
 *     Returns the to_phrase or NULL.  If it's not in the table,
 *     it adds it to the table.
 ************************************************************/
PHRASE
*whats(PHRASE *from_phrase)
{
  int i;
  for (i = 0; i < no_in_table; i++)
    {
      switch (before(from_phrase->string, from_table[i].from.string))
	{
	case NO:
	  continue;
	  break;
	case YES:
	  break;
	case MAYBE:
	  if (starts_same(from_table[i].to.string, UNKNOWN))
	    {
	      return NULL;
	    }
	  return fix_phrase(from_phrase, i);
	  break;
	}
      break;  /* only the NO case */
    }
  for (i = 0; i < no_in_table; i++)
    {
      switch (before(from_phrase->string, to_table[i].to.string))
	{
	case NO:
	  continue;
	  break;
	case YES:
	  break;
	case MAYBE:  /* already in to language */
	  return NULL;
	  break;
	}
      break;  /* only NO falls through */
    }
  /* not in table */
  if (new_no_in_table >= TABLELEN)
    {
      fprintf(stderr,"\ntrans: whats: Out of table space.\n");
      exit(1);
    }
  copy_phrase(&(from_table[new_no_in_table].from), from_phrase);
  copy_phrase(&(from_table[new_no_in_table++].to), &unknown_phrase);
  return NULL;
}
/************************************************************
 *         Finds each phrase in file_name and translates
 *         it into the new source code.
 *************************************************************/
void
write_code(char * file_name)
{
  char name[LEN + 1];
  char ext[10];
  int i = 0;
  int starts, string_ends, ends, len;
  FILE *fp;
  PHRASE * new_phrase;
  PHRASE * trans;
  int j;
  int no_args;
  YESorNO seen_quote;
  strcpy(name, file_name);
  len = strlen(name);
  while (--len > 0 && name[len] != '.')
    ;
  strcpy(ext, &name[len]);
  sprintf(&name[strlen(name)-strlen(ext)], "_%s%s", to.flag, ext);
  if ((fp = fopen(name, "w")) == NULL)
    {
      fprintf(stderr, "\nCan't open %s for translated %s.\n",
	      name, file_name);
      exit(1);
    }
  if (debug)
    printf("\n\nParsing %s and writing %s.\n", file_name, name);
  while ((new_phrase = next_phrase(&starts, &string_ends, &ends)) 
	 != NULL)
    {
      if (debug)
	printf("\n%s:%s\n", from.flag, new_phrase->string);
      for (; i < starts; i++)
	{
	  fputc(file[i], fp);
	}
      if ((trans = whats(new_phrase)) == NULL)
	continue;  /* no translation, leave untranslated */
      if (debug)
	printf("\n%s:%s\n", to.flag, trans->string);
      if (starts == string_ends)  /* single character constant */
	{
	  fputc(isupper(file[starts]) ?
		toupper(trans->string[0]) : tolower(trans->string[0]),
		fp);
	  /* write file including comment delimiter */
	  for (i++ ; file[i-2] != '/' || file[i-1] != '*'; i++)
	    {
	      fputc(file[i], fp);
	    }
	  fprintf(fp, " %s */", trans->string);
	  i = ends;
	  continue;
	}
      no_args = 0;
      for (j = 0; trans->string[j]; j++)
	{
	  switch (trans->string[j])
	    {
	    case '%':
	      fputc('%', fp);  /* put back extra '%' */
	      break;
	    case '\n':
	      fprintf(fp,"\\n");
	      /*							fprintf(fp,"\"\n\t\"\\n"); */
	      continue;
	    case '\t':
	      fprintf(fp,"\\t");
	      continue;
	    case '\"':
	      fprintf(fp,"\\\"");
	      continue;
	    case '<':
	      if (trans->string[j+1] == '%')
		{
		  j += arglen(&(trans->string[j]));
		  fprintf(fp, "%s", trans->arg[no_args++].conv);
		  continue;
		}
	      break;
	    default:
	      break;
	    }
	  fputc(trans->string[j], fp);
	}
      seen_quote = NO;
      for (i = string_ends + 1; i < length; i++)
	{
	  if (seen_quote &&
	      ((file[i] == ')' || file[i] == ';' || file[i] == ':'
		|| (file[i] == ',' && no_args == 0))))
	    break;
	  fputc(file[i], fp);
	  if (file[i] == '"')
	    seen_quote = YES;
	  if (file[i] == ',')
	    break;
	}
      for (; i < length; i++)
	{
	  if (IS_WHITE(file[i]))
	    fputc(file[i], fp);
	  else
	    break;
	}
      print_args(fp, trans);
      i = ends;
    }
  for (; i < length; i++)
    {
      fputc(file[i], fp);
    }
  fclose(fp);
}
