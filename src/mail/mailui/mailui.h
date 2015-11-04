/* $Id: mailui.h,v 1.4 2003/01/15 18:54:09 marilyndavis Exp $ */ 
/************************************************************
 *    Declarations and global variables for the
 *    mail interface for eVote.
 ***********************************************************
 **********************************************************/
#ifndef mailui_h
#define mailui_h
#ifdef __cplusplus
extern "C" {
#endif
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<time.h>
#include"maildef.h"
#include"../../Clerklib/Clerk.h"
#include"../message/message.h"
#include"../../tools/filters/filter.h"
/************************************************************
 *  If you are adding a language to the multi-lingual       *
 *  petition facility ---------->    Start here -------o    *
 ************************************************      |    *
                                                *      |    */
typedef enum{DO_PETITION, FINISH_UNSIGN,       /*      |    */
	       DISPLAY_PETITION_INFO,          /*      |    */
	       DISPLAY_PETITION_RESULTS,       /*      |    */
	       DISPLAY_PETITION_TEXT,          /*      |    */
	       SET_COMMENT,                    /*      |    */
	       START_PETITION_DISPLAYS,        /*      |    */
	       BIG_FINISH}                     /*      |    */
           WHICH_FUNCTION;                     /*      |    */
                                               /*      |    *
*********************************************************** *
*  Be sure your language is in this list -- add it!    |    *
*                                                     \|/   */
typedef enum {NAHUATL, FRANCAIS, ESPANOL, ENGLISH, DEUTSCH, ITALIANO
} LANGUAGE;
/************************************************************ *
 *  Make a copy of this declaration and add it, but change
 *  the name to be translate_xx(WHICH_FUNCTION ...
 *  where _xx is your language flag.  There should be one of
 *  these declarations for each language supported.     |     *
 **************************************************    \|/    */
void translate_en(LANGUAGE which_language, WHICH_FUNCTION function, 
		  ITEM_INFO * p_item, YESorNO just_checking, 
		  YESorNO new_pet, YESorNO do_header, char *name,
		  int * pcc, YESorNO from_owner);
void translate_fr(LANGUAGE which_language, WHICH_FUNCTION function, 
		  ITEM_INFO * p_item, YESorNO just_checking, 
		  YESorNO new_pet, YESorNO do_header, char *name,
		  int * pcc, YESorNO from_owner);
void translate_es(LANGUAGE which_language, WHICH_FUNCTION function, 
		  ITEM_INFO * p_item, YESorNO just_checking, 
		  YESorNO new_pet, YESorNO do_header, char *name,
		  int * pcc, YESorNO from_owner);
/**************************************************     |     *
 That's all in this file                            =========
************************************************************/
#define MAX_LANGUAGES 10
#define DEFAULT_LANGUAGE ENGLISH  /* language that the bulk of the code
				     is written in -- not pet_out.c */
/************************************************************
 *  
 *   Functions and variables defined in ..
 *
 ***********************************************************/
/* ../list.c */

extern char *listdir;          
void add_reader(char *name, char * whole_line, char *list_name);
void bounce_drop_error(void);
OKorNOT check_reader_files(void);
char *drop_reader(char *from, char *list);
void err_need_to_subscribe(void);
int expire_bounces(long old);
void find_path(char *path);
char *get_config(char * looking_for);
int get_list(int argc, char* argv[], YESorNO * sub);
char *get_next_list(YESorNO finished);
char* get_next_name(char * list_name, YESorNO finished, char **whole_line);
YESorNO is_listed(char *name, char *list_name);
YESorNO is_password(char * pw);
int move_reader(char *was, char *is);
void process_vacation(YESorNO on);
void subs(int argc, char* argv[]);
void vacation_back_msg(char * guy);
void vacation_msg(void);
void vacation_redundant_msg(YESorNO going);
void vacation_voter(void);

/************************************************************
 *   Functions and variables defined in this directory.
 ***********************************************************/
/* confirm.c */
#define CONFIRM_LEN 6
#define CONFIRM_FORMAT "Confirm Key: %s %s"
typedef enum {STARTING, CHECK, VERIFIED, NOT_NEEDED}CONFIRM_STATUS;
typedef enum {MYSTERY, FOR_SIG, FOR_CLOSE, FOR_DROP,
	      FOR_BACK, FOR_UNSUB}CONFIRM_WHAT;
typedef struct
{
  CONFIRM_STATUS status;
  char key[CONFIRM_LEN + 1];
} CONFIRMER ;

extern CONFIRMER confirm_this;

CONFIRM_WHAT check_confirm(void);
char * collect_confirm(CONFIRM_WHAT what);
OKorNOT drop_confirm_dir(void);
int expire_confirms(long old);
int expire_sig_confirms(char * listname, long old);
YESorNO read_confirm(CONFIRM_WHAT * what, char ** file_key, FILE **pfp);
void send_confirm(CONFIRM_WHAT what);
char * strip_key(void);

/* do_eVote.c */

void do_eVote(int cc);
OKorNOT initialize(void);
int parse_cf(void);
OKorNOT set_up_eVote(void);

/* help.c */
void get_help(void);            
void send_help(char * category, int whom, char * prefix);

/* listm.c */
extern char * approval_string;
extern char *list;
extern char *lclist;

void adjust_reader_list(char * target, char * line,
			ACTION old_action, ACTION new_action);
void approve(void);
void make_lclist(void);
void members(void);
OKorNOT start_list(char *list);
OKorNOT sync_list(char *list_name, YESorNO read_only);
char * who_sync(void);

/* lock.c */
extern char tmp_file_name[]; /* FNAME dependent */

YESorNO exists(char * full_path_name);
FILE * lock_to_tmp(char * fname);
OKorNOT unlock_to_tmp(YESorNO force);

/* new_poll.c */

void check_message(void);
void process_poll(void); 

/* petition.c */

extern YESorNO petition; 

#define MIN_KEY_LEN 5 /* min characters for key into petition files */
#define MAX_KEY_LEN 10   /* max characters for key into petition 
			    files */
#define EXTRA "-eVote"  /* flags subjects to associate a regular
			   poll with a petition */

extern char * extra_subject;
extern int no_pet_votes;
extern ITEM_INFO* p_first_vote_item;
extern YESorNO remove_sig;

void enter_signer(void);
void finish_petition(int whom, ITEM_INFO *p_item, 
		     YESorNO just_checking, YESorNO confirm);
void forget_petition(int whom, ITEM_INFO * p_item, YESorNO checking);
char *pet_file_key(char * str);
char * pet_fname(char * name, ITEM_INFO * p_item);
void process_names(void);
void send_petition_info(int whom,ITEM_INFO * maybe_NULL,
			YESorNO new_pet, YESorNO from_insert, 
			YESorNO just_checking);
OKorNOT sort_out_petition(int argc, char * argv[]);
OKorNOT sync_petitions(void);
void translate(LANGUAGE which_language, WHICH_FUNCTION function,
	       ITEM_INFO * p_item, 
	       YESorNO just_checking, 
	       YESorNO new_pet, YESorNO do_header, char * name,
	       int * pcc, YESorNO from_owner);
/* form.c */

#define FIELD_LEN 80
typedef struct
{
  char name[MAX_LANGUAGES][FIELD_LEN + 1];
  YESorNO required;
  char format[FIELD_LEN + 1];
} FIELD;

/* in form.c */
extern FIELD *field;
extern YESorNO form_exists;
extern YESorNO found_comment;
extern short no_of_fields;

OKorNOT check_format(int i);
OKorNOT drop_form(ITEM_INFO *p_item);
YESorNO is_comment(char* str);
int parse_form(int cc);
int print_fields(FILE *fp);  /* used by signatures.c and form.c */
int read_form_template(ITEM_INFO *p_item);
OKorNOT store_form_template(ITEM_INFO * p_item);

/* poll.c */
extern char author_name[];  /* MAX_ADDRESS + 1 */
extern YESorNO just_checking;

void copy_poll(YESorNO fussy);
void display_info(ITEM_INFO *p_item, YESorNO new_poll);
void display_stats(ITEM_INFO *p_item, YESorNO new_poll, YESorNO closing);
void get_mail_stats(short dropping_id,
		    unsigned long* readers, char *str1, char *str2);
unsigned long get_mail_voters(short dropping_id);
void instruct(int whom, ITEM_INFO* p_item, YESorNO new_poll); 
char * pollsdir(void);
void print_list_line(int i);
void process_close(YESorNO force);
void process_drop(YESorNO force, YESorNO silent);
void report_ratios(short dropping_id);
void send_stats_only(void);
void start_displays(ITEM_INFO * p_item);

/* poll_list.c */

extern short dropping_id;
extern ITEM_INFO * p_item_copy;
extern PRIV_TYPE ptype;

void list_polls(YESorNO for_error);

/* queriesm.c */

void failed_uid_report(void);
void gen_info(char *kind);
short how_voted(char * guy, unsigned long how_uid);
void process_how(void);
void process_who(int cc);

/* report.c */

int read_report_instructions(ITEM_INFO *);
void ship_reports(unsigned long signers);

/* signal.c */
void react_to_signal(int signo);
void emergency(char * message, char * cat_file); /*used by report.c */

/* signatures.c */

void display_signatures(ITEM_INFO * p_item);
OKorNOT drop_signature(unsigned long uid, FILE* fp_out, time_t when);
char *it_pet_fname(ITEM_INFO *p_item, YESorNO finished);
OKorNOT make_sig_dir(ITEM_INFO * p_item, char * list);
char *pet_signers_fname(ITEM_INFO *, time_t ); 
OKorNOT print_signature(char * address, unsigned long uid, time_t);
OKorNOT sync_sigs(ITEM_INFO *p_item);
OKorNOT write_signature(FILE *fp, YESorNO show_long_date,
			YESorNO display_date);
typedef struct
{
  unsigned long uid;
  time_t tstamp;
  YESorNO found;
} SIG;

/* spread.c */

OKorNOT fill_spread(void);
int get_cols(int *pcc);
void gen_spread(void);
YESorNO is_spread(void);
void process_spread(int cc);

/* subject.c */
/* char * subject is in ../../tools/filters/filter.c */

extern char *original_subject;          
extern LANGUAGE the_language;

OKorNOT strip_subject(void);
void unqp_subject(void);

/* table.cc */
/* table.h is #included by table.cc only */
void drop_translations(char *list, ITEM_INFO * p_item); 
OKorNOT find_and_switch(int argc, char *argv[], YESorNO *confirm_petition);
char * get_trans(char * subject, LANGUAGE lang, YESorNO force);
YESorNO is_flag(char * str, LANGUAGE * lang);
void print_petitions(YESorNO want_public);
void read_translations(void);  /* exported only for pet_outxx.c files */
void table_add(char *list, char *subject, char *trans,
	       char * name, PRIV_TYPE ptype, YESorNO confirm_petition);
YESorNO table_exists(char *subject, char *list, char *default_subject,
		     char * language_flag);
char * whole_name(char *flag);
OKorNOT write_translations(void);

extern LANGUAGE default_language;
extern YESorNO forced_language;
extern int no_languages;

extern int no_of_answers;
typedef struct
{
  char * str;
  int answer;
} ANSWER;
extern ANSWER answer_list[];

typedef struct language_def
{
  char * name;
  char * comment;
  char * help;
  char * info;
  char * remove;
  char * end;
  void(*translate_fn)(LANGUAGE which_language, WHICH_FUNCTION function, 
		      ITEM_INFO * p_item, YESorNO just_checking, 
		      YESorNO new_pet, YESorNO do_header, char * name,
		      int * pcc, YESorNO from_owner);
} TONGUE;
extern TONGUE tongue[];

/* text.c */
void display_poll_text(ITEM_INFO * p_item, YESorNO just_checking);
void drop_poll_text(ITEM_INFO * p_item);
void dump_poll_message(FILE* fp);
OKorNOT make_poll_dir(char* list);
char * poll_text_name(ITEM_INFO * p_item);
void store_poll_text(ITEM_INFO * p_item);

/* util.c  */
extern char file_error[];  /* to help translators */

void address(char * hey, char * you);
unsigned long atoul(char * str);
char * convert_hex( char * input);
int days_since(struct tm * date_in);
int diff_time(struct tm * one, struct tm * two);
char * fgetsn(char *line, int len, FILE *fp) ;
void highlight(const char *);
YESorNO is_end(char *str);
YESorNO is_option(int* pcc, char* lside, char* rside,
		  int *number_found);
char * lower(char *,char *);
void lowlight(char *);
char * make_flag(char *name);
OKorNOT make_path(char *to_here);
char * make_string(char *);
int pick(int min, int max);
char * raiseup(char *, char*);
int strCmp(char *str1, char *str2);
int strNcmp(char *str1, char *str2, int n);
char *replace(char *str, char *this_string, char *with_this);
char *ratio_string(float ratio);

/* voterm.c  */

OKorNOT add_mail_voter(char *, ACTION); 
void bad_vote_message(char * good_message);              
OKorNOT drop_mail_voter(char *list, YESorNO only_if_non_voter);
OKorNOT enter_Clerk(void);  
YESorNO ever_voted(void);
void move_voter(char *was, char *is); 
void on_vacation(YESorNO going);
char *print_action(ACTION action);
OKorNOT process_action(ACTION, char* who, ACTION * old_action);
void process_mail_vote(int cc);

void process_remove(YESorNO from_petition);

void sub_to_eVote(char *name, char *dowhat, ACTION action);
OKorNOT try_entering(YESorNO force);

#define IS_WHITE(ch) (ch == ' ' || ch == '\t' || ch == '\n'? 1 : 0)
#ifdef __cplusplus
}
#endif
#endif

