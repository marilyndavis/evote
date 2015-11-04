/* $Id: unhtml.c,v 1.3 2003/01/15 18:54:13 marilyndavis Exp $ */ 
#define PATH "/usr/local/majordomo/"   /* Insert default path here if required */
#define TRACE "/tmp/unhtml.dbg"  /* Insert debug file name & path here if required*/
/****************************************************************************
 *      Program to convert to text a file which is html encoded
 *            For command line arguments the program to pipe into is next, 
 *            followed by its command line arguments.
 *                unhtml [<file>][command line arguments]
 *    Contributed by John Jacq. (ver 000415)
 *********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#define MAXLEN 80  /* maximum length of string without white space */
#define URLLEN 60  /* maximum printable length of a URL */
typedef enum {TEOF, TWORD, TLONW, TAMP, TBLANK, TEX,
	      TEQ, TSLASH, TSEMI, TGT, TLT, TNL, TCR} TOKEN;
typedef enum {FALSE, TRUE} BOOLEAN;
typedef enum {EOL, NOP, NL, NL2, NL6, NUM, STAR, ANC,
	      FORM, TXT, COM, HTM, TAB, ENDT} ACTION;
typedef struct { char* tname; ACTION act; } TAGLIST;
/*Global Variables*/
FILE* fp_out; /*= stdout;    Changes to stderr on error */
FILE* fp_error; /*= stderr;  Changes to fptrace on TRACE */
#ifdef TRACE
FILE * fptrace;
#endif
/* subroutines */
void send(int, char **, char ** );
TOKEN gettoken(char *, TOKEN, FILE *) ;
int rmhtml(FILE*, FILE*);
int strComp(char*, char* );
int read_url(FILE*, char* );
int check_match(char*, TAGLIST* );
int html2ascii(char *);
/****************************************************************************
 *      Program to convert a file which is html encoded
 *       uses rmhtml and send as major functions.
 ***************************************************************************/  
int
main(int argc, char *argv[], char ** env)
{
  int errno;
#ifdef TRACE
  int cc;
  fptrace = fopen(TRACE, "w");
  fp_error = fptrace;
  fprintf(fptrace,"\n\nunhtml called as:\n");
  for (cc = 0; cc < argc ; cc++)
    fprintf(fptrace," %s", argv[cc]);
  fprintf(fptrace,"\nEnvironment is:\n");
  for (cc = 0; env[cc] != NULL; cc++)
    fprintf(fptrace, "%s ", env[cc]);
  fprintf(fptrace,"\n\n");
  fflush(fptrace);
#endif
  fp_out = stdout;
  fp_error = stderr;
  /* We send the rest of the arguments to the next program and launch it*/
  if (argc > 1 )  /* Now we check the rest of the command line */
    send (argc, argv, env);    /* open pipe to argv[1] */
  errno = rmhtml(stdin, fp_out); /* let's do the real work */
  if (errno != 0 )
    {
      fprintf(fp_error,"End of file encountered while writing to ouput device!\nOperation cut short--");
      exit(errno);   /* problems ! */
      /*
	errno = 1 error was with putc() function
              = 2 error was with fprintf() function
              = 3 error was with sprintf() function
      */
    }
  exit(0);       /* 0 means no error to me */
}
/***********************************************************************
 * searches for tags and tagends matching html tags. Prints the rest
 * and converts all html character set to proper ascii. 
 ************************************************************************/
