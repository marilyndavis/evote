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

/* $Id: util.c,v 1.5 2003/01/15 18:54:09 marilyndavis Exp $ */ 
/**********************************************************
 *  util.c -  a few comparison functions and a date string.
 *            And  atoul -- ascii to unsigned long.
 *            And ratio_string to format ratios
 *********************************************************
 **********************************************************/
#include<stdio.h>
#include<sys/types.h>
#include<dirent.h>
#include"mailui.h"
#include<stdlib.h>
#include<time.h>
#include<limits.h>
#include <string.h>
#include<sys/types.h>
#include<sys/stat.h>
#define TESTING 0
time_t now;
#define IS_HEX(X) (  ( ((X) <= '9' && (X) >= '0') || \
		   ((X) <= 'F' && (X) >= 'A') \
		 || ((X) <= 'f' && (X) >= 'a')) ? 1 : 0)
#define HEXNUM(X) ( ((X) <= '9' && (X) >= '0') ? ((X) - '0') \
	 : ((X) <= 'F' && (X) >= 'A') ? ((X) - 'A' + 10) \
	 : ((X) <= 'f' && (X) >= 'a') ? ((X) - 'a' + 10): -1 )
#define IS_LEAP(Y) (((Y%4 == 0) && (Y%4 != 100)) \
				|| (Y%400 == 400))
char file_error[] = "\nERROR! Unable to open file:\n";
char accents[]={'�','�','�','�','�','�','�',
		'�','�','�','�','�','�','�',
		'�','�','�','�','�','�',
		'�','�','�','�','�','�','�',
		'�','�','�','�','�','�','�',
		'�','�','�','�','�','�','�',
		'�','�','�','�','�','�','�',
		'�','�','�','�','�','�','�','\0'};
char ascii[] = {'A','A','A','A','A','A','C','E','E','E','E','I','I',
		'I','I','N','O','O','O','O','O','O','U','U','U',
		'U','Y','a','a','a','a','a','a','c','e','e','e',
		'e','i','i','i','i','n','o','o','o','o','o','o',
		'u','u','u','u','y','y','\0'};
static int check_file_error(int cc, char * dir_name, char * fail_name);
static unsigned char ltoupper(unsigned char c);
/************************************************************
 *     Makes a language translatable version of TO someone
 *     making the decoration right.
 *************************************************************/
void
address(char * hey, char * you)
{
  int i, lenh, leny;
  printf("\n\n");
  lenh = strlen(hey);
  for (i = 0; i < lenh; i++)
    printf("=");
  printf(" ");
  leny = strlen(you);
  for (i = 0; i < leny ; i++)
    printf("=");
  printf("\n%s %s\n", hey, you);
  for (i = 0; i < lenh; i++)
    printf("=");
  printf(" ");
  for (i = 0; i < leny ; i++)
    printf("=");
}
/************************************************************
 *      Returns the unsigned long that's in the string.
 *      0 on error so don't trust 0.
 *************************************************************/
unsigned long
atoul(char * str)
{
  unsigned long answer = 0L;
  int i;
  int len = strlen(str);
  for (i = 0; i < len; i++)
    {
      if (str[i] < '0' || str[i] > '9')
	return 0L;
      answer *= 10;
      answer += (str[i] - '0');
    }
  return answer;
}
/************************************************************
 * int check_file_error(int cc, char * dir_name, char * file_name)
 *  Figures out why make_path() didn't work.
 *************************************************************/
int
check_file_error(int cc, char * dir_name, char * file_name)
{  
  int i;
  sprintf(error_msg, "\nFailed to make a %s directory at %s for an eVoted list for %s.\nERROR: ",
	  dir_name, file_name, list);
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
  return OK;
}
/************************************************************
 *        Looks through the input string for %xx 
 *        It returns a string that is the same as the input
 *        string but any %xx's are replaced by the ascii
 *        character they are trying to be.
 ************************************************************/
char *
convert_hex(char * input)
{
  char * answer;
  int in_len = strlen(input);
  int i, j;
  YESorNO do_it = NO;
  int check;
  if (in_len == 0)
    return input;
  for (i = 0; input[i+1] ; i++)
    {
      if (input[i] == '%' && IS_HEX(input[i + 1]))
	{
	  do_it = YES;
	  break;
	}
    }
  if (do_it == NO)
    return input;
  if ((answer = malloc(in_len)) == NULL)
    {
      sprintf(error_msg,"\nNo space to copy subject in convert_hex.\n");
      return NULL;
    }
  for (i = 0, j= 0; input[i]; i++, j++)
    {
      if (input[i] != '%'
	 || (input[i+1] != '\0' && !IS_HEX(input[i+1]) 
	     && input[i+2] != '\0' && !IS_HEX(input[i+2])))
	{
	  answer[j] = input[i];
	  continue;
	}
      check = HEXNUM(input[i+1]) * 16 + HEXNUM(input[i+2]);
      answer[j] = check;
      i += 2;
    }
  answer[j] = '\0';
  return answer;
}
/************************************************************
 *      Returns the number of days since Jan 1, 1995
 *************************************************************/
