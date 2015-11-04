/* $Id: poetry.c,v 1.3 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/******************************************************
 *      poetry.c  - poems for the demo.
 *********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include "../eVote.h"
#include<unistd.h>
#define NO_POEMS 20
struct poem_def
{
  YESorNO more;
  char *key;
  char *screen[NO_LINES];
}poem[NO_POEMS]={
  /*0*/{YES ,"Another Chance",{
    "ANOTHER CHANCE",  
    "                ",
    "Once we had a notion",
    "to be the best of friends.",
    "My heart leans to make the means",
    "to justify the end.",
    " ",
    "There is no retribution",
    "when lovers miss their start.",
    "To lose so much, with one false touch",
    "is not fair to the heart.",
    " ",
    "I'm looking for another chance",
    "                another chance with you.",
    " ",
    "Once you had a notion, ",
    "that we could work it out.",
    "Now I see you look at me",
    "Your face is filled with doubt.",
    " ",
    " ",
    " ",
    " "
  }},
  /*0*/{YES,"Another Chance",{
    "Too enthusiastic,",
    "I put horses after carts.",
    "My feeble try, my reasons why",
    "Just left you in the dark.",
    " ",
    "I'm looking for another chance",
    "                another chance with you.",
    " ",
    "My heart fell wide open",
    "and rolled out on the floor.",
    "Seeing that, you took your hat",
    "and backed right out my door.",
    "",
    "It's hard to see where truth lies",
    "Once your heart's run dry.",
    "Meanings part when mind and heart",
    "don't see eye to eye.",
    " ",
    "I'm looking for another chance",
    "                another chance with you.",
    " ",
    " ",
    " "
  }},
  /*0*/{NO,"Another Chance",{
    " ",
    " ",
    "They say love is hopeless",
    "once it's gone amiss.",
    "It may be true, but me and you",
    "are worth just one more kiss.",
    "",
    "With a little patience",
    "your heart will come around.",
    "Lightening cracks and clouds roll back",
    "when love is lost and found.",
    "",
    "I'm looking for another chance",
    "                another chance with you.",
    " ",
    " ",
    " ",
    "Copyright 1992, by Marilyn Davis ",
    ". ",
    " ",
    " ",
    " ",
    " "
  }},
  /*0*/{YES ,"Fourth",{
    "  Fourth Grade - by Marilyn Davis",
    "",
    "	 No No No",
    "	 I have better things to do",
    "	 I'm not here to make your dreams come true",
    "	 I don't want to be what you want me to be",
    "	 No no I want to discover me",
    "",
    "	 You hand me world full of lovelessness",
    "	 And you want me carry on the same",
    "	 We're coming to the end of the world here",
    "	 Maybe we should do it a new way.",
    "",
    "	 Maybe the things you teach aren't relevant now",
    "	 Maybe your view is small",
    "	 We have to see beyond beyond your ways",
    "	 Or maybe there'll be no seeing at all.",
    "",
    "	 You're pushing a round thing into a square",
    "	 You don't want to see how I'm made.",
    "	 Maybe your own roundness aches",
    "	 since it was lost in fourth grade.",
    ""
  }},
  /*0*/{NO ,"Fourth",{
    "",
    "	 Your generation died in the wars",
    "	 Your generation brought the world near its end",
    "	 Your generation doesn't know how to live",
    "	 That's the result of your discipline.",
    "",
    "	 I trust creation to bring me my needs",
    "	 I trust my nature to build me",
    "	 I know that if I be what Nature dictates,",
    "	 She'll make the best use of me.",
    "",
    "  Copyright 1992, Marilyn Davis.	",
    "",
    ".",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    ""
  }},
  /*0*/{YES ,"Change",{
    "",
    "",
    "	CHANGE",
    "",
    "",
    "	You know there's something wrong,",
    "	And it's been wrong all along.",
    "",
    "	Gotta make a change",
    "	Gotta make a change",
    "",
    "	What they say is true,",
    "	It doesn't ring true for you.",
    "",
    "	Gotta make a change ",
    "	Gotta make a change",
    "",
    "	It's always been the same,",
    "	But we can't last this way.",
    "",
    "	Gotta make a change ",
    "	Gotta make a change",
    ""
  }},
  /*0*/{YES ,"Change",{
    "	A storm is coming up,",
    "	You know we've gotta stop.",
    "",
    "	Gotta make a change ",
    "	Gotta make a change",
    "",
    "	Old ways block the new.",
    "	Gotta find something to do.",
    "",
    "	Gotta make a change ",
    "	Gotta make a change",
    "",
    "	It's always been this way,",
    "	But we've been led astray.",
    "",
    "	Gotta make a change ",
    "	Gotta make a change",
    "",
    "	Connections are all lost,",
    "	Find them at any cost.",
    "",
    "	Gotta make a change ",
    "	Gotta make a change"
  }},
  /*0*/{NO ,"Change",{
    "",
    "	The earth can't be like this,",
    "	Life can't be so loveless.",
    "",
    "	Gotta make a change ",
    "	Gotta make a change",
    "",
    " Copyright 1992, Marilyn Davis",
    "",
    ".",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    ""
  }},
  /*0*/{NO ,"Spiritual Teacher",{
    "",
    "You walk this earth with your state of mind",
    "You know how to be free",
    "The rest of us watch and wonder",
    "How free we could be.",
    "",
    "You walk this earth as a friend of mine,",
    "Though you may never think about me.",
    "You live in the corner of my mind",
    "That shows me how to be.",
    "",
    "You walk this earth with your state of mind",
    "that taught my mind to mend.",
    "I thank my stars I knew you once ",
    "though I may never see you again.",
    "",
    "You walk this earth with your state of mind",
    "That taught me what to do",
    "I learned from you what I learned from you",
    "Because you always",
    "are faithful                                     Copyright 1991",
    "to the truth.                                 by Marilyn Davis.",
    ""
  }},
  /*0*/{YES ,"In the car",{
    " In the Car",
    " by Marilyn Davis",
    "",
    " Ancient plants and dinosaurs",
    " Make pollution in our cars.  ",
    " In the car.  ",
    " In the car.",
    "",
    " Crawl along like little ants",
    " Trapped up in our fuel tanks. ",
    " In the car. ",
    " In the car.",
    "",
    " World goes rushing by us",
    " Death grip attention to the road",
    " Daydreaming about our lovers",
    " Each of us, alone.",
    " In the car.",
    "",
    " Woman on the radio",
    " tells me what I need to know. ",
    " In the car.",
    " In the car."
  }},
  /*0*/{YES ,"In the car",{
    " Accidents and traffic jams",
    " Gonna ruin all my plans.",
    " In the car.",
    " In the car.",
    "",
    " Each of us, in isolation",
    " List our shoulds and our musts",
    " Steel ourselves to the situation",
    " And the road rides us.",
    " In the car.",
    "",
    " Driving in adversity",
    " Anxiety burns into me.  ",
    " In the car. ",
    " In the car.",
    "",
    " Tanker trucks with toxic waste",
    " Tripping down the interstate. ",
    " In the car.",
    " In the car.",
    "",
    "",
    ""
  }},
  /*0*/{NO ,"In the car",{
    " When you get the chance",
    " You take the inside",
    " Straight through to the end",
    " Keep it coming round past the outside",
    " Again, again, again, and again.",
    " In the car.",
    "",
    " Peering through the smog and haze",
    " Rats running through a maze.  ",
    " In the car.",
    " In the car.",
    " ",
    " Wiper blades and steering wheels",
    " Rude moves and diesel smells. ",
    " In the car.",
    " In the car.",
    "",
    " The world goes rushing by us.",
    " Death grip attention to the road.",
    " Daydreaming about past lovers.",
    " Each of us alone.                                Copyright 1992 ",
    " In the car.	                                     by Marilyn Davis.",
    ""
  }},
  /*0*/{YES ,"New Day",{
    "",
    "A new day is coming!",
    "Gonna rule your roost",
    "From the other end.",
    "Oh yes, a new day!",
    "New kind of new day",
    "When the sun's gonna shine",
    "Til there's a brighter shade of blue.",
    "You're gonna throw away your troubles",
    "And witness something new.",
    "That tunnel's disappearing",
    "Cause the light is bursting through.",
    "Yeh on a new day.",
    "Bright eggs and bacon new day",
    "Cock a doodle doo",
    "cock a doodle do do do do do",
    "Cock a doodle cock a doodle cock a doodle cock a doodle dooo.",
    "",
    ".",
    "",
    "",
    "",
    ""
  }},
  /*0*/{YES ,"New Day",{
    "All those dark clouds, with shiny silver lining,",
    "gonna turn inside-out.",
    "It's gonna rain rain rain sweet love",
    "And wash down every doubt.",
    "Yeah, on a new day, ",
    "A bright new new day,",
    "When there'll be sails in the sunrise",
    "Coming home to you.",
    "You're gonna throw 'way all your dreams",
    "Cause better dreams are coming true.",
    "There'll be the nector of the goddess",
    "In that morning dew,",
    "Of a new day,",
    "I mean a new day.",
    "Cockadoodle doo",
    "Cockadoodle do do do do do",
    "Cockadoodle cockadoodle cockadoodle cockadoodle dooo.",
    "",
    ".",
    "",
    "",
    "",
    ""
  }},
  /*0*/{NO ,"New Day",{
    "You're gonna find a haystack made of needles",
    "So you can sew this world, to the palm of your hand.",
    "You're gonna need, a new way to be being,",
    "When that old rock and that hard place,",
    "Crumble into sand.",
    "On that new day,",
    "Wonderful new day.",
    "We'll only know the truth",
    "And it'll open every door,",
    "We'll look back on all our suffering",
    "Wonder what'd we do that for,",
    "They'll throw away their weapons",
    "and study war no more,",
    "These days will be the old days",
    "And you'll worry worry worry worry worry no more",
    "On a new day.",
    "",
    "Copyright 1991, by Marilyn Davis.",
    "",
    ".",
    "",
    "",
    ""
  }},
  /*0*/{YES ,"Take Me To The Moon",{
    "	Take Me To The Moon ",
    "",
    "	by Marilyn Davis",
    "",
    "",
    "	I'm lost here on this planet,",
    "	Take me to the moon.",
    "",
    "",
    "	Take me on your spaceship",
    "	Take me off this planet",
    "	Take me on a field trip",
    "	One way to the moon.",
    "",
    "	Fly me on that old cow",
    "	Let's get going right now",
    "	I know somehow",
    "	You can take me to the moon.",
    "",
    "	Dancing round in circles",
    "	In some forgotton dream.",
    "	Running round the hurdles",
    "	Saying what I mean."
  }},
  /*0*/{NO ,"Take Me To The Moon",{
    "	You tell me, tell me",
    "	And tell each other too.",
    "	I don't know what you know,",
    "	But I know what to do.",
    "",
    "	Use your esp,  ",
    "	Do voodoo over me,			",
    "	Or drink a magic tea",
    "	To take me to the moon.",
    "",
    "	Catch a falling star",
    "	or bounce me off of mars",
    "	Or take me in your arms",
    "	But take me to the moon.",
    "	",
    "	I never miss a chance",
    "	To ask, if I really care",
    "	I struck by that old moon",
    "	And you can get me there.",
    "	I never miss a chance ",
    "	to make a dream come true.",
    "	The best is yet to come                          Copyright 1992",
    "	Take me to the moon.                             by Marilyn Davis."
  }},
  /*0*/{NO ,"",{
    ".",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    ""
  }}
};
OKorNOT
try_poetry(ITEM_INFO *p_item, char **new_input)
{
  short i;
  char * show_poem(short number);
  static YESorNO seen_warning = NO;
  for (i = 0; poem[i].key[0] != '\0'&& poem[i].screen[0][0] != '.' 
	 && poem[i].key != NULL && i < NO_POEMS; i++)
    {
#ifdef TITLE
      if (strncmp(p_item->eVote.title, poem[i].key, 3) == 0)
#endif
	break;
    }
#ifdef TITLE
  if (strncmp(p_item->eVote.title, poem[i].key, 3) == 0)
#endif
    {
      if (seen_warning == NO)
	{
	  seen_warning = YES;
	  printf("\n\nThe Poetry Conference is special because there are");
	  printf("\npoems hard-coded into it.  So, you can actually see them.\n");
	  printf("\nRegrettably, you can't contribute. \n\n");
	  sleep(3);
	}
      *new_input = show_poem(i);
      return OK;
    }
  return NOT_OK;
}
/**************************************************************************/
char *
show_poem(short i)
{
  char * input;
  short line;
  YESorNO stifle = NO;
  short j;
  do
    {
      if (stifle == NO)
	{
	  for (line = 0; line < NO_LINES; line++)
	    {
	      printf("\n");
	      if (poem[i].screen[line][0] == '.')
		{
		  break;
		}
	      for (j = 0; j < NO_COLS; j++)
		{
		  if (poem[i].screen[line][j] == '\0')
		    j = NO_COLS;
		  else
		    printf("%c", poem[i].screen[line][j]);
		}
	    }
	}
      stifle = NO;
      if (poem[i].more == YES)
	{
	  input = GetArg("\nContinued ...            < ENTER > to continue, or 'q' to return to Conf? ");
	  switch (*input)
	    {
	    case 'b':
	      if (i > 0)
		i--;
	      continue;
	      break;
	    case '?':
	      printf("\n\n  There are a few poems hardcoded into the eVote demo.");
	      printf("\n  You are seeing one of those poems.");
	      stifle = YES;
	      continue;
	      break;
	    case 't':
	      explain("x teacher");
	      stifle = YES;
	      continue;
	      break;
	    case '\0':
	      i++;
	      continue;
	      break;
	    case 'q':
	      return NULL;
	      break;
	    default:
	      return input;
	      break;
	    }
	}
      else return NULL;
    }
  while (1);
  return NULL;  /* never happens */
}
