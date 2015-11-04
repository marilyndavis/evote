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

/* $Id: whom.c,v 1.4 2003/01/15 18:54:12 marilyndavis Exp $ */ 
#include<stdio.h>
#include<sys/types.h>
#include<pwd.h>
#include<unistd.h>
int main(void)
{
  unsigned int who_am_i();
  
  printf("I am %u.\n",who_am_i());
  return 0;
}


unsigned int who_am_i()
{
  struct passwd * pw;  
  /*this worked in interactive unix
    pw = getpwnam((char*)logname()); */
  
  /* Other stuff to test */
  /*	printf("\n cuserid says I am %s. \n", cuserid(NULL));*/
  printf("\n getuid says I am %u\n", (unsigned int)getuid());
  printf("\n getlogin says I am %s. \n", getlogin());
  
  /*  This seems to work for LINUX */
  pw = getpwnam((char*)getlogin());
  return (unsigned int)pw->pw_uid; 
}


