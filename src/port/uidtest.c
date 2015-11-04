/* $Id: uidtest.c,v 1.3 2003/01/15 18:54:12 marilyndavis Exp $ */ 
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <pwd.h>
unsigned long who_am_i();
int main(void)
{
	printf("%lu is who i am.\n", who_am_i());
	exit(0);
}

unsigned long who_am_i()
{
	struct passwd * pw;
	static unsigned long i_am = 0L;
	char *name;

	if(i_am > 0L)
		return i_am;
#ifdef EDEBUG
	if(edebug & STRESS) 
		{
			i_am = (unsigned long)getuid();
			return i_am;
		}
#endif
	printf("\ngetuid = %d, geteuid = %d.\n", getuid(), geteuid());
	name = (char*)getlogin();
	if(name == NULL || name[0] == '\0')
		name = (char*)cuserid(NULL);
	pw = getpwnam(name);
	i_am = (unsigned long)pw->pw_uid; 
	return i_am;
}
