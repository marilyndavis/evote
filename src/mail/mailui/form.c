/* $Id: form.c,v 1.4 2003/01/15 18:54:09 marilyndavis Exp $ */ 
/**********************************************************
 *   form.c  Functions that control the form for a petition.   
 *        Some functions for forms are in pet_out_xx.c files
 *        because they need translation.
 **********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include<errno.h>
#include"mailui.h"
/* functions defined here */
#define FIELD_BUNCH 2
FIELD * field;  /* shared with petition.c */
short no_of_fields;
/* shared with pet_out_xx.c */
char (*answer)[FIELD_LEN + 1];    /* externed in pet_out_xx.c */
YESorNO form_exists = NO;
YESorNO found_comment = NO;
/* locals */
static void zero_fields(FIELD *field, int no_fields);
static OKorNOT check_translations(FIELD *field, int no_fields);
/************************************************************
 *    Checks the format of the ith field
 ************************************************************/
OKorNOT
check_format(int i)
{
  int j;
  for (j = 0; field[i].format[j]; j++)
    {
      switch (field[i].format[j])
	{
	case '9':
	  if (answer[i][j] < '0'
	     || answer[i][j] > '9')
	    return NOT_OK;
	  break;
	case 'X':
	  if (!((answer[i][j] >= 'a' && answer[i][j] <= 'z')
	       || (answer[i][j] >= 'A' && answer[i][j] <= 'Z')))
	    return NOT_OK;
	  break;
	default:
	  break;
	}
    }
  return OK;
}
/************************************************************
 *      This is called at the end of parse form to make
 *      sure that if one field has a French translation,
 *      they all do.
 ***********************************************************/
OKorNOT
check_translations(FIELD * field, int no_fields)
{
  int i, t;
  YESorNO trans_exists;
  for (t = 0; t < no_languages; t++)
    {
      trans_exists = YES;
      if (field[0].name[t][0] == '\0')
	trans_exists = NO;
      for (i = 1; i < no_fields; i++)
	{
	  if (field[i].name[t][0] == '\0' && trans_exists)
	    {
	      sprintf(error_msg,"You don't have a %s translation for \"%s\" but you do for \"%s\".\n",
		      tongue[t].name, field[i].name[the_language], field[0].name[the_language]);
	      return NOT_OK;
	    }
	  if (field[i].name[t][0] != '\0' && !trans_exists)
	    {
	      sprintf(error_msg,
		      "You have a %s translation for %s = %s but not for %s.\n",
		      tongue[t].name, field[i].name[the_language], field[i].name[t],
		      field[0].name[the_language]);
	      return NOT_OK;
	    }
	}
    }
  return OK;
}
/************************************************************/
YESorNO
is_comment(char* str)
{
  int l;
  if (samen(str, tongue[the_language].comment,
	    strlen(tongue[the_language].comment)))
    return YES;
  for (l = 0 ; l < no_languages; l++)
    {
      if (l == the_language || tongue[l].comment[0] == '\0')
	continue;
      if (samen(str, tongue[l].comment,
	       strlen(tongue[l].comment)))
	return YES;
    }
  return NO;
}
/*********************************************************
 *       Parses the form from stdin by inspecting tokens.
 *       cc is the delimiter from the current token.
 *       Returns the delimiter from the current token again.
 *       Bounces an error to the sender if there's trouble.
 **********************************************************/ 
