/* $Id: who.c,v 1.3 2003/01/15 18:54:12 marilyndavis Exp $ */ 
#include<stdio.h>
#include<sys/types.h>
#include<pwd.h>
#include<unistd.h>
void main()
{
	unsigned int who_am_i();

	who_am_i();
}


unsigned int who_am_i()
{
/*	struct passwd * pw;
	pw = getpwnam((char*)logname());
	return (unsigned int)pw->pw_uid; */
  printf("\n getlogin says I am %u\n", (unsigned int)getlogin());
	printf("\n getuid says I am %u\n", (unsigned int)getuid());

}

	
