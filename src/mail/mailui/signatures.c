/* $Id: signatures.c,v 1.6 2003/10/20 17:07:34 marilyndavis Exp $ */ 
/**********************************************************
 *   signatures.c Functions that manipulate the signature 
 *                text for a petition.
 *********************************************************
 **********************************************************/
#include<stdio.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<errno.h>
#include <dirent.h>
#include"mailui.h"
#include<fcntl.h>
extern YESorNO give_email;
char *message(void);   /* in ipc_msg.c */
static OKorNOT get_esigners(ITEM_INFO* p_item, SIG ** ehere,
		     unsigned long * no_signers);
static OKorNOT sort_sigs(SIG ** esig, int no_sigs);
OKorNOT update_tstamp(ITEM_INFO * p_item, SIG * sig);
static OKorNOT vitals_collect(FILE *fp, SIG *here);
void vitals_store(FILE *fp, SIG *here);
#define EXTRAS 1
/************************************************************/
/* Take this out after 2.5 */
void
fix_signatures(ITEM_INFO * p_item)
{
  FILE *fp;
  FILE *fp_tmp;
  SIG sig;
  char * fname;
  char line[MAX_LINE + 1];
  char rest[MAX_LINE + 1];
  char name[200];
  char command[2*FNAME + 5];
  YESorNO forced;
  char new_fname[FNAME + 5];
  int i, j;
  while ((fname = it_pet_fname(p_item, NO)) != NULL)
    {
      if ((fp = fopen(fname, "r")) == NULL)
	{
	  printf("\n%s", file_error);
	  printf("\n\t%s \n\nto read the signatures for \"%s\".\n",
		 fname, p_item->eVote.title);
	  printf("\nPlease forward this to %s.\n\n", 
		 eVote_mail_to);
	  fprintf(stderr,"%s\n\t%s \n\nto read the signatures for %s.\n\n",
		  file_error,	fname, p_item->eVote.title);
	  perror("");
	  return;
	}
      if ((fp_tmp = lock_to_tmp(fname)) == NULL)
	{
	  fprintf(stdout,"\n\nERROR! %s \n\nCan't drop your signature for \"%s\".\n\nPlease forward this to %s.\n\n", 
		  error_msg, p_item_copy->eVote.title, eVote_mail_to);
	  fprintf(stderr,"\nERROR! %s \n\nCan't drop the signature for %s on %s.\n\n", 
		  error_msg, from, p_item_copy->eVote.title);
	  perror("");
	  exit(1);
	}
      printf("\n\nFixing %s:\n", fname);
      while (fgets(line, MAX_LINE, fp) != NULL)
	{
	  if (strncmp(line,"- - - ", 6) == 0)
	    {
	      forced = NO;
	      sscanf(line,"- - - %s - - -", name);
	      if ((sig.uid = who_num(name, NO)) == 0)
		{
		  forced = YES;
		  sig.uid = who_num(name, YES);
		}
	      strcpy(rest, &line[strlen(name) + 7]);
	      fscanf(fp, "%ld\n", &sig.tstamp);
	      printf("\n%s\t%5lu %s %s", ctime(&sig.tstamp),
		     sig.uid, name, 
		     (sig.uid ? (forced ? "forced into who list" :
				 "already in who list") : "rejected by who list"));
	      fputs(rest, fp_tmp);
	      vitals_store(fp_tmp, &sig);
	      continue;
	    }
	  fputs(line, fp_tmp);
	}
      fflush(fp_tmp);
      fclose(fp);
      for (i = 0, j = 0; fname[i]; i++, j++)
	{
	  if (fname[i] == '.')
	    {
	      sprintf(command,"rm %s", fname);
	      system(command);
	      for (; new_fname[j] != '/'; j--)
		;
	      for (i++, j++; fname[i]; i++, j++)
		{
		  new_fname[j] = fname[i];
		}
	      break;
	    }
	  new_fname[j] = fname[i];
	}
      new_fname[j] = '\0';
      sprintf(command, "cp %s %s", tmp_file_name, new_fname);
      system(command);
      chmod(fname, 00660);
      if (unlock_to_tmp(NO) != OK)
	{
	  fprintf(stderr,"\nERROR! Unable to remove tmp file: %sT.\n",
		  fname);
	}			
    }
}      
/************************************************************/
OKorNOT 
add_esigner(ITEM_INFO *p_item, SIG sig)
{
  short old_vote;
  static char name[400];
  int cc;
  if (no_pet_votes)
    return NOT_OK;
  if ((from = who_is(sig.uid)) == NULL)
    return UNDECIDED;
  strcpy(name, from);
  from = name;
  mail_voter = sig.uid;
  if ((cc = try_entering(YES)) == UNDECIDED)
    {
      send_joining(TRY, NO);
      change_action(SIGNER);
    }
  if (cc == PROBLEM)
    {
      sprintf(error_msg,"\n%s is rejected by the Clerk\n", name);
      return NOT_OK;
    }
  send_vote(p_item, 1, &old_vote);
  push_time(p_item, sig.tstamp);
  i_am_leaving();
  return OK;
}
/************************************************************/
OKorNOT 
drop_esigner(ITEM_INFO * p_item, SIG sig)
{
  short old_vote;
  OKorNOT ret_cc = OK;
  RTYPE cc;
  char who_voter[300];
  char * whothis;
  if ((whothis = who_is(sig.uid)) == NULL)
    return NOT_OK;
  strcpy(who_voter, whothis);
  mail_voter = sig.uid;
  if (try_entering(NO) != OK)
    {
      sprintf(error_msg, "\nsignatures.c:drop_esigner: %s=%lu can't get into eVote.\n", 
	      who_voter, sig.uid);
      return UNDECIDED;
    }
  if (have_i_voted(p_item) == NO)
    {
      sprintf(error_msg,"%s=%lu has not signed, according to the Clerk.\n",
	      who_voter, sig.uid);
      i_am_leaving();
      return UNDECIDED;
    }
  cc = send_vote(p_item, READ, &old_vote);
  if (cc == GOOD && no_pet_votes)
    {
      if (have_i_voted(p_item+1) == NO)
	{
	  sprintf(error_msg,"%s=%lu has not voted, according to the Clerk.\n",
		  who_voter, sig.uid);
	  i_am_leaving();
	  return UNDECIDED;
	}
      cc = send_vote(p_item + 1, READ, &old_vote);
    }
  switch (cc)
    {
    case FAILURE:
      sprintf(error_msg, "\nSystem troubles at %s: %s's %s was not removed.\n", 
	      whereami, who_voter, (no_pet_votes ? "vote" : "signature"));
      ret_cc = UNDECIDED;
      break;
    case NO_MODS:
      sprintf(error_msg, "\nsignatures.c:drop_sig: Programming error\n");
      ret_cc = UNDECIDED;
      break;
    case NO_CHANGE:  /* first vote makes all others 0 */
    case GOOD:  
      printf("\n%s = %lu dropped from count in Clerk.\n",
	     who_voter, mail_voter);
      break;
    default:
      /* impossible */
      break;
    }
  i_am_leaving();
  return ret_cc;
}
/*********************************************************/
void
display_signatures(ITEM_INFO * p_item)
{
  FILE *fp;
  char * fname;
  char line[MAX_LINE + 1];
  YESorNO started = NO;
  YESorNO something = YES;
  YESorNO full_line = YES;
  SIG sig;
  if (p_item == NULL)
    p_item = p_item_copy;
  printf("\n");
  highlight("PETITION SIGNATURES"); 
  while ((fname = it_pet_fname(p_item, NO)) != NULL)
    {
      if ((fp = fopen(fname, "r")) == NULL)
	{
	  printf("\n%s", file_error);
	  printf("\n\t%s \n\nto read the signatures for \"%s\".\n",
		 fname, p_item->eVote.title);
	  printf("\nPlease forward this to %s.\n\n", 
		 eVote_mail_to);
	  fprintf(stderr,"%s\n\t%s \n\nto read the signatures for %s.\n\n",
		  file_error,	fname, p_item->eVote.title);
	  perror("");
	  return;
	}
      started = NO;
      while (fgets(line, MAX_LINE, fp) != NULL)
	{
	  if (started == NO)
	    {
	      int i;
	      for (i = 0; line[i]; i++)
		{
		  if (line[i] != ' ' && line[i] != '\t' && line[i] != '\n')
		    {
		      started = YES;
		      something = YES;
		    }
		  break;
		}
	      if (started == NO)
		continue;
	    }
	  if (strncmp(line,"- - - ", 6) == 0)
	    {
	      vitals_collect(fp, &sig);
	      strcat(line,"\n");
	      if ((p_item->eVote.priv_type == PRIVATE
		 || p_item->eVote.priv_type == IF_VOTED)
		 && p_item->eVote.author == mail_voter)
		{
		  printf("\n- - - %s ", who_is(sig.uid));
		} 
	    }
	  else if (full_line)
	    {
	      printf("   ");
	    }
	  fputs(line, stdout);
	  if (line[strlen(line)-1] == '\n')
	    full_line = YES;
	  else
	    full_line = NO;
	}
      fclose(fp);
    }
  if (!something)
    {
      printf("No signatures yet.\n");
      return;
    }
}
/**********************************************************
 *   Removes the signature and prints it to fp_out
 *   unless fp_out is NULL.
 **********************************************************/	
