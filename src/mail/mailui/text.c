/* $Id: text.c,v 1.6 2003/10/20 17:07:34 marilyndavis Exp $ */ 
/**********************************************************
 *  text.c - Functions that deal with the poll's text.
 *********************************************************
 **********************************************************/
#include<stdio.h>
#include<errno.h>
#include"mailui.h"
#include<sys/types.h>
#include<sys/stat.h>
/*********************************************************
 *    Prints the poll text on stdout.
 **********************************************************/ 
void
display_poll_text(ITEM_INFO * p_item, YESorNO just_checking)
{
  FILE *fp;
  char *fname;
  int ch;
  printf("\n");
  highlight("POLL TEXT");
  if (just_checking == YES)
    {
      dump_poll_message(stdout);
      return;
    }
  while ((fp = fopen(fname = poll_text_name(p_item), "r")) 
	== NULL)
    {
      printf("\n%s", file_error);
      printf("\n\t%s \n\n", fname);
      printf("to read this poll's text.\n\nPlease forward this to %s.\n\n",
	     eVote_mail_to);
      fprintf(stderr,"%s\n\t%s \n\nto read this poll's text.\n\n", 
	      file_error, fname);
      perror("");
      return;
    }
  if ((ch = fgetc(fp)) == EOF)
    {
      char er[] = "\n\nERROR! Empty file: \n\n";
      char er2[]="\n\nCan't read this poll's text.\n\n";
      printf("%s%s%sPlease forward this to %s.\n\n", 
	     er, fname, er2, eVote_mail_to);
      fprintf(stderr,"%s%s%s", er, fname, er2);
      fclose(fp);
      return;
    }
  if (ch != '\n')
    putchar('\n');
  putchar(ch);
  while ((ch = fgetc(fp)) != EOF)
    putchar(ch);
  fclose(fp);
}
/*********************************************************
 *     Deletes the file containing the poll's text.
 **********************************************************/ 
void
drop_poll_text(ITEM_INFO * p_item)
{
  char *fname;
  fname = poll_text_name(p_item);
  chmod(fname, 00660);
  if (unlink(fname) != 0)
    {
      sprintf(error_msg, "Can't drop poll text for %s", p_item->eVote.title);
      perror(error_msg);
      error_msg[0] = '\0';
    }
  return;
}
/*********************************************************
 *    dumps the message waiting in the token queue
 *    to the output indicated.
 *    Called from mailui/text.c
 *********************************************************/
void
dump_poll_message(FILE* fp)
{
  YESorNO some_message = NO;
  YESorNO some_space = NO;
  read_to_end();
  if (token[0] != '\0' && (same(token, "message") || same(token,"message:")))
    {
      while (tokens_read_to <= end_mark )
	{ 
	  if (buffer[tokens_read_to] != ' '
	     && buffer[tokens_read_to] != '\n'
	     && buffer[tokens_read_to] != '\t')
	    {
	      some_message = YES;
	    }
	  if (buffer[tokens_read_to] == '\n')
	    some_space = YES;
	  fputc(buffer[tokens_read_to++], fp);
	}
    }
  if (some_message == NO)
    {
      if (!some_space)
	fprintf(fp,"\n");
      fprintf(fp,"(None given.)\n");
    }
}
/*********************************************************
 *   Called from List:start_list, this creates the
 *   directory for storing the texts for the list's polls.
 **********************************************************/ 
OKorNOT
make_poll_dir(char * list)
{
  char * dir_name;
  struct stat buf;

  if (stat(listdir, &buf) != 0)
    {
      char up_one[FNAME + 1];
      int i;

      strcpy(up_one, listdir);
      for(i = strlen(up_one); up_one[i] != '/'; i--)
	;
      up_one[i] = '\0';
      if (stat(up_one, &buf) != 0)
	{
	  sprintf(error_msg,"\nCan't find owner of %s or %s\n", 
		  listdir, up_one);
	  return NOT_OK;
	}
    }
  if (make_path(dir_name = poll_text_name(NULL)) != OK)
    return NOT_OK;
  chown(dir_name, buf.st_uid, buf.st_gid);
  chmod(dir_name, 00775);
  return OK;
}
/*********************************************************
 * Returns the file name for the text about p_item.
 * If p_item == NULL, it returns just the directory.
 **********************************************************/ 
char *
poll_text_name(ITEM_INFO * p_item)
{
  static char fname[FNAME + 1];
  if (fname[0] != '\0')
    return fname;
  if (p_item == NULL)
    sprintf(fname, "%s/%s", pollsdir(), lclist);
  else
    sprintf(fname, "%s/%s/P%lu", pollsdir(), lclist, 
	    p_item->static_id.local_id);
  return fname;
}
/*********************************************************
 *     Creates a file for storage of the poll's text.
 **********************************************************/ 
void
store_poll_text(ITEM_INFO * p_item)
{
  FILE * fp;
  char *fname;
  fname = poll_text_name(p_item);
  if ((fp = fopen(fname, "w")) == NULL)
    {
      int no_items = 1;
      ITEM_INFO * item_copy;
      sprintf(error_msg, "\neVote is unable to store your poll text in %s now.\nPlease try later", fname);
      if (p_item->eVote.type == PLAIN)
	no_items += (p_item+1)->eVote.no_in_group;
      /* If p_item points into to the item_info array maintained
	 by the Clerk, there is a danger that the array will change
	 before our call to drop goes in.  To be sure, we'll make
	 a copy.  */
      if ((item_copy = malloc(no_items * sizeof(ITEM_INFO)))
	 == NULL)
	item_copy = p_item;  /* give it up */
      else
	memcpy(item_copy, p_item, no_items * sizeof(ITEM_INFO));
      if (drop_items(item_copy, no_items) != no_items)
	{
	  strcat(error_msg, "\n\nPoll is in eVote but poll text is not stored!\n");
	}
      bounce_error(SENDER | OWNER | ADMIN);
    }
  dump_poll_message(fp);
  fclose(fp);
  chmod(fname, 00440);
  return;
}
