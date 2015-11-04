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
