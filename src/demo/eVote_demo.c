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

/* $Id: eVote_demo.c,v 1.3 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/**********************************************************
 *  eVote_demo.c   - main for the demo.
 ***********************************************************
 *  This file needs to be torn apart and the functionality
 *  needs to be embedded into your application
 *  code.  Read the comments for help.
 *    =====================================
 *   |                                     |
 *   |   NOTE:  Set your tab width to 2    |
 *   |          to read this code.         |
 *   |                                     |    
 *    =====================================
 *********************************************************
 **********************************************************/
#include <sys/types.h>
#include <unistd.h>
#include "eVote.h"
int
main(int argc, char* argv[])
{
  /* local functions */
  char * contents(char *input);
  OKorNOT eVote_add();
  void eVote_drop(char* input);
  /* local variables */
  int i;
  YESorNO did_add = NO;
  short drop_days = 0;
  YESorNO get_input = YES;
  YESorNO in_a_conf;
  char *input;
  ITEM_INFO item_copy;
  int item_num = 0;
  char *next_conf;
  short prompt_no = 0;
  YESorNO something_happened;
  char this_conf[CONFLEN + 1];
  YESorNO to_eVote;
  OKorNOT ret;
  (void)do_blurb(NULL);
  printf("\n");
  start_up(NO);
  hello_Clerk();
  get_args(argc, argv, this_conf, &drop_days);
  /*  sleep(2); */
  (void)do_blurb(NULL);
  input = GetArg("\nPress <ENTER> to continue ... ");
  if (*input != '\0')
    get_input = NO;
  if (this_conf[0] != '\0')
    {
      if (does_conf_exist(this_conf) == NO)
	{
	  eVote_conf(this_conf, drop_days);
	}
      if ((ret = i_am_entering(this_conf, NO)) == OK)
	{
	  printf("\nYou are in the %s conference.", current_conf);  
	  in_a_conf = YES;
	}
    }
  if (this_conf[0] == '\0' || ret == NOT_OK)
    {
      in_a_conf = NO;
    }
  while (1)
    {
      if (get_input == YES)
	{
	  if (in_a_conf == YES)
	    {
	      if (++prompt_no % 2)
		input = GetArg(BIG_PROMPT);
	      else
		input = GetArg(BIG_PROMPT2);
	    }
	  else
	    input = GetArg("\n(l)ist, (g)o, (q)uit, e(x)planation, (t)eacher, or ? \nConf?  ");
	}
      get_input = YES;
      something_happened = NO;
      switch (*input)
	{
	case 'a':
	case 'A':
	  if (in_a_conf == NO)
	    {
	      printf("\nYou must be in a conference before you can add an item.");
	      break;
	    }
	  did_add = YES;
	  eVote_add();
	  break;
	case 'c':
	case 'C':
	  if (in_a_conf == NO)
	    {
	      printf("\nYou must be in a conference before you can see a contents screen.");
	      break;
	    }
	  if ((input = contents(&input[1])) != NULL)
	    get_input = NO;
	  break;
	case 'd':
	case 'D':
	  if (in_a_conf == NO)
	    {
	      printf("\nYou must be in a conference, and you must add items ");
	      printf("\nto that conference before you can drop items.");
	      break;
	    }
	  eVote_drop(&input[1]);
	  break;
	case 'g':
	case 'G':
	  something_happened = YES;
	  if ((next_conf = collect_conf_name(input)) == NULL)
	    break;
	  if (in_a_conf == YES)
	    i_am_leaving();
	  {  /* scope delimiter for keep_conf */
	    /* next_conf points into the same input buffer
	       as all other questions are answered into.
	       We save it in keep_conf before we ask another
	       question */
	    char keep_conf[CONFLEN + 1];
	    strcpy(keep_conf, next_conf);
	    if (does_conf_exist(next_conf) == NO)
	      {
		printf("\nThere is no %s conference.", next_conf);
		input = GetArg("  Do you want to start one? ");
		if (*(input) != 'y' && *input != 'Y')
		  {
		    if (in_a_conf == YES)
		      i_am_entering(this_conf, NO);
		    break;
		  }
		if ((drop_days = eVote_asknum(DROP_DAYS, 0, 9999, NO)) < 0)
		  {
		    printf("\nPlease see your eVote User's Manual or contact Frontier Systems");
		    printf("\nfor an explanation of %cdrop_days%c before you start a new conference.\n\n", 34, 34);
		    if (in_a_conf == YES)
		      i_am_entering(this_conf, NO);
		    break;
		  }
		eVote_conf(keep_conf, drop_days);
	      }
	    strcpy(this_conf, keep_conf);
	  }
	  if ((ret = i_am_entering(this_conf, NO)) == OK)
	    in_a_conf = YES;
	  else
	    in_a_conf = NO;
	  break;
	case 'h':
	case 'H':
	  printf("\n\nThis eVote demo has three help features:");
	  printf("\n\n   'x'  -  to eXplain a word: 'x VTR' explains VTR.");
	  printf("\n   '?'  -  tells you your current choices.");
	  printf("\n   't'  -  accesses the teacher.\n");
	  break;
	case 'l':
	case 'L':
	  list_confs();
	  something_happened = YES;
	  break;
	case 'q':
	case 'Q':
	  something_happened = YES;
	  if (did_add == NO)
	    {
	      printf("\nYou haven't added a vote item. You haven't experienced the");
	      printf("\nreal power of eVote until you have invented a vote question.\n");
	    }
	  printf("\nDo you really want to quit eVote? ");
	  input = GetArg("Press 'q' once more if you do. ");
	  if (*input != 'q' && *input != 'Q' && *input != 'y' 
	      && *input != 'Y')
	    {
	      get_input = NO;
	      break;
	    }
	  if (in_a_conf == YES)
	    i_am_leaving(); 
	  exit(0);
	  break;
	case 's':
	case 'S':
	  toggle_blurbs();
	  something_happened = YES;
	  break;
	case 't':
	case 'T':
	  strcpy(input,"x teacher");
	case 'x':
	case 'X':
	  explain(input);
	  something_happened = YES;
	  break;
	case '?':
	  if (in_a_conf == YES)
	    (void)do_blurb(NULL);
	  else
	    do_blurbette();
	  something_happened = YES;
	  break;
	case 'v':
	case 'V':
	  if (in_a_conf == NO)
	    {
	      printf("\nYou must be in a conference and you must read an item before");
	      printf("\nyou can vote or access any eVote features.");
	      break;
	    }
	  if (item_num == 0)
	    {
	      printf("\nPlease read an item first by entering the item #.\n");
	      break;
	    }
	  if ((input = process_eVote(&item_copy, input, NO)) != NULL)
	    get_input = NO;
	  break;
	case '#':
	  printf("\n  # to read\n");
          printf("\n    means enter:\n");
	  printf("\n  3   to read item number 3.");
	  printf("\n 17   to read item number 17.");
	  printf("\n      etc.\n");
	  break;
	case '\0':
	  printf("\n\nWhen you don't know what to do, enter '?'\n\n");
	  break;
	default:
	  if (in_a_conf == NO)
	    {
	      printf("\nYou must be in a conference before you can do much of anything.");
	      break;
	    }
	  i = 0;
	  to_eVote = NO;
	  while (input[++i] != '\0')
            {
	      if (input[i] >= '0' && input[i] <= '9' )
		continue;
	      if (input[i] == ' ')
		{
		  input[i] = '\0';
		  continue;
		}
	      if (input[i] == 'v' || input[i] == 'V')
		{
		  input[i] = '\0';
		  to_eVote = YES;
		  break;
		}
	    }
	  if ((item_num = atoi(input)) == 0)
	    break;
	  /* If your item_num is not the dropping_id,
	     you want to translate it to be the
	     dropping_id for the item.  
	     See make_dropping() in ./eVote_io.c */
	  if (item_num > *p_no_items)
	    {
	      printf("\nNo item #%d yet.\n", item_num);
	      item_num = 0;
	      break;
	    }
	  /* Here we copy the item rather than use the item
	     in shared memory.  It could happen that someone
	     else drops an item ahead of this item, or they
	     could drop this very item, and the item_info
	     array would change under us. */
	  item_copy = item_info[item_num];
	  /*** The application code displays the text for
	       item number item_num ***************/
	  if (to_eVote == NO ||
	     have_i_read(item_num) == NO)
	    {
	      char *new_input;
	      if ((new_input = do_blurb(&item_copy)) != NULL)
		{
		  input = new_input;
		  get_input = NO;
		}
	      i_just_read(&item_copy);
	    }
	  if (to_eVote == YES)
	    if ((input = process_eVote(&item_copy, 
				      &input[i+1], YES)) != NULL)
	      get_input = NO;
	  break;
	}
      if (in_a_conf == NO && something_happened == NO)
	{
	  printf("\n\nEnter 'l' to see a list of the available conferences.\n");
	  printf("\nOr enter '?' at any time to see your current choices.\n");
	}
    }
}					
/* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * 
 *  CONTENTS SCREEN 
 *  The next two functions control the listing of the items
 *  on the contents screen.
 * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */
