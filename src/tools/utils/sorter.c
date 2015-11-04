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

/* $Id: sorter.c,v 1.5 2003/10/17 23:07:50 marilyndavis Exp $ */ 
/**********************************************************
 *   sorter.c   -   called by wrapper
 *    by John J Jacq and Marilyn Davis
 *   Sorts the incoming messages by the language spoken in the
 *   message body and mails it on to the appropriate address
 *   by calling the sendmail command as given on the command
 *   line:
 *   sorter -x address -y address ... -o address sendmail_command line
 *     where -x and -y can be
 *      -en address  to send English to the address
 *      -es address  to send Espa~ol to the address
 *      -no address to send Nordic to the address
 *      -na address to send Nahuatl to the address
 *      etc.
 *      -o address to send unidentified languages to the address
 *      This *must* be there and it must be last.
 *      The sendmail command will be executed with its arguments.
 *      Probably:  "sendmail -ba -t"
 *********************************************************
 **********************************************************/
char * sendmail_path = "/usr/lib/sendmail";
char * trace_file = "/tmp/sorter.trace";
char * top =
"--\n"
"This message is forwarded to you as a service of Zapatistas Online.\n"
"Comments and volunteers are welcome.  Write zo-editors@deliberate.com.\n"
"Send submissions to chiapas-i@eco.utexas.edu\n\n";
char * bottom =
"\n--\nTo unsubscribe from this list send a message containing the words\n"
"unsubscribe chiapas95 to majordomo@eco.utexas.edu.  Previous messages\n"
"are available from http://www.eco.utexas.edu/faculty/Cleaver/chiapas95.html\n"
"or gopher://eco.utexas.edu.\n";
/*
*********************************************************/
#include"sorter.h"
char * address = NULL;
int
main(int argc, char *argv[], char *env[])
{
  LANGUAGE language;
  int i;
  if (trace_file[0])
    {
      int cc;
      if ((fptrace = fopen(trace_file, "w")) == NULL)
	{
	  trace_file[0]= '\0';
	}
      else
	{
	  fprintf(fptrace,"\n\nsorter ");
	  fprintf(fptrace,"called as:\n");
	  for (cc = 0; cc < argc ; cc++)
	    fprintf(fptrace," %s", argv[cc]);
	  fprintf(fptrace,"\nEnvironment is:\n");
	  for (cc = 0; env[cc] != NULL; cc++)
	    fprintf(fptrace, "%s ", env[cc]);
	  fprintf(fptrace,"\n\n");
	  fflush(fptrace);
	}
    }
  argc_copy = argc;
  argv_copy = argv;
  env_copy = env;
  parse_header();
  strip_subject();
  init_dico();
  if ((language = is_error()) != ERRORS)
    {
      language = langtest();
    }
  address = find_address(language);
  send(address);
  gen_header(address, language);
  if (strcmp(address,"tester") == 0)
    return 0;
  if (language == DONT_KNOW)
    {
      printf("\nWord count report.\n\n");
      for (i = 0; i < NO_LANG; i++)
	{
	  if (sort[i]->count == 0)
	    continue;
	  printf("%d %s\n", sort[i]->count, sort[i]->name);
	}
    }									
  if (trace_file[0])
    {
      fprintf(fptrace,"\nLanguage is %s, going to %s.\n",
	      dico[language].name, address);
      fflush(fptrace);
    }
  dump_message(stdout);
  return 0;
}
/************************************************************
 *     It picks the language's address off the command
 *     line and resets argv_copy and argc_copy to start
 *     after all the input for sorter has been eaten up.
 *     Later we'll call the next program in the command
 *     line.
 *************************************************************/
