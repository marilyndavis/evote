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

/* $Id: pick.c,v 1.5 2003/01/15 18:54:13 marilyndavis Exp $ */ 
/*********************************************************
 *   pick.c   -   Can be called by wrapper from the
 *                  sendmail aliases file:
 *    in_address: "|/usr/local/wrapper pick -l en english_address \
 *          -l es spanish_address -l do don't_know_address \
 *          -f from_address"
 *     will pick out the english language messages and pass them on to
 *     english_address, etc.  The sent message will be From: from_address.
 *     Certain strings can be eliminated from the incoming message.
 *     Specify those strings in pick.h
 ************************************************************
 *   To compile, alter the makefile and type:
 *   make
 *   Put the resulting executable in with your majordomo
 *   scripts and you're ready.
 *********************************************************
 *    by Marilyn Davis and John J. Jacq
 **********************************************************/
#include "filter.h"
#include "pick.h"
int
main(int argc, char *argv[], char *env[])
{
  LANGUAGE language;
  int langs, i;
  program = "pick";
  langs = read_command_line(argc, argv);  /* also gets addresses */
  read_message(argc, argv, env);
  language = langtest();
  for (i = 0 ; i < langs; i++)
    {
      if (pick[i] == language && send_address[i] != NULL)
	{
	  mail_it(send_address[i], from_address, subject);
	  dump_message(stdout, YES, NO, NO);
	  break;
	}
    }
  exit(0);
}
/************************************************************
 *     Returns the number of language flags.
 *     external forward_to_address is gathered.
 *     Generates errors to stderr.
 *************************************************************/
int
read_command_line(int argc, char *argv[])
{
  int k, i;
  for (k = 0, i = 1 ; i < argc ; i++)
    {
      if (same(argv[i], "-l"))
	{
	  if (k >= MAX_LANGS)
	    {
	      fprintf(stderr,"Only %d languages can be picked.\n",
		      MAX_LANGS);
	      arg_err();
	    }
	  pick[k] = which_is(argv[++i]);
	  send_address[k++] = argv[++i];
	  continue;
	}
      if (same(argv[i], "-f"))
	{
	  from_address = argv[++i];
	  continue;
	}
    }
  if (k == 0 || !from_address)
    {
      arg_err();
    }
  return k;
}
/************************************************************
 *    Help message for command line errors.
 *************************************************************/
void
arg_err(void)
{
  fprintf(stderr,
	  "\npick -l do confused_address -l en english_address -l es spanish_address ...-f from_address\n");
  exit(1);
}
/************************************************************
 *     Initializes itself to msg_start and returns tokens
 *     from the message buffer.
 *     It skips any strings listed in skippers[] in sorter.h
 *************************************************************/
#define TOKENLEN 20
char *
next_token(void)
{
  static char * so_far;
  static char token[TOKENLEN +1];
  int i = 0;
  if (so_far == 0)
    {
      so_far = &buffer[msg_start];
    }
  while (*so_far && IS_WHITE(*so_far))
    so_far++;
  if (*so_far == '\0')
    return NULL;
  check_skippers(&so_far);
  while (*so_far && !IS_WHITE(*so_far))
    {
      if (i < TOKENLEN)
	{
	  token[i++] = *so_far;
	}
      so_far++;
    }
  token[i] = '\0';
  return token;
}
/************************************************************
 *    Checks if the text at *here should be skipped
 *************************************************************/
int
check_skippers(char **here)
{
  char *ptr = *here;
  int i, j;
  static int no_skippers = sizeof(skipper)/sizeof(char*);
  static YESorNO started = NO;
  static int * len;
  if (!started)  /* do these things only once */
    {
      started = YES;
      len = malloc(sizeof(int) * no_skippers);
      for (i = 0; i < no_skippers; i++)
	{
	  j = 0;
	  while (IS_WHITE(skipper[i][j]))
	    j++;
	  skipper[i] = &skipper[i][j];
	  len[i] = strlen(skipper[i]);
	}
    }
  for (i = 0; i < no_skippers; i++)
    {
      if (strncmp(ptr, skipper[i], len[i]) == 0)
	{
	  ptr += len[i];
	  while (IS_WHITE(*ptr))
	    {
	      len[i]++;
	      ptr++;
	    }
	  *here = ptr;
	  return len[i];
	}
    }
  return 0;
}
/**************************************************************
 *      Program to scan a file and guess the language 
 *       Contributed by John Jacq with help from Marilyn Davis.
 *       (ver 980929)
 **************************************************************
 *       The function checks words against a collection of lexicons,
 *       each time a match is found, the lexicon counter is
 *       incremented.
 *          0 = unknown  *
 *          1 = English  *
 *          2 = Francais   *
 *          3 = Deutsch   *
 *          4 = Nahuatl
 *          5 = Espanol  *
 *          6 = Portuguese
 *          7 = Nordic (Swedish/Norwegian/Danish)
 *          8 = Italian  *
 *          9 = Croatian *
 *       The options followed by "*" are enabled now, others are 
 *       to come soon
 **************************************************************/
