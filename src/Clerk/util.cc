//
// eVote - Software for online consensus development.
// Copyright (C) 2015 Marilyn Davis <marilyn@deliberate.com>
//
// This file is part of eVote.
//
// eVote is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// eVote is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with eVote.  If not, see <http://www.gnu.org/licenses/>.
//

/* $Id: util.cc,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// util.cc -- file backup utilities and string utilities
/*********************************************************
 **********************************************************/
extern "C" {
#include <stdlib.h>
}
#include<sys/types.h>
#include<sys/stat.h>
#include <stdio.h>
#include <fstream.h>
#include <pwd.h>
#include "evotedef.h"
#include "inq.h"
GLOBAL_INCS
extern InQ inq;
OKorNOT remove_file(char* conf_name, char *ext);
// *********************
//  backup_file  backs up the .dat or .inf file as ~00conf.dat or ~00conf.inf
//    This happens whenever the file changes a lot - like resizing
//    the records or when the conference is adjourned.
//    If ~00conf.xxx exists, it'll make ~01conf.xxx.  After ~99conf.xxx
//    exists, it starts over with ~00conf.xxx.
#define MAX_BACKUPS 20
OKorNOT
backup_file(char *name, char *ext, YESorNO remove)
{
  char command[400];
  unsigned int len;
  char new_name[200];
  char new_prefix[4];
  int prefix_digit = -1;
  int perm = DATA_PERM;
  fstream new_file;
  char next_name[200];
  struct stat buf;
  if (strcmp(ext, INFO_EXT) == 0)
    perm = INFO_PERM;
  len = (unsigned int)strlen(output_dir);
  do  // open files until you find a filename that doesn't exist already
    {
      strcpy(new_name, output_dir);
      sprintf(new_prefix,"~%02d", ++prefix_digit % MAX_BACKUPS);
      strcat(new_name, new_prefix);
      strcat(new_name, name);
      if ((unsigned int)strlen(new_name) - len > MAX_FILE_NAME_LEN)
	new_name[len + MAX_FILE_NAME_LEN] = '\0';
      strcat(new_name, ext);
      // See if the file exists.
      new_file.open(new_name, ios::nocreate||ios::out, perm);
      if (!new_file)
	break;        // break here, no file by this name
      new_file.close();
    }  
  while (1);
  /* remove the next in the sequence in case we've cycled */
  sprintf(next_name, "%s~%02d%s%s", output_dir, 
	  ++prefix_digit % MAX_BACKUPS, name, ext);
  if (stat(next_name, &buf) == 0)
    {
/*      sprintf(command, "echo %s >> %s%s.old", next_name, name, ext);
      system(command);
      sprintf(command, "cat %s >> %s%s.old", next_name, name, ext);
      system(command); */
      sprintf(command, "rm %s ", next_name);
      system(command);
    }
  strcpy(command,"cp ");
  strcat(command, output_dir);
  strcat(command, name);
  strcat(command, ext);
  strcat(command," ");
  strcat(command, new_name);
  if (system(command) == -1)
    {
      char msg[80];
      clerklog << "Unable to copy files in backup_file().  \n   Perhaps there are too many processes running.";
      sprintf(msg, FNE_QUIT, QUIT, (unsigned long)0);
      inq.send_myself(msg, 1l);
      return NOT_OK;
    }
  if (remove == YES)
    remove_file(name, ext);
  return OK;
}
//  **************************************************************
//   makes sure that the name string ends with a '/'.
void
fix_slash(char *name)
{
  unsigned int i;
  i = (unsigned int)strlen(name);
  if (name[--i] != '/')
    {
      name[++i] = '/';
      name[++i] = '\0';
    }
}
//  **************************************************************
char *
get_name(unsigned long uid)
{
  static char the_name[25];
  struct passwd* pwd;
  if ((pwd = getpwuid(uid)) == NULL)
    {
      sprintf(the_name,"was %lu", uid);
    }
  else
    {
      strcpy(the_name, pwd->pw_name);
    }
  return the_name;
}
#include "../Clerklib/mtype.c"
// This code is shared with the application
// It's the definition of get_itype() &  get_rtype().
// I don't know how to push the makefile to compile it for this.
//  **********************************************************
//   removes the file.  Only call after backup_file has been called
OKorNOT
remove_file(char* conf_name, char *ext)
{
  char name[200];
  strcpy(name, "rm ");
  strcat(name, output_dir);
  strcat(name, conf_name);
  strcat(name, ext);
  if (system(name) == -1)
    {
      char msg[80];
      clerklog << "Unable to remove file in remove_file().  \n   Perhaps there are too many processes running.";
      sprintf(msg, FNE_QUIT, QUIT, (unsigned long)0);
      inq.send_myself(msg, 1l);
      return NOT_OK;
    }
  return OK;
}
//  **************************************************************
char *
uid_string(unsigned long uid)
{
  static char str[15];
  if (uid == 0)
    strcpy(str, "root");
  else
    sprintf(str,"%lu", uid);
  return str;
}