/**********************************************************
 *    contents() is called when an 'c' is pressed.  It lists the
 *    items currently in the conference.  If the user presses 
 *    anything meaningful, it gets passed back to the caller.
 *    Otherwise, it returns a NULL.
 *    If there is a number in *input, it starts at that item
 *    number.
 *********************************************/
#define LINES_PER 23	
int line;	
char*
contents(char *inp)
{
  int get_msg(YESorNO wait);
  char* get_name(unsigned long uid);
  short get_my_sum(short dropping_id);
  unsigned long get_voters(short dropping_id);
  char* get_stats(short dropping_id);
  char *new_screen();
  char *cc;
  short ccc;
  int i = 1;
  VOTE_TYPE last_type = ZIP;
  if (*inp != '\0')
    {
      if ((ccc = (short)atoi(inp)) <= *p_no_items
	 && ccc > 0)
	i = ccc;
    }
  get_msg(NO);  /* The input message queue is flushed so that
		   if anyone else has changed the item_info[]
		   our item_stat[] and dropping_id's match.  */
  printf("\n---- %s Conference ----", current_conf);
  line = 1;
  if (*p_no_items == 0)
    printf("\n\nThere are no items in the %s conference.\n",
	   current_conf);
  for ( ; i <= *p_no_items; i++)
    {
				/* counting the data line way up here!*/
      if (++line > LINES_PER )  
	{
	  if ((cc = new_screen()) != NULL)
	    return cc;
	  line++;
	}
      if (item_info[i].eVote.type != last_type || line == 2) 
	/* 2 means new screen, 1 for conf, 1 for data */
	{
	  last_type = item_info[i].eVote.type;
	  /* one line for heading, one for dotted line */
	  if ((line += 2) > LINES_PER )
	    {
	      if ((cc = new_screen()) != NULL)
		return cc;
	      line += 3; /* data, head & dot */
	    }
	  switch (item_info[i].eVote.type)
	    {
	    case PLAIN:
	      printf("%s", PLAIN_HEAD);
	      break;
	    case TALLIEDY:
	      printf("%s", TALLIED_YHEAD);
	      break;
	    case TALLIEDN:
	      printf("%s", TALLIED_NHEAD);
	      break;
	    case GROUPEDn:
	    case GROUPEDN:
	    case GROUPEDY:
	      /* count instruction line */
	      if (++line > LINES_PER)
		{
		  if ((cc = new_screen()) != NULL)
		    return cc;
		  line += 4;
		}
	      if (line == 5  /* starting a new page */
		  && item_info[i].eVote.no_in_group !=
		  item_info[i].eVote.more_to_come + 1)
		{  
		  short boiler_lines = 5;
		  if (i + item_info[i].eVote.more_to_come  >=
		      *p_no_items)
		    {
		      /* group contains last item in conf - add a
			 boiler plate line for the end of conf 
			 message */
		      boiler_lines += 2;
		    }
		  /* back up to get as complete a group
		     as possible */
		  if (item_info[i].eVote.no_in_group 
		      <= LINES_PER - boiler_lines) 
		    { /* the group will fit on the screen */
		      i -= (item_info[i].eVote.no_in_group
			    -item_info[i].eVote.more_to_come -1);
		    }
		  else
		    {
		      /* the group will not fit on the screen */
		      if (i + item_info[i].eVote.more_to_come + 1
			  - (LINES_PER - boiler_lines) < i)
			i += (item_info[i].eVote.more_to_come + 1
			      - (LINES_PER - boiler_lines));
		    }
		}
	      ccc = get_voters(i);
	      if (item_info[i].eVote.type == GROUPEDY)
		{
		  printf("\n   \\\\ Vote YES on %d of the next %d items: %d voter%c //",
			 item_info[i].eVote.sum_limit, 
			 item_info[i].eVote.no_in_group,
			 ccc, ((ccc == 1) ? ' ' : 's'));
		  if (item_info[i].eVote.more_to_come + 1 !=
		      item_info[i].eVote.no_in_group)
		    printf(" ... continued ");
		  printf("%s", GROUPED_YHEAD);
		}
	      else
		{
		  printf("\n   \\\\ Distribute %d votes over the next %d items: %d voter%c //",
			 item_info[i].eVote.sum_limit, 
			 item_info[i].eVote.no_in_group,
			 ccc, ((ccc == 1) ? ' ' : 's'));
		  if (item_info[i].eVote.more_to_come + 1 !=
		      item_info[i].eVote.no_in_group)
		    printf(" ... continued ");
		  printf("%s", GROUPED_NHEAD);
		}
	      break;
	    default:
	      /* impossible */
	      break;
	    }
	  printf("%s", DOTTED_LINE);
	}
      printf("\n%4d     0  %s    %-36s  %s", i, get_stats(i),
	     print_title(i), 
	     get_name(item_info[i].eVote.author));
      if (item_info[i].eVote.type >= GROUPED
	  && item_info[i].eVote.more_to_come == 0)
	{
	  if (line++ < LINES_PER)
	    {
	      printf("\n >>> Your sum = %4d of %d allowed on the group above: %d items.",
		     get_my_sum(i), item_info[i].eVote.sum_limit,
		     item_info[i].eVote.no_in_group);
	    }
	  else
	    i--;
	  if (line++ < LINES_PER)
	    printf("%s", DOTTED_LINE);
	  last_type = ZIP;
	}
      /* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * 
       *
       * That call to get_stats() is important to you. 
       * char* get_stats(short i); returns a pointer to a string,
       * 12 chars long (+ '\0') that contains the stats for the
       * item with dropping_id == i.  You need to embed this call
       * into your display for each item.
       *
       * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */
    }
  printf("\n* - * - * End of %s Conference * - * - *", current_conf);
  return NULL;
}
/***************************************************
 * This is called by contents() above when it is time 
 * to wait for a key stroke before continuing.
 *********************************************************/
