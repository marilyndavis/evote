/* $Id: confirm.c,v 1.3 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/*********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include "mailui.h"
/************************************************************
 *  confirm.c  - functions relating to the confirmation of
 *               critical messages.
 *************************************************************/
CONFIRMER confirm_this;
static char confirm_file_name[500];
static int expire_dir(char * base, DIR *dirp, long old);
static int expire_list_confirms(char * listname, long old);
static FILE * open_confirm_file(CONFIRM_WHAT *, char * mode);
static void remove_confirm_file(void);
static char * gen_key(void);    
static CONFIRM_WHAT str_to_what(char *str);
static char * what_str(CONFIRM_WHAT what);
/************************************************************
 *    Checks for a confirm file from this voter with the
 *    right key.  confirm_this.key has the key from the
 *    subject line.  confirm_this.status = CHECK when
 *    called.  Returns VERIFIED or sends an error message.
 *    If VERIFIED, it has also switched stdin to be the
 *    confirmed message.
 *    There is a near copy of this function "check_sig_confirm" in
 *    pet_out.c for translating to various languages for the
 *    petition facility.
 ************************************************************/
CONFIRM_WHAT
check_confirm(void)
{
  FILE *fp;
  char buf[500];
  char * file_key;
  YESorNO cc;
  CONFIRM_WHAT what = MYSTERY;
  /* This call can be anything except FOR_SIG */
  switch (cc = read_confirm(&what, &file_key, &fp))
    {
    case YES:
      return what;
      break;
    case NO:
    case MAYBE:
      sprintf(error_msg, "\nYour message about:\n\n\t%s\n\n"
	      "looks like a confirmation reply for the %s list \n"
	      "with the confirmation key \"%s\".  However, \n"
	      "we have no record of this confirmation key for you for\n"
	      "%s.\n", subject, list, confirm_this.key, 
	      list);
      if (cc == NO)
	{
	  char msg[500];
	  sprintf(msg, "\nIf you wish to communicate with eVote about this subject, please"
		  "\ntry again but don't put \"Confirm: %s\" in your subject line.\n",
		  confirm_this.key);
	  strcat(error_msg, msg);
	  bounce_error(SENDER);
	  break;
	}
      /* case MAYBE: found a file_key but wrong one */
      sprintf(buf, "Confirm: %s ", file_key);
      gen_header(SENDER, buf, NO);
      printf(error_msg);
      printf("\nHowever, you do have a confirmation key %s pending on\n"
	     "%s.\n\n"
	     "Note that eVote only keeps your most recent attempt.  The following\n"
	     "is the message that awaits confirmation.  To confirm it, reply to \n"
	     "this message.\n"
	     "\nIf you wish to send in a new message for %s,"
	     "\ntry again, but don't put \"Confirm: %s\" in your subject.\n",
	     file_key, subject, subject, confirm_this.key);
      printf("\n\n ----- YOUR MESSAGE AWAITING CONFIRMATION -----\n\n");
      while (fgets(buf, 400, fp))
	fputs(buf, stdout);
      fclose(fp);
      big_finish(0);
      break;
    default:
      return what;  /* never happens */
    }
  /* Never happens */
  return what;
}
/************************************************************
 *     Saves a file for later confirmation.
 *     Returns the key.
 ************************************************************/
char *
collect_confirm(CONFIRM_WHAT what)
{
  FILE *fp;
  if ((fp = open_confirm_file(&what, "w")) == NULL)
    {
      return NULL;  /* error_msg already stored */
    }
  fprintf(fp, CONFIRM_FORMAT "\n", 
	  strcpy(confirm_this.key, gen_key()), what_str(what));
  dump_message(fp, NO, NO, NO);
  fclose(fp);
  return confirm_this.key;
}
/************************************************************
 *     Deletes old confirms.  Old is in days.
 *     It returns the number of files deleted and prints
 *     a report to stdout.
 ************************************************************/
