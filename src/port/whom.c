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