OKorNOT
drop_signature(unsigned long uid, FILE* fp_out, time_t when)
{
  FILE * out_to;
  FILE* fp;
  FILE* fp_tmp;
  char *fname;
  YESorNO skip = NO;
  YESorNO dropped = NO;
  char command[2*FNAME + 5];
  char line[MAX_LINE + 1];
  SIG sig;
  YESorNO have_vitals = NO;
  int lineno = 0;
  if (when == 0)
    when = now;
  fname = pet_signers_fname(p_item_copy, when);
  if ((fp = fopen(fname, "r")) == NULL)
    {
      fprintf(fp_out,"\n%s\n\t", file_error);
      fprintf(fp_out, "%s \n\nto drop your signature for \"%s\".\n\n"
	      "Please forward this to %s.\n\n", 
	      fname, p_item_copy->eVote.title, eVote_mail_to);
      fprintf(stderr,"\n\n%s\n\n\t%s"
	      "\n\nto drop the signatures for %s on %s.\n\n", 
	      file_error, fname, from, p_item_copy->eVote.title);
      perror("");
      return NOT_OK;
    }
  if ((fp_tmp = lock_to_tmp(fname)) == NULL)
    {
      fprintf(fp_out,"\n\nERROR! %s \n\nCan't drop your signature for \"%s\".\n\nPlease forward this to %s.\n\n", 
	      error_msg, p_item_copy->eVote.title, eVote_mail_to);
      fprintf(stderr,"\nERROR! %s \n\nCan't drop the signature for %s on %s.\n\n", 
	      error_msg, from, p_item_copy->eVote.title);
      perror("");
      return NOT_OK;
    }
  while (fgets(line, MAX_LINE, fp) != NULL)
    {
      lineno++;
      if (dropped == NO)
	{
	  if (strncmp(line,"- - - ", 6) == 0)
	    {
	      if (skip == YES)
		{
		  skip = NO;
		  dropped = YES;
		}
	      else
		{
		  vitals_collect(fp, &sig);
		  if (uid == sig.uid)
		    {
		      fprintf(fp_out,"\n");
		      strcat(line,"\n");
		      skip = YES;
		    }
		  else
		    {
		      have_vitals = YES;
		    }
		}
	    }
	}
      out_to = (skip == NO ? fp_tmp : fp_out);
      if (out_to != NULL)
	{
	  fputs(line, out_to);
	  if (have_vitals)
	    {
	      vitals_store(out_to, &sig);
	      have_vitals = NO;
	    }
	}
    }
  fflush(fp_tmp);
  fclose(fp);
  sprintf(command, "cp %s %s", tmp_file_name, fname);
  system(command);
  chmod(fname, 00660);
  if (unlock_to_tmp(NO) != OK)
    {
      fprintf(stderr,"\nERROR! Unable to remove tmp file: %sT.\n",
	      fname);
    }			
  if (dropped == YES)
    return OK;
  return UNDECIDED;
}
/************************************************************
 *    returns into ehere, a list of everyone who signed the 
 *    petition according to the Clerk.
 *************************************************************/
