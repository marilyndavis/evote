/* $Id: whotest.c,v 1.4 2003/01/15 18:54:12 marilyndavis Exp $ */ 

#include "../demo/eVote.h"
int time_out = 60;
/*  Tests the who file.  Start The Clerk first.   */

#define STATIC_GROUP 400   /* Generates this many users and leaves them
			      in the list */
#define USERS 300        /*  Generates this many more -- some will
			     be dropped */
#define MAX_USERS 100    /*  All but this many will be dropped as
			     we go along */
#define NAME_SPACE 100
struct name_def
{
  char name[NAME_SPACE + 1];
  int num;
}list[MAX_USERS];


int main(int argc,char * argv[])
{
  RTYPE cc;
  char* pwho;
  unsigned long who_id;
  char * who_is(unsigned long);
  char *gen_name();
  int i, j;
  
  /**** COMMAND LINE ****/
  
  start_up(NO);
  srand(20);
  strcpy(list[0].name, "eVote@netcom.com");
  
  if((cc = who_drop(10, NO)) != FAILURE)
    printf("\n says it dropped when it didn't.");
  
  
  printf("\n %lu is %s.", who_id = who_num(list[0].name, NO), list[0].name);
  printf("\n %lu is %s.", who_id = who_num(list[0].name, YES), list[0].name);
  printf("\n %lu is %s.", (who_id = who_num(list[0].name, NO)), list[0].name);
  if(strcmp(list[0].name, (pwho = who_is(who_id))) != 0)
    printf("\nWho_is got %s for %s, id = %ld.", pwho, list[0].name, who_id);
  
  strcpy(list[0].name, "chdavis");
  printf("\n %lu is %s.", who_num(list[0].name, NO), list[0].name);
  printf("\n %lu is %s.", who_num(list[0].name, YES), list[0].name);
  printf("\n %lu is %s.", who_id = who_num(list[0].name, NO), list[0].name);
  if(strcmp(list[0].name, (pwho = who_is(who_id))) != 0)
    printf("\nWho_is got %s for %s, id = %ld.", pwho, list[0].name, who_id);
  
  strcpy(list[0].name, "chdavis@aimnet.com");
  printf("\n %lu is %s.", who_num(list[0].name, NO), list[0].name);
  printf("\n %lu is %s.", who_num(list[0].name, YES), list[0].name);
  printf("\n %lu is %s.", who_id = who_num(list[0].name, NO), list[0].name);
  if(strcmp(list[0].name, (pwho = who_is(who_id))) != 0)
    printf("\nWho_is got %s for %s, id = %ld.", pwho, list[0].name, who_id);
  
  strcpy(list[0].name, "chdavis@a.com");
  printf("\n %lu is %s.", who_num(list[0].name, NO), list[0].name);
  printf("\n %lu is %s.", who_num(list[0].name, YES), list[0].name);
  printf("\n %lu is %s.", who_id = who_num(list[0].name, NO), list[0].name);
  if(strcmp(list[0].name, (pwho = who_is(who_id))) != 0)
    printf("\nWho_is got %s for %s, id = %ld.", pwho, list[0].name, who_id);
  strcpy(list[0].name, "chdavis");
  printf("\n %lu is %s.", who_num(list[0].name, NO), list[0].name);
  printf("\n %lu is %s.", who_num(list[0].name, YES), list[0].name);
  printf("\n %lu is %s.", who_id = who_num(list[0].name, NO), list[0].name);
  if(strcmp(list[0].name, (pwho = who_is(who_id))) != 0)
    printf("\nWho_is got %s for %s, id = %ld.", pwho, list[0].name, who_id);
  
  strcpy(list[0].name, "chdavis@aimnet.com");
  printf("\n %lu is %s.", who_num(list[0].name, NO), list[0].name);
  printf("\n %lu is %s.", who_num(list[0].name, YES), list[0].name);
  printf("\n %lu is %s.", who_id = who_num(list[0].name, NO), list[0].name);
  if(strcmp(list[0].name, (pwho = who_is(who_id))) != 0)
    printf("\nWho_is got %s for %s, id = %ld.", pwho, list[0].name, who_id);
  
  strcpy(list[0].name, "chdavis@a.com");
  printf("\n %lu is %s.", who_num(list[0].name, NO), list[0].name);
  printf("\n %lu is %s.", who_num(list[0].name, YES), list[0].name);
  printf("\n %lu is %s.", who_id = who_num(list[0].name, NO), list[0].name);
  if(strcmp(list[0].name, (pwho = who_is(who_id))) != 0)
    printf("\nWho_is got %s for %s, id = %ld.", pwho, list[0].name, who_id);
  
  strcpy(list[0].name, "cdavis@aimnet.com");
  printf("\n %lu is %s.", who_num(list[0].name, NO), list[0].name);
  printf("\n %lu is %s.", who_num(list[0].name, YES), list[0].name);
  printf("\n %lu is %s.", who_id = who_num(list[0].name, NO), list[0].name);
  if(strcmp(list[0].name, (pwho = who_is(who_id))) != 0)
    printf("\nWho_is got %s for %s, id = %ld.", pwho, list[0].name, who_id);
  
  cc = who_drop(who_num(list[0].name, NO), NO);
  if(cc == FAILURE)
    printf("\nCouldn't drop one");
  printf("\n %lu is %s.", who_num(list[0].name, NO), list[0].name);
  printf("\n %lu is %s.", who_num(list[0].name, YES), list[0].name);
  printf("\n %lu is %s.", who_id = who_num(list[0].name, NO), list[0].name);
  if(strcmp(list[0].name, (pwho = who_is(who_id))) != 0)
    printf("\nWho_is got %s for %s, id = %ld.", pwho, list[0].name, who_id);
  
  strcpy(list[0].name, "cdavis@aimnet.com");
  printf("\n %lu is %s.", who_num(list[0].name, NO), list[0].name);
  strcpy(list[0].name, "chdavis@aimnet.com");
  printf("\n %lu is %s.", who_num(list[0].name, NO), list[0].name);
  strcpy(list[0].name, "eVote@netcom.com");
  printf("\n %lu is %s.", who_num(list[0].name, NO), list[0].name);
  
  printf("\n who is %d?  %s",3,who_is(3));
  printf("\n who is %d?  %s",14,who_is(14));
  printf("\n who is %d?  %s",1,who_is(1));
  
  for(i = 0; i < STATIC_GROUP; i++)
    {
      strcpy(list[0].name, gen_name());
      printf("\n %lu is %s.", who_num(list[0].name, YES), list[0].name);
    }
  
  for(i = 0; i < MAX_USERS; i++)
    list[i].num = -1;
  
  for(i = 0; i < USERS; i++)
    {
      j = rand()%100;
      if(list[j].num > -1)
	{
	  if(strcmp(list[j].name, (pwho = who_is(list[j].num))) != 0)
	    {
	      printf("\nWho_is got %s for %s, id = %d.",
		     pwho, list[j].name, list[j].num);
	      exit(1);
	    }
	  if(list[j].num != (who_id = who_num(list[j].name, NO)))
	    {
	      printf("\nWho_num thinks %s is %ld but I think it's %d",
		     list[j].name, who_id, list[j].num);
	      exit(1);
	    }
	  if((cc = who_drop(who_id,NO)) == FAILURE)
	    {
	      printf("\nCouldn't drop %ld, %s.", who_id, list[j].name);
	      exit(1);
	    }
	  printf("\n%ld, %s was dropped.", who_id, list[j].name);
	}
      
      strcpy(list[j].name,gen_name());
      list[j].num = who_num(list[j].name, YES);
      printf("\n %d is %s.", list[j].num, list[j].name);
      fflush(stdout);
    }
  
  for(i = 0; i < MAX_USERS; i++)
    {
      if(list[i].num == -1)
	continue;
      if((who_id = who_num(list[i].name, NO)) != list[i].num)
	{
	  printf("\nWrong.  list says %s is %d.  Wholist says %d.\n",
		 list[i].name, list[i].num, cc);
	}
    }
  
  exit(0);
}

char * gen_name()
{
  
  static char name[200];
  int len, i;
  int times;
  
  len = (rand() % 15) + 1;
  
  for (i = 0; i < len; i++)
    {
      name[i] = (rand() % 75) + 48;
      switch(name[i])
	{
	case '@':
	case '\\':
	case '"':
	  i--;
	  break;
	default:
	  break;
	}
    }
  name[i++] = '@';
  
  times = 0;
  do
    {
      len = rand()%15 + 1 + i;
      for( ; i < len ; i++)
	{
	  name[i] = (rand() % 75) + 48;
	  switch(name[i])
	    {
	    case '@':
	    case '\\':
	    case '"':
	      i--;
	      break;
	    default:
	      break;
	    }
	}
      name[i++] = '.';
    }
  while(rand()%2 == 0 && ++times < 3);
  
  name[i++] = '\0';
  
  switch(rand()%10)
    {
    case 0:
      strcat(name,"net");
      break;
    case 1:
    case 2:
    case 3:
      strcat(name,"edu");
      break;
    case 4:
      strcat(name, "gov");
      break;
    default:
      strcat(name,"com");
      break;
    }
  return name;
}



