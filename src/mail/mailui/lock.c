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

/* $Id: lock.c,v 1.3 2003/01/15 18:54:09 marilyndavis Exp $ */ 
/**********************************************************
 *  lock.c -  Two functions to provide a fancy locking
 *            scheme based on a file semiphore.
 *            And one function to test the existence of a
 *            file.
 *            Also,  exists(..), to test for the existence
 *            of a file.
 *********************************************************
 **********************************************************/
#include<stdio.h>
#include<sys/types.h>
#include<dirent.h>
#include"mailui.h"
/**************     EXISTENCE   ***************************
 *   Returns YES if the file exists, NO if not.
 **********************************************************/
YESorNO
exists(char * full_path_name)
{
  DIR *dirp;
  YESorNO found = NO;
  char * fname;
  char * path;
  int i;
  struct dirent * dp;
  i = strlen(full_path_name) + 1;
  while (--i >= 0 && full_path_name[i] != '/')
    ;
  if (i > 0)
    {
      full_path_name[i] = '\0';
      fname = &full_path_name[i+1];
      path = full_path_name;
    }
  else if (i == 0)  /* slash only at i == 0 */
    {
      fname = &full_path_name[i+1];
      path = "/";
    }
  else            /* i == -1 no slash at all */
    {
      path = ".";
      fname = full_path_name;
    }
if ((dirp = opendir(path)) == NULL)
{
  sprintf(error_msg,"\nCan't open directory %s.\n",
	  path);
  fprintf(stderr,"%s", error_msg);
  return PUNT;
}
while ((dp = readdir(dirp)) != NULL)
{
  if (strcmp(dp->d_name, fname) == 0)
    {
      found = YES;
      break;
    }
}
closedir(dirp);
if (i > 0)
     full_path_name[i] = '/';
     return found;
     }
/**************     LOCKING   ***************************/
char tmp_file_name[FNAME + 1] = "";
static FILE * fp_tmp = NULL;
static char failed_tmp_file_name[FNAME + 1] = "";
#define WAIT 1
#define TRIES 2
/**********************************************************
 *   This function returns a file pointer to an open temporary
 *   file = fname+"T".  If the file already exists, it waits
 *   until it doesn't.
 **********************************************************/
FILE *
lock_to_tmp(char * fname)
{
  int count = 0;
  if (tmp_file_name[0] != '\0')
    {
      sprintf(error_msg, "\nA lock is already implemented on %s.\n",
	      tmp_file_name);
      return NULL;
    }
  sprintf(tmp_file_name, "%sT", fname);
  while (exists(tmp_file_name) && ++count <= TRIES)
    sleep(WAIT);
  if (exists(tmp_file_name))
    {
      sprintf(error_msg,"\nlock_to_tmp: Temporary file %s is permanent for %d seconds.\nGiving up.\n", 
	      tmp_file_name, count * WAIT);
      strcpy(failed_tmp_file_name, tmp_file_name);
      tmp_file_name[0] = '\0';
      return NULL;
    }
if ((fp_tmp = fopen(tmp_file_name,"wr")) == NULL)
{
  sprintf(error_msg, "\nUnable to open temporary file %s.\n",
	  tmp_file_name);
  tmp_file_name[0] = '\0';
  return NULL;
}
return fp_tmp;
}
/**********************************************************
 *   Expects there to be a temporary file name and fp_tmp in 
 *   static storage.  It removes the file and blanks the storage.
 *   If force == YES, it deletes the lock file no matter what.
 ***********************************************************/
OKorNOT
unlock_to_tmp(YESorNO force)
{
  char command[FNAME + 80];
  if (fp_tmp == NULL || tmp_file_name[0] == '\0')
    {
      if (force == NO)
	return NOT_OK;
      if (failed_tmp_file_name[0] == '\0')
	return NOT_OK;
      sprintf(command, "rm %s", failed_tmp_file_name);
    }			
  else
    {
      fclose(fp_tmp);
      sprintf(command, "rm %s", tmp_file_name);
    }
system(command);
fp_tmp = NULL;
tmp_file_name[0] = '\0';
return OK;
}
