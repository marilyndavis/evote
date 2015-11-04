/* $Id: token.c,v 1.5 2003/10/20 17:04:46 marilyndavis Exp $ */ 
 /*********************************************************
 *  token.c  -- Functions to tokenize the incoming message --
 *              past the headers.
 *********************************************************
 **********************************************************/
#include<stdio.h>
#include "../mailui/mailui.h"
char token[MAX_TOKEN + 1];
int tokens_read_to = -1;
static int token_mark;
static char last_token[MAX_TOKEN + 1];
/* static YESorNO paragraph_ends;*/
/************************************************************
 *      Resets tokens_read_to to be before the current token.
 *      Puts the last token in the token and returns the
 *      delimiter before the last token.
 *      You can only call this once and then you have to read
 *      a token before you can call it again.
 ************************************************************/
int
back_one_token(void)
{
  do
    {
      if (buffer[tokens_read_to] != ' '
	 && buffer[tokens_read_to] != '\t'
	 && buffer[tokens_read_to] != '\n')
	break;
    }
  while (--tokens_read_to > 0);
  do
    {
      if (buffer[tokens_read_to] == ' '
	 || buffer[tokens_read_to] == '\n'
	 || buffer[tokens_read_to] == '\t')
	break;
    }
  while (--tokens_read_to > 0);
  tokens_read_to++;
  strcpy(token, last_token);
  return buffer[tokens_read_to];
}
/*************************************************************
 *     blocks the token in the message so that an outgoing
 *     message doesn't repeat it -- it's a password.
 *************************************************************/
void
block_token(void)
{
  int worker;
  worker = tokens_read_to;
  do
    {
      if (buffer[worker] != ' '
	 && buffer[worker] != '\t'
	 && buffer[worker] != '\n')
	break;
    }
  while (--worker > 0);
  do
    {
      if (buffer[worker] == ' '
	 || buffer[worker] == '\n'
	 || buffer[worker] == '\t')
	break;
      buffer[worker] = '#';
    }
  while (--worker > 0);
  return;
}
/************************************************************
 *     Finds the first token, stripping out '>'s at the 
 *     beginning of the line.
 *************************************************************/
int
fix_first_token(void)
{
  char * here;
  int i, cc;
  tokens_read_to = -1;
  do
    {
      if ((cc = get_token()) == EOF)
	{
	  return EOF;
	}
      for (i = 0; token[i] == '>'; i++)
	{
	}
      here = &token[i];
    }
  while (*here == '\0');
  strcpy(token, here);
  return cc;
}
/****************************************************
 *       parses buffer for tokens. 
 *       Tokens are stored in extern char token[].
 *       The return value is the character that delimited
 *       the token.
 *       token[0] = '\0' if we're beyond EOF.
 *******************************************************/
int
get_token(void)
{
  int i, j = 0;
  static int last_return_ch = 0;
  YESorNO get_out = NO;
  YESorNO seen_newline = NO;

  if (tokens_read_to == -1)
    tokens_read_to = msg_start;
  strcpy(last_token, token);
  /* eat white space */
  while (buffer[tokens_read_to])
    {
      switch (buffer[tokens_read_to])
	{
	case '\\':
	  if (buffer[tokens_read_to+1] == '\n')
	    tokens_read_to++;
	  else 
	    break;
	case ' ':
	case '\n':
	case '\r':
	case '\t':
	  tokens_read_to++;
	  continue;
	default:
	  break;
	}
      break;
    }
  /* collect until white space */
  while (buffer[tokens_read_to] && !get_out)
    {
      switch (buffer[tokens_read_to])
	{
	case '\\':
	  if (buffer[tokens_read_to + 1] == '\n')
	    {
	      tokens_read_to++;
	      tokens_read_to++;
	      continue;
	    }
	  break;
	case ' ':
	case '\n':
	case '\r':
	case '\t':
	  get_out = YES;
	  continue;
	default:
	  break;
	}
      token[j++] = buffer[tokens_read_to++];
      if (j > MAX_TOKEN)
	break;
    }
  token[j] = '\0';
  i = tokens_read_to;
  last_return_ch = 0;
  /* eat white space again */
  while (buffer[i])
    {
      switch (buffer[i])
	{
	case '\\':
	  if (buffer[i+1] == '\n')
	    i++;
	  else
	    break;
	case '\n':
	case '\r':
	  seen_newline = YES;
	case ' ':
	case '\t':
	  i++;
	  continue;
	default:
	  break;
	}
      break;
    }
  tokens_read_to = buffer[i] ? i - 1 : i ;

  if (seen_newline)
    {
      last_return_ch = '\n';
    }
  else
    {
      last_return_ch = (buffer[tokens_read_to]? buffer[tokens_read_to] : EOF);
    }
  return last_return_ch;
}
/************************************************************
 *     Sets the token_mark to the current tokens_read_to.
 *     Used by petition.c:parse_and_store_translations
 *************************************************************/