char *
new_screen(void)
{
  char * input;
  input = GetArg("\nPress < ENTER > to continue ...  or any other command: ");
  switch (*input)
    {
    case '\0':
      break;
    default:
      return input;
      break;
    }
  printf("\n---- %s CONFERENCE ----", current_conf);
  line = 1;
  return NULL;
}
/* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * 
 *  ADDING ITEMS 
 *  The next three functions are for adding items.
 * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */
/****************************************************************
 *  Called when the user asks to add a new item.  It starts the
 *  dialog to determine the item's vote characteristics.  If
 *  the item is grouped, the processing is carefully handled by 
 *  set_up_item_group() so that the whole group goes into the
 *  application database in one lock session.
 **********************************************************/
OKorNOT
eVote_add(void)
{
  OKorNOT eVote_new_items(ITEM_INFO* p_eVote_item, int no_to_make);
  /* eVote_new_items() is in Clerklib.a */
  OKorNOT collect_eVote_details(YOUR_ITEM ** pp_your_item, 
				ITEM_INFO **pp_eVote_item, 
				YESorNO checking,
				short lines_printed);
  /* collect_eVote_details() is in eVoteui/item.c */
  OKorNOT set_up_item_group (YOUR_ITEM * pp_your_item,
			     ITEM_INFO * p_eVote_item);
  /* set_up_item_group() is in eVoteui/item.c  */
  short help_her(MODE mode, YESorNO checking); /* in item.c */
  OKorNOT cc;
  ITEM_INFO eVote_item;
  YOUR_ITEM your_item;
  ITEM_INFO *p_eVote_item;
  YOUR_ITEM *p_your_item;
  p_eVote_item = &eVote_item;
  p_your_item = &your_item;
  do
    {
      if (collect_eVote_details(&p_your_item, 
			       &p_eVote_item, NO, 0) != OK)
	{
	  return NOT_OK;
	}
      if (eVote_item.eVote.type >= GROUPED)
	{
	  /* set_up_item_group() calls add_group() below */
	  if ((cc = set_up_item_group(&your_item, &eVote_item))
	     == UNDECIDED)
	    {
	      help_her(GET, YES);
	      continue;
	    }
	  return cc;
	}
      else 
	{
	  /* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * 
	   *   
	   *  Here, lock up your database and add the item.
	   *
	   * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */
	  cc = eVote_new_items(&eVote_item, 1);
	  /* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * 
	   *   
	   *  Here, unlock your database.
	   *
	   * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */
	  return cc;
	}
    }
  while (1);
  return UNDECIDED;
}
/*******************************************************
 *   Actually enters the group's data into both data bases.
 *******************************************************/