char *
find_address(LANGUAGE language)
{
  int i, j;
  YESorNO right;
  YESorNO other_found = NO;
  for (i = 1; i < argc_copy; i++)
    {
      if (argv_copy[i][0] == '-')
	{
	  right = YES;
	  for (j = 1; argv_copy[i][j]; j++)
	    {
	      if (TOLOWER(argv_copy[i][j]) 
		 != TOLOWER(dico[language].name[j-1]))
		{
		  right = NO;
		  break;
		}
	    }
	  if (right == YES)
	    {
	      address = argv_copy[i+1];
	    }
	  if (same(argv_copy[i], "-o"))  /* last address */
	    {
	      if (address == NULL)
		{
		  address = argv_copy[i+1];
		}
	      argc_copy -= i + 2;
	      argv_copy = &argv_copy[i + 2];
	      if (same(address, "return"))
		address = from;
	      return address;
	    }
	}
    }
  fprintf(stderr, "sorter needs -o other_address\n");
  exit(1);
}
/**************************************************************
 *      Program to scan a file and guess the language 
 *       Contributed by John Jacq with help from Marilyn Davis.
 *       (ver 980929)
 **************************************************************
 *       The function checks words against a collection of lexicons,
 *       each time a match is found, the lexicon counter is
 *       incremented.
 *          0 = unknown  *
 *          1 = English  *
 *          2 = Francais   *
 *          3 = Deutsch   *
 *          4 = Nahuatl
 *          5 = Espanol  *
 *          6 = Portuguese
 *          7 = Nordic (Swedish/Norwegian/Danish)
 *          8 = Italian  *
 *          9 = Croatian *
 *       The options followed by "*" are enabled now, others are 
 *       to come soon
 **************************************************************/
LANGUAGE
langtest(void)
{
  int i, count = 0;
  int collapse(int *, int max_len);
  /* read in each token */
  while (get_token() != EOF)
    {
      /* collapse removes all the ' and ~ and the coded
	 accents and returns the
	 length of the resultant string  -- up to 
	 MAX_WORD_LEN + 1 which is in token still */
      if (collapse(token, MAX_WORD_LEN) <= MAX_WORD_LEN)
	{
	  if (++count > 1000)  /* quit after 1000 words */
	    break;
	  checkword(token);  /* if 3 letter word, do a check*/
	}
    } /* End of while */
  return decide_language();
}
/************************************************************
 *   Check the  word against the lexicon and updates count.
 ***********************************************************/
void
checkword(int * s)
{
  int i, j;
  for (j = 1; j < NO_LANG; j++)
    {
      for (i=0; i < NO_WORDS; i++)
	{
	  if (dico[j].lex[i] == NULL)  /* for short lexicons */
	    break;
	  if (strIcmp(dico[j].lex[i], s, MAX_WORD_LEN) == 0) 
	    {
	      dico[j].count++; /* match then increment count */  
	      break;
	    }
	}
    }
  return;
}
/************************************************************
 *      Here, the counting is done and the decision is made.
 *************************************************************/
LANGUAGE
decide_language(void)
{
  int i, j;
  LANGUAGE tongue;
  DICO extra;
  tongue= DONT_KNOW;
  sort[0] = dico;
  /*	for (i = 0; i < NO_LANG; i++)
	{
	printf("%d: %s %d\n", i, dico[i].name, dico[i].count);
	} */
  for (i = 1; i < NO_LANG; i++)
    {
      for (j = i - 1; j >= 0; j--)
	{
	  if (dico[i].count > sort[j]->count)
	    {
	      sort[j + 1] = sort[j];
	    }
	  else
	    {
	      break;
	    }
	}
      sort[j + 1] = &dico[i];
      /*
	printf("\n");
	for (j = 0 ; j <= i; j++)
	{
	printf("%d: %s %d\n", j, sort[j]->name, sort[j]->count);
	}
      */
    }
  if (sort[0]->count > sort[1]->count + 2 || sort[0]->count > 1 &&
     sort[1]->count == 0 )
    tongue = sort[0]->tongue;
  return tongue;
}
/************************************************************
 *     Initializes the dico array.
 *************************************************************/