LANGUAGE
langtest(void)
{
  int count = 0;
  char * token;
  int collapse(char *, int max_len);
  /* read in each token */
  while ((token = next_token()) != NULL)
    {
      /* collapse removes all the ' and ~ and the coded
	 accents and returns the
	 length of the resultant string  -- up to 
	 MAX_WORD_LEN + 1 which is in token still */
      if (collapse(token, MAX_WORD_LEN) <= MAX_WORD_LEN)
	{
	  if (++count > 1000)  /* quit after 1000 words */
	    break;
	  checkword(token);  /* if 3 letter word, do a check*/
	}
    } /* End of while */
  return decide_language();
}
/************************************************************
 *   Check the  word against the lexicon and updates count.
 ***********************************************************/
void
checkword(char * s)
{
  int i, j;
  for (j = 1; j < NO_LANG; j++)
    {
      for (i=0; i < NO_WORDS; i++)
	{
	  if (dico[j].lex[i] == NULL)  /* for short lexicons */
	    break;
	  if (same(dico[j].lex[i], s))
	    {
	      dico[j].count++; /* match then increment count */  
	      break;
	    }
	}
    }
  return;
}
/************************************************************
 *      Here, the counting is done and the decision is made.
 *************************************************************/
LANGUAGE
decide_language(void)
{
  int i, j;
  LANGUAGE tongue = DONT_KNOW;
  sort[0] = dico;
  /*	for (i = 0; i < NO_LANG; i++)
	{
	printf("%d: %s %d\n", i, dico[i].name, dico[i].count);
	} */
  for (i = 1; i < NO_LANG; i++)
    {
      for (j = i - 1; j >= 0; j--)
	{
	  if (dico[i].count > sort[j]->count)
	    {
	      sort[j + 1] = sort[j];
	    }
	  else
	    {
	      break;
	    }
	}
      sort[j + 1] = &dico[i];
      /*
	printf("\n");
	for (j = 0 ; j <= i; j++)
	{
	printf("%d: %s %d\n", j, sort[j]->name, sort[j]->count);
	}
      */
    }
  if (sort[0]->count > sort[1]->count + 2 || (sort[0]->count > 1 &&
					      sort[1]->count == 0 ))
    tongue = sort[0]->tongue;
  return tongue;
}
/************************************************************
 *      takes out the '\'', '`','*','^',':' and replaces '~' 
 *      or "n~" with 'n' and puts the result in the incoming 
 *      address and returns the new strlen.
 *      Also de-accents the coded accents.
 *      It gives up and returns max_word_len + 1 when the
 *      result is longer than that.
 *************************************************************/
int
collapse(char * in, int max_word_len)
{
  /*  char deaccent(unsigned char ch); */
  char *out = in, *start = in;
  in--;
  while (*++in)
    {
      if (out - start > max_word_len)
	{
	  return out - start;
	}
      /*      *in = deaccent(*in); */
      *in = DEACCENT((unsigned char)*in);
      switch (*in)
	{
	case '~':
	  if (*(in-1) != 'n' && *(in-1) != 'N')
	    {
	      *out++ = 'n';
	    }
	  continue;
	  break;
	case '\'':
	case '`':
	case '^':
	case ':':
	case '*':
	  continue;
	  break;
	default:
	  *out++ = *in;
	  break;
	}
    }
  *out = '\0';
  return out - start;
}
/************************************************************
 *    Looks at the first two chars of the string, or, if the
 *    first char is '-', it looks at the 2nd and 3rd chars
 *    and returns the LANGUAGE that matches.
 *************************************************************/
LANGUAGE
which_is(char *str)
{
  int i;
  if (*str == '-')
    str++;
  for (i = 0; i < NO_LANG; i++)
    {
      if (samen(str, dico[i].name, 2))
	return (LANGUAGE)i;
    }
  return DONT_KNOW;
}
/*
char deaccent(unsigned char ch)
{
  return ((ch>=192 && ch<=197) || (ch>=224 && ch<=229)) ? 'a':
((ch >=200 && ch<=203 || ch >=232 && ch<=235)?'e':
 ((ch >=204 && ch<=207 || ch >=236 && ch<=239)?'i':
 ((ch >=210 && ch<=214 || ch >=242 && ch<=246)?'o':
 ((ch >=217 && ch<=220 || ch >=249 && ch<=252)?'u':
 ((ch ==221 || ch==207 || ch ==253 || ch==255)?'y':
 ((ch ==199 || ch==231) ? 'c': 
 ((ch ==209 || ch==241) ? 'n': ch))))))));
}
*/