int
expire_confirms(long old)
{
  YESorNO do_all = NO;
  char * listname;
  int did, count = 0;
  get_next_list(YES);  /* to initialize */
  printf("Deleting confirm messages that are at least %ld days old.",
	 old);
  if (same(list, "ALL"))
    {
      do_all = YES;
    }
  while (do_all == NO || (list = get_next_list(NO)) != NULL)
    {
      listname = list;
      if (!does_conf_exist(listname))
	{
	  printf("\n%s is not eVoted.\n", listname);
	  continue;
	}
      if (samen(listname,"petition", 8))
	{
	  printf("\nChecking signature confirms for %s.", listname);
	  count += (did = expire_sig_confirms(listname, old));
	  printf("\n%s deleted %d messages.", listname, did);
	}
      if (!do_all)
	break;
      printf("\nChecking admin confirms for %s.", listname);
      count += (did = expire_list_confirms(listname, old));
      printf("\n%s deleted %d messages.", listname, did);
    }
  return count;
}
/************************************************************
 *    dirp should be a freshly open DIR * , old is number of
 *    days old.  All files that old or older will be deleted
 *    and a report goes to stdout.
 *    The number of files deleted is returned.
 ************************************************************/
int
expire_dir(char * base, DIR *dirp, long old)
{
  time_t biggest;
  struct stat ss_buf;
  struct dirent *dp_now;
  struct tm * file_tms;
  struct tm * p_now_tms;
  struct tm now_tms;
  long elapsed;
  int count = 0;
  char  name[PATHLEN + 1];
  p_now_tms = localtime(&now);
  now_tms = *p_now_tms;
  while ((dp_now = readdir(dirp)) != NULL)
    {
      if (dp_now->d_name[0] == '.')
	continue;
      sprintf(name, "%s/%s", base, dp_now->d_name);
      if (stat(name, &ss_buf) == -1)
	{
	  if (errno != EACCES)
	    {
	      sprintf(error_msg,"Can't stat %s for %s or errno %d.",
		      name, 
		      (errno == EACCES ? "lack of permission"
		       : "some reason"), errno);
	      bounce_error(ADMIN);
	    }
	}
      biggest = (ss_buf.st_mtime > ss_buf.st_atime ?
		 ss_buf.st_mtime : ss_buf.st_atime);
      biggest = (biggest > ss_buf.st_ctime ?
		 biggest : ss_buf.st_ctime);
      file_tms = localtime(&biggest);
      if ((elapsed = diff_time(&now_tms, file_tms)) >= old)
	{
	  printf("\n%ld days: %s", elapsed, name);
	  count++;
	  if (unlink(name) != 0)
	    {
	      sprintf(error_msg,"Can't unlink %s for %s or errno %d.",
		      name, 
		      (errno == EACCES ? "lack of permission"
		       : "some reason"), errno);
	      bounce_error(ADMIN);
	    }
	}
    }
  return count;
}
/************************************************************
 *     Deletes the admin confirms saved for the 
 *     listname that are at least old days old.
 *     It returns the number of files deleted and prints
 *     the addresses to stdout.
 *************************************************************/
int
expire_list_confirms(char * listname, long old)
{
  char fname[PATHLEN + 1];
  int count;
  DIR *dirp = NULL;
  sprintf(fname, "%s/%s/confirm", pollsdir(), list);
  if ((dirp = opendir(fname)) == NULL)
    {
      if (errno != EACCES)
	return 0;
      sprintf(error_msg,"\nYou don't have permission to search %s.",
	      fname);
      bounce_error(SENDER | ADMIN);
    }
  count = expire_dir(fname, dirp, old);
  closedir(dirp);
  return count;
}
/************************************************************
 *     Deletes the signature confirms saved for the 
 *     listname that are at least old days old.
 *     It returns the number of files deleted and prints
 *     the addresses to stdout.
 *************************************************************/
