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

/* $Id: resend_test.c,v 1.4 2003/01/15 18:54:12 marilyndavis Exp $ */ 
/* This little program is a fake "resend".  Resend is majordomo's
 * program for sending a message to a list.  This is for testing and
 * sends the stdin to stdout, and that is all.
 */

#include<stdio.h>
// #define TOSTDOUT 

int 
main(int argc, char *argv[], char *env[])
{
  int bytes = 0;
  int ch;
  int i;
  FILE * fp;
  
#ifdef TOSTDOUT
  fp = stdout;
#else
  fp = fopen("./tmp/r.out", "w");
#endif
  
  fprintf(fp,"\n ----------- \n resend called as: ");
  for(i = 0; i < argc; i++)
    fprintf(fp," %s",argv[i]);
  /*	fprintf(fp,"\nEnv list: ");
	i = -1;
	while(env[++i] != NULL)
	{
	fprintf(fp," %s",env[i]);
	} */
  fprintf(fp,"\n\nStdin says: \n");
  
  while((ch = getchar()) != EOF)
    {
      bytes++;
      fputc(ch,fp);
    }
  fflush(stdout);
#ifdef TOSTDOUT
  
#else
  fclose(fp);
#endif
  return 0;
}