int
days_since(struct tm * date_in)
{
  int mo_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int days = 0;
  int i;
  for (i = 95; i < date_in->tm_year; i++)
    {
      days += 365;
      if (IS_LEAP(i))
	days++;
    }
  for (i = 0; i < date_in->tm_mon; i++)
    {
      days += mo_days[i];
      if (i == 1 && IS_LEAP(date_in->tm_year))
	days++;
    }
  return (days += date_in->tm_mday);
}
/************************************************************
 *       Returns the difference in days between the
 *       two time structures.
 *       Expects the tm_year, tm_mon, and tm_mday members
 *       to have good values.
 *************************************************************/
int
diff_time(struct tm * one, struct tm * two)
{
  int days_since(struct tm * date_in);
  return days_since(one) - days_since(two);
}
/************************************************************
 *    strips the '\n' from a fgets call
 *************************************************************/
char *
fgetsn(char *line, int len, FILE *fp) 
{
  int i;
  char * ret;
  if ((ret = fgets(line, len, fp)) == NULL)
    return ret;
  for (i = 0; line[i] ; i++)
    {
      if (line[i] == '\n')
	line[i] =  '\0';
    }
  return ret;
}
/************************************************************
 *     Wraps lines of ==='s around the string.
 *************************************************************/
void
highlight(const char *str)
{
  int i, j;
  for (i = 0; i < 2; i++)
    {
      printf("\n");
      for (j = 0; str[j]; j++)
	{
	  putchar(str[j] == ' '? ' ' : '=');
	}
      if (i == 0)
	printf("\n%s", str);
    }
  putchar('\n');
}
/************************************************************
 *   Checks that the string only contains
 *   the end keyword and white space.
 *************************************************************/
YESorNO
is_end(char *str)
{
  int i;
  int len;
  while (*str == ' ' || *str == '\t' || *str == '\n')
    str++;
  for (i = 0; i < no_languages; i++)
    {
      len = strlen(tongue[(LANGUAGE)i].end);
      if (samen(str, tongue[(LANGUAGE)i].end, len))
	break;
    }
  if (i == no_languages)
    return NO;
  for (i = len; str[i] != '\n'; i++)
    {
      if (str[i] != ' ' && str[i] != '\t')
	{
	  return NO;
	}
    }
  return YES;
}
/***********************************************************
 *        returns YES if the token is about left_side=right_side.
 *        left_side and right_side must be all lower_case.
 *   pcc has the delimiter for the last token.
 *   if right_side == "number", then the right side is a number
 *       and the number found is returned.
 ***********************************************************/
YESorNO
is_option(int* pcc, char* lside, char* rside,
		  int *number_found)
{
  int i;
  char *ptoken = NULL;
  char left_side[MAX_TOKEN + 1];
  char right_side[MAX_TOKEN + 1];
  strcpy(left_side, lside);
  strcpy(right_side, rside);
  if (token[0] != left_side[0] && token[0] != left_side[0] - 'a' + 'A')
    return NO;
  i = -1;
  while (token[++i] != '=' && token[i] != '\0')
    ;
  if (token[i] == '=')
    {
      token[i] = '\0';
      ptoken = &token[i+1];
      if (*ptoken == '\0')  /* still don't have right side */
	ptoken = NULL;
    }
  if (strlen(token) < strlen(left_side))
    {
      left_side[strlen(token)] = '\0';
    }
  if (!same(token, left_side))
    return NO;
  if (ptoken == NULL) /* equals in next token? */
    {
      *pcc = get_token();
      if (token[0] == '\0')
	return NO;
      if (strcmp(token,"=") == 0)
	{
	  *pcc = get_token();
	  if (token[0] == '\0')
	    return NO;
	}
      if (token[0] == '=') /* and there's more stuff in the token */
	{
	  ptoken = &token[1];
	}
      else
	ptoken = token;
    }
  /* Now ptoken has the right side */
  if (same(right_side,"number"))
    {
      *number_found = atoi(ptoken);
      while (*ptoken != '\0')
	{
	  if (*ptoken < '0' || *ptoken > '9')
	    return NO;
	  ptoken++;
	}
      return YES;
    }
  if (strlen(ptoken) < strlen(right_side))
    {
      right_side[strlen(ptoken)] = '\0';
    }
  if (same(ptoken, right_side))
    return YES;
  return NO;
}
/************************************************************
 *  Contributed by Laurent Chemla.  This strips out the
 *  accents as well as raises l.c. to upper.
 *************************************************************/
unsigned char
ltoupper(unsigned char c)
{
  char *ptr;
  if ((ptr=strchr(accents, c)) != NULL)
    {
      c=ascii[ptr-accents];
    }
  if (c <= 'z' && c >= 'a')
    {
      c = c + 'A' - 'a';
    }
  return c;
}
/**********************************************************
 *         Changes the string to all upper case and
 *         puts it in result and returns result.
 ************************************************************/
char
*lower(char *result, char *str)
{
  int i;
  for (i = 0; str[i]; i++)
    {
      result[i] = (str[i] >= 'A' && str[i] <= 'Z' ? str[i] + 'a' - 'A' :
		   str[i]);
    }
  result[i] = '\0';
  return result;
}
/************************************************************
 *     Puts a line of ==='s under the string.
 *************************************************************/