OKorNOT
get_esigners(ITEM_INFO* p_item, SIG ** ehere,
		     unsigned long * no_signers)
{
  SIG * esigner;
  RTYPE cc;
  *no_signers = 0L;
  *ehere = NULL;
  if (no_of_participants == 0)
    return OK;
  if ((esigner = (SIG*)(malloc(sizeof(SIG) * no_of_participants)))
     == NULL)
    {
      sprintf(error_msg,"\nsignatures.c:get_esigners:Can't get memory.\n");
      return UNDECIDED;
    }
  switch (cc = who_signed(p_item))
    {
    case FAILURE:  /* message written to stderr */
      free(esigner);
      return UNDECIDED;
      break;
    case STRING_OUT:
      free(esigner);
      sprintf(error_msg, "%s", message());
      if (samen(error_msg,"\nNo one", 7))
	{
	  error_msg[0] = '\0';
	  return UNDECIDED;
	}
      break;
    case UID_LIST:
    case UID_LIST_MORE:
      /* here we user uid_report() to iterate through
	 the list of signers the Clerk returned */
      {
	short i = -1;
	char *vote_str;
	while ((vote_str = uid_report(&esigner[++i].uid)) != NULL)
	  {
	    sscanf(vote_str," %ld", &esigner[i].tstamp);
	    esigner[i].found = NO;
	  }
	if (esigner[i].uid == 0)
	  {
	    failed_uid_report();
	  }  
	*ehere = esigner;
	*no_signers = i;
	return sort_sigs(ehere, i);
      }/* short i */
      break;  
    default:
      sprintf(error_msg, "\nUnexpected return in process_who: %d = %s\n",
	      cc, get_rtype(cc));
      free(esigner);
      return UNDECIDED;
      break;
    } /* end of switch */      
  return UNDECIDED;
}
/************************************************************
 *      returns each signature file, one at a time and
 *      in order.  Send in a finished == YES to reset.
 *************************************************************/