int
expire_sig_confirms(char * listname, long old)
{
  int count = 0;
  DIR *dirp = NULL;
  struct dirent *dp;
  DIR *dirp_deep = NULL;
  char  base[PATHLEN + 1];
  char  name[PATHLEN + 1];
  int i;
  strcpy(base, listdir);
  i = strlen(base) + 1;
  while (base[--i] != '/')
    ;
  sprintf(&base[i], "/polls/%s", listname);
  dirp = opendir(base);
  if (dirp == NULL)
    {
      sprintf(error_msg,"\nYou don't have permission to search %s.",
	      base);
      bounce_error(ADMIN);
    }
  while ((dp = readdir(dirp)) != NULL)
    {
      if (dp->d_name[0] == '.')
	continue;
      sprintf(name, "%s/%s/confirm", base, dp->d_name);
      if ((dirp_deep = opendir(name)) == NULL)
	{
	  if (errno == EACCES)
	    {
	      sprintf(error_msg,"\nYou don't have permission to search %s.",
		      name);
	      bounce_error(ADMIN);
	    }
	  continue;
	}
      count += expire_dir(name, dirp_deep, old);
      closedir(dirp_deep); 
    }
  closedir(dirp);
  return count;
}
/************************************************************
 *   Called when a poll is dropped.
 *************************************************************/
OKorNOT
drop_confirm_dir(void)
{
  struct stat ss;
  char command[300];
  sprintf(confirm_file_name, "%s.confirm", 
	  poll_text_name(p_item_copy));
  if (stat(confirm_file_name, &ss) != 0)
    return OK;
  sprintf(command, "rm -r %s", confirm_file_name);
  if (system(command) == 0)
    return OK;
  return NOT_OK;
}
/************************************************************
 *   generates and returns a sequence of random characters
 *   CONFIRM_LEN long.
 *************************************************************/
char *
gen_key(void)
{
  int i;
  int pick(int min, int max);
  static char key[CONFIRM_LEN + 1];
  for (i = 0; i < CONFIRM_LEN; i++)
    {
      key[i] = pick(' ' + 1, '~');
    }
  key[i] = '\0';
  return key;
}
/************************************************************/
FILE *
open_confirm_file(CONFIRM_WHAT * what, char * mode)
{
  char lowered[200];
  FILE * fp;
  if (confirm_file_name[0] == '\0')
    {
      switch (*what)
	{
	case FOR_SIG:
	  strcpy(confirm_file_name, pet_fname("confirm", p_item_copy));
	  break;
	default:
	  sprintf(confirm_file_name, "%s/%s/confirm", pollsdir(),
		  list);
	  break;
	}
      if (make_path(confirm_file_name) != OK)
	return NULL;
      sprintf(confirm_file_name, "%s/%s", confirm_file_name,
	      lower(lowered, from));
    }
  if ((fp = fopen(confirm_file_name, mode)) == NULL)
    {
      sprintf(error_msg, "\nUnable to open %s for %s for a confirm.\n",
	      confirm_file_name, mode);
    }
  return fp;
}
/************************************************************
 *    Checks for a confirm file from this user with the
 *    right key.  confirm_this.key has the key from the
 *    subject line.  confirm_this.status = CHECK when
 *    called.  Returns:
 *       YES,  status == VERIFIED and stdin reading confirmed file
 *        NO,  status == CHECK if there's no file
 *     MAYBE,  status == CHECK if there is a file but the key in
 *                             the file doesn't match.  Stdin is
 *                             still the current message.  
 *                 p_file_key  has the key in the file.
 *                        pfp  has the open file pointer.
 *    If *what == FOR_SIG, it's for a signature on a petition,
 *    otherwise, the correct CONFIRM_WHAT is put there.
 ************************************************************/
YESorNO
read_confirm(CONFIRM_WHAT *what, char ** p_file_key,
		     FILE ** pfp)
{
  char buffer[400];
  static char file_key[CONFIRM_LEN + 1] = "";
  char what_str[11];
  FILE *fp;
  int cc;
  *p_file_key = file_key;
  if ((fp = open_confirm_file(what, "r")) == NULL)
    {
      return NO;
    }
  fgets(buffer, 100, fp);
  if ((cc = sscanf(buffer, CONFIRM_FORMAT, file_key, what_str)) != 2)
    {
      sprintf(error_msg, "%s confirm file messed up.",
	      confirm_file_name);
      bounce_error(SENDER | OWNER | ADMIN);
    }
  *what = str_to_what(what_str);
  if (strcmp(file_key, confirm_this.key) == 0)
    {
      fclose(fp);
      confirm_this.status = VERIFIED;
      close(0);
      dup(open(confirm_file_name, O_RDONLY));
      fgets(buffer, 300, stdin);  /* now stdin is ready to pipe old file */
      buff_up();
      parse_message(NO);
      remove_confirm_file();
      return YES;
    }
  *pfp = fp;
  return MAYBE;
}
/************************************************************/
void
remove_confirm_file(void)
{
  char command[800];
  sprintf(command,"rm %s", confirm_file_name);
  system(command);
}
/************************************************************
 *    Sends a confirm request to the user and saves this
 *    message for confirmation.
 *   CONFIRM_WHAT =  FOR_CLOSE   
 *                   FOR_DROP
 *                   FOR_BACK
 *                   FOR_UNSUB
 *          FOR_SIG is handled in pet_out.c:
 *************************************************************/