OKorNOT
add_group(YOUR_ITEM * your_list, ITEM_INFO * item_list)
{
  OKorNOT all_ok = OK;
  short i;
  OKorNOT eVote_new_items(ITEM_INFO *p_eVote_item, int no_to_make);

  /* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * 
   *Now, all the info about the group has been gathered and
   *checked and we are ready to enter the info into both the
   *application database and eVote's database.
   *   
   *The application database should be locked during the
   *execution of this loop.  This insures that the items
   *appear consecutively in the conference and that eVote's
   *notions of the order of things matches the application's
   *notions.
   * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */

  /* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * 
   *locknf(io,'n');     sets application lock
   * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */

  for (i = 0 ; i < item_list[0].eVote.no_in_group; i++)
    {
      /* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * 
       * Enter the new item into the application database
       * This bit of code is copied from Notesfile code.
       *
       * if ((retcode = putitem (io, &your_list[i].where, 
       *                         print_title[i], stat, 
       *                         item_list[i].dropping_id, 
       *                         &your_list[i].auth, NOPOLICY, 
       *			 NOLOCKIT, COPYID, System, 
       *                         1)) == 0)  // successful *
       *    continue;
       *
       * // if we get here, there was trouble somewhere *
       *   all_ok = NOT_OK;
       *
       *       // drop the items that you already added that belong
       *          in this group and consider the whole group a failure.
       *
       *   break;     
       *                  
       * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */
    }
  if (all_ok == OK)
    {
      all_ok = eVote_new_items(item_list, item_list[0].eVote.no_in_group);
      /* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * 
       *
       *   if (all_ok != OK)
       *      drop all the items you added to your database.
       *
       *   unlocknf(io,'n');  releases application lock
       * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */
    }
  return all_ok;
}
/******************************************************
 *    This function collects the information about a new
 *    item that is kept by the application database:  the
 *    item id, the text, and the title.
 *    This is one of a group of related functions, do_xxx().
 *    The other do_xxx() functions are defined in eVoteqry.c
 *    along with the driving function, collect_eVote_details().
 *    There is one do_xxx() function for each "field" in the
 *    data being collected.  
 *	 These do_xxx() functions (including this one) return:
 *			OK if the question was appropriately answered. 
 *			UNDECIDED if the user was very confused. 
 *			NOT_OK if there was a system resource problem.
 *			STOP if the user indicated she wanted to quit.
 *    The family of functions can be called in four modes:
 *      GET  -  to get the orginal values.
 *      SHOW -  shows the value collected in GET mode.
 *      CHANGE  looks to see if changeno == *pfield.
 *              If so, the user indicated that she wants to
 *              change this field.
 *      FALL    Once CHANGE has been satisfied, *pmode
 *              changes to FALL and processing falls
 *              through the rest of the fields.
 *    Although the text for the item is collected in this 
 *    function in GET mode, the user is not invited to 
 *    change the text when the voting parameters are 
 *    verified.  Similarly, the item id is assigned in 
 *    GET mode but it is never displayed or changed.
 *    These things are alterable, if you wish.
 **********************************************************/