char *
it_pet_fname(ITEM_INFO * p_item, YESorNO finished)
{
  #define PET_NAME_LEN 12
/* 6 chars for name -- extra for old version */
  DIR *dirp = NULL;
  struct dirent *dp;
  static char  base[PATHLEN + 1];
  static char  name[PATHLEN + 1];
  static char (*list)[PET_NAME_LEN];  
  static int i;
  static int no;
  int j;
  if (no == 0)
    {
      strcpy(base, pet_fname("signatures", p_item));
      dirp = opendir(base);
      if (dirp == NULL)
	{
	  sprintf(error_msg,"\nYou don't have permission to search Majordomo's data directory.  \nYour command can't be completed.");
	  bounce_error(SENDER|ADMIN);
	}
      if ((list = (char(*)[PET_NAME_LEN])malloc(20 * 12 * sizeof(char[PET_NAME_LEN])))
	  == NULL)
	{
	  sprintf(error_msg,"\nCan't make space in it_pet_fname.\n");
	  return NULL;
	}
      no = 0;
      while ((dp = readdir(dirp)) != NULL)
	{
	  if (dp->d_name[0] == '.'
	     || dp->d_name[strlen(dp->d_name) -1] == '~')
	    continue;
	  for (j = no-1; j >= 0; j--)
	    {
	      if (strcmp(list[j], dp->d_name) > 0)
		strcpy(list[j+1], list[j]);
	      else
		break;
	    }
	  strcpy(list[j+1], dp->d_name);
	  no++;
	}
      closedir(dirp);
      i = -1;
    }
  if (finished != YES && ++i < no)
    {
      sprintf(name,"%s/%s", base, list[i]);
      return name;
    }
  i = -1;
  free(list);
  no = 0;
  return NULL;
}
/*********************************************************/
OKorNOT
make_sig_dir(ITEM_INFO * p_item, char * list)
{
  char * dir_name;
  int cc;
  int i;
  unsigned int perm =  S_IXUSR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
  YESorNO last_try = NO;
  dir_name = pet_fname("signatures", p_item);
  while ((cc = mkdir(dir_name, perm)) != 0)
    {
      cc = errno;
      if (last_try == YES)
	{
	  break;
	}
      if (cc == ENOENT)
	{
	  for (i = strlen(dir_name)-1; 
	      dir_name[i] != '\0' && dir_name[i] != '/';
	      i--)
	    ;
	  if (dir_name[i] == '/')
	    dir_name[i] = '\0';
	  if ((cc = mkdir(dir_name, perm)) != 0)
	    {
	      cc = errno;
	      break;
	    }
	  chmod(dir_name, 00770);
	  last_try = YES;
	  dir_name[i] = '/';
	  continue;
	}
      else 
	{
	  break;
	}
    }			 
  if (cc != 0)
    {
      sprintf(error_msg, "\nFailed to make a %s directory for keeping petition"
	      "\nsignatures for %s on the %s list.\nERROR: ",
	      dir_name, p_item->eVote.title, list);
      switch (cc)
	{
	case ENOTDIR:
	  strcat(error_msg,"A component of the path prefix is not a directory.\n");
	  break;
	case ENOENT:
	  strcat(error_msg,"A component of the path prefix does not exist.\n");
	  break;
	case EACCES:
	  strcat(error_msg,"Either a component of the path prefix denies \nsearch permission, or write permission is denied on the parent \ndirectory of the directory to be created.\n");
	  break;
	case EEXIST:
	  error_msg[0] = '\0';
	  break;
	case EIO:
	  strcat(error_msg,"An I/O error has occurred while accessing the file system.\n");
	  break;
	default:
	  i = strlen(error_msg);
	  sprintf(&error_msg[i],"%d returned from mkdir(2).\n", cc);
	  break;
	}
      if (error_msg[0] != '\0')
	return NOT_OK;
    }
  chmod(dir_name, 00770);
  return OK;
}
/*********************************************************
 *      Returns the file name for the signers about p_item.
 *      If p_item == NULL, it returns just the directory.
 **********************************************************/ 