int
parse_form(int cc)
{
  int space;
  int i = -1, j;
  YESorNO line_started = NO;
  YESorNO name_done = NO;
  YESorNO name_started = NO;
  YESorNO format_started = NO;
  YESorNO format_done = NO;
  YESorNO need_translation = NO;
  YESorNO translation_started = NO;
  LANGUAGE new_language;
  if (MAX_LANGUAGES < no_languages)
    {
      sprintf(error_msg,"MAX_LANGUAGES needs to be bigger and eVote_petition recompiled.\n");
      bounce_error(SENDER | ADMIN);
    }
  no_of_fields = 0;
  if ((!same(token, "form") && !same(token, "form:") )
     || cc == EOF)
    return cc;
  if ((field = malloc((space = FIELD_BUNCH) * sizeof(FIELD)))
     == NULL)
    {
      sprintf(error_msg,
	      "\nThere are no resources to parse your form at this time.  Please try later.\n");
      bounce_error(SENDER | ADMIN);
    }
  zero_fields(field, FIELD_BUNCH);
  if (same(token, "form"))  /* get ':' if it's separated from "form" */
    {
      cc = get_token();
    }
  form_exists = YES;
  do
    {
      if (++i + 1 > space)
	{
	  if ((field = realloc(field, (space += FIELD_BUNCH) * sizeof(FIELD)))
	     == NULL)
	    {
	      sprintf(error_msg,
		      "\nThere are no resources to parse your form at this time.  Please try later.\n");
	      bounce_error(SENDER | ADMIN);
	    }
	  zero_fields(&field[i], FIELD_BUNCH);
	}
      line_started = NO;
      name_done = NO;
      name_started = NO;
      line_started = NO;
      format_started = NO;
      format_done = NO;
      need_translation = NO;
      new_language = DEFAULT_LANGUAGE;
      translation_started = NO;
      do    /* gather the translations */
	{
	  do /* gather each line */
	    {
	      cc = get_token();
	      if (same(token,"message:") || same(token,"message")
		 || same(token,"report") || same(token, "report:"))
		{
		  if (check_translations(field, i) != OK)
		    send_help("petition", SENDER, "Error:");
		  return cc;
		}
	      if (!line_started)
		{
		  line_started = YES;
		  if (token[0] == '*')
		    {
		      field[i].required = YES;
		      if (strlen(token) == 1)  /* '*' only as token */
			{
			  continue;
			}
		      for (j = 1; token[j]; j++)
			{
			  token[j-1] = token[j];
			}
		      token[j-1] = token[j];
		    }
		}
	      if (!name_done)
		{
		  if (strlen(field[i].name[the_language]) + strlen(token) + 1 >
		     FIELD_LEN)
		    {
		      sprintf(error_msg,"Your field names must be less than %d characters each.\n", FIELD_LEN);
		      send_help("petition", SENDER, "Error:");
		    }
		  if (!name_started)  /* delimit only after first word */
		    {
		      name_started = YES;
		    }
		  else if (strcmp(token, ":") != 0)
		    {
		      strcat(field[i].name[the_language], " ");
		    }
		  strcat(field[i].name[the_language], token);
		  if (token[strlen(token)-1] == ':')
		    {
		      name_done = YES;
		      field[i].name[the_language]
			[strlen(field[i].name[the_language]) -1] = '\0';
		      no_of_fields++;
		      for (j = 0; j < i; j++)
			{
			  if (same(field[i].name[the_language], 
				    field[j].name[the_language]))
			    {
			      sprintf(error_msg, "Each field must have a unique name.  \nYou have two fields called \"%s\".\n", field[i].name[the_language]);
			      send_help("petition", SENDER, "Error:");
			    }
			}
		    }
		  continue;
		}
	      if (!need_translation && is_flag(token, &new_language))
		{
		  if (field[i].name[new_language][0] != '\0')
		    {
		      sprintf(error_msg,"There is already a field name for %s, \"%s\".\n",
			      tongue[default_language].name,
			      field[i].name[default_language]);
		      send_help("petition", SENDER, "Error:");
		    }
		  need_translation = YES;
		  translation_started = NO;
		  continue;
		}
	      if (!need_translation && token[0] == '-')
		{
		  sprintf(error_msg,"%s is not a supported language.\n",
			  token);
		  send_help("petition", SENDER, "Error:");
		}
	      if (need_translation)
		{
		  if (strlen(field[i].name[new_language]) 
		     + strlen(token) + 1 > FIELD_LEN)
		    {
		      sprintf(error_msg,"Your field name translations must be less than %d characters each.\n", FIELD_LEN);
		      send_help("petition", SENDER, "Error:");
		    }
		  if (!translation_started)  /* delimit only after first word */
		    {
		      translation_started = YES;
		    }
		  else if (strcmp(token, ":") != 0)
		    {
		      strcat(field[i].name[new_language], " ");
		    }
		  strcat(field[i].name[new_language], token);
		  if (token[strlen(token)-1] == ':')
		    {
		      need_translation = NO;
		      field[i].name[new_language][strlen(field[i].name[new_language]) -1] = '\0';
		      for (j = 0; j < i; j++)
			{
			  if (same(field[i].name[new_language], field[j].name[new_language]))
			    {
			      sprintf(error_msg, 
				      "Each field must have a unique name.  "
				      "\nYou have two %s fields called \"%s\"."
				      "\n", tongue[new_language].name,
				      field[i].name[new_language]);
			      send_help("petition", SENDER, "Error:");
			    }
			}
		    }
		  continue;
		}
	      if (!format_done)
		{
		  if (strlen(field[i].format) + strlen(token) + 1 > FIELD_LEN)
		    {
		      sprintf(error_msg,"The format for your field must be less than %d characters.\n", FIELD_LEN);
		      send_help("petition", SENDER, "Error:");
		    }
		  if (!format_started)
		    {
		      format_started = YES;
		    }
		  else
		    {
		      strcat(field[i].format, " ");
		    }
		  strcat(field[i].format, token);
		}
	    }
	  while (cc != '\n' && cc != EOF);
	  format_done = YES;
	  if (cc == EOF)
	    break;
	  cc = get_token();
	  if (!is_flag(token, &new_language))
	    {
	      back_one_token();
	      break;
	    }
	  if (new_language == the_language)
	    {
	      sprintf(error_msg, "\n%s is the default language so you can't"
		      " also have a %s \ntranslation of %s", token, token,
		      field[i].name[the_language]);
	      send_help("petition", SENDER, "Error:");
	    }
	  if (field[i].name[new_language][0] != '\0')
	    {
	      sprintf(error_msg,"\nYou can't give two %s translations for %s.\n",
		      token, field[i].name[the_language]);
	      send_help("petition", SENDER, "Error:");
	    }
	  need_translation = YES;
	  translation_started = NO;
	  back_one_token();
	}
      while (need_translation);
      for (j = 0; field[i].format[j]; j++)
	if (field[i].format[j] != 'X' && field[i].format[j] != '9'
	   && field[i].format[j] != ' ' && field[i].format[j] != '\t'
	   && field[i].format[j] != '-')
	  {
	    sprintf(error_msg,
		    "\nThe format for your field can not contain the character '%c'.  \nUse only '9's for numbers and 'X's for letters.  You can use spaces \nand hyphens.\n",
		    field[i].format[j]);
	    send_help("petition", SENDER, "Error:");
	  }
    }
  while (cc != EOF);
  if (check_translations(field, i) != OK)
    send_help("petition", SENDER, "Error:");
  return cc;
}
/*************************************************************/
int
print_fields(FILE *fp)
{
  int i;
  for (i = 0; i < no_of_fields; i++)
    {
      if (!same(field[i].name[the_language], tongue[the_language].comment))
	{
	  if (field[i].name[the_language][0] == '\0')
	    fprintf(fp, "%s: %s\n", field[i].name[default_language], answer[i]);
	  else
	    fprintf(fp, "%s: %s\n", field[i].name[the_language], answer[i]);
	}
    }
  /*	if (found_comment == MAYBE)
	fprintf(fp,"%s:\n", tongue[the_language].comment); */
  return no_of_fields;
}
/************************************************************
 *    Reads the fields from the form_template
 *    if there are any.
 *    Returns the number of fields read.
 ************************************************************/
