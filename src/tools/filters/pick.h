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

/* $Id: pick.h,v 1.3 2003/01/15 18:54:13 marilyndavis Exp $ */ 
/*  pick.h -- header for pick.c -- program to sort messages
 *            by language.    
 *********************************************************
 **********************************************************/
#include<stdio.h>
#define MAX_LANGS 10
/*************************************************************
 *  Add or subtract strings to the skipper list.  If the skipper
 *  text is found in the message, the text won't be counted as
 *  words in any language.
 *************************************************************/
char * skipper[] =
{"This message is forwarded to you by the editors of the Chiapas95
newslists.  To contact the editors write to: <chiapas@eco.utexas.edu>.
To submit material for posting send to: <chiapas-i@eco.utexas.edu>.",
"To unsubscribe from this list send a message containing the words
unsubscribe chiapas95 (or chiapas95-lite, or chiapas95-english, or
chiapas95-espanol) to majordomo@eco.utexas.edu.  Previous messages
are available from http://www.eco.utexas.edu/faculty/Cleaver/chiapas95.html
or gopher to Texas, University of Texas at Austin, Department of
Economics, Mailing Lists."};
typedef enum {DONT_KNOW, ENGLISH, FRANCAIS, DEUTSCH, NAHUATL, 
	      ESPANOL, PORTUGUESE, NORDIC, ITALIAN, CROATIAN, 
	      ERRORS, NO_LANG} LANGUAGE;
LANGUAGE pick[MAX_LANGS];
char * send_address[MAX_LANGS];
char * from_address;
void arg_err(void);
int check_skippers(char **here);
int read_command_line(int argc, char *argv[]);
char * replace_subject(int pairs);
LANGUAGE which_is(char *);
/* deaccenting test macro functions*/
#define DEACCENT(ch) (((ch>=192 && ch<=197) || (ch>=224 && ch<=229)) ? 'a':\
(((ch >= 200 && ch <= 203) || (ch >= 232 && ch <= 235)) ? 'e':\
 (((ch >= 204 && ch <= 207) || (ch >= 236 && ch <= 239)) ? 'i':\
 (((ch >= 210 && ch <= 214) || (ch >= 242 && ch <= 246)) ? 'o':\
 (((ch >= 217 && ch <= 220) || (ch >= 249 && ch <= 252)) ? 'u':\
 ((ch == 221 || ch == 207 || (ch == 253 || ch == 255)) ? 'y':\
 ((ch == 199 || ch == 231) ? 'c': \
 ((ch == 209 || ch == 241) ? 'n': ch))))))))
#define NO_WORDS 50		 
#define MAX_WORD_LEN 5
typedef struct
{
  unsigned long count;
  LANGUAGE tongue;
  char *name;             /* language name */
  char *lex[NO_WORDS];
} DICO; /* dictionary type definition - including word count*/
DICO * sort[NO_LANG];
LANGUAGE langtest(void);
void init_dico(void);
void checkword(char *);
LANGUAGE decide_language(void);
DICO dico[NO_LANG] ={
{ 0, DONT_KNOW, "Don't Know",
  { NULL}},
{ 0, ENGLISH, "English",
  {"the", "to", "of", "in", "and", "they", "a", "for", "is", "be", 
     "you", "on", "sets", "will", "are", "set", "that", "some", "have", "as", 
     "by", "name", "not", "this", "can", "with", "it", "which", "has", "from",
     "see", "when", "your", "if", "also", "but", "all", "must", "do", "or", 
     "into", "other", "an", "its", "under", "right", "free", "more", "so", 
     "any"}},
{0, FRANCAIS,"Francais",
  {"de", "la", "le", "et", "a", "il", "les", "un", "en", "du", "pas", "que", 
     "une", "des", "ne", "se", "dans", "vous", "est", "je", "qui", "au", 
     "sur", "son", "ce", "mais", "etait", "plus", "pour", "lui", "sa", 
     "bien", "avec", "tout", "dit", "on", "ses", "par", "elle", "avait", 
     "cette", "ces", "ou", "si", "comme", "sans", "quand", "aux", "y", 
     "meme"}},
{0, DEUTSCH, "Deutsch",
  {
    "der", "nicht", "die", "und", "ist", "ein", "den", "auch", "das", "sich", 
      "auf", "er", "von", "wenn", "wird", "an", "so", "sein", "immer", "wer", 
      "fuer", "pfeil", "zu", "eine", "zum", "wie", "einem", "alle", "bei", 
      "dies", "ende", "genau", "weiss", "noch", "ich", "dass", "einen", 
      "ganze", "darf", "frag", "gerne", "gar", "doch", "guckt", "erst", 
      "mir", "meine", "sonst", "soll", "oder"}},
{0, NAHUATL, "Nahuatl",
  {NULL}},
{0, ESPANOL, "Espanol",
  {
    "de", "la", "y", "el", "que", "los", "a", "en", "no", "se", "las", 
      "del", "con", "por", "para", "un", "es", "su", "al", "una", 
      "como", "lo", "todos", "mas", "paz", "esta", "solo",
      "sus", "pero", "todo", "si", "le", "ya", "ha", "ni",
      "estos", "asi", "nos", "son", "pais", "ser", "tan",
      "este", "desde", "o", "ley", "lado", "hay", "otra", "abajo"}},
{0, PORTUGUESE, "Portuguese",
  {
    "a", "de", "o", "da", "e", "do", "um", "que", "os",
      "feira", "para", "pelo", "dos", "uma", "em", "na", "sobre",
      "nano", "nos", "sem", "por", "foi", "trens", "hoge",
      "ano", "das", "anos", "as", "causa", "no", "grau", "neste",
      "greve", "pela", "num", "esta", "agora", "cinco", "mais",
      "apoyo", NULL}},
{0, NORDIC,"Nordic",
  {NULL}},
{0, ITALIAN,"Italian",
  {
    "di", "e", "che", "il", "per", "i", "a", "in", "le",
      "un", "la", "non", "del", "una", "gli", "se", "della",
      "con", "dei", "da", "come", "si", "mondo", "loro", "tutti",
      "delle", "sono", "pi", "ma", "dal", "ocse", "ci", "anche",
      "ed", "alla", "tutte", "degli", "o", "noi", "piu", "nelle",
      "ai", "nel", "alle ", "modo", "anni", "al", "ogni", "stato", "sugli"}},
{0, CROATIAN, "Croatian",
  { 
    "u", "je", "i", "sve", "se", "da", "smo", "imamo", "o",
      "ali", "treba", "ne", "prvi", "na", "svake", "za", "drugi",
      "netko", "sto", "ako", "nas", "jos", "put", "me", "tome",
      "ste", "svoje", "nego", "bolje", "druge", "bolji", "sam",
      "toga", "prije", "svega", "sa", "dobro", "moze", "misle",
      "to", "mnogi", "spas", "radu", "biti", "svi", "sada",
      "svim ", "narod", "kakav", "bilo"}},
{0, ERRORS,"Errors",
  {NULL}}};