void
mark_token(void)
{
  token_mark = tokens_read_to;
}
/************************************************************
 *   prints the first line of the message to stdout.
 *************************************************************/
void
print_first_line(void)
{
  int i;
  YESorNO started = NO;
  for (i = msg_start; buffer[i]; i++)
    {
      if (!IS_WHITE(buffer[i]))
	started = YES;
      if (started && buffer[i] == '\n')
	break;
      putchar(buffer[i]);
    }
  printf("\n");
}
/*********************************************************
 *    This prints out the tokens read so far -- 
 **********************************************************/ 
void
print_tokens(YESorNO one_line)
{
  int i, j;
  YESorNO started = NO;
  YESorNO new_line_last = NO;
  YESorNO jump_out = NO;
  printf("\n");
  if (end_mark > 0)
    {
      j = end_mark;
    }
  else
    {
      j = tokens_read_to;
      jump_out = YES;
    }
  for (; j > 0 ; j--)
    {
      if (buffer[j] != '\0' && buffer[j] != ' ' 
	 && buffer[j] != '\n' && buffer[j] != '\t')
	break;
    }
  if (j < msg_start)
    {
      print_first_line();
      return;
    }
  for (i = msg_start; i <= j; i++)
    {
      if (!IS_WHITE(buffer[i]))
	started = YES;
      if (started && one_line && buffer[i] == '\n')
	break;
      new_line_last = buffer[i] == '\n' ? YES : NO;
      putchar(buffer[i]);
    }
  /*  if (!one_line && !jump_out)
    {
      printf("\n**** !one_line && !jump_out is happening ***\n");
      for (; buffer[i]; i++)
	{
	  new_line_last = buffer[i] == '\n' ? YES : NO;
	  putchar(buffer[i]);
	}
	}*/
  if (!new_line_last)
    printf("\n");
}
/************************************************************
 *      Puts the buffer, up and including the end keyword
 *      or the EOF, whichever comes first.
 *      end_mark is set to the last char before "end".
 *************************************************************/
void
read_to_end(void)
{
  int last_new_line = tokens_read_to;
  int i;
  YESorNO some_white = NO;
  if (end_mark != -1)
    return;
  for (i = msg_start; buffer[i]; i++)
    {
      if (buffer[i] == '\n')
	{
	  if (is_end(&buffer[last_new_line + 1]))
	    {
	      end_mark = last_new_line;
	      return;
	    }
	  last_new_line = i;
	}
    }
  for (i--; i > 0 ; i--)
    {
      if (buffer[i] != '\n' && buffer[i] != ' ' && buffer[i] != '\t')
	{
	  end_mark = some_white? i+1 : i;
	  return;
	}
      some_white = YES;
    }
}
/****************************************************
 *       makes some space for the next token and
 *       saves the token in the space and returns
 *       a pointer to the stored token.  The return
 *       code for get_token() is returned by reference.
 *******************************************************/
char *
save_token(int * cc)
{
  char * where;
  *cc = get_token();
  if (token[0] == '\0')
    return NULL;
  if ((where = malloc(strlen(token) +1)) == NULL)
    {
      sprintf(error_msg,"\nNo resources for your command now.\n");
      bounce_error(SENDER | ADMIN);
    }
  strcpy(where, token);
  return where;
}
/*************************************************************
 *  Copies the buffer from the token_mark to tokens_read_to.
 *************************************************************/
void
write_from_mark(FILE *fp, YESorNO drop_last_token)
{
  int i = tokens_read_to;
  YESorNO new_line = NO;
  if (drop_last_token)  
    {  /* this is copied from back_one_token() */
      do
	{
	  if (buffer[i] != ' '
	     && buffer[i] != '\t'
	     && buffer[i] != '\n')
	    break;
	}
      while (--i > 0);
      do
	{
	  if (buffer[i] == ' '
	     || buffer[i] == '\n'
	     || buffer[i] == '\t')
	    break;
	}
      while (--i > 0);
    }
  for (; ; i--)
    {
      if (buffer[i] == '\n')
	new_line = YES;
      if (buffer[i] && buffer[i] != '\n' && buffer[i] != ' '
	 && buffer[i] != '\t')
	break;
    }
  for (; token_mark <= i; token_mark++)
    {
      fputc(buffer[token_mark], fp);
    }
  if (new_line)
    fputc('\n', fp);
}