int
read_form_template(ITEM_INFO *p_item)
{
  FILE *fp;
  char * fname;
  char buf[MAX_LINE + 1];
  int i, j, k, l, m;
  YESorNO name_done = NO;
  YESorNO name_started = NO;
  YESorNO format_started = NO;
  LANGUAGE new_language;
  LANGUAGE working_language = DEFAULT_LANGUAGE;
  static YESorNO done = NO;
  if (done)
    {
      return no_of_fields;
    }
  done = YES;
  fname = pet_fname("form_template", p_item);
  if ((fp = fopen(fname, "r")) == NULL)
    {
      return no_of_fields = 0;
    }
  if(fgets(buf, MAX_LINE, fp) == NULL
     || sscanf(buf, "Fields: %hd\n", &no_of_fields) != 1
     || no_of_fields == 0)
    {
      fclose(fp);
      return no_of_fields = 0;
    }
  form_exists = YES;
  if ((field = malloc(no_of_fields * sizeof(FIELD)))
     == NULL)
    {
      sprintf(error_msg,
	      "\nThere are no resources to check your form at this time.  Please try later.\n");
      bounce_error(SENDER | ADMIN);
    }
  for (i = 0; i < no_of_fields; i++)
    {
      for (l = 0; l < no_languages; l++)
	{
	  field[i].name[(LANGUAGE)l][0] = '\0';
	}
      field[i].format[0] = '\0';
    }
  i = -1;
  while (fgets(buf, 2*FIELD_LEN + 2, fp) == buf)
    {
      /* new field */
      i++;
      field[i].required = MAYBE;
      k = -1;
      name_done = NO;
      name_started = NO;
      format_started = NO;
      working_language = DEFAULT_LANGUAGE;
      for (j = 0; buf[j] ; j++)
	{
	  if (name_started == NO)
	    if (buf[j] == ' ')
	      continue;
	  if (field[i].required == MAYBE)
	    {
	      if (buf[j] == '*')
		{
		  field[i].required = YES;
		  continue;
		}
	      else
		{
		  field[i].required = NO;
		}
	    }
	  if (!name_done)
	    {
	      name_started = YES;
	      field[i].name[working_language][++k] = buf[j];
	      if (buf[j] == ':')
		{
		  name_done = YES;
		  field[i].name[working_language][k] = '\0';
		  k = -1;
		  /* check for a translation of the field */
		  for (m = j+1; buf[m] != '\n'; m++)
		    if (buf[m] != ' ')
		      break;
		  if (is_flag(&buf[m], &new_language))
		    {
		      working_language = new_language;
		      j = m+3;
		      name_started = NO;
		      name_done = NO;
		    }
		  continue;
		}
	    }
	  else
	    {
	      if (format_started == NO)
		if (buf[j] == ' ')
		  continue;
	      format_started = YES;
	      working_language = the_language;
	      field[i].format[++k] = buf[j];
	    }
	}
      field[i].format[k] = '\0';
    }
  if (++i < no_of_fields)
    {
      sprintf(error_msg,"\nStored form is messed up for %s.\n",
	      subject);
      bounce_error(SENDER | ADMIN);
    }
  if (check_translations(field, no_of_fields) != OK)
    {
      bounce_error(SENDER | ADMIN);
    }
  return no_of_fields;
}	
/************************************************************
 *       Stores the form template
 *************************************************************/