void
init_dico(void)
{
  LANGUAGE i;
  dico = malloc(sizeof(DICO) * NO_LANG);
  for (i = 0; i < NO_LANG; i++)
    {
      dico[i].lex[0] = NULL;
      dico[i].tongue = i;
      dico[i].count = 0;
    }
  dico[DONT_KNOW].name = "Don't Know";
  dico[ENGLISH].name = "English";
  dico[FRANCAIS].name = "Francais";
  dico[DEUTSCH].name = "Deutsch";
  dico[NAHUATL].name = "Nahuatl";
  dico[ESPANOL].name = "Espanol";
  dico[PORTUGUESE].name = "Portuguese";
  dico[NORDIC].name = "Nordic";
  dico[ITALIAN].name = "Italian";
  dico[CROATIAN].name = "Croatian";
  dico[ERRORS].name = "Errors";
  /*FRANCAIS lexicon*/                  
  dico[FRANCAIS].lex[ 0]="de";
  dico[FRANCAIS].lex[ 1]="la";
  dico[FRANCAIS].lex[ 2]="le";      
  dico[FRANCAIS].lex[ 3]="et";      
  dico[FRANCAIS].lex[ 4]="a";    
  dico[FRANCAIS].lex[ 5]="il";     
  dico[FRANCAIS].lex[ 6]="les";     
  dico[FRANCAIS].lex[ 7]="un";      
  dico[FRANCAIS].lex[ 8]="en";      
  dico[FRANCAIS].lex[ 9]="du";      
  dico[FRANCAIS].lex[10]="pas";     
  dico[FRANCAIS].lex[11]="que";     
  dico[FRANCAIS].lex[12]="une";     
  dico[FRANCAIS].lex[13]="des";     
  dico[FRANCAIS].lex[14]="ne";      
  dico[FRANCAIS].lex[15]="se";      
  dico[FRANCAIS].lex[16]="dans";    
  dico[FRANCAIS].lex[17]="vous";    
  dico[FRANCAIS].lex[18]="est";     
  dico[FRANCAIS].lex[19]="je";      
  dico[FRANCAIS].lex[20]="qui";      
  dico[FRANCAIS].lex[21]="au";      
  dico[FRANCAIS].lex[22]="sur";     
  dico[FRANCAIS].lex[23]="son";     
  dico[FRANCAIS].lex[24]="ce";      
  dico[FRANCAIS].lex[25]="mais";    
  dico[FRANCAIS].lex[26]="etait";   
  dico[FRANCAIS].lex[27]="plus";    
  dico[FRANCAIS].lex[28]="pour";    
  dico[FRANCAIS].lex[29]="lui";     
  dico[FRANCAIS].lex[30]="sa";     
  dico[FRANCAIS].lex[31]="bien";    
  dico[FRANCAIS].lex[32]="avec";    
  dico[FRANCAIS].lex[33]="tout";    
  dico[FRANCAIS].lex[34]="dit";     
  dico[FRANCAIS].lex[35]="on";      
  dico[FRANCAIS].lex[36]="ses";     
  dico[FRANCAIS].lex[37]="par";    
  dico[FRANCAIS].lex[38]="elle";    
  dico[FRANCAIS].lex[39]="avait";   
  dico[FRANCAIS].lex[40]="cette";   
  dico[FRANCAIS].lex[41]="ces";     
  dico[FRANCAIS].lex[42]="ou";      
  dico[FRANCAIS].lex[43]="si";      
  dico[FRANCAIS].lex[44]="comme";   
  dico[FRANCAIS].lex[45]="sans";   
  dico[FRANCAIS].lex[46]="quand";   
  dico[FRANCAIS].lex[47]="aux";     
  dico[FRANCAIS].lex[48]="y";      
  dico[FRANCAIS].lex[49]="meme";   
  /*English lexicon*/         
  dico[ENGLISH].lex[ 0]="the";   
  dico[ENGLISH].lex[ 1]="to";   
  dico[ENGLISH].lex[ 2]="of";   
  dico[ENGLISH].lex[ 3]="in";   
  dico[ENGLISH].lex[ 4]="and";   
  dico[ENGLISH].lex[ 5]="they";   
  dico[ENGLISH].lex[ 6]="a";   
  dico[ENGLISH].lex[ 7]="for";   
  dico[ENGLISH].lex[ 8]="is";   
  dico[ENGLISH].lex[ 9]="be";   
  dico[ENGLISH].lex[10]="you";   
  dico[ENGLISH].lex[11]="on";   
  dico[ENGLISH].lex[12]="sets";   
  dico[ENGLISH].lex[13]="will";   
  dico[ENGLISH].lex[14]="are";   
  dico[ENGLISH].lex[15]="set";   
  dico[ENGLISH].lex[16]="that";   
  dico[ENGLISH].lex[17]="some";   
  dico[ENGLISH].lex[18]="have";   
  dico[ENGLISH].lex[19]="as";   
  dico[ENGLISH].lex[20]="by";   
  dico[ENGLISH].lex[21]="name";   
  dico[ENGLISH].lex[22]="not";   
  dico[ENGLISH].lex[23]="this";   
  dico[ENGLISH].lex[24]="can";   
  dico[ENGLISH].lex[25]="with";   
  dico[ENGLISH].lex[26]="it";   
  dico[ENGLISH].lex[27]="which";   
  dico[ENGLISH].lex[28]="has";   
  dico[ENGLISH].lex[29]="from";   
  dico[ENGLISH].lex[30]="see";   
  dico[ENGLISH].lex[31]="when";   
  dico[ENGLISH].lex[32]="your";   
  dico[ENGLISH].lex[33]="if";   
  dico[ENGLISH].lex[34]="also";   
  dico[ENGLISH].lex[35]="but";   
  dico[ENGLISH].lex[36]="all";   
  dico[ENGLISH].lex[37]="must";   
  dico[ENGLISH].lex[38]="do";   
  dico[ENGLISH].lex[39]="or";   
  dico[ENGLISH].lex[40]="into";   
  dico[ENGLISH].lex[41]="other";   
  dico[ENGLISH].lex[42]="an";   
  dico[ENGLISH].lex[43]="its";   
  dico[ENGLISH].lex[44]="under";   
  dico[ENGLISH].lex[45]="right";   
  dico[ENGLISH].lex[46]="free";   
  dico[ENGLISH].lex[47]="more";   
  dico[ENGLISH].lex[48]="so";   
  dico[ENGLISH].lex[49]="any";   
  /*Italian lexicon*/                       
  dico[ITALIAN].lex[ 0]="di";	      
  dico[ITALIAN].lex[ 1]="e";	      
  dico[ITALIAN].lex[ 2]="che";	      
  dico[ITALIAN].lex[ 3]="il";	      
  dico[ITALIAN].lex[ 4]="per";	      
  dico[ITALIAN].lex[ 5]="i";	      
  dico[ITALIAN].lex[ 6]="a";	      
  dico[ITALIAN].lex[ 7]="in";	      
  dico[ITALIAN].lex[ 8]="le";	      
  dico[ITALIAN].lex[ 9]="un";	      
  dico[ITALIAN].lex[10]="la";	      
  dico[ITALIAN].lex[11]="non";	      
  dico[ITALIAN].lex[12]="del";	      
  dico[ITALIAN].lex[13]="una";	      
  dico[ITALIAN].lex[14]="gli";	      
  dico[ITALIAN].lex[15]="se";	      
  dico[ITALIAN].lex[16]="della";	      
  dico[ITALIAN].lex[17]="con";	      
  dico[ITALIAN].lex[18]="dei";	      
  dico[ITALIAN].lex[19]="da";	      
  dico[ITALIAN].lex[20]="come";	      
  dico[ITALIAN].lex[21]="si";	      
  dico[ITALIAN].lex[22]="mondo";	      
  dico[ITALIAN].lex[23]="loro";	      
  dico[ITALIAN].lex[24]="tutti";	      
  dico[ITALIAN].lex[25]="delle";	      
  dico[ITALIAN].lex[26]="sono";	      
  dico[ITALIAN].lex[27]="pi";	      
  dico[ITALIAN].lex[28]="ma";	      
  dico[ITALIAN].lex[29]="dal";	      
  dico[ITALIAN].lex[30]="ocse";	      
  dico[ITALIAN].lex[31]="ci";	      
  dico[ITALIAN].lex[32]="anche";	      
  dico[ITALIAN].lex[33]="ed";	      
  dico[ITALIAN].lex[34]="alla";	      
  dico[ITALIAN].lex[35]="tutte";	      
  dico[ITALIAN].lex[36]="degli";	      
  dico[ITALIAN].lex[37]="o";	      
  dico[ITALIAN].lex[38]="noi";	      
  dico[ITALIAN].lex[39]="piu";	      
  dico[ITALIAN].lex[40]="nelle";	      
  dico[ITALIAN].lex[41]="ai";	      
  dico[ITALIAN].lex[42]="nel";	      
  dico[ITALIAN].lex[43]="alle ";	      
  dico[ITALIAN].lex[44]="modo";	      
  dico[ITALIAN].lex[45]="anni";	      
  dico[ITALIAN].lex[46]="al";	      
  dico[ITALIAN].lex[47]="ogni";	      
  dico[ITALIAN].lex[48]="stato";	      
  dico[ITALIAN].lex[49]="sugli";	      
  /*DEUTSCH lexicon*/              
  dico[DEUTSCH].lex[ 0]="der";	   	
  dico[DEUTSCH].lex[ 1]="nicht";	   	
  dico[DEUTSCH].lex[ 2]="die";	   	
  dico[DEUTSCH].lex[ 3]="und";	   	
  dico[DEUTSCH].lex[ 4]="ist";	   	
  dico[DEUTSCH].lex[ 5]="ein";	   	
  dico[DEUTSCH].lex[ 6]="den";	   	
  dico[DEUTSCH].lex[ 7]="auch";	   	
  dico[DEUTSCH].lex[ 8]="das";	   	
  dico[DEUTSCH].lex[ 9]="sich";	   	
  dico[DEUTSCH].lex[10]="auf";	   	
  dico[DEUTSCH].lex[11]="er";	   	
  dico[DEUTSCH].lex[12]="von";	   	
  dico[DEUTSCH].lex[13]="wenn";	   	
  dico[DEUTSCH].lex[14]="wird";	   	
  dico[DEUTSCH].lex[15]="an";	   	
  dico[DEUTSCH].lex[16]="so";	   	
  dico[DEUTSCH].lex[17]="sein";	   	
  dico[DEUTSCH].lex[18]="immer";	   	
  dico[DEUTSCH].lex[19]="wer";	   	
  dico[DEUTSCH].lex[20]="fuer";	   	
  dico[DEUTSCH].lex[21]="pfeil";	   	
  dico[DEUTSCH].lex[22]="zu";	   	
  dico[DEUTSCH].lex[23]="eine";	   	
  dico[DEUTSCH].lex[24]="zum";	   	
  dico[DEUTSCH].lex[25]="wie";	   	
  dico[DEUTSCH].lex[26]="einem";	   	
  dico[DEUTSCH].lex[27]="alle";	   	
  dico[DEUTSCH].lex[28]="bei";	   	
  dico[DEUTSCH].lex[29]="dies";	   	
  dico[DEUTSCH].lex[30]="ende";	   	
  dico[DEUTSCH].lex[31]="genau";	   	
  dico[DEUTSCH].lex[32]="weiss";	   	
  dico[DEUTSCH].lex[33]="noch";	   	
  dico[DEUTSCH].lex[34]="ich";	   	
  dico[DEUTSCH].lex[35]="dass";	   	
  dico[DEUTSCH].lex[36]="einen";	   	
  dico[DEUTSCH].lex[37]="ganze";	   	
  dico[DEUTSCH].lex[38]="darf";	   	
  dico[DEUTSCH].lex[39]="frag";	   	
  dico[DEUTSCH].lex[40]="gerne";	   	
  dico[DEUTSCH].lex[41]="gar";	   	
  dico[DEUTSCH].lex[42]="doch";	   	
  dico[DEUTSCH].lex[43]="guckt";	   	
  dico[DEUTSCH].lex[44]="erst";	   	
  dico[DEUTSCH].lex[45]="mir";	   	
  dico[DEUTSCH].lex[46]="meine";	   	
  dico[DEUTSCH].lex[47]="sonst";	   	
  dico[DEUTSCH].lex[48]="soll";	   	
  dico[DEUTSCH].lex[49]="oder";	   	
  /*ESPANOL lexicon*/       
  dico[ESPANOL].lex[ 0]="de";
  dico[ESPANOL].lex[ 1]="la";
  dico[ESPANOL].lex[ 2]="y";
  dico[ESPANOL].lex[ 3]="el";
  dico[ESPANOL].lex[ 4]="que";
  dico[ESPANOL].lex[ 5]="los";
  dico[ESPANOL].lex[ 6]="a";
  dico[ESPANOL].lex[ 7]="en";
  dico[ESPANOL].lex[ 8]="no";
  dico[ESPANOL].lex[ 9]="se";
  dico[ESPANOL].lex[10]="las";
  dico[ESPANOL].lex[11]="del";
  dico[ESPANOL].lex[12]="con";
  dico[ESPANOL].lex[13]="por";
  dico[ESPANOL].lex[14]="para";
  dico[ESPANOL].lex[15]="un";
  dico[ESPANOL].lex[16]="es";
  dico[ESPANOL].lex[17]="su";
  dico[ESPANOL].lex[18]="al";
  dico[ESPANOL].lex[19]="una";
  dico[ESPANOL].lex[20]="como";
  dico[ESPANOL].lex[21]="lo";
  dico[ESPANOL].lex[22]="todos";
  dico[ESPANOL].lex[23]="mas";
  dico[ESPANOL].lex[24]="paz";
  dico[ESPANOL].lex[25]="esta";
  dico[ESPANOL].lex[26]="solo";
  dico[ESPANOL].lex[27]="sus";
  dico[ESPANOL].lex[28]="pero";
  dico[ESPANOL].lex[29]="todo";
  dico[ESPANOL].lex[30]="si";
  dico[ESPANOL].lex[31]="le";
  dico[ESPANOL].lex[32]="ya";
  dico[ESPANOL].lex[33]="ha";
  dico[ESPANOL].lex[34]="ni";
  dico[ESPANOL].lex[35]="estos";
  dico[ESPANOL].lex[36]="asi";
  dico[ESPANOL].lex[37]="nos";
  dico[ESPANOL].lex[38]="son";
  dico[ESPANOL].lex[39]="pais";
  dico[ESPANOL].lex[40]="ser";
  dico[ESPANOL].lex[41]="tan";
  dico[ESPANOL].lex[42]="este";
  dico[ESPANOL].lex[43]="desde";
  dico[ESPANOL].lex[44]="o";
  dico[ESPANOL].lex[45]="ley";
  dico[ESPANOL].lex[46]="lado";
  dico[ESPANOL].lex[47]="hay";
  dico[ESPANOL].lex[48]="otra";
  dico[ESPANOL].lex[49]="abajo";
  /*Croatian lexicon*/        
  dico[CROATIAN].lex[ 0]="u";
  dico[CROATIAN].lex[ 1]="je";
  dico[CROATIAN].lex[ 2]="i";
  dico[CROATIAN].lex[ 3]="sve";
  dico[CROATIAN].lex[ 4]="se";
  dico[CROATIAN].lex[ 5]="da";
  dico[CROATIAN].lex[ 6]="smo";
  dico[CROATIAN].lex[ 7]="imamo";
  dico[CROATIAN].lex[ 8]="o";
  dico[CROATIAN].lex[ 9]="ali";
  dico[CROATIAN].lex[10]="treba";
  dico[CROATIAN].lex[11]="ne";
  dico[CROATIAN].lex[12]="prvi";
  dico[CROATIAN].lex[13]="na";
  dico[CROATIAN].lex[14]="svake";
  dico[CROATIAN].lex[15]="za";
  dico[CROATIAN].lex[16]="drugi";
  dico[CROATIAN].lex[17]="netko";
  dico[CROATIAN].lex[18]="sto";
  dico[CROATIAN].lex[19]="ako";
  dico[CROATIAN].lex[20]="nas";
  dico[CROATIAN].lex[21]="jos";
  dico[CROATIAN].lex[22]="put";
  dico[CROATIAN].lex[23]="me";
  dico[CROATIAN].lex[24]="tome";
  dico[CROATIAN].lex[25]="ste";
  dico[CROATIAN].lex[26]="svoje";
  dico[CROATIAN].lex[27]="nego";
  dico[CROATIAN].lex[28]="bolje";
  dico[CROATIAN].lex[29]="druge";
  dico[CROATIAN].lex[30]="bolji";
  dico[CROATIAN].lex[31]="sam";
  dico[CROATIAN].lex[32]="toga";
  dico[CROATIAN].lex[33]="prije";
  dico[CROATIAN].lex[34]="svega";
  dico[CROATIAN].lex[35]="sa";
  dico[CROATIAN].lex[36]="dobro";
  dico[CROATIAN].lex[37]="moze";
  dico[CROATIAN].lex[38]="misle";
  dico[CROATIAN].lex[39]="to";
  dico[CROATIAN].lex[40]="mnogi";
  dico[CROATIAN].lex[41]="spas";
  dico[CROATIAN].lex[42]="radu";
  dico[CROATIAN].lex[43]="biti";
  dico[CROATIAN].lex[44]="svi";
  dico[CROATIAN].lex[45]="sada";
  dico[CROATIAN].lex[46]="svim ";
  dico[CROATIAN].lex[47]="narod";
  dico[CROATIAN].lex[48]="kakav";
  dico[CROATIAN].lex[49]="bilo";
  /*Portuguese lexicon*/        
  dico[PORTUGUESE].lex[ 0]="a";
  dico[PORTUGUESE].lex[ 1]="de";
  dico[PORTUGUESE].lex[ 2]="o";
  dico[PORTUGUESE].lex[ 3]="da";
  dico[PORTUGUESE].lex[ 4]="e";
  dico[PORTUGUESE].lex[ 5]="do";
  dico[PORTUGUESE].lex[ 6]="um";
  dico[PORTUGUESE].lex[ 7]="que";
  dico[PORTUGUESE].lex[ 8]="os";
  dico[PORTUGUESE].lex[ 9]="feira";
  dico[PORTUGUESE].lex[10]="para";
  dico[PORTUGUESE].lex[11]="pelo";
  dico[PORTUGUESE].lex[12]="dos";
  dico[PORTUGUESE].lex[13]="uma";
  dico[PORTUGUESE].lex[14]="em";
  dico[PORTUGUESE].lex[15]="na";
  dico[PORTUGUESE].lex[16]="sobre";
  dico[PORTUGUESE].lex[17]="nano";
  dico[PORTUGUESE].lex[18]="nos";
  dico[PORTUGUESE].lex[19]="sem";
  dico[PORTUGUESE].lex[20]="por";
  dico[PORTUGUESE].lex[21]="foi";
  dico[PORTUGUESE].lex[22]="trens";
  dico[PORTUGUESE].lex[23]="hoge";
  dico[PORTUGUESE].lex[24]="ano";
  dico[PORTUGUESE].lex[25]="das";
  dico[PORTUGUESE].lex[26]="anos";
  dico[PORTUGUESE].lex[27]="as";
  dico[PORTUGUESE].lex[28]="causa";
  dico[PORTUGUESE].lex[29]="no";
  dico[PORTUGUESE].lex[30]="grau";
  dico[PORTUGUESE].lex[31]="neste";
  dico[PORTUGUESE].lex[32]="greve";
  dico[PORTUGUESE].lex[33]="pela";
  dico[PORTUGUESE].lex[34]="num";
  dico[PORTUGUESE].lex[35]="esta";
  dico[PORTUGUESE].lex[36]="agora";
  dico[PORTUGUESE].lex[37]="cinco";
  dico[PORTUGUESE].lex[38]="mais";
  dico[PORTUGUESE].lex[39]="apoyo";
  dico[PORTUGUESE].lex[40]=NULL;
}
/************************************************************
 *      takes out the '\'', '`','*','^',':' and replaces '~' 
 *      or "n~" with 'n' and puts the result in the incoming 
 *      address and returns the new strlen.
 *      Also de-accents the coded accents.
 *      It gives up and returns max_word_len + 1 when the
 *      result is longer than that.
 *************************************************************/
int
collapse(int * in, int max_word_len)
{
  int *out = in, *start = in;
  int i = 0;
  in--;
  while (*++in)
    {
      if (out - start > max_word_len)
	{
	  return out - start;
	}
      *in = DEACCENT(*in);
      switch (*in)
	{
	case '~':
	  if (*(in-1) != 'n' && *(in-1) != 'N')
	    {
	      *out++ = 'n';
	    }
	  continue;
	  break;
	case '\'':
	case '`':
	case '^':
	case ':':
	case '*':
	  continue;
	  break;
	default:
	  *out++ = *in;
	  break;
	}
    }
  *out = '\0';
  return out - start;
}