char *
pet_signers_fname(ITEM_INFO * p_item, time_t when)
{
  static char fname[PATHLEN + 1];
  struct tm *p_tm;
  char buf[8];
  strcpy(fname, pet_fname("signatures", p_item));
  if (p_item == NULL || when == 0L)
    return fname;
  p_tm = localtime(&when);
  sprintf(buf, "/%4d%02d", p_tm->tm_year + 1900,
	  p_tm->tm_mon + 1);
  strcat(fname, buf);
  return fname;
}
/**********************************************************
 *         Prints the signature for address to stdout.
 **********************************************************/	
OKorNOT
print_signature(char * address, unsigned long uid, time_t when)
{
  FILE* fp;
  char *fname;
  YESorNO found = NO;
  char line[MAX_LINE + 1];
  SIG sig;
  fname = pet_signers_fname(p_item_copy, when);
  if ((fp = fopen(fname, "r")) == NULL)
    {
      printf("\n%s\n\t", file_error);
      printf("%s \n\nto read a signature for \"%s\"."
	     "\n\nPlease forward this to %s.\n\n", 
	     fname, p_item_copy->eVote.title, eVote_mail_to);
      fprintf(stderr,"\n%s"
	      "\n\t%s \n\nto read a signatures for %s on %s.\n\n", 
	      file_error, fname, from, p_item_copy->eVote.title);
      perror("");
      return NOT_OK;
    }
  while (fgets(line, MAX_LINE, fp) != NULL)
    {
      if (strncmp(line,"- - - ", 6) == 0)
	{
	  if (found)
	    break;
	  strcat(line,"\n");
	  vitals_collect(fp, &sig);
	  if (uid == sig.uid)
	    {
	      found = YES;
	      printf("\n%s:\n\n", address);
	    }
	}
      if (found)
	{
	  printf("   ");
	  fputs(line, stdout);
	}
    }
  fclose(fp);
  if (found == YES)
    return OK;
  return NOT_OK;
}
/************************************************************
 *     Sorts on time stamp.
 ************************************************************/
OKorNOT 
sort_sigs(SIG ** esig, int no_sigs)
{
  int i, j;
  SIG *sort_list;
  SIG *old_list = *esig;
  if ((sort_list = malloc(no_sigs * sizeof(SIG))) == NULL)
    {
      sprintf(error_msg, "\nNo resources to sort sigs.\n");
      return UNDECIDED;
    }
  sort_list[0] = old_list[0];
  for (i = 1; i < no_sigs; i++)
    {
      for (j = i - 1; j >= 0; j--)
	{
	  if (sort_list[j].tstamp > old_list[i].tstamp)
	    sort_list[j+1] = sort_list[j];
	  else
	    break;
	}
      sort_list[j+1] = old_list[i];
    }
  free(old_list);
  *esig = sort_list;
  return OK;
}
/************************************************************
 *     Syncronizes the Clerk's data to match the data in
 *     the signature file for each of the petitions in
 *     lclist.
 *   Returns OK if all is ok.
 *       NOT_OK if there is some discrepancy in the data
 *    UNDECIDED if there is a system or programming problem
 *    When this is called, we are into eVote with an
 *    enter_admin -- it's fixed up so we leave the same way.
 *************************************************************/