OKorNOT
store_form_template(ITEM_INFO * p_item)
{
  int i, k;
  FILE * fp;
  char * fname;
  char buf[MAX_LINE + 1];
  char * mode = "w";
  fname = pet_fname("form_template", p_item);
  if ((fp = fopen(fname, "r")) != NULL)
    {
      mode = "a";
      /*			sprintf(error_msg, "\n\nERROR! There is already a file: \n\n%s \n\n"
				"Trying to start such a file for storing the form for a petition.\n\n\t\"%s\"",
				fname, subject);
				return NOT_OK; */
    }
  if ((fp = fopen(fname, mode)) == NULL)
    {
      sprintf(error_msg, "\n%s\n%s \n\nto store the form for the petition \n\n\t\"%s\".",
	      file_error,	fname, subject);
      return NOT_OK;
    }
  sprintf(buf, "Fields: %d\n", no_of_fields);
  fputs(buf, fp);
  for (i = 0; i < no_of_fields; i++)
    {
      sprintf(buf, "%c %s:", (field[i].required? '*' : ' '),
	      field[i].name[the_language]);
      fputs(buf, fp);
      for (k = 0; k < no_languages; k++)
	{
	  if (k == the_language)
	    continue;
	  if (field[i].name[k][0] == '\0')
	    continue;
	  sprintf(buf, " -%.2s %s:", tongue[k].name, field[i].name[k]);
	  fputs(buf, fp);
	}
      sprintf(buf," %s\n", field[i].format);
      fputs(buf, fp);
    }
  fclose(fp);
  return OK;
}
void
zero_fields(FIELD *field, int no_fields)
{
  int i, j;
  for (j = 0; j < no_fields; j++)
    {
      for (i = 0; i < no_languages; i++)
	{
	  field[j].name[i][0] = '\0';
	}
      field[j].required = NO;
      field[j].format[0] = '\0';
    }
}
/* NOTE:  All the rest of the code in this file is commented out! */
/************************************************************
 *  void print_form(FILE * fp)
 *       Prints the form to fp.  fp should be open.
 *************************************************************
void print_form(FILE *fp)
{
  int i, l;
  for(i = 0; i < no_of_fields; i++)
    {
      fprintf(fp,"%c %s: %s\n", (field[i].required == YES ? '*' : ' '),
	      field[i].name[the_language], field[i].format);
      for(l = 0; l < no_languages; l++)
	{
	  if(i == the_language)
	    continue;
	  if(field[i].name[l][0] == '\0')
	    continue;
	  fprintf(fp, "-%.2s: %s\n", tongue[l].name,
		  field[i].name[l]);
	}
    }
}
************************************************************
 *  OKorNOT skip_form(FILE *fp, FILE * fp_new)
 *    Expects that fp should be the open signature file positioned
 *    to the start of the file.
 *    When it returns, the file is still open but it is positioned
 *    at the start of signatures.
 *    If fp_new != NULL, it copies the read lines into
 *    the new file.
 ************************************************************
OKorNOT skip_form(FILE *fp, FILE * fp_new)
{
  int ch, i;
  char buf[2 * FIELD_LEN + 2];
  ch = fgetc(fp);
  ungetc(ch, fp);
  if(ch == '\n' || ch == EOF)
    {
      return OK;
    }
  fgets(buf, 2 * FIELD_LEN + 2, fp);
  if(fp_new != NULL)
    fputs(buf, fp_new);
  sscanf(buf, "Fields: %d\n", &no_of_fields);
  if(no_of_fields == 0)
    return OK;
  i = 0;
  while(fgets(buf, 2 * FIELD_LEN + 2, fp) == buf)
    {
      if(fp_new != NULL)
	fputs(buf, fp_new);
      i++;
    }
  if(i != no_of_fields)
    {
      sprintf(error_msg,"\nStored form is messed up for %s.\n",
	      subject);
      return NOT_OK;
    }
  return OK;
  }	*/