extern struct questions_def *ask;   /* in input.c */
OKorNOT
do_yours(int *pfield, YOUR_ITEM *p_your_item,
		 ITEM_INFO *p_eVote_item, 
		 MODE * pmode, int changeno)
{
  int len = -1;
  int mistakes = 0;
  char *answer;
  ++(*pfield);			/* Tracks the field number */  
  while (mistakes < MISTAKES)
    {
      switch (*pmode)
	{
	case FALL:
	  return OK;
	case CHANGE:
	  if (changeno != *pfield)
	    return OK;
	  /* else fall into GET */
	  *pmode = FALL;  /*  so that it will fall 
			      through the other fields*/
	case GET:
	  /* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * 
	   *
	   *   Assign the item ids.  Be sure they are in both your 
	   *   structure and eVote's structure.
	   *
	   *   For the demo, uniqid and dropping_ids are set to 0.
	   *   This prods eVote_clerk to assign values.
	   *   If you put in values > 0, eVote_clerk will
	   *   respect them. 
	   *
	   * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */
	  p_eVote_item->static_id.network_id = MY_HOST_ID;
	  p_eVote_item->static_id.local_id = 0l;
	  p_eVote_item->dropping_id = 0;
	  /* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * 
	   *
	   *   Here you want to collect the text for the item and
	   *   do whatever else you need to do for a new item -- except
	   *   collect the title.  That happens below.
	   *
	   * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */
	  do
	    {	
	      len = strlen(answer = GetArg(ask[TITLER].qprompt));
	      switch (*answer)
		{
		case '?':
		  if (strcmp(answer,"?") == 0)
		    {
		      len = TITLEN + 1;  /* flag help */
		    }
		case 'q':
		  if (strcmp(answer,"quit") == 0 
		      || strcmp(answer,"q") == 0)
		    return STOP;
		case 'Q':
		  if (strcmp(answer,"Quit") == 0 
		      || strcmp(answer,"QUIT") == 0
		      || strcmp(answer,"Q") == 0)
		    return STOP;
		default:
		  if (len > TITLEN && mistakes + 1 < MISTAKES)
		    printf("Your title is %d characters long.  You only get %d characters.  \nPlease try again.", len, TITLEN);
		  else if (len <= TITLEN)
		    {
		      strcpy(p_eVote_item->eVote.title, answer);
      /* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * 
       *
       *              strcpy(p_your_item->title, answer);
       *
       * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */
		      return OK;
		    }
		  break;
		}
	    }
	  while (++mistakes < MISTAKES && len > TITLEN);
	  break;
	case SHOW:
	  printf("\n   %d: %s %s", *pfield, ask[TITLER].qprompt,
		 p_eVote_item->eVote.title);
	  return OK;
	  break;
	}
    }
  return UNDECIDED;
}		
/* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * 
 *   DROPPING ITEMS
 *   The next function drops items.
 * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */
