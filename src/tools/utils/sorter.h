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

#include<stdio.h>

typedef enum {DONT_KNOW,ENGLISH,FRANCAIS,DEUTSCH,NAHUATL,ESPANOL,PORTUGUESE,
							NORDIC,ITALIAN,CROATIAN, ERRORS, NO_LANG} LANGUAGE;

/* deaccenting test macro functions*/

#define DEACCENT(ch) ((ch>=192 && ch<=197 || ch>=224 && ch<=229) ? 'a':\
											((ch >=200 && ch<=203 || ch >=232 && ch<=235)?'e':\
											 ((ch >=204 && ch<=207 || ch >=236 && ch<=239)?'i':\
												((ch >=210 && ch<=214 || ch >=242 && ch<=246)?'o':\
												 ((ch >=217 && ch<=220 || ch >=249 && ch<=252)?'u':\
													((ch ==221 || ch==207 || ch ==253 || ch==255)?'y':\
													 ((ch ==199 || ch==231) ? 'c': \
														((ch ==209 || ch==241) ? 'n': ch))))))))
		 
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
DICO * dico;
LANGUAGE langtest(void);
void init_dico();
void checkword(int *);
char * find_address(LANGUAGE language);
LANGUAGE decide_language(void);

extern FILE* fptrace;
extern char *trace_file;
extern char *sendmail_path;
extern char **argv_copy;
extern char **env_copy;
extern int argc_copy;

typedef enum{NO, YES, MAYBE} YESorNO;
typedef enum{NOT_OK, OK, UNDECIDED} OKorNOT;
extern int msg_start;
extern int token[];        

void dump_message(FILE *fp);
void gen_header(char *address, LANGUAGE);
int get_token();              
OKorNOT parse_header(void);
void print_tokens();

void send(char *address);

#define MAX_ERROR   500  /* maximum error message size */
#define MAX_TOKEN   200  /* maximum size of one word */
#define MAX_LINE    300  /* max line length */

extern char *subject;
extern char *from;
extern char *address;
#define BLOCK 1024
extern char error_msg[];

void dump_message(FILE* where);

YESorNO same(char* x, char* y);
#define TOLOWER(x) (x >= 'A' && x <= 'Z' ? x - 'A' + 'a' : x)
OKorNOT parse_header(void);