OKorNOT
sync_sigs(ITEM_INFO * p_item)
{
  SIG * esigner;
  unsigned long no_signers;
  SIG * except;
  unsigned long no_excepts = 0;
  unsigned int ie = 0;
  unsigned int i;
  char line[MAX_LINE];
  char * fname;
  SIG stored;
  OKorNOT ret_cc = OK;
  YESorNO sig_into_eVote = YES;
  OKorNOT cc;
  FILE *fp;
  unsigned int stop;
  char * name;
  static ITEM_INFO item_copy;
  static int count = 0;
  char * found_str(YESorNO found);
  if (item_copy.eVote.title[0] == '\0')
    item_copy = *p_item;
  if ((except = malloc(sizeof(SIG) * no_of_participants)) 
     == NULL)
    {
      sprintf(error_msg, "signatures.c:sync_sigs: Can't make memory.");
      return UNDECIDED;
    }
  if ((cc = get_esigners(&item_copy, &esigner, &no_signers)) != OK)
    {
      return cc;
    }
  while ((fname = it_pet_fname(&item_copy, NO)) != NULL)
    {
      if ((fp = fopen(fname, "r")) == NULL)
	{
	  sprintf(error_msg, "\n%s", file_error);
	  if (!sig_into_eVote)
	    send_enter_admin(lclist);
	  return UNDECIDED;
	}
#ifdef EXTRAS
      printf("\nReading %s:\n", fname);
#endif
      while (fgets(line, MAX_LINE, fp) != NULL)
	{
	  if (strncmp(line,"- - - ", 6) == 0)
	    {
	      if (vitals_collect(fp, &stored) == UNDECIDED)
		{  /* then this is data that needs to
		      be transformed to the new format.
		      Take out after 2.5 */
		  if (++count >= 2)
		    {
		      printf("\nOnce is enough.\n");
		      exit(1);
		    }
		  it_pet_fname(&item_copy, YES);
		  fclose(fp);
		  free(except);
		  free(esigner);
		  fix_signatures(&item_copy);
		  return sync_sigs(&item_copy);
		}
	      stored.found = NO;
	      stop = ie;
#ifdef EXTRAS
	      printf("\n%s\t%5lu = %s ", ctime(&stored.tstamp),
		     stored.uid, who_is(stored.uid));
#endif
	      do
		{
		  if (esigner == NULL)
		    break;  /* no participants in Clerk's files */
		  if (esigner[ie].uid == stored.uid
		     && esigner[ie].tstamp == stored.tstamp)
		    {
		      for (i = 0; i < no_excepts; i++)
			{
			  if (except[i].uid == esigner[ie].uid)
			    {
			      switch (except[i].found)
				{
				case DROPIT:
				  break;
				case MAYBE: /* wrong tstamp before */
				  printf("\nFound a good sig for %s = %lu and tstamp = %ld = %s.",
					 who_is(esigner[ie].uid), 
					 esigner[ie].uid,
					 esigner[ie].tstamp, 
					 ctime(&esigner[ie].tstamp));
				case PUNT:  /* add it */
				  except[i].found = DROPIT;
				  printf("\nDropping earlier signature for %lu = %s with tstamp %ld = %s.",
					 except[i].uid, 
					 who_is(except[i].uid),
					 except[i].tstamp,
					 ctime(&except[i].tstamp));
				  break;
				default:
				  /* impossible */
				  break;
				}
			    }
			}
		      if (esigner[ie].found != YES) /* second sig */
			{
			  stored.found = YES;
			  esigner[ie].found = YES;
			}
		      else 
			{
			  /* should drop the first find */
			  stored.found = DROPIT; 
			}
		      break;
		    }
		  if (esigner[ie].uid == stored.uid
		     && esigner[ie].tstamp != stored.tstamp)
		    {
		      printf("\n%s = %lu has timestamp = %ld = %s",
			     who_is(esigner[ie].uid), esigner[ie].uid,
			     esigner[ie].tstamp, ctime(&esigner[ie].tstamp));
		      printf("in the Clerk and a signature with %ld = %s"
			     "in the signature file.\n",
			     stored.tstamp, ctime(&stored.tstamp));
		      if (esigner[ie].found == YES)
			{
			  printf("\nAlready found a good sig, dropping this one.\n");
			  stored.found = DROPIT;  /* duplicate */
			}
		      else
			{
			  stored.found = MAYBE; /* wrong tstamp */
			  esigner[ie].found = MAYBE;
			}
		      break;
		    }
		  ie = ++ie % no_signers;
		}
	      while (ie != stop);

	      switch (stored.found)
		{
		case YES:
#ifdef EXTRAS
	      printf("\n%lu = %s at %s\t%s\n", stored.uid, who_is(stored.uid),
		     ctime(&stored.tstamp), found_str(stored.found));
#endif
		  continue;
		  break;
		case DROPIT:  /* already gave up */
		  break;
		case PUNT: /* impossible -- means add it */
		  break;
		case MAYBE:  /* wrong time stamp */
		  break;
		case NO:
		  if (no_pet_votes == 0  /* fix it up */
		     && (name = who_is(stored.uid)) != NULL
		     && strncmp(name, "No user", 7) != 0)
		    {
		      stored.found = PUNT;  /* add it */
		    }
		  break;
		}
#ifdef EXTRAS
	      printf("%lu = %s at %s\texcept # %2ld %s\n", stored.uid, 
		     who_is(stored.uid),
		     ctime(&stored.tstamp), no_excepts, 
		     found_str(stored.found));
#endif
	      ret_cc = NOT_OK;
	      except[no_excepts++] = stored;
	    }
	}
      fclose(fp);
    }
#ifdef EXTRAS
  printf("\neVote's list has %ld:\n", no_signers);
#endif
  for (i = 0; i < no_signers; i++)
    {
#ifdef EXTRAS
      printf("\n%3d: %lu = %s \n\t%s\t%s", i, esigner[i].uid,
	     who_is(esigner[i].uid), ctime(&esigner[i].tstamp),
	     found_str(esigner[i].found));
#endif
      if (esigner[i].found == NO)
	{
	  if (sig_into_eVote)
	    {
	      sig_into_eVote = NO;
	      leave_admin();
	    }
	  printf("\nNo match in signature file:");
	  if (drop_esigner(&item_copy, esigner[i]) != OK)
	    ret_cc = UNDECIDED;
	}
    }
  for (ie = 0; ie < no_excepts; ie++)
    {
      if (sig_into_eVote)
	{
	  sig_into_eVote = NO;
	  leave_admin();
	}
      switch (except[ie].found)
	{
	case PUNT:
	  if (add_esigner(&item_copy, except[ie]) == OK)
	    {
	      printf("\nFound %s = %lu in the signature file with "
		     "\ntimestamp = %ld.  Added it to the Clerk's data.\n",
		     from, except[ie].uid, except[ie].tstamp);
	      /* add_esigner puts the name in from */
	      continue;
	    }
	  break;
	case MAYBE:
	  printf("\nUpdating timestamp:");
	  if (update_tstamp(&item_copy, &except[ie]) != OK)
	    ret_cc = UNDECIDED;
	  break;
	default:
	  /* impossible */
	  break;
	}
    }
  for (ie = 0; ie < no_excepts; ie++)
    {
      if (!sig_into_eVote)
	{
	  sig_into_eVote = YES;
	  send_enter_admin(lclist);
	}
      switch (except[ie].found)
	{
	case NO:
	case DROPIT:
	  p_item_copy = &item_copy;
	  printf("\nDropping this orphaned signature from the signature file:\n");
	  if (drop_signature(except[ie].uid, stdout, except[ie].tstamp)
	     != OK)
	    {
	      ret_cc = UNDECIDED;
	    }
	  break;
	default:
	  /* impossible */
	  break;
	}
    }
  if (except)
    free(except);
  if (esigner)
    free(esigner);
  count = 0;
  item_copy.eVote.title[0] = '\0';
  return ret_cc;
}
/************************************************************/
char *
found_str(YESorNO found)
{
  switch (found)
    {
    case YES:
      return "was found.";
      break;
    case DROPIT:
      return "duplicate dropping this.";
      break;
    case MAYBE:
      return "found with wrong time stamp.";
      break;
    case PUNT:
      return "not found but we have address -- add it to eVote";
      break;
    case NO:
      return "not found -- better drop.";
      break;
    default:
      return "impossible case.";
      break;
    }
  return "impossible";
}
/************************************************************/
OKorNOT
update_tstamp(ITEM_INFO * p_item, SIG * sig)
{
  static char name[400];
  strcpy(name, who_is(sig->uid));
  from = name;
  mail_voter = sig->uid;
  if (try_entering(NO) != OK)
    {
      sprintf(error_msg,"\nupdate_tstamp:Can't get back into Clerk with %s = %lu.\n",
	      from, sig->uid);
      return UNDECIDED;
    }
  printf("Updating time stamp for %s = %lu \nfrom %ld to %ld.\n",
	 name, sig->uid, push_time(p_item, sig->tstamp), sig->tstamp);
  i_am_leaving();
  return OK;
}
/************************************************************/
OKorNOT
vitals_collect(FILE *fp, SIG *here)
{
  char line[MAX_LINE];
  here->found = MAYBE;
  fgets(line, MAX_LINE, fp);
  if (sscanf(line,"%lu:%ld", &here->uid, &here->tstamp) != 2)
    return UNDECIDED;
  return OK;
}
/************************************************************/
void
vitals_store(FILE *fp, SIG *here)
{
  fprintf(fp,"%lu:%ld\n", here->uid, here->tstamp);
  return;
}
#define CLUMP 128
#define TOUPPER(X) (((X) <= 'z'&& (X) >= 'a') ? (X) + 'A' - 'a' : (X))
/**********************************************************/
OKorNOT
write_signature(FILE *fp, YESorNO show_time, 
			YESorNO display_date)
{
  int ch;
  YESorNO line_started = NO;
  char oldch = '\n';
  YESorNO some_message = NO;
  static char first_time[MAX_LINE + 1] = "";
  int fields = 0;
  int blank_lines = 0;
  int spaces = 0;
  int tabs = 0;
  int start = sig_start;
  SIG sig;
#ifdef ROSA
  OKorNOT write_mexico(FILE *fp, YESorNO show_time, 
		       YESorNO display_date);
  if (same("Mexico March", subject))
    return write_mexico(fp, show_time, display_date);
#endif
  if (start == 0)
    start = msg_start;
  if (first_time[0] == '\0')
    {
      read_to_end();
      strcpy(first_time, date_str(0));
      first_time[strlen(first_time) - 1] = '\0';
    }
  if (display_date)
    {
      fprintf(fp,"\n\n- - - %s - - - \n",
	      first_time);
    }
  if (show_time)
    {
      sig.uid = mail_voter;
      sig.tstamp = now;
      vitals_store(fp, &sig);
    }
  else
    {
      fprintf(fp, "\n");
    }
  if (give_email)
    {
      fprintf(fp, "From: %s\n", from);
    }
  fields = print_fields(fp);
  for (ch = start; ch <= end_mark ; ch++)
    {
      if (some_message == NO)
	{
	  if (buffer[ch] == '\n')
	    continue;
	}
      if (buffer[ch] != ' ' && buffer[ch] != '\t'
	 && buffer[ch] != '\n')
	{
	  if (blank_lines > 2)
	    blank_lines = 2;
	  while (blank_lines-- > 0)
	    fputc('\n', fp);
	  blank_lines = 0;
	  while (tabs-- > 0)
	    fputc('\t', fp);
	  while (spaces-- > 0)
	    fputc(' ', fp);
	  some_message = YES;
	  line_started = YES;
	}
      else if (buffer[ch] == '\n')
	{
	  spaces = 0;
	  tabs = 0;
	  blank_lines++;
	  line_started = NO;
	}
      else if (line_started == NO && buffer[ch] == ' ')
	{
	  spaces++;
	}
      else if (line_started == NO && buffer[ch] == '\t')
	{
	  tabs++;
	}
      if (line_started == NO)
	continue;
      /*  Here we're checking that the signature delimiter isn't part
	  of the signature file.  If it is, we change the signature 
	  file a tad.  */
      if (ch > start + 5  
	 && strncmp(&buffer[ch-5], "- - - ", 6) == 0)
	buffer[ch] = '.';
      fputc(buffer[ch], fp);
      oldch = buffer[ch];
    }
  if (oldch != '\n')
    fputc('\n', fp);
  if (some_message == NO && fields == 0)
    fprintf(fp,"\n(No signature message sent.)\n");
  return OK;
}