void
lowlight(char * str)
{
  int j;
  printf("\n%s\n", str);
  for (j = 0; str[j]; j++)
    {
      putchar(str[j] == ' '? ' ' : '=');
    }
  putchar('\n');
}
/************************************************************/
char *
make_flag(char * name)
{
  static char flag[4];
  sprintf(flag,"-%.2s", name);
  return flag;
}
/************************************************************/
OKorNOT
make_path(char *to_here)
{
  char fname[FNAME + 1];
  int i, len;
  struct stat ss;
  strcpy(fname, to_here);
  len = i = strlen(fname);
  do
    {
      if (stat(fname, &ss) == 0)
	{
	  if (i == len)
	    return OK;
	  break;
	}
      while (i > 0 && fname[i] != '/')
	i--;
      fname[i] = '\0';
    }
  while (i > 0 && mkdir(fname, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
	!= 0);
  if (i == 0)
    {
      if (check_file_error(errno, to_here, fname) != OK)
	return NOT_OK;
    }
  chmod(fname, 0770);
  do
    {
      while (fname[i] != '\0')
	i++;
      fname[i] = '/';
      if (stat(fname, &ss) != 0)
	{
	  if (mkdir(fname, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
	     != 0 && errno != EEXIST)
	    {
	      if (check_file_error(errno, to_here, fname) != OK)
		return NOT_OK;
	    }
	}
      chmod(fname, 0770);
      if (strcmp(fname, to_here) == 0)
	return OK;
    }
  while (i < len);
  return OK;
}
/**********************************************************
 *    Mallocs some space for a copy of string, copies
 *    and returns a pointer to the copy.
 *********************************************************/
char *
make_string(char *str)
{
  char *where;
  if ((where = malloc(strlen(str)+1)) == NULL)
    return NULL;
  strcpy(where, str);
  return where;
}
/************************************************************
 *   This little function provides a pseudo-random number
 *   between min and max.
 ************************************************************/
int
pick(int min, int max)
{
  static int seeded = 0;
  if (seeded == 0)
    {
      seeded = 1;
      srand((unsigned int)time(NULL));
    }
#ifdef TESTING
  return '1';
#else
  return (int)((float)(max - min + 1)*rand()/(RAND_MAX+1.0) + min);
#endif
}
/**********************************************************
 *         Changes the string to all upper case and
 *         puts it in result and returns result.
 ************************************************************/
char
*raiseup(char *result, char *str)
{
  int i;
  for (i = 0; str[i]; i++)
    {
      result[i] = (str[i] >= 'a' && str[i] <= 'z' ? str[i] + 'A' - 'a' :
		   str[i]);
    }
  result[i] = '\0';
  return result;
}
/*************************************************************
 *         Non-case string compare.
 ************************************************************/
int
strCmp(char *str1, char *str2)
{
  int i;
  for (i = 0; str1[i] && str2[i]; i++)
    {
      if (ltoupper(str1[i]) < ltoupper(str2[i]))
	return -1;
      if (ltoupper(str1[i]) > ltoupper(str2[i]))
	return 1;
    }
  if (str1[i] != '\0')
    return 1;
  if (str2[i] != '\0')
    return -1;
  return 0;
}
/**********************************************************
 *  case insensitive   string compare.
 **********************************************************/
int
strNcmp(char *str1, char *str2, int n)
{
  while (*str1 != '\0' && *str2 != '\0')
    {
      if (ltoupper(*str1) != ltoupper(*str2))
	return (ltoupper(*str2) - ltoupper(*str1));
      if (--n == 0)
	return 0;
      str1++;
      str2++;
    }
  if (*str1 != '\0')
    return(*str1);
  else return(*str2);
}
/************************************************************
 *          Replaces the this string within the str
 *          with with_this.
 ************************************************************/
char
*replace(char *str, char *this_one, char *with_this)
{
  int i;
  int len_this = strlen(this_one);
  int len_str = strlen(str);
  int len_with = strlen(with_this);
  char *new_str;
  for (i = 0; i <= len_str - len_this ; i++)
    {
      if (strncmp(&str[i], this_one, len_this) == 0)
	{   /* hit */
	  if ((new_str = malloc(len_str + len_with - len_this))
	     == NULL)
	    {
	      sprintf(error_msg,"\nCan't allocate space for new subject.\n");
	      return NULL;
	    }
	  strncpy(new_str, str, i);
	  strcat(new_str, with_this);
	  strcat(new_str, &str[i + len_this]);
	  return new_str;
	}
    }
  return str;
}
/*************************************************************
 *       sends back a string that represents the ratio
 ************************************************************/
char
*ratio_string(float ratio)
{
  static char answer[6];
  strcpy(answer, "  - ");
  if (ratio < -.00001)
    return answer;
  if (ratio < .00001)
    return "   0";
  if (ratio < 10)
    sprintf(answer, "%4.2f", ratio);
  else if (ratio < 100)
    sprintf(answer, "%4.1f", ratio);
  else 
    sprintf(answer, "%5.0f", ratio);
  return answer;
}
