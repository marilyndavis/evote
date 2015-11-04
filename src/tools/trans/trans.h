/* $Id: trans.h,v 1.4 2003/01/15 18:54:13 marilyndavis Exp $ */ 
/* trans.h -- header file for the trans program
 *********************************************************
 **********************************************************/
#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#define FILELEN 128000        /* limit to the length of a file to process*/
/********************************************************
 *  Add a language by adding to this list.  Also, add below.
 **********************************************************/
typedef enum {CORSICAN, DEUTSCH, ENGLISH, ESPANOL, FRANCAIS, ITALIANO} TONGUE;
/**********************************************************/
/*                                                        */
/* Definition of a "language".  Don't mess with this:     */
typedef struct                                /*    *     */
{                                             /*    :     */
  TONGUE tongue;                              /*    *     */
  char * name;                                /*    :     */
  char * flag;                                /*    *     */
} LANGUAGE;                                   /*    :     */
LANGUAGE language[] =                         /*    *     */
{                                             /*    *     */
  /***********************************************************
   *  Add a language by adding to this list and the one above
   *************************************************************/
  {CORSICAN, "Corsican", "co"},
  {DEUTSCH, "Deutsch", "de"},
  {ENGLISH, "English", "en"},
  {ESPANOL, "Espanol", "es"},
  {FRANCAIS, "Francais", "fr"},
  {ITALIANO, "Italiano", "it"}
  /************************************************************/
};   /* <------ don't touch this! */
/************************************************************
 *   Or anything else from here down!
 *************************************************************/
/*  Eye-catchers in the table */
#define STARS  "* * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n"
#define DASHES "- - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n"
#define UNKNOWN "????\n"
typedef enum{NO, YES, MAYBE} YESorNO;
typedef enum{NOT_OK, OK, UNDECIDED} OKorNOT;
/* for processing upper/lower case */
typedef enum{MIX, NONE, STARTS, ALL} CAPS; 
int no_languages;
LANGUAGE from;
LANGUAGE to;
#define CONVLEN 10
#define WHITELEN 20
#define VARLEN 200
#define ARGLEN 15
typedef struct
{
  char conv[CONVLEN + 1];
  char whites[WHITELEN + 1];
  char variable[VARLEN + 1];
} ARG;
typedef struct
{
  char * string;
  ARG arg[ARGLEN];
  int no_args;
}PHRASE;
PHRASE unknown_phrase =   {UNKNOWN,
			   {{"","",""},
			    {"","",""},
			    {"","",""},
			    {"","",""},
			    {"","",""},
			    {"","",""},
			    {"","",""},
			    {"","",""},
			    {"","",""},
			    {"","",""},
			    {"","",""},
			    {"","",""},
			    {"","",""},
			    {"","",""},
			    {"","",""}},
			   0};
typedef struct
{
  PHRASE  from;
  PHRASE  to;
}TABLE;
/* for case-insensitive sorting */
#define TOSORT(X) ((X) >= 'A' && (X) <= 'Z' ? ((X) - 'A') : \
		    ((X) >= 'a' && (X) <= 'z' ? ((X) - 'a') : \
		     ((X) < 'A' ? (X) - 'A' : 25 + ((X) - 'Z'))))
#define IS_CONV_CHAR(X) ((X) == 'd' || (X) == 'e' || (X) == 'f'\
	 || (X) == 'x' || (X) == 'X' || (X) == 'i'\
 || (X) == 'o' || (X) == 'u'\
 || (X) == 'g' || (X) == 'G'\
 || (X) == 's' || (X) == 'c'? YES : NO)
#define IS_WHITE(X) ((X) == '\n' || (X) == '\t' || (X) == ' ' ? YES : NO)
#define LEN 200
#define STRINGLEN 2048
#define TABLELEN 2048
/* c file processing */
char file[FILELEN + 1];       /* whole .c file in memory */
long length;                   /* length of the .c file in memory */
/* table processing */
TABLE from_table[TABLELEN + 1];       /* whole table stored */
TABLE to_table[TABLELEN + 1];         /* pointers sorted by to language */
int no_in_table;
int new_no_in_table;
YESorNO table_exists = NO;
/* list of phrases to leave untranslated */
char **skip;
int no_skips;
/* list of #ifdef's to be excluded */
char **ifdef;
int no_ifdefs;
int debug = 0;
/* function declarations */
int arglen(char *str);
void arg_err(void);
YESorNO before(char *str1, char *str2);
int big_trunc(char *str);
OKorNOT check_args(char * from_str, char *to_str);
CAPS check_caps(char *str);
void	collect(char *where, FILE * fp, char *quitter);
OKorNOT collect_file(FILE *fp);
void collect_skips(void);
int command_args(int argc, char **argv);
void copy_phrase(PHRASE *dest, PHRASE *source);
void copy_table(TABLE *dest, TABLE* source);
int count_args(char *);
void fill_tables(void);
void find_languages(char *is_from, char * is_to);
PHRASE * fix_phrase(PHRASE *phrase, int index);
char * get_arg(int i, ARG * arg, int * end_args, YESorNO new_file);
LANGUAGE get_language(FILE *);
YESorNO is_ifdef(long *pl, FILE * fp);
YESorNO is_something(char *str, int * starts);
void make_caps(char *str, CAPS how);
void make_to_table(void);
YESorNO match_args(char *str1, char *str2);
PHRASE * next_phrase(int *starts, int * string_ends, int * ends);
TABLE next_table(FILE *);
int parenlen(char *str);
void parse_args(PHRASE * phrase);
void print_args(FILE* fp, PHRASE *p_phrase);
void print_entry(FILE *fp, TABLE *entry);
void print_table(void);
OKorNOT process_file(char * name);
YESorNO samevar(char *str1, char* str2);
int skip_arg(char *str);
void sort_tables(YESorNO print_dups);
YESorNO starts_same(char* str1, char * str2);
YESorNO strsame(char *str1, char *str2);
int trans_arg(char *);
int trunc(char *str);
PHRASE *whats(PHRASE *from_phrase);
void write_code(char * file_name);