int 
rmhtml(FILE * ifp, FILE * ofp)
{
  /************************************************************************
   * The list of all known (by me)html tags follows. The tags are followed
   * by an attribute which tells the software what to do if such a tag is 
   * encountered. The attributes are:
   *       ENDT = Endtag material - print nothing until the matching closing
   *              tag is met.
   *       ANC  = Anchor detected, search for "href"
   *       NUM  = Increment (or decrement) numerical paragraph prefix
   *       STAR = Increment (or decrement) asterisk paragraph prefix
   *       FORM = Format a new paragraph prefix
   *       NL   = New line required
   *       NL2  = 2 new lines required (ie new paragraph, skip a line)
   *       NL6  = New line followed by 6 spaces
   *       TXT  = Entering (or leaving) TEXT mode
   *       HTM  = Entering (or leaving) HTML mode
   *       COM  = Entering comment mode
   *       NOP  = No operation required (except removal of the tag itself)
   *       EOL  = "End Of List" marker, must be the last entry in the list!
   *************************************************************************/
  TAGLIST  tags[] =
  {
    {"--",          COM},
    {"a",           ANC},
    {"abbr",        NOP},
    {"abbrev",      NOP},
    {"acronym",     NOP},
    {"address",     NOP},
    {"applet",      ENDT},
    {"area",        NOP},
    {"b",           NOP},
    {"base",        NOP},
    {"basefont",    NOP},
    {"bdo",         NOP},
    {"big",         NOP},
    {"bgsound",     NOP},
    {"blink",       NOP},
    {"blockquote",  NL2},
    {"body",        NOP},
    {"bold",        NOP},
    {"br",          NL},
    {"caption",     NL},
    {"center",      NOP},
    {"cite",        NOP},
    {"dd",          NL6},
    {"del",         ENDT},
    {"dfn",         NOP},
    {"dir",         STAR},
    {"div",         NL2},
    {"dl",          NUM},
    {"doctype",     NOP},
    {"dt",          FORM},
    {"em",          NOP},
    {"font",        NOP},
    {"form",        NOP},
    {"frame",       NOP},
    {"frameset",    NOP},
    {"h1",          NL},
    {"h2",          NL},
    {"h3",          NL},
    {"h4",          NL},
    {"h5",          NL},
    {"h6",          NL},
    {"h7",          NL},
    {"head",        NOP},
    {"hr",          NOP},
    {"html",        HTM},
    {"i",           NOP},
    {"img",         NOP},
    {"input",       NOP},
    {"italic",      NOP},
    {"lh",          NL},
    {"li",          FORM},
    {"link",        NOP},
    {"map",         NOP},
    {"meta",        NOP},
    {"noframes",    NOP},
    {"noscript",    ENDT},
    {"ol",          NUM},
    {"option",      NL6},
    {"p",           NL2},
    {"pre",         TXT},
    {"size",        NOP},
    {"script",      ENDT},
    {"select",      NL},
    {"small",       NOP},
    {"span",        NL},
    {"strong",      NOP},
    {"style",       ENDT},
    {"sub",         NOP},
    {"sup",         NOP},
    {"table",       NL2},
    {"textarea",    NOP},
    {"th",          NL},
    {"title",       ENDT},
    {"td",          TAB},
    {"tr",          NL},
    {"tt",          NOP},
    {"u",           NOP},
    {"ul",          STAR},
    {"var",         NOP},
    {"*E*O*L*",     EOL}
  };
  BOOLEAN slash=FALSE, blank= FALSE, newline= FALSE, comment= FALSE;
  BOOLEAN href_flag=FALSE;
  TOKEN tok = TEOF, prevtok = TEOF;
  enum {TEXT, HTML, TAG, RBCKT, CLOSE, COMT, HREF,
	ENDCOMT, ENDTAG, ASCII, SEMI} state = TEXT, base =TEXT;
  char buffer[MAXLEN+1];
  char matchtag[MAXLEN+1];
  char para[40];
  char url[URLLEN+1];
  int olul[]= {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int tagix, i, olulix=0;
  buffer[MAXLEN+1]='\0';
  /* Here's where the fun begins!!!! */
  while ((tok=gettoken(buffer, tok, ifp)))/*tok=0 when buffer is exhausted*/
    {
      switch (state)
	{
	  /*************************************************************
	   *  CASE TEXT primary base state
	   *
	   *      This is the base default step, characters are output
	   *      verbatim until a "<" or "&" is encountered which changes
	   *      the state to TAG or ASCII respectively.
	   **************************************************************/
	case TEXT:
	  switch (tok)   /* Test the token */
	    {
	    case TEQ:
	    case TWORD:
	    case TBLANK:
	    case TLONW:
	    case TSEMI:
	    case TSLASH:
	    case TGT:
	    case TNL:
	    case TEX:
	      if (fprintf(ofp,"%s", buffer) < 0)
		return 2;         /* 2 means error in fprintf */
	      break;
	    case TAMP:            /* Could be a special html character */
	      state = ASCII;
	      break;
	    case TLT:             /* Could be a html tag start */
	      state = TAG;
	      break;
	    default:
	      break;
	    }
	  prevtok = tok;
	  break; /* End of TEXT state */
	  /*************************************************************
	   *  CASE HTML alternative base state
	   *       (occurs after < html > and until < /html >
	   *      Similar to TEXT but carriage returns are stripped and
	   *      ignored. Changes of state caused by "<" and "&" leading 
	   *      to TAG or ASCII respectively.
	   **************************************************************/
	case HTML:
	  switch (tok)
	    {
	    case TEQ:    
	    case TLONW: 
	    case TSEMI:
	    case TSLASH:
	    case TGT:
	    case TEX:
	    case TWORD:  /* prepend a space to separate from previous word */
	      if (newline)
		{
		  if (putc('\n', ofp) == EOF)
		    return -1;
		}
	      else if (blank)
		{
		  if (putc(' ', ofp) == EOF)
		    return 1;
		}
	      newline = FALSE;
	      blank = FALSE;
	      if (fprintf(ofp,"%s", buffer) < 0)
		return 2;
	      prevtok = tok;
	      break;               
	    case TNL:  /* we ignore typed new lines, print  only html ones. */
	    case TBLANK:
	      blank = TRUE;
	      prevtok = tok;
	      break;               
	    case TAMP:            /* Could be a special html character */
	      if (blank) /* if & follows a blank, print the blank! */
		{
		  if (putc(' ', ofp) == EOF)
		    return 1;
		  blank=FALSE;
		}
	      state = ASCII;
	      break;
	    case TLT:             /* Could be a html tag start */
	      if (blank)
		if (putc(' ', ofp) == EOF) /* print the blank in advance */
		  return 1;
	      blank=FALSE;
	      state = TAG;
	      break;
	    default:
	      break;
	    }
	  break;  /* end of HTML state */
	  /*************************************************************
	   *  CASE TAG reads the token following a "<"
	   *     a "/" changes the state to close
	   *     html changes base to HTML
	   *     head and title send us to ENDTAG mode to wait for /head 
	   *     or /title before returning to base state 
	   *     ol, ul, li cause the paragraph header to be formatted 
	   **************************************************************/
	case TAG:       /* A < character was parsed, next token decides */
	  switch (tok)
	    {
	    case TSLASH:
	      state = CLOSE;  /* it is a close tag, change state */
	      break;
	    case TEX:
	      state = COMT; /* it's either a comment or doctype declare */
	      break;
	    case TWORD: /*
			  This is the case where the tag recognition is
			  carried out. The word is compared to the tag list
			  and the corresponding action is carried out 
			*/
	      strcpy(matchtag, buffer); /* save the tag name*/             
	      /* check_match returns the matching tag index,              
		 or -1 if none is found */                                
	      if ((tagix=check_match(matchtag, tags)) == -1)
		{  
		                /* unrecognized keyword or not html */
		  state = base; /* in case of doubt, send the bloody thing
				    out. Better than losing it */          
		  if (fprintf(ofp,"%s%s","<", matchtag) < 0)
		    return 2;
		}                                             
	      else if (tags[tagix].act == ENDT) /* Identified as ENDTAG*/
		{
		  state = ENDTAG; /*for those tags we wait for matching closing
				    tag to switch back to base state */
		  slash = FALSE;  
		}
	      else if (tags[tagix].act == ANC) /* Identified an ANCHOR */
		{
		  state = HREF; /*for those tags we wait check for a href
				  string */
		  slash = FALSE;  
		}
	      else if (tags[tagix].act == NUM)  /* TAG= ol - Ordered List */
		{
		  if (olulix < 20)
		    olul[olulix++]= 0; /* We set a 0 in the next level*/
		  state = RBCKT; /* We now wait for closing bracket */
		}
	      else if (tags[tagix].act == STAR)  /* TAG= ul - Unordered List */
		{
		  if (olulix < 20)
		    olul[olulix++]=-1; /* We set a -1 in the next level*/
		  state = RBCKT; /* We now wait for closing bracket */
		}
	      else if (tags[tagix].act == FORM)  /* TAG= li - List */
		{
		  /* first we increment the index if required */
		  if (olul[olulix-1] >= 0)
		    ++olul[olulix-1];
		  /* We sort out the prefix to the paragraph */
		  if (olul[0] > 0)
		    {
		      if (sprintf(para,"%i", olul[0]) < 0)
			return 3; /* 3 is error in sprintf */
		    }
		  else
		    {
		      if (sprintf(para,"%s","* ") < 0)
			return 3;
		    }
		  for (i=1; i < olulix ; i++)
		    {
		      if (olul[i] > 0)
			{
			  if (strlen(para) < 30 ) /* Stop malicious overflow
						     attack by clamping 
						     at no more than 30 char.*/
			    if (sprintf(para,"%s.%i", para, olul[i]) < 0)
			      return 3;
			}
		      else
			{
			  if (sprintf(para,"%s","* ") < 0)
			    return 3;
			}
		    }
		  if (fprintf(ofp,"\n%s  ", para) < 0)
		    return 2; /* paragraph break */
		  newline = FALSE;
		  blank=FALSE; /* set the flag, we just did 1 line feed */
		  state = RBCKT; /* We now wait for closing bracket */
		}
	      else if (tags[tagix].act == NL2) /* beginnings of paragraphs */
		{
		  if (fprintf(ofp,"%s","\n\n") < 0)
		    return 2; /* 2 line feeds to separate paragraphs */
		  newline = FALSE;
		  blank = FALSE; 
		  state = RBCKT;
		}
	      else if (tags[tagix].act == NL) /* new line */
		{
		  if (fprintf(ofp,"%s","\n") < 0)
		    return 2; /* 1 line feed to mark new line */
		  newline = FALSE;
		  blank = FALSE;
		  state = RBCKT;
		}
	      else if (tags[tagix].act == NL6) /* new line+ 6 spaces */
		{
		  if (fprintf(ofp,"%s","\n     ") < 0)
		    return 2; /* 1 line feed to mark new line */
		  newline = FALSE;
		  blank = FALSE;
		  state = RBCKT;
		}
	      else if (tags[tagix].act == TXT) /* "pre"formatted paragraphs */
		{
		  base = TEXT ;
		  state = RBCKT;
		}
	      else if (tags[tagix].act == HTM) /* html tag, set base to HTML */
		{
		  base = HTML;
		  state = RBCKT;
		}
	      else if (tags[tagix].act == TAB) /* table cells separator */
		{
		  if (fprintf(ofp,"%s","   ") < 0)
		    return 2; /* 3 spaces to separate table cells */
		  blank = FALSE; /* not a newline but avoids a 4th blank */
		  state = RBCKT;
		}
	      else
		state = RBCKT; /* We now wait for the closing bracket */
	      break;
	    default:    /* None of the above, we print and return to 
			   base state */
	      if (fprintf(ofp,"< %s", buffer) < 0)
		return 2;
	      state = base;
	      break;
	    }
	  break; /* end of TAG case */
	  /*************************************************************
	   *  CASE HREF  waiting for href=
	   *              
	   **************************************************************/
	case HREF:  /* waiting for the closing token */
	  if (! href_flag)
	    {
	      switch (tok)
		{
		case TGT:    /* >  the HTML foible is over, revert
				to base state */
		  state = base;
		  break;
		case TWORD:
		  if (strComp(buffer,"href"))
		    href_flag = TRUE;
		default:     /* otherwise just wait patiently */
		  break;
		}
	    }
	  else
	    {
	      /* Read the Earl of URL*/
	      read_url(ifp, url);
	      state = RBCKT;
	    }
	  break; /* End of HREF state */
	  /*************************************************************
	   *  CASE RBCKT  waiting for right angle bracket before
	   *              continuing.  .....BORING!!!
	   **************************************************************/
	case RBCKT:  /* waiting for the closing token */
	  switch (tok)
	    {
	    case TGT:    /* >  the HTML foible is over, revert
			    to base state */
	      state = base;
	    default:     /* otherwise just wait patiently */
	      break;
	    }
	  break; /* End of RBCKT state */
	  /*************************************************************
	   *  CASE COMT  check for comment
	   *             
	   **************************************************************/
	case COMT:  /* waiting for the closing token */
	  switch (tok)
	    {
	    case TWORD:
	      strcpy(matchtag, buffer); /* save the tag name*/             
	      /* check_match returns the matching tag index,              
		 or -1 if none is found */                                
	      if ((tagix=check_match(matchtag, tags)) == -1)
		{  
		                /* unrecognized keyword or not html */
		  state = base; /* in case of doubt, send the bloody thing
				    out. Better than losing it */          
		  if (fprintf(ofp,"%s%s","<", matchtag) < 0)
		    return 2;
		}                                             
	      else if (tags[tagix].act == COM)
		state = ENDCOMT;
	      else
		state = RBCKT;
	      break;
	    default:     /* otherwise we just return to base */
	      state = base;
	      break;
	    }
	  break; /* End of COMT state */
	  /*************************************************************
	   *  CASE CLOSE 
	   *      a < / has arrived, let's see what follows
	   *      if it is a TWORD then we may have something to do:
	   *      ol, ul, li decrement and reformat the paragraphs headers.
	   *      pre means the end of preformatted, switch back to HTML
	   *      html means the end of html, switch back to TEXT.
	   **************************************************************/
	case CLOSE:  /* waiting for the closing tag name */
	  switch (tok)
	    {
	    case TWORD:
	      strcpy(matchtag, buffer); /* save the tag name*/
	      /* check_match returns the matching tag index,
		 or -1 if none is found */
	      tagix=check_match(matchtag, tags);
	      if (tagix == -1) /* unrecognized keyword or not html */
		{
		  state = base; /* in case of doubt, send the bloody thing
				   out. Better than losing it */
		  if (fprintf(ofp,"%s%s"," < /", matchtag) < 0)
		    return 2;
		}
	      else if (tags[tagix].act == NL || tags[tagix].act == NL2)
		/* Make sure a new line follows */
		{   
		  newline = TRUE; /* a new line will be carried out */
		  state = RBCKT; /* We now wait for closing bracket */
		}
	      else if (tags[tagix].act == NUM)  /* TAG= /ol - Ordered List */
		{   
		  if (olulix > 0)
		    olul[--olulix]=0; /* We reset the level*/
		  state = RBCKT; /* We now wait for closing bracket */
		}
	      else if (tags[tagix].act == STAR)  /* TAG= /ul - Unordered List */
		{
		  if (olulix > 0)
		    olul[--olulix]=0; /* We reset the level*/
		  state = RBCKT; /* We now wait for closing bracket */
		}
	      else if (tags[tagix].act == TXT) /*"/pre" formatted paragraphs */
		{
		  base = HTML ; /* revert to HTML base mode */ 
		  state = RBCKT;
		}
	      else if (tags[tagix].act == HTM) /* /html tag, set base to TXT */
		{
		  if (putc('\n', ofp) == EOF)
		    return 1;
		  base = TEXT;     /* This is the end of the html rubbish*/
		  state = RBCKT;  /* back to TXT base mode  */
		}
	      else if (tags[tagix].act == ANC) /* /anchor tag, check href */
		{
		  if (href_flag)    /* dump the url */
		    {
		      href_flag=FALSE;
		      if (fprintf(ofp," (%s) ", url) < 0)
			return 2;
		    }
		  state = RBCKT;  /* go wait for >  */
		}
	      else                                                        
		state = RBCKT; /* We now wait for the closing bracket */
	      break;                                                   
	    default:    /* None of the above, we print and return to 
			   base state */
	      if (fprintf(ofp,"%s%s","< /", matchtag) < 0)
		return 2;
	      state = base;
	      break;
	    }
	  break; /* end of CLOSE state */
	  /*************************************************************
	   *  CASE ENDTAG
	   *      we wait for a TSLASH, TWORD combination where
	   *      the TWORD token matches variable matchtag. 
	   *      when this happens, we switch to RBCKT state.
	   *      
	   **************************************************************/
	case ENDTAG: /* waiting for matching ENDTAG */
	  switch (tok)
	    {
	    case TSLASH:
	      slash = TRUE;
	      break;
	    case TWORD:
	      if ((strComp(matchtag, buffer) == 1) && slash)
		state = RBCKT; /* wait for ">" then return */
	      else
		slash = FALSE; /* wrong slash, we reset the flag */
	      break;
	    default:
	      break;
	    }
	  break; /* End of ENDTAG state */ 
	  /*************************************************************
	   *  CASE ENDCOMT
	   *      we wait for a --, > combination  
	   *      when this happens, we switch to base state.
	   *      
	   **************************************************************/
	case ENDCOMT: /* waiting for matching comment close */
	  switch (tok)
	    {
	    case TWORD:
	      if (strComp(matchtag, buffer) == 1)
		comment = TRUE; /* set flag */
	      else
		comment = FALSE; /* wrong word, we reset the flag */
	      break;
	    case TGT:
	      if (comment)
		state = base; /* No break as we want to reset comment */
	    default:
	      comment = FALSE;
	      break;
	    }
	  break; /* End of ENDCOMT case */
	  /*************************************************************
	   *  CASE ASCII
	   *
	   *      This is the step where html characters are output.
	   *      triggered by a "&" which changes the state to ASCII
	   *      terminated by ";", TWORD in between is decoded.
	   **************************************************************/
	case ASCII:
	  switch (tok)
	    {
	    case TWORD:    /* a word follows the & */
	      if ((i=html2ascii(buffer)) > 0)
		{
		  if (putc(i, ofp) == EOF) /* Successful decoding */
		    return 1;
		  state=SEMI; /* wait for semi colon */
		}
	      else
		{                               
		  if (fprintf(ofp,"&%s", buffer) < 0) /* Unknown, dump original*/
		    return 2;
		  state = base; /* and revert to base mode */
		}
	      break;
	    case TBLANK:
	    case TNL:
	      if (putc('&', ofp) == EOF)
		return 1;
	      blank = TRUE;
	      prevtok = tok;
	      state = base;
	      break;               
	    default: 
	      if (fprintf(ofp,"&%s", buffer) < 0)
		return 2;         /* With proper html, we should never*/
	      state = base;       /* arrive here, but with B.Gates...if */
	      break;              /* we do, then it's better to start */
	    }                     /* writing again */
	  break; /* End of ASCII state */
	case SEMI:
	  switch (tok)
	    {
	    case TSEMI:           /* closing token delimiting a html  */
	      state = base;       /* character, now we switch back to */
	      break;              /* TEXT or HTML state */
	    default:  /* if not ";" print and return to base */            
	      if (fprintf(ofp,"%s", buffer) < 0)
		return 2;
	      state = base;       
	      break;              
	    }                     
	  break; /* End of SEMI state */
	} /* End of "state" cases */
    } /* End of while loop */
  return 0;   /* 0 means "OK" here */
}
/********************************************************************
 *  returns the index of the array where a match was found or -1 if
 *  none.
 **********************************************************************/
int 
check_match(char*	match, TAGLIST* list)
{
  int count = 0;
  while (list[count].act != EOL)
    {
      if (strComp(list[count].tname, match))
	  return count;
      count++;
    }
  return -1;
}
/************************************************************************
 * Compares 2 strings regardless of the case.
 * returns 0 for non match, returns one for a match.
 ************************************************************************/
int
strComp(char* first, char* second)
{
  int i=0;
  int a, b;
      /* compare one letter at a time in lower case */
      while (first[i] != '\0' && second[i] != '\0')
	{                          
	  a = first[i];
	  if (a < 'a')
	    a += 'a' - 'A';
	  b = second [i];
	  if (b < 'a')
	    b += 'a' - 'A';
	  if (a != b)
	    return 0;
	  i++;
	}
      if (first[i] || second[i]) /* in case one is longer than the other*/
	return 0;
      return 1;
 }
/************************************************************************
 *  Read the url / 0 = OK, 1 = EOF
 *************************************************************************/
int 
read_url(FILE* ifp, char * url)
{
  int c, count=0;
  while ((c=getc(ifp)) != EOF )
    {
      if (c != '=' && c != '"')
	{
	  if (c == '?' || c == ' ' || c == '\n')  /* URL search mode -- discard (boring) */ 
	    break;
	  if (c == '>') /* Oops -- gone too far, back-off one */
	    {
	      ungetc(c, ifp);   /* return the character we've just read
				  and we return */
	      break;
	    }
	  *url++ = c;
	  if (++count >= URLLEN) /* 60 characters enough -- quit */
	    break;
	}
    }
  *url='\0';
  if (c == EOF)
    return 1;
  return 0;
}
/************************************************************************
 * returns the token (ie separator or type of word). The word maximum length
 * is set by MAXLEN (typically 80). lasttoken is required in case the
 * previous token was TLONW or TLONQ (signifies a long unterminated string).
 *************************************************************************/
TOKEN 
gettoken(char *word, TOKEN lasttoken, FILE* ifp) /* collect and class
							  tokens */
{
  enum {NEUTRAL,WORD} state; /* state machine
					    definitions  */
  int c;
  int i = 1;
  char *w;
    /* 
     If a long unquoted string is encountered, which exceeds the buffer
     length, TLONG is returned so we can resume in the correct state (WORD)
     on the next call.
     */
 
  switch (lasttoken)
    {
    case TLONW:
      state = WORD;
      i = 0;
      break;
    default:
      state = NEUTRAL;
      break;
    }
  w = word; /* we start with the pointer at the start of word */
  while ((c = getc(ifp)) != EOF)
    {
      switch (state)
	{
	case NEUTRAL:
	  switch (c)
	    {
	    case '<':         /* start of html tag ? */
	      *w++ = c;
	      *w = '\0';
	      return (TLT);
	    case '>':         /* end of html tag ? */
	      *w++ = c;
	      *w = '\0';
	      return (TGT);
	    case ' ':
	    case '\t':
	      *w++ = c;
	      *w = '\0';
	      return (TBLANK);
	    case '\n':
	      *w++ = c;
	      *w = '\0';
	      return (TNL);
	    case '\r':
	      *w++ = c;
	      *w = '\0';
	      return (TCR);
	    case '&':        /* case of an html escaped character ? */
	      *w++ = c;
	      *w = '\0';
	      return(TAMP);
	    case ';':
	      *w++ = c;
	      *w = '\0';
	      return (TSEMI); /* end of html escaped character ? */
	    case '!':
	      *w++ = c;
	      *w = '\0';
	      return (TEX);
	    case '/':
	      *w++ = c;
	      *w = '\0';
	      return (TSLASH);
	    case '=':
	      *w++ = c;
	      *w = '\0';
	      return (TEQ);
	    default:
	      state = WORD;
	      *w+ += c;
	      continue;
	    }
	case WORD:     /* we hit on some letters */
	  switch (c)
	    {
	    case '<':     /* we quit on any token terminator */
	    case '>':
	    case ' ':
	    case '\t':
	    case '\n':
	    case '\r':
	    case '&':
	    case ';':
	    case '!':
	    case '/':
	    case '=':
	      ungetc(c, ifp);   /* return the character we've just read
				   and we return */
	      *w='\0';
	      return (TWORD);
	    default:
	      *w++ = c;
	      if (++i >= MAXLEN)
		{
		  *w = '\0';
		  return (TLONW);    /* If we exceed the buffer, we quit 
					before we overflow */
		}
	      continue;
	    }
	}
    }
  return (TEOF);   /* if we're here it's 'cause there ain't nuttin left */
}
/****************************************************************************
 *   Converts the string to an ASCII integer if possible or -1 if not found
 ****************************************************************************/
int
html2ascii(char * htmlc)
{
  typedef struct { char * html; int let; } AMPCHAR;
  /**************************************************************************
   * HTML extended character set is below. Pretty much complete I think.
   *************************************************************************/
  AMPCHAR htmlchar[] =
  {
    {"reg", 168}, {"copy", 169}, {"amp",'&'}, {"lt",'<'}, {"gt",'>'}, {"quot",'\"'}, {"ensp",' '},
    {"emsp",' '}, {"nbsp",' '}, {"endash",'-'}, {"emdash",'-'}, {"laquo", 171}, {"raquo", 187},
    {"Agrave", 192}, {"Aacute", 193}, {"Acirc", 194}, {"Atilde", 195}, {"Auml", 196}, {"Aring", 197},
    {"AElig", 198},
    {"Ccedil", 199},
    {"Egrave", 200}, {"Eacute", 201}, {"Ecirc", 202}, {"Euml", 203},
    {"Igrave", 204}, {"Iacute", 205}, {"Icirc", 206}, {"Iuml", 207},
    {"ETH", 208}, {"Ntilde", 209},
    {"Ograve", 210}, {"Oacute", 211}, {"Ocirc", 212}, {"Otilde", 213}, {"Ouml", 214}, {"Oslash", 215},
    {"Ugrave", 217}, {"Uacute", 218}, {"Ucirc", 219}, {"Uuml", 220},
    {"Yacute", 221}, {"THORN", 222}, {"szlig", 223},
    {"agrave", 224}, {"aacute", 225}, {"acirc", 226}, {"atilde", 227}, {"auml", 228}, {"aring", 229},
    {"aelig", 230},
    {"ccedil", 231},
    {"egrave", 232}, {"eacute", 233}, {"ecirc", 234}, {"euml", 235},
    {"igrave", 236}, {"iacute", 237}, {"icirc", 238}, {"iuml", 239},
    {"eth", 240}, {"ntilde", 241},
    {"ograve", 242}, {"oacute", 243}, {"ocirc", 244}, {"otilde", 245}, {"ouml", 246}, {"oslash", 247},
    {"ugrave", 249}, {"uacute", 250}, {"ucirc", 251}, {"uuml", 252},
    {"yacute", 253}, {"thorn", 254}, {"yuml", 255},
    {"#32", 32}, {"#33", 33}, {"#34", 34}, {"#35", 35},
    {"#36", 36}, {"#37", 37}, {"#38", 38}, {"#39", 39},
    {"#40", 40}, {"#41", 41}, {"#42", 42}, {"#43", 43},
    {"#44", 44}, {"#45", 45}, {"#46", 46}, {"#47", 47},
    {"#48", 48}, {"#49", 49}, {"#50", 50}, {"#51", 51},
    {"#52", 52}, {"#53", 53}, {"#54", 54}, {"#55", 55},
    {"#56", 56}, {"#57", 57}, {"#58", 58}, {"#59", 59},
    {"#60", 60}, {"#61", 61}, {"#62", 62}, {"#63", 63},
    {"#64", 64}, {"#65", 65}, {"#66", 66}, {"#67", 67},
    {"#68", 68}, {"#69", 69}, {"#70", 70}, {"#71", 71},
    {"#72", 72}, {"#73", 73}, {"#74", 74}, {"#75", 75},
    {"#76", 76}, {"#77", 77}, {"#78", 78}, {"#79", 79},
    {"#80", 80}, {"#81", 81}, {"#82", 82}, {"#83", 83},
    {"#84", 84}, {"#85", 85}, {"#86", 86}, {"#87", 87},
    {"#88", 88}, {"#89", 89}, {"#90", 90}, {"#91", 91},
    {"#92", 92}, {"#93", 93}, {"#94", 94}, {"#95", 95},
    {"#96", 96}, {"#97", 97}, {"#98", 98}, {"#99", 99},
    {"#100", 100}, {"#101", 101}, {"#102", 102}, {"#103", 103},
    {"#104", 104}, {"#105", 105}, {"#106", 106}, {"#107", 107},
    {"#108", 108}, {"#109", 109}, {"#110", 110}, {"#111", 111},
    {"#112", 112}, {"#113", 113}, {"#114", 114}, {"#115", 115},
    {"#116", 116}, {"#117", 117}, {"#118", 118}, {"#119", 119},
    {"#120", 120}, {"#121", 121}, {"#122", 122}, {"#123", 123},
    {"#124", 124}, {"#125", 125}, {"#126", 126}, {"#127", 127},
    {"#128", 128}, {"#129", 129}, {"#130", 130}, {"#131", 131},
    {"#132", 132}, {"#133", 133}, {"#134", 134}, {"#135", 135},
    {"#136", 136}, {"#137", 137}, {"#138", 138}, {"#139", 139},
    {"#140", 140}, {"#141", 141}, {"#142", 142}, {"#143", 143},
    {"#144", 144}, {"#145", 145}, {"#146", 146}, {"#147", 147},
    {"#148", 148}, {"#149", 149}, {"#150", 150}, {"#151", 151},
    {"#152", 152}, {"#153", 153}, {"#154", 154}, {"#155", 155},
    {"#156", 156}, {"#157", 157}, {"#158", 158}, {"#159", 159},
    {"#160", 160}, {"#161", 161}, {"#162", 162}, {"#163", 163},
    {"#164", 164}, {"#165", 165}, {"#166", 166}, {"#167", 167},
    {"#168", 168}, {"#169", 169}, {"#170", 170}, {"#171", 171},
    {"#172", 172}, {"#173", 173}, {"#174", 174}, {"#175", 175},
    {"#176", 176}, {"#177", 177}, {"#178", 178}, {"#179", 179},
    {"#180", 180}, {"#181", 181}, {"#182", 182}, {"#183", 183},
    {"#184", 184}, {"#185", 185}, {"#186", 186}, {"#187", 187},
    {"#188", 188}, {"#189", 189}, {"#190", 190}, {"#191", 191},
    {"#192", 192}, {"#193", 193}, {"#194", 194}, {"#195", 195},
    {"#196", 196}, {"#197", 197}, {"#198", 198}, {"#199", 199},
    {"#200", 200}, {"#201", 201}, {"#202", 202}, {"#203", 203},
    {"#204", 204}, {"#205", 205}, {"#206", 206}, {"#207", 207},
    {"#208", 208}, {"#209", 209}, {"#210", 210}, {"#211", 211},
    {"#212", 212}, {"#213", 213}, {"#214", 214}, {"#215", 215},
    {"#216", 216}, {"#217", 217}, {"#218", 218}, {"#219", 219},
    {"#220", 220}, {"#221", 221}, {"#222", 222}, {"#223", 223},
    {"#224", 224}, {"#225", 225}, {"#226", 226}, {"#227", 227},
    {"#228", 228}, {"#229", 229}, {"#230", 230}, {"#231", 231},
    {"#232", 232}, {"#233", 233}, {"#234", 234}, {"#235", 235},
    {"#236", 236}, {"#237", 237}, {"#238", 238}, {"#239", 239},
    {"#240", 240}, {"#241", 241}, {"#242", 242}, {"#243", 243},
    {"#244", 244}, {"#245", 245}, {"#246", 246}, {"#247", 247},
    {"#248", 248}, {"#249", 249}, {"#250", 250}, {"#251", 251},
    {"#252", 252}, {"#253", 253}, {"#254", 254}, {"#255", 255},
    {NULL, 0}
  };
  int i=0;
  while (htmlchar[i].let)
    {
      if (strcmp(htmlchar[i].html, htmlc) == 0)
	return htmlchar[i].let;
      i++;
    }
  return -1;
}
/**********************************************************
*    This forks and execs a process:
*        It execs argv[1] and copies the other arguments
*        to argv[1]' command line.
*        The parent's stdout is attached to the child's
*        stdin.
***********************************************************/
void
send(int argc, char ** argv, char ** env)
{
  int pfd[2];
  int i;
  char *path;
  char next_arg[100];
  char error_msg[1000];
  void send_err(char * program, char * error_msg);
  if (pipe(pfd) == -1)
    {
      send_err(argv[1], "\nCan't open pipe.\n");
    }
  switch (fork())
    {
    case -1:
      send_err(argv[1], "\nCan't fork a new process.\n");
      break;
    case 0:  /* child process */
      if (close(0) == -1)   /* close stdin */
	{
	  send_err(argv[1],  "\nChild: Can't close stdin.\n");
	  break;
	}
      if (dup(pfd[0]) != 0)  /* pipe input is stdin */
	{
	  send_err(argv[1],  "\nChild: Can't dup pipe reading to stdin.\n");
	  break;
	}
      if (close(pfd[0]) == -1 || close(pfd[1]) == -1)
	{
	  fprintf(fp_error, "\nunhtml: Child: Can't close pipe's extra descriptors.  Continuing.\n");
	}
      if ((path=malloc(strlen(PATH)+strlen(argv[1])+1)) == NULL)
	{
	  send_err(argv[1],  "\nCan't malloc space for path.\n");
	}
      strcpy(path, PATH);
      strcat(path, argv[1]);
#ifdef TRACE
      fprintf(fptrace,"\nexecve called on %s, args are :\n", path);
      for (i = 0; (argv+1)[i] != NULL; i++)
	fprintf(fptrace," %s", (argv+2)[i]);    
      fprintf(fptrace,"\nEnv passed is:\n");
      for (i=0; (env)[i] != NULL; i++)
	fprintf(fptrace, " %s", env[i]);
      fflush(fptrace);
#endif
      execve(path, argv+1, env);  
      sprintf(error_msg,"\nExec Failed, ERROR = %d\n\tCall was \"%s\".  Args were:", errno, path);
      for (i = 2; argv[i] != NULL; i++)
	{
	  sprintf(next_arg, " %s", argv[i]);
	  strcat(error_msg, next_arg);
	}
      strcat(error_msg,"\n");
      send_err(argv[1], error_msg);
      break;
    default:  /* parent */
      break;
    }
/* now we're the parent */
  if (close(1) == -1)   /* close stdout */
    {
      send_err(argv[1], "\nParent: Can't close stdout.\n");
    }
  if (dup(pfd[1]) != 1)  /* pipe output is stdout */
    {
      send_err(argv[1], "\nParent: Can't dup pipe writing to stdout.\n");
    }
  if (close(pfd[0]) == -1 || close(pfd[1]) == -1)
    {
      fprintf(fp_error, "\nParent: Can't close pipe's extra descriptors.  Continuing.\n");
    }
}
/**********************************************************************
*    Upon error in send(), this changes the stdout to stderr
*    and prints an error message.
**********************************************************************/
void
send_err(char * program, char * error_msg)
{
  fp_out = fp_error;
  fprintf(fp_error,"\nunhtml: Failed to pipe to %s.\n", program);
  fprintf(fp_error, error_msg);
  fprintf(fp_error,"\nLost message follows:\n");
}