/*********************************************************
 *  drops an item or a range of new items from the 
 *  current conference.
 ***********************************************************/
void
eVote_drop(char *input)
{
  short process_drops(short *starting_item, 
		      short *ending_item, YESorNO testing);
  short dash = 0;
  short dropped;
  short i = 1, start, end;
  if (input[0] == '\0')
    {
      input = GetArg("\nPlease enter the item number or a range separated by '-': ");
      i = 0;
    }
  for (; input[i] != '\0'; i++)
    {
      switch (input[i])
	{
	case '-':
	  input[i] = '\0';
	  dash = i;
	  break;
	case 'q':
	case 'Q':
	  return;
	default:
	  if ((input[i] >= '0' && input[i] <= '9') 
	      || input[i] == ' ')
	    break;
	  return;
	}
    }
  start = atoi(&input[0]);
  end = start;
  if (dash > 0)
    end = atoi(&input[dash + 1]);
  /* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * 
   *   
   *  Here, lock up your database but wait to drop the items
   *  until after eVote has dropped them.
   *
   * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */
  dropped = process_drops(&start, &end, NO);
  /* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * 
   *  We sent the addresses of start and end into eVote because
   *  eVote will adjust them to be sure that groups are dropped
   *  as wholes.  
   *
   *  if (dropped == end - start + 1)
   *    {
   *      you drop the items from your database.
   *    }
   *  Actually, if it failed somehow, dropped == 0. 
   *  
   *  Now, unlock your database.
   *
   * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */
  printf("\nDropped %d items.\n", dropped);
  return;
}		