void
send_confirm(CONFIRM_WHAT what)
{
  char buf[1000];
  if (collect_confirm(what) == NULL) 
    {
      bounce_error(SENDER | OWNER | ADMIN);
    }
  sprintf(buf, "Confirm: %s", confirm_this.key);
  gen_header(SENDER, buf, NO);
  switch (what)
    {
    case FOR_UNSUB:
      printf("Thank you for your unsubscribe message on the %s list.\n",
	     list);
      printf("\nUnsubscribing from the list deletes your ballot and subtracts your"
	     "\nyour influence from the statistics."
	     "\n\nIs this really what you want to do?"
	     "\n\nAt this time, the flow of messages has been stopped and your ballot"
	     "\nhas been put on \"vacation\" which means that you can send an"
	     "\n\"eVote back\" message to the list to resume the flow of mail and your"
	     "\nballot will be as you left it."
	     "\n\nIf you really want to leave forever, you need to verify that the"
	     "\n\"unsubscribe\" message really came from you.");
      break;
    case FOR_DROP:
    case FOR_CLOSE:
      printf("Thank you for your \"%s\" message on \n\n\t%s", 
	     (what == FOR_DROP ? "drop" : "close"), original_subject);
      break;
    case FOR_BACK:
      printf("Thank you for your \"eVote back\" on the %s list.",
	     list);
      break;
    default:
      /* impossible */
      break;
    }
  printf("\n\nTo verify that your message really came from you, please confirm"
	 "\nthis message by using \"Reply-To\".  You don't need to type anything"
	 "\nnew in your confirmation message.  eVote will read your subject line"
	 "\nand your address; everything else will be ignored.\n\n");
  big_finish(0);
  return;
}
/************************************************************
 *  Strips the confirmation key from the front of the subject
 *  line.  "Confirm:" is already gone from strip_subject.
 *  Returns the key.
 ************************************************************/
char *
strip_key(void)
{
  static char key[CONFIRM_LEN + 1];
  int i;
  for (i = 0; subject[i] != ' ' && subject[i] ; i++)
    ;
  if (i != CONFIRM_LEN)
    return "";
  strncpy(key, subject, CONFIRM_LEN);
  if (!subject[i]) /* no more subject */
    strcpy(subject, "Your Message");
  else
    strcpy(subject, &subject[CONFIRM_LEN + 1]);
  return key;
}
/************************************************************/
CONFIRM_WHAT
str_to_what(char * str)
{
  if (strcmp(str, "drop") == 0)
    return FOR_DROP;
  if (strcmp(str,"close") == 0)
    return FOR_CLOSE;
  if (strcmp(str, "unsub") == 0)
    return FOR_UNSUB;
  if (strcmp(str, "signature") == 0)
    return FOR_SIG;
  if (strcmp(str,"back") == 0)
    return FOR_BACK;
  if (strcmp(str,"mystery") == 0)
    return MYSTERY;
  return (CONFIRM_WHAT)9999;
}
/************************************************************
 *   Returns a string for the what.
 *************************************************************/
char
*what_str(CONFIRM_WHAT what)
{
  switch (what)
    {
    case MYSTERY:
      return "mystery";
      break;
    case FOR_UNSUB:
      return "unsub";
      break;
    case FOR_DROP:
      return "drop";
      break;
    case FOR_CLOSE:
      return "close";
      break;
    case FOR_SIG:
      return "signature";
      break;
    case FOR_BACK:
      return "back";
      break;
    }
  return "trouble";
} 
