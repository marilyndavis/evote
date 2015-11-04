/* $Id: pet_out_fr.c,v 1.8 2003/10/20 17:07:34 marilyndavis Exp $ */ 
/**********************************************************
 *  ../eVote/src/mail/mailui/pet_out.c
 *         has code that needs translating
 *********************************************************
 ***********************************************************
 *     pet_out.c  default petition output functions.
 *       To provide a Spanish translation, for example,
 *       1.  In mailui.h, be sure the new language is one of the
 *           LANGUAGEs listed in the enumerated type.  Add it.
 *       2.  If the number of languages is greater than
 *           MAX_LANGUAGES, raise MAX_LANGUAGES.
 *       3.  Add a new function declaration in mailui.h:
 *           void translate_es(WHICH_FUNCTION ...
 *           Make the arguments just like translate_en(...) and
 *           declare it just after translate_en(...).
 *       4.  In table.cc, add an entry for your language to the
 *           table of tongues near the top of the file.  Be sure
 *           to keep your table of tongues in the same order as
 *           your list of LANGUAGEs in mailui.h 
 *           If your language is already there, check the trans-
 *           lations and change the translate_xx entry to be
 *           translate_es if ESPANOL is your language.      
 *       5.  Translate this file to Spanish.
 *             1.   trans en es pet_out.c
 *             2.   Edit the en-es.new file that results so that
 *                  all the phrases are translated.  Rename the
 *                  file en_es.
 *             3.   Rerun trans en es pet_out.c.  Check that
 *                  the new en_es.new file is empty.
 *       6.  Change the LOCAL_LANGUAGE below to be your language,
 *           all in caps -- in the translated version of this file.
 *       7.  Change the names of the function in this file:
 *           translate_en   --> translate_es  (also translated version)
 *       8.  Fix up the makefile in this directory to include 
 *           pet_out_es.o in the OBJS.  Note that there is a tab
 *           at the beginning of the lines in the list of OBJS,
 *           and that each line (except the last) ends with a '\'
 *       9.  Fix up the makefile in the .. directory 
 *           (EVOTE_HOME_DIR/src/mail) to include mailui/pet_out_es.o
 *           in the list at the end.
 *      10.  make the new executables.
 ************************************************************/
#include<stdio.h>
#include <sys/stat.h>
#include "mailui.h"
static void do_petition(void);
static void start_petition_displays(ITEM_INFO * p_item, YESorNO new_pet,
				    LANGUAGE which);
static void store_signature(ITEM_INFO * p_item);
static void display_petition_info(LANGUAGE which_language, 
				  ITEM_INFO * p_item, 
				  YESorNO just_checking, 
				  YESorNO new_pet, YESorNO do_header);
static void display_petition_results(ITEM_INFO *p_item);
static void display_petition_text(ITEM_INFO * p_item, YESorNO just_checking,
				  LANGUAGE which);
static void finish_unsign(char * name, int * pcc, YESorNO from_owner);
static void big_petition_finish(int exit_code);
static void no_pet_message(void);
static void vote_the_petition(void);
extern YESorNO give_email;
extern YESorNO into_eVote; /* petition.c but worked here */
/*************************************************************
 *  Change this LOCAL_LANGUAGE from ENGLISH to your LANGUAGE *
 *                       \|/                               ***/
#define LOCAL_LANGUAGE FRANCAIS
/*************************************************************
 *              |     After translating this file, change the 
 *              |     name of this function to be translate_xx 
 *              |     where xx is the flag for your language.
 ************* \|/ ******************************************/
void
translate_fr(LANGUAGE which_language, WHICH_FUNCTION function, 
		  ITEM_INFO * p_item, YESorNO just_checking, 
		  YESorNO new_pet, YESorNO do_header, char *name,
		  int * pcc, YESorNO from_owner)
{
  switch (function)
    {
    case DO_PETITION:
      do_petition();
      break;
    case FINISH_UNSIGN:
      finish_unsign(name, pcc, from_owner);
      break;
    case DISPLAY_PETITION_RESULTS:
      display_petition_results(p_item);
      break;
    case DISPLAY_PETITION_INFO:
      display_petition_info(which_language, p_item, just_checking, 
			    new_pet, do_header);
      break;
    case DISPLAY_PETITION_TEXT:
      display_petition_text(p_item, just_checking, which_language);
      break;
    case START_PETITION_DISPLAYS:
      start_petition_displays(p_item, new_pet, which_language);
      break;
    case BIG_FINISH:
      big_petition_finish(0);
      break;
    default:
      /* impossible */
      break;
    }
}
extern YESorNO new_signer;
static void bounce_info_error(void);
static OKorNOT check(RTYPE rtype);
static int check_form(int cc);
static OKorNOT check_sig_confirm(void);
static void display_howto(LANGUAGE);
static void display_priv_type(ITEM_INFO *);
static void display_remove(ITEM_INFO *, LANGUAGE);
static int explain_form(LANGUAGE);
static void list_petitions(void);
extern void leave();  /* in petition.c */
static int parse_vote(int * cc, char * *, int vote_item);
static void remove_signer(int reports);
static void report_vote(void);
extern void too_late();  /* in petition.c */
extern char (*answer)[FIELD_LEN + 1];  /* in form.c */
extern int no_petitions;
extern int no_publics;
extern int no_privates;
static int * pet_vote;
/************************************************************/
void
big_petition_finish(int exit_code)
{
  printf("\n  ================     DEBUT DU MESSAGE RECU    ===================\n\n");
  dump_message(stdout, NO, NO, NO);
  printf("\n\n  =====================    FIN DU MESSAGE   =======================\n");
  finish(exit_code);
}
/*******************************************************/
static void
bounce_petition_error(int whom)
{
  leave();
  /* This call to send splits the process and comes
     back the parent with the child's stdin sucking
     from this stdout */
  /* generate a message header on stdout */
  gen_header(whom, "Erreur:", YES);
  printf("\nEn r�ponse � votre message qui commen�ait par:\n");
  /*  print_tokens(YES); */
  print_first_line();
  printf("\n----\n");
  printf("%s", error_msg);
  if (list != NULL && list[0] != '\0')
    {
      if ((whom & OWNER && whom != OWNER) 
	 || (whom & APPROVAL && whom != APPROVAL))
	{
	  printf("\nCe message a aussi �t� envoy� � owner-%s", list);
	  if (whereami != NULL && whereami[0] != '\0')
	    printf("@%s\n", whereami);
	}
    }
  if (whom & ADMIN)
    {
      printf("\nCe message a aussi �t� envoy� � owner-majordomo");
      if (whereami != NULL && whereami[0] != '\0')
	printf("@%s\n", whereami);
    }
#ifdef EVOTE_ERRORS
  if (whom & DEVELOPER)
    printf("\nCe message a aussi �t� envoy� � %s.\n", 
	   EVOTE_ERRORS);
#endif
  big_finish(0);
}
/************************************************************
 *    Checks for a confirm file from this voter with the
 *    right key.  confirm_this.key has the key from the
 *    subject line.  confirm_this.status = CHECK when
 *    called.  Returns VERIFIED or sends an error message.
 *    If VERIFIED, it has also switched stdin to be the
 *    confirmed message.
 ************************************************************/
OKorNOT
check_sig_confirm(void)
{
  FILE *fp;
  char buf[500];
  char * file_key;
  YESorNO cc;
  CONFIRM_WHAT what = FOR_SIG;
  switch (cc = read_confirm(&what, &file_key, &fp))
    {
    case YES:
      return OK;
      break;
    case NO:
    case MAYBE:
      sprintf(error_msg, "\nV�tre message sur:\n\n\t%s\n\nressemble � une r�ponse de confirmation pour \"%s\" \navec la cl� de confirmation \"%s\".  Mais nous ne vous avons\npas donn� ce num�ro pour \"%s\".\n", original_subject, subject, confirm_this.key, subject);
      if (cc == NO)
	{
	  bounce_info_error();
	  break;
	}
      /* case MAYBE: found a file_key but wrong one */
      sprintf(buf, "Confirmez: %s ", file_key);
      gen_header(SENDER, buf, NO);
      printf(error_msg);
      printf("\nCependant, vous avez la clef \"%s\" en attente de confirmation de\n\"%s\".\n\nRemarquez bien qu'eVote ne garde que votre plus r�cent message. Ce\nqui suit est le message attendant d'�tre confirm�. Pour le confirmer,\nr�pondez � ce message.\n\nSi vous d�sirez envoyer un nouveau message pour \"%s\", \nessayez � nouveau mais ne mettez pas \"Confirmez: %s\" dans le sujet.\n",
	     file_key, subject, subject, confirm_this.key);
      printf("\n\n ----- VOTRE MESSAGE EN INSTANCE DE CONFIRMATION -----\n\n");
      while (fgets(buf, 400, fp))
	fputs(buf, stdout);
      fclose(fp);
      big_petition_finish(0);
      break;
    default:
      return NOT_OK;  /* never happens */
    }
  /* Never happens */
  return NOT_OK;
}
/************************************************************
 *  Returns the last cc of get_token or -1 on error
 *************************************************************/
static int
collect_votes(void)
{
  int no = 0;
  int end;
  int i, cc;
  YESorNO some_vote = NO;
  YESorNO no_good = NO;
  char *answer;
  char local_token[100];
  if ((pet_vote = malloc(sizeof(int) * no_pet_votes))
     == NULL)
    {
      sprintf(error_msg,"\nPas assez de ressources syst�mes sur %s pour l'instant.\nVeuillez renvoyer votre message plus tard.\n", whereami);
      return -1;
    }
  for (no = 0; no < no_pet_votes; no++)
    pet_vote[no] = UNKNOWN;
  while (1)
    {
      no_good = NO;
      end = strlen(token)-1;
      if (cc == EOF || end == 0 
	 || (token[end] != ':' && token[end] != '.' && token[end] != '-'
	     && token[end] != ','))
	{
	  no_good = YES;
	}
      strcpy(local_token, token);
      local_token[end] = '\0';
      if (no_good || (no = atoi(local_token)) == 0 )
	{
	  if (!some_vote)
	    {							
	      sprintf(error_msg,"\neVote s'attend � une r�ponse de vote du genre \"1. Oui\" ici.\n");
	      return -1;
	    }
	  for (i = 0; i < no_pet_votes; i++)
	    {
	      if (pet_vote[i] == UNKNOWN)
		pet_vote[i] = 0;
	    }
	  return cc;
	}
      if (no > no_pet_votes)
	{
	  sprintf(error_msg,"Il y a seulement %d questions dans ce scrutin.\n",
		  no_pet_votes);
	  return -1;
	}
      if (pet_vote[no-1] != UNKNOWN)
	{
	  sprintf(error_msg,"\nVous ne pouvez voter qu'une fois sur la question %d.\n",
		  no);
	  return -1;
	}
      if ((pet_vote[no-1] = parse_vote(&cc, &answer, no-1)) == UNKNOWN)
	{ /* parse_vote puts an error_msg in */
	  if ((p_first_vote_item+no)->eVote.min == -1
	     && (p_first_vote_item+no)->eVote.max == 1)
	    {
	      sprintf(error_msg, "\neVote Votre r�ponse sur la question %d n'est pas comprise.\nVous avez r�pondu \"%s\". Il faut r�pondre\n\"Oui\", \"Non\", ou \"Je ne sais pas\"\n", 
		      no, answer);
	    }
	  else
	    {
	      sprintf(error_msg, "\neVote Votre r�ponse sur la question %d n'est pas comprise.\nVous avez r�pondu \"%s\".  Il faut r�pondre un nombre entre %d et %d.\n",
		      no, answer,
		      (p_first_vote_item+no)->eVote.min,
		      (p_first_vote_item+no)->eVote.max);
	    }
	  return -1;
	}
      some_vote = YES;
      cc = get_token();
    }
  return cc;
}
/********************************************************
 *     Displays information about how to sign.
 *********************************************************/
void
display_howto(LANGUAGE which)
{
  printf("\n");
  highlight((no_pet_votes ? "POUR VOTER": "POUR SIGNER CETTE P�TITION"));
  printf("\n1.  Envoyez un message �:");
  printf("\n\n\teVote@%s", whereami);
  printf("\n");
  printf("\n2.  Votre sujet doit �tre:");
  printf("\n\n\t%s", get_trans(subject, which, YES));
  printf("\n");
  if (!form_exists && !no_pet_votes)
    {
      printf("\n3.  Si votre message est vide, seule votre adresse e-mail sera enregistr�e.\n\n    Sinon\n\n    Entrez vos nom, affiliation et pays.\n");
    }
  else  /* also explains votes */
    {
      explain_form(which);
    }
  printf("\n4  Si votre message comporte un fichier de signature ou un autre \n   texte que vous voulez exclure, vous avez juste � �crire sur une \n   ligne la commande \"fin\".\n");
}
/************************************************************/
void
display_petition_info(LANGUAGE which_language,
			   ITEM_INFO * p_item, YESorNO just_checking, 
                           YESorNO new_pet, YESorNO do_header)
{
  int reports;
  int fields;   /* also in no_of_fields, external */
  if (p_item == NULL)
    {
      no_pet_message();
      return;
    }
  if (do_header)
    {
      char head[200];
      char uppers[100];
      printf("\n");
      sprintf(head, "INSTRUCTIONS POUR %s", 
	      raiseup(uppers, tongue[which_language].name));
      highlight(head);
    }
  start_petition_displays(p_item, new_pet, which_language);
  display_petition_text(p_item, just_checking, which_language);
  reports  = read_report_instructions(p_item);
  fields = read_form_template(p_item);
  display_howto(which_language);
    display_priv_type(p_item);
  display_remove(p_item, which_language);
  /*	if (just_checking || do_header || new_pet) */
  return;
  /*	big_petition_finish(0); */
}
/*******************************************************/
void
display_petition_results(ITEM_INFO *p_item)
{
  unsigned long voters;
  voters = get_mail_voters(p_item->dropping_id);
  printf("\n");
  highlight("R�SULTATS");
  if (no_pet_votes)
    {
      printf("\n%lu voteur%s particip� � ce scrutin.\n",
	     voters, (voters == 1L? " a" : "s ont"));
    }
  else
    printf("\nCette p�tition a recueilli %lu signature%s.\n",
	   voters, (voters == 1L? "" : "s"));
}
/*********************************************************
 *    Prints the petition text on stdout.
 **********************************************************/ 
void
display_petition_text(ITEM_INFO * p_item, YESorNO just_checking,
			   LANGUAGE which)
{
  FILE *fp;
  char fname[PATHLEN + 1];
  int ch;
  char * default_name;
  int tries = 0;
  /*  printf("\n"); */
  highlight((no_pet_votes ? "TEXTE SCRUTIN" : "TEXTE P�TITION"));
  default_name = pet_fname("text", p_item);
  sprintf(fname, "%s/%s", default_name, tongue[which].name);
  while ((fp = fopen(fname, "r")) == NULL)
    {
      sprintf(fname, "%s/%s", default_name, tongue[DEFAULT_LANGUAGE].name);
      if (++tries < 2)
	continue;
      printf("\n%s", file_error);
      printf("\n\t%s \n\n", fname);
      printf("pour lire le texte de la p�tition.\n\nVeuillez renvoyer ceci � %s.\n\n",
	     eVote_mail_to);
      fprintf(stderr,"%s\n       %s\n\n\npour lire le texte de cette p�tition.\n\n", 
	      file_error, fname);
      perror("");
      return;
    }
  if ((ch = fgetc(fp)) == EOF)
    {
      char er[] = "\n\nERREUR! Fichier vide: \n\n";
      char er2[] = "\n\nImpossible de lire le texte de la p�tition/scrutin.\n\n";
      printf("%s%s%sVeuillez renvoyer ceci � %s.\n\n", 
	     er, fname, er2, eVote_mail_to);
      fprintf(stderr,"%s%s%s", er, fname, er2);
      fclose(fp);
      return;
    }
  if (ch != '\n')
    putchar('\n');
  putchar(ch);
  while ((ch = fgetc(fp)) != EOF)
    {
      putchar(ch);
    }
  fclose(fp);
}
/*******************************************************
 *    Displays messages about the privacy type.
 *******************************************************/
void
display_priv_type(ITEM_INFO *p_item)
{
  switch (p_item->eVote.priv_type)
    {
    case PRIVATE:
    case IF_VOTED:
      printf("\n");
      highlight("P�TITION PRIV�E");
      printf("\nLes signatures pour cette p�tition, et les adresses email, ne sont\naccessibles que pour %s et pour l'administrateur syst�me\nde ce site. Ils sont seuls responsables de la mani�re dont l'information\nsera utilis�e.\n",
	     author_name);
      break;
    default:
      printf("\n");
      highlight((no_pet_votes ? "SCRUTIN PUBLIC" : "P�TITION PUBLIQUE"));
#ifdef ROSA
      if (same(subject,"Kopilli Ketzalli"))
	{
	  printf("\nCeci est une *p�tition publique*. Vos nom, pays et commentaires (mais pas\nvotre adresse e-mail), seront publi�s sur notre page Web.  A l'avenir, vos\namis pourront interroger ce syst�me (s'ils connaissent votre adresse e-mail)\net prendre connaissance de votre message de signature.\n\nNous nous r�servons le droit d'effacer toute contribution que nous\nconsid�rerions comme inappropri�e. Vous pouvez retirer votre contribution �\ntout instant. Cependant si votre signature reste pr�sente pendant plus d'une\nsemaine, vous pourrez toujours la retirer des donn�es �lectroniques mais il\nest alors possible qu'elle aie d�j� �t� comptabilis�e en Autriche.\n\nNous utilisons l'adresse e-mail pour nous assurer qu'il n'y � qu'une seule\nsignature par adresse, et pour rien d'autre.\n");
	}
      else
#endif
	{
	  printf("\nCeci est une p�tition *publique*. Les informations que vous donnez peuvent\n�tre lues par toute personne qui signera la p�tition et par tous les\nlecteurs de la liste de discussion %s@%s.\n", list, whereami);
	  printf("\nCe qui signifie que ces informations sont quasiment du domaine \npublic et qu'il ne peut y avoir aucun contr�le sur leur usage.\n");
	  printf("\nVotre adresse email peut cependant �tre vue par l'administrateur\nsyt�me de %s.\n\nVotre adresse email ne sert qu'� v�rifier qu'il n'y a qu'une\nsignature par adresse email et ne sera utilis�e pour rien d'autre.\n", whereami);
	}
    }
}
/*******************************************************/
void
display_remove(ITEM_INFO *p_item, LANGUAGE lang)
{
  /*  printf("\n"); */
  highlight((no_pet_votes ? "RETIRER VOTRE VOTE" : "POUR RETIRER VOTRE SIGNATURE"));
  printf("\nSi vous pensez avoir %s par erreur, vous\npouvez retirer votre %s:",
	 (no_pet_votes ? "vot�" : "sign�"),
	 (no_pet_votes ? "vot�" : "sign�"));
  printf("\n\n1.  Envoyez un message �:");
  printf("\n\n\teVote@%s", whereami);
  printf("\n\n2.  Votre sujet doit �tre:");
  printf("\n\n\t%s", get_trans(subject, lang, YES));
  printf("\n\n3.  Vous message doit �tre:");
  printf("\n\n\tretirer\n");
#ifdef ROSA
  if (same(subject,"Kopilli Ketzalli"))
    {
      printf("\nSi vous souhaitez retirer votre signature, faites-le vite. Apr�s une\nsemaine elle sera transmise � Vienne pour comptage.\n");
      printf("\n");
      highlight("BULLETIN");
      printf("\nPour recevoir le bulletin �lectronique des Yanakuikanahuak, veuillez:\n");
      printf("\n1.  Envoyez un message �:");
      printf("\n\n\tmajordomo@deliberate.com");
      printf("\n\n2.  La ligne de sujet n'a pas d'importance.");
      printf("\n\n3.  Vous message doit �tre:");
      printf("\n\n\tsubscribe");
      printf(" anahuak-en");
      printf("\n\tend");
      printf("\n\nIl y a aussi des bulletins en Espagnol (\"anahuak-es\"), en Allemand\n(\"anahuak-de\"), et en Anglais (\"anahuak-en\") auxquels vous pouvez aussi\nvous abonner.\n");
      printf("\n");
      highlight("DISCUSSIONS");
      printf("\nPour rejoindre la liste de discussion concernant Kopilli Ketzalli et son\nretour au Mexique:\n");
      printf("\n1.  Envoyez un message �:");
      printf("\n\n\tmajordomo@deliberate.com");
      printf("\n\n2.  La ligne de sujet n'a pas d'importance.");
      printf("\n\n3.  Vous message doit �tre:");
      printf("\n\n\tsubscribe");
      printf(" kopilli-en");
      printf("\n\tend");
      printf("\n\nIl y a aussi des discussions en Espagnol (\"kopilli-es\"), en Allemand\n(\"kopilli-de\"), et en Anglais (\"kopilli-en\") auxquelles vous pouvez aussi\nvous abonner.\n");
      printf("\n");
      highlight("BILINGUE?");
      printf("\nSi vous �crivez en Allemand, Espagnol ou Anglais aussi bien qu'en\nFran�ais et que vous voulez aidez en traduisant des documents pour\nYankuikanahuak:");
      printf("\n\n1.  Envoyez un message �:");
      printf("\n\n\tmajordomo@%s", whereami);
      printf("\n\n2.  La ligne de sujet n'a pas d'importance.");
      printf("\n\n3.  Vous message doit �tre:");
      printf("\n\n\tsubscribe");
      printf(" es");
      printf("\n\tend");
      printf("\n\npour rencontrer les traducteurs en espagnol. Il y a aussi des groupes\nde traduction pour l'allemand (\"de\") et pour l'anglais (\"en\") que vous\npourriez aussi bien rejoindre.\n");
      printf("\nVous pouvez envoyez plusieurs commandes � majordomo@deliberate.com dans\nun seul message:");
      printf("\n\n\tsubscribe");
      printf(" anahuak-en");
      printf("\n\tsubscribe");
      printf(" kopilli-en");
      printf("\n\tsubscribe");
      printf(" anahuak-es");
      printf("\n\tsubscribe");
      printf(" kopilli-es");
      printf("\n\tsubscribe");
      printf(" es");
      printf("\n\tend");
      printf("\n\nvous abonnera � cinq listes.\n");
    }
#endif
}
/*******************************************************
 *     Processes a message when it came into the system
 *     through the petition facility. When called:
 *     confirm_this.status = 
 *                           VERIFIED - this message confirmed
 *                           NOT_NEEDED - no confirm necessary
 *                           STARTING - this needs error or
 *                                      to be confirmed.
 *******************************************************/
void
do_petition(void)
{
  int cc;
  unsigned long  readers, voters;
  char vote_str[10] = "  - ";
  char result[10] = "  - ";
  short old_vote;
  int reports = 0;
  int fields;
  int i;
  void register_mexico(void);
  if (confirm_this.status != CHECK)
    {
      cc = fix_first_token();
      if (same(token,"eVote"))
	{
	  sprintf(error_msg,"\nPour utiliser les commandes d'eVote, vous devez �tre membre de la liste\n\n        %s@%s\n\net les commandes doivent �tre envoy�es directement � l'adresse de la liste.\n",
	      list, whereami);
	  bounce_error(SENDER);
	}
      for (i = 0; i < no_languages; i++)
	{
	  if (same(token, tongue[(LANGUAGE)i].remove))
	    {
	      remove_sig = YES;
	      break;
	    }
	}
    }
  if (!into_eVote && set_up_eVote() != OK)  /* Identifies the list, and user */
    /* if we get this far, communication to the list is working
       the list exists and, if mail_voter != 0, we're into
       the database.  */
    {
      if (remove_sig)
	{
	  sprintf(error_msg, "\nVous n'avez pas particip� au vote �lectronique � %s.\n",
		  whereami);
	}
      enter_signer();
    }
  if (subject == NULL || subject[0] == '\0')
    {
      sprintf(error_msg, "\nVous devez donner un titre de p�tition comme sujet de votre message.\n");
      list_petitions();
    }
  if (current_action == READ_ONLY)
    {
      sprintf(error_msg,"\nVous ne pouver pas participer au vote.\nVous �tes membre en lecture seule de la liste %s.\n", list);
      bounce_petition_error(SENDER | OWNER);
    }
  if (!into_eVote)
    {
      copy_poll(YES); /* If dropping_id != 0, we have a polled
			 subject */
      into_eVote = YES;
    }
  if (confirm_this.status == CHECK)
    /* Confirm: found in subject */
    {
      check_sig_confirm();  /* places VERIFIED in status */
      /* it starts over with the waiting message */
      do_petition();
      /* or it generates an error message and quits */
    }  
  if (dropping_id == 0 || petition != YES 
     || (p_item_copy->eVote.type != TALLIEDY 
	 && p_item_copy->eVote.type != TIMESTAMP))
    {
      gen_header(SENDER, "ERREUR:", NO);
      no_pet_message();
    }
  /* Check for remove message and switch to correct language */
  for (i = 0; i < no_languages; i++)
    {
      if (same(token, tongue[(LANGUAGE)i].remove))
	{
	  remove_sig = YES;
	  if (!same(tongue[the_language].remove, tongue[(LANGUAGE)i].remove))
	    {
	      the_language = (LANGUAGE)i;
	      back_one_token();
	      translate(the_language, DO_PETITION, NULL, 
			MAYBE, MAYBE, MAYBE, NULL, NULL, MAYBE);
	    }
	  break;
	}
    }  
  if (error_msg[0] != '\0')  /* remove_sig and never eVoted */
    {
      bounce_info_error();
    }
  if (same(tongue[the_language].help, token) 
     || same(tongue[the_language].info, token))
    send_petition_info(SENDER, p_item_copy, NO, NO, NO);
  /* Switch to the right language ??? */
  for (i = 0; i < no_languages; i++)
    {
      if (same(token, tongue[(LANGUAGE)i].help)
	 || same(token, tongue[(LANGUAGE)i].info))
	{
	  the_language = (LANGUAGE)i;
	  send_petition_info(SENDER, p_item_copy, NO, NO, NO);
	}
    }
  if (p_item_copy->eVote.vstatus == CLOSED)
    {
      if (remove_sig == YES)
	{
	  sprintf(error_msg,"\nIl est trop tard pour retirer votre %s:\n\n\t%s\n\n%s.\n", 
		  (no_pet_votes ? "votes" : "signature de") ,
		  original_subject, 
		  (no_pet_votes ? "votes" : "signature de") );
	}
      else
	{
	  sprintf(error_msg,"\nIl est trop tard pour %s sur:\n\n\t%s\n\n%s est close.\n",
		  (no_pet_votes ? "voter" : "signer la p�tition"),
		  original_subject,
		  (no_pet_votes ? "voter" : "signer la p�tition"));
	}
      too_late();
    }
  reports  = read_report_instructions(p_item_copy);
  fields = read_form_template(p_item_copy);
  /* remove_sig is set in strip_subject() if [R] in subject
     line -- good for javascript */
  if (remove_sig == YES)
    {
      remove_signer(reports); /* collects confirm */
    }
  if (no_pet_votes && (cc = collect_votes()) == -1)
    bounce_info_error(); 
  if (form_exists)         /* check that the user filled in the form */
    {
      cc = check_form(cc);
    }
  if (no_pet_votes || form_exists)
    {
      set_sig_start(cc);
    }
  switch (send_vote(p_item_copy, 1, &old_vote))
    {
    case FAILURE:
      sprintf(error_msg, "\nProbl�me syst�me sur %s: Votre %s n'a pas �t� enregistr�e.\n", 
	      whereami,
	      (no_pet_votes ? "vote" : "signature"));
      bounce_petition_error(SENDER | ADMIN | OWNER);
      break;
    case NO_MODS:
      sprintf(error_msg,"\nIl est trop tard pour %s sur:\n\n\t%s\n\n%s.\n",
	      (no_pet_votes
	       ? (confirm_this.status == VERIFIED ? "confirmer v�tre vote" : "voter")
	       : (confirm_this.status == VERIFIED ? "confirmer v�tre signature" 
		  : "signer la p�tition")),
	      original_subject,
	      (no_pet_votes ? "Le scrutin est ferm�" : "La p�tition est finie"));
      list_petitions();   /* never returns */
      break;
    case NO_CHANGE:
      sprintf(error_msg, "\nMerci. Vous aviez d�j� %s sur\n\n\t%s\n\nVous ne pouvez pas %s deux fois.\n", 
	      (no_pet_votes ? "vot�" : "sign� la p�tition"),
	      original_subject, 
	      (no_pet_votes ? "voter" : "signer la p�tition"));
      bounce_info_error();
      break;
    case GOOD:  
      get_mail_stats(p_item_copy->dropping_id,
		     &readers, vote_str, result);
      if (confirm_this.status != STARTING)  /* NOT_NEEDED or VERIFIED */
	{
	  store_signature(p_item_copy);
	  if (no_pet_votes)
	    vote_the_petition();
	  gen_header(SENDER, "eVote Rcpt:", NO);
	}
      else
	{
	  char buf[100];
	  check(send_vote(p_item_copy, READ, &old_vote));
	  get_mail_stats(p_item_copy->dropping_id,
			 &readers, vote_str, result);
	  if (collect_confirm(FOR_SIG) == NULL)  /* sets key */
	    {
	      bounce_petition_error(SENDER | OWNER | ADMIN);
	    }
	  sprintf(buf, "Confirmez: %s", confirm_this.key);
	  gen_header(SENDER, buf, NO);
	  printf("\n%s sur\n\"%s\" %s.",
	     (no_pet_votes == 0 ? "Votre signature" : (no_pet_votes == 1 ?
				"Votre vote" : "Vos votes")),
	     original_subject,
	     (no_pet_votes == 0 ? " n'a pas �t� encore
enregistr�e" : (no_pet_votes == 1 ? " n'a pas �t� encore enregistr�" : "
n'ont pas �t� encore enregistr�s")));
	  printf("\n\nPour v�rifier que votre message provient bien de vous, veuillez confirmer \nce message par \"renvoi\" (Reply-to). Il ne vous sera pas n�cessaire de \ntaper quoi que ce soit dans ce message de confirmation. eVote ne lira que \nla ligne sujet et votre adresse.  Tout le reste sera ignor�. C'est\nn�cessaire pour assurer l'int�grit� de toutes les p�titions eVote.");
	  printf("\n\nIl faudra envoyer votre renvoi avant que la p�tition ne finisse ou\nvotre signature ne sera pas compt�e.\n");
	}
      printf("\nMerci de %s sur\n\n\t%s", 
	     (no_pet_votes == 0 ? " votre signature" : (no_pet_votes == 1 ?
				"votre vote" : "vos votes")),
	     original_subject);
      voters= atoul(vote_str);
      printf("\n\nA ce jour, %lu %s %s.\n", voters,
	     (voters == 1? "personne a" : "personnes ont"),
	     (no_pet_votes ? "vot�" : "sign� cette p�tition"));
      if (confirm_this.status == STARTING)
	{
	  printf("\nApr�s avoir r�pondu � ce message, ce qui suit sera enregistr�:\n\n");
	}
      highlight(no_pet_votes == 0 ? "VOTRE SIGNATURE" : 
		 (no_pet_votes == 1 ? "VOTRE VOTE" : "VOS VOTES"));
      if (no_pet_votes)
	{
	  printf("\nVos votes %s enregistr�s comme:\n", (confirm_this.status == STARTING ? "seront" : "sont"));
	  report_vote();
	  printf("\n");
	  highlight("VOS COMMENTAIRES");
	}
      if (confirm_this.status == STARTING)
	{
	  printf("\n%s comme:", (no_pet_votes ? "Vos commentaires seront enregistr�s" :
							 "votre signature sera enregistr�e"));
	  printf("\n\n - - - - d�but de signature - - - - \n");
	  cc = write_signature(stdout, NO, NO);
	  printf("\n - - - - fin de signature - - - - \n");
	}
      else
	{
	  printf("\n%s comme:", (no_pet_votes ? "Vos commentaires sont enregistr�s" :
							 "votre signature est enregistr�e"));
	  cc = write_signature(stdout, NO, YES);
	  printf("\n - - - - fin de signature - - - - ");
	}
      if (cc != OK)
	{
	  sprintf(error_msg, "\neVote ne peut stocker votre %s pour %s maintenant.\n\nVeuillez r�essayer plus tard\n", 
		  (no_pet_votes? "vote" : "signature"), original_subject);
	  bounce_petition_error(SENDER | OWNER | ADMIN);
	}
      display_priv_type(p_item_copy);
      if (confirm_this.status != STARTING)
	display_remove(p_item_copy, the_language);
      display_petition_text(p_item_copy, NO, the_language);
      break;
    case NOT_ALLOWED:
      sprintf(error_msg, "\nHow can a petition signer be on vacation?");
      bounce_petition_error(SENDER | OWNER | ADMIN);
    default:
      /* impossible */
      break;
    }
  /*   For testing the timeout
       {
       long i;
       for (i = 0; i >= 0; i++)
       fprintf(stderr, "%d", i);
       }
  */
  if (reports > 0 && confirm_this.status != STARTING)
    {
      big_petition_finish(-2);  /* This puts the end of the email message
				   on stdout and returns, doesn't finish
				   with an exit.  This is so that ship_reports
				   can fork and exec an ftp process and
				   close stdout in the parent and get the
				   message send */
      ship_reports(voters);
      exit(0);
    }
  else
    {
      big_petition_finish(0);
    }
}
/**********************************************************
 * Sends the list of open petitions to the sender.  
 **********************************************************/
void
list_petitions(void)
{
  gen_header(SENDER, "Re:", YES);
  if (error_msg[0] != '\0')
    {
      printf("%s", error_msg);
    }
  printf("\nLes p�titions actuellement disponibles sur %s sont list�es ci-dessous.", 
	 whereami);
  printf("\n\nPour avoir des informations sur une p�tition particuli�re:");
  printf("\n\n1.  Envoyez un message �:");
  printf("\n\n\teVote@%s\n\n", whereami);
  printf("2.  La ligne de sujet doit correspondre au titre de la p�tition.");
  printf("\n\n3.  Vous message doit �tre: ");
  printf("\n\n\tinfo");
  printf("\n\n    pour recevoir le texte de la p�tition et les instructions de signature.\n");
  lowlight("Priv�e");
  printf("\nDans la liste ci-dessous, une p�tition:\n\nPUBLIQUE  permet � n'importe quel abonn� � la liste \n          %s@%s \n          de lire les signatures appos�es sur la p�tition mais pas les\n          adresses email des signataires.  De m�me, toute personne qui\n          signe cette p�tition peut lire les signatures. Il n'y\n          a aucun contr�le de l'usage de votre signature.  Votre adresse\n          email ne sert qu'� v�rifier qu'il n'y a qu'une signature par\n          adresse et n'est pas utilis�e pour autre chose.\n\nPRIV�E    ne permet qu'� l'initiateur de la p�tition et � l'administrateur\n          de %s de connaitre les signatures.  Les adresses email\n          sont � leur disposition aussi.\n",
	 list, whereami, whereami);
  if (no_petitions == -1)
    read_translations();
  if (no_publics == 0)
    printf("\nIl n'y a pas de p�tition PRIV�E.\n\n");
  else if (no_publics == 1)
    {
      printf("\nIl y a une p�tition PUBLIQUE:\n\n");
      print_petitions(YES);
    }
  else
    {
      printf("\nP�titions PUBLIQUES:\n\n");
      print_petitions(YES);
    }
  if (no_privates == 0)
    printf("\nIl n'y a pas de p�tition PRIV�E.\n\n");
  else if (no_privates == 1)
    {
      printf("\nIl y a une p�tition PRIV�E:\n\n");
      print_petitions(NO);
    }
  else
    {
      printf("\nP�titions PRIV�ES:\n\n");
      print_petitions(NO);
    }
  leave();
  big_petition_finish(0);
}
/************************************************************/
void
no_pet_message(void)
{
  printf("\nIl n'y a pas de p�tition associ�e au sujet \"%s\".\n\nVeuillez v�rifier la ligne de sujet de votre message.\n", 
	     original_subject);
  list_petitions();
}
/*************************************************************/
static int
parse_vote(int * pcc, char **ans, int item_offset)
{
  static char answer[200];
  char delimiter[2] = " ";
  int cc, i, j;
  int vote;

  answer[0] = '\0';
  *ans = answer;
  while (1)
    {
      cc = get_token();
      strcat(answer, token);
      if (cc == EOF || cc == '\n')
	break;
      delimiter[0] = cc;
      strcat(answer, delimiter);
    }
  *pcc = cc;
  for (i = 0, j = 0; answer[i]; i++)
    {
      if (answer[i] == '\'' || answer[i] == '"' || answer[i] == '.'
	 || answer[i] == ',' || answer[i] == ';' || answer[i] == ':'
	 || answer[i] == '!')
	continue;
      answer[j++] = answer[i];
    }
  answer[j] = '\0';
  if ((p_first_vote_item+item_offset)->eVote.min == -1
     && (p_first_vote_item+item_offset)->eVote.max == 1)
    {
      for (i = 0; i < no_of_answers; i++)
	{
	  if (same(answer_list[i].str, answer))
	    return answer_list[i].answer;
	}
    }
  if (sscanf(answer,"%d", &vote) != 1)
    return UNKNOWN;
  if ((p_first_vote_item+item_offset)->eVote.min <= vote
     || (p_first_vote_item+item_offset)->eVote.max >= vote)
    return vote;
  return UNKNOWN;
}
/************************************************************/
static OKorNOT
check(RTYPE rtype)
{
  switch (rtype)
    {
    case FAILURE:
      sprintf(error_msg, "\nDifficult�s du syst�me � %s: Votre %s n'a pas �t� enlev�%s.\n", 
	      whereami, (no_pet_votes ? "vote" : "signature"), (no_pet_votes ? "" : "e"));
      return NOT_OK;
      break;
    case NO_MODS:
      sprintf(error_msg, "\nErreur de programmation dans pet_out.c:check.\n");
      return NOT_OK;
      break;
    case NO_CHANGE:  /* first vote makes all others 0 */
    case GOOD:  
      return OK;
    default:
      /* impossible */
      return NOT_OK;
      break;
    }
}
/************************************************************/
static void
remove_signer(int reports)
{
  unsigned long  readers, voters;
  char vote_str[10] = "  - ";
  char result[10] = "  - ";
  int cc;
  if (reports > 0)
    {
      get_mail_stats(p_item_copy->dropping_id,
		     &readers, vote_str, result);
      voters= atoul(vote_str) -1;
    }
  finish_unsign(from, &cc, NO);
  if (cc != OK)
    bounce_petition_error(SENDER);
  if (reports > 0)
    {
      big_petition_finish(-2);
      ship_reports(voters);
      exit(0);
    }
  else
    {
      big_petition_finish(0);
    }
}
/**********************************************************/
void
start_petition_displays(ITEM_INFO * p_item, YESorNO new_pet,
			     LANGUAGE which)
{
  unsigned long voters;
  printf("\nLe %s un scrutin a �t� initi� sur:\n\n\t \"%s\"\n",
	 date_str(p_item->eVote.open), get_trans(subject, which, YES));
  if (new_pet)
    voters = 0L;
  else
    voters = get_mail_voters(p_item->dropping_id);
  if (p_item->eVote.vstatus == CLOSED) 
    {
      printf("\nCette %s � �t� lanc�e le %s mais est maintenant close depuis\nle %s",
	     (no_pet_votes ? "scrutin" : "p�tition"),
	     date_str(p_item->eVote.open), date_str(p_item->eVote.close));
      printf("\n%lu %s%s �t� recueilli%s%s.\n",
	     voters, (no_pet_votes ? "vote" : "signature"),
	     (voters == 1L ? " a" : "s ont"),
	     (no_pet_votes ? "" : "e"),
	     (voters == 1L ? "" : "s"));
    }
  else if (!new_pet)
    {
      printf("\n%lu %s%s �t� recueilli%s%s jusqu'ici.\n",
	     voters, (no_pet_votes ? "vote" : "signature"),
	     (voters == 1L ? " a" : "s ont"),
	     (no_pet_votes ? "" : "e"),
	     (voters == 1L ? "" : "s"));
    }
}
/**********     FORMS  ******************/
/****************************************************
 *  called from check_form when the user doesn't
 *  fill it in properly.
 *****************************************************/
void
bounce_info_error(void)
{
  /* generate a message header on stdout */
  gen_header(SENDER, "Erreur:", NO);
  printf("\nEn r�ponse � votre message qui commen�ait par:\n");
  print_tokens(NO);
  /*  print_first_line(); */
  printf("%s", error_msg);
  error_msg[0] = '\0';
  if (dropping_id && item_info)
    {
	printf("\nLes instructions pour \"%s\" pourront peut-�tre vous aider.\n",
	       original_subject);
      send_petition_info(SENDER, NULL, NO, NO, NO);
    }  
  leave();
  printf("\n\nVotre message complet suit:\n");
  big_finish(0);
}
/************************************************************
 *  This checks the answers to the form that are supplied by the 
 *  signer.  If there is an error, it bounces the message.
 *  The int returned and cc are the delimiter for the
 *  current token.
 ************************************************************/
static	YESorNO could_match(char * right_name, char * try_name, int len);
static	char *force_name(int i);
int
check_form(int cc)
{
  YESorNO possible;
  int len, missing_fields;
  char delimit[2];
  char name[FIELD_LEN +1];
  int i, j, l;
  YESorNO nines, xes;
  extern char (*answer)[FIELD_LEN + 1];  /* in form.c */
  if (no_of_fields == 0)
    return cc;
  if ((answer = (char (*)[FIELD_LEN + 1])calloc((FIELD_LEN +1), no_of_fields)) 
     == NULL)
    {
      sprintf(error_msg, "\nPas assez de ressources syst�me pour traiter maintenant votre %s\npour \"%s\".  Veuillez r�essayer plus tard.\n",
	      (no_pet_votes? "vote" : "signature"), original_subject);
      bounce_petition_error(ADMIN | OWNER | SENDER);
    }
  while (cc != EOF)
    {
      name[0] =  '\0';
      do
	{
	  if (name[0] != '\0')
	    {
	      possible = NO;
	      sprintf(delimit, "%c", cc);
	      strcat(name, delimit);
	      cc = get_token();
	    }
	  strncat(name, token, FIELD_LEN);
	  if (name[strlen(name) - 1] == ':')
	    name[strlen(name) -1] = '\0';
	  possible = NO;
	  len = strlen(name);
	  for (i = 0; i < no_of_fields; i++)
	    {
	      if (could_match(field[i].name[the_language], name, len))
		{
		  possible = YES;
		  break;
		}
	    }
	  if (possible == NO)  /* try other languages */
	    {
	      for (i = 0; i < no_of_fields; i++)
		{
		  for (l = 0; l < no_languages; l++)
		    {
		      if ((LANGUAGE)l == the_language
			 || field[i].name[(LANGUAGE)l][0] == '\0')
			continue;
		      if (could_match(field[i].name[l], name, len))
			{
			  possible = YES;
			  break;
			}
		    }
		  if (possible)
		    break;
		}
	    }
	  if (!possible)
	    break;
	}
      while (cc != '\n' && cc != EOF 
	    && token[strlen(token) -1] != ':');
      if (possible == NO  && !is_comment(name))
	{
	  /* unrecognized line coming in */
	  for (i = 0; i < no_of_fields; i++)
	    {
	      if (field[i].required == YES && answer[i][0] == '\0'
		 && !same(field[i].name[the_language], 
			  tongue[the_language].comment))
		{
		  sprintf(error_msg, 
			  "\nIl faut commencer par donner votre \"%s:\"\navant d'ajouter un commentaire pour signer la p�tition\n\n        \"%s\"\n", force_name(i),
			  original_subject);
		  bounce_info_error();
		}
	    }
	  /* assume it's an unmarked comment at the end */
	  cc = back_one_token();
	  found_comment = MAYBE;
	  break;
	}
      for (i = 0; i < no_of_fields; i++) 
	{
	  if (same(field[i].name[the_language], name))
	    {
	      break;
	    }
	}
      if (i == no_of_fields)  /* try other languages */
	{
	  for (i = 0; i < no_of_fields; i++)
	    {
	      for (l = 0; l < no_languages; l++)
		{
		  if ((LANGUAGE)l == the_language
		     || field[i].name[(LANGUAGE)l][0] == '\0')
		    continue;
		  if (could_match(field[i].name[l], name, len))
		    {
		      break;
		    }
		}
	      if (l < no_languages)
		break;
	    }
	}
      if (i >= no_of_fields)
	{
	  if (is_comment(name))
	    {
	      cc = back_one_token(); 
	      found_comment = YES;
	      break;
	    }
	  else
	    {
	      sprintf(error_msg, "\n\"%s\" ne fait pas partie du formulaire pour:\n\n\t\"%s\".\n",
		      name, original_subject);
	      bounce_info_error();
	    }
	}
      if (answer[i][0] != '\0')
	{
	  sprintf(error_msg, "\nVeuillez ne donner %s qu'une seule fois lorsque vous signez\nla p�tition sur \"%s\".\n", force_name(i), original_subject);
	  bounce_info_error();
	}
      if (same(field[i].name[the_language], tongue[the_language].comment) )
	{
	  back_one_token();
	  found_comment = YES;
	  break;
	}
      while (cc != '\n' && cc != EOF)
	{
	  cc = get_token();
	  if (strlen(token) + strlen(answer[i]) + 2 > FIELD_LEN)
	    {
	      sprintf(error_msg, "\nVeuillez limiter votre r�ponse pour \"%s:\" � %d caract�res.\n", field[i].name[the_language], FIELD_LEN);
	      bounce_info_error();
	    }
	  strcat(answer[i], token);
	  if (cc != '\n' && cc != EOF)
	    {
	      sprintf(delimit, "%c", cc);
	      strcat(answer[i], delimit);
	    }
	}
      if (cc != EOF)
	cc = get_token();
    }
  /*  The form has been read to the EOF or to a comment: field */
  for (i = 0; i < no_of_fields; i++)
    {
      if (same("email", field[i].name[ENGLISH]) 
	 && answer[i][0] != '\0'
	 && !same(answer[i], "Non") && !same(answer[i], "Non"))
	{
	  give_email = YES;
	}
      if ((field[i].required == YES && answer[i][0] == '\0')
	 && (!same(field[i].name[the_language], tongue[the_language].comment) 
	     || found_comment == NO))
	{
	  missing_fields = 1;
	  for (j = i+1; j < no_of_fields; j++)
	    {
	      if ((field[j].required == YES && answer[j][0] == '\0')
		 && (!same(field[j].name[the_language],
			   tongue[the_language].comment) 
		     || found_comment == NO))
		missing_fields++;
	    }
	  sprintf(error_msg, "\nPour %s sur\n\n\t\"%s\"\n\nvous devez donner votre \"%s\"", 
		  (no_pet_votes ? "voter" : "signer la p�tition"),
		  original_subject, force_name(i));
	  if (missing_fields == 1)
	    strcat(error_msg, ".\n");
	  else
	    {
	      for (j = i+1; j < no_of_fields; j++)
		{
		  if ((field[j].required == YES && answer[j][0] == '\0')
		     && (!same(field[j].name[the_language],
			       tongue[the_language].comment) 
			 || found_comment == NO))
		    {
		      switch (--missing_fields)
			{
			case 0:
			  break;
			case 1:
			  strcat(error_msg, " et ");
			  strcat(error_msg, force_name(j));
			  strcat(error_msg, ".\n");
			  break;
			default:
			  strcat(error_msg, ", ");
			  strcat(error_msg, force_name(j));
			  break;
			}
		    }
		}
	    }
	  bounce_info_error();
	}
      if (field[i].required == YES && check_format(i) != OK)
	{
	  char instruction[400];
	  sprintf(instruction, "\nlorsque vous entrez votre \"%s,\" veuillez vous\nassurer que votre r�ponse est de la forme:\n\n        %s: %s\n\n", field[i].name[the_language], field[i].name[the_language], field[i].format);
	  nines = NO;
	  xes = NO;
	  for (j=0; field[i].format[j]; j++)
	    {
	      if (field[i].format[j] == '9')
		nines = YES;
	      if (field[i].format[j] == 'X')
		xes = YES;
	    }
	  if (nines == NO)
	    {
	      sprintf(error_msg,"%so� vous remplacez tous les X par les lettres correctes.\n", instruction);
	    }
	  else if (xes == NO)
	    {
	      sprintf(error_msg,"%so� vous remplacez tous les 9 par les valeurs correctes.\n", instruction);
	    }
	  else
	    {
	      sprintf(error_msg,"%so� vous remplacez tous les 9 par les valeurs correctes et\ntous les X par les lettres correctes.\n", instruction);
	    }
	  bounce_info_error();
	}
    }
  return cc;
}
/************************************************************/
static YESorNO
could_match(char * right_name, char * try_name, int len)
{
  if (strNcmp(right_name, try_name, len) == (unsigned)0
     && (len == (int)strlen(right_name) || right_name[len] == ' '))
    {
      if (len == (int)strlen(right_name))
	{
	  if (right_name[len] == '\0' && token[strlen(token)-1] == ':')
	    return YES;
	  else
	    return NO;
	}
      return YES;
    }
  return NO;
}
/************************************************************/
static char
*force_name(int j)
{
  return (field[j].name[the_language][0] == '\0'?
	  field[j].name[default_language] :
	  field[j].name[the_language]);
}
/************************************************************
 *  Called from display_howto when there is a form.
 *  Returns the number of notes printed.
 ************************************************************/
int
explain_form(LANGUAGE which)
{
  int i, j;
  YESorNO numbers = NO;
  YESorNO letters = NO;
  int len;
  int notes = 0;
  int notes_printed = 0;
  int last_required = -1;
  int required = 0;
  int last_optional = -1;
  int optional = 0;
  YESorNO comment = NO, name_field = NO;
  LANGUAGE do_this = which;
  printf("\n3.  Ecrivez votre message comme ceci s'il vous plait: \n");
  printf("\n--- couper ici --- \n");
  for (i = 1; i <= no_pet_votes; i++)
    {
      printf("\n%d. Votre vote", i);
    }
  if (no_of_fields > 0)
    {
      if (field[0].name[which][0] == '\0')
	do_this = DEFAULT_LANGUAGE;
      for (i = 0; i < no_of_fields; i++)
	{
	  if (strNcmp(field[i].name[do_this],
		     tongue[do_this].comment, 
		     strlen(tongue[do_this].comment)) == 0)
	    comment = YES;
	  if (strNcmp(field[i].name[do_this],
		     "nom", 4) == 0)
	    name_field = YES;
	  printf("\n%s: %s",  field[i].name[do_this], field[i].format);
	  if (field[i].required)
	    {
	      last_required = i;
	      required++;
	    }
	  else
	    {
	      last_optional = i;
	      optional++;
	    }
	  for (j = 0; field[i].format[j]; j++)
	    {
	      if (field[i].format[j] == 'X')
		letters = YES;
	      if (field[i].format[j] == '9')
		numbers = YES;
	    }
	}
    }
  if (!comment && name_field)
    {
      printf("\nVous pouvez ajouter un %s ici.",
	     tongue[do_this].comment);
    }
  else if (!comment && !name_field)
    {
      printf("\nVous pouvez ajouter votre nom et/ou un %s ici.",
	     tongue[do_this].comment);
    }
  else if (comment && !name_field)
    {
      printf("\nVous pouvez ajouter votre nom ici.");
    }
  printf("\n\n--- couper ici --- \n");
  if (no_pet_votes)
    {
      printf("\n    o� chaque \"1.\", \"2.\", etc. repr�sente le num�ro de la question. Au lieu de\n    \"Votre vote\", vous pouvez r�pondre \"Oui\", \"Non\", ou \"Je ne sais pas.\"\n");
    } 
  notes = (required ? 1 : 0) + (optional ? 1 : 0)
    + letters + numbers;
  if (notes == 0)
    return notes;
  printf("\n      %s:\n", (notes > 1 ? "REMARQUES" : "REMARQUE"));
  if (required)
    {
      ++notes_printed;
      printf("\n      *  ");
      len = 39;
      printf("Vous devez donner %s pour ", (required == 1?"une r�ponse":"des r�ponses"));
      for (i = 0; i < no_of_fields; i++)
	{
	  if (field[i].required == NO)
	    continue;
	  if ((len += strlen(field[i].name[do_this]) + 5) >= 75) /* colon +
								   comma +
								   space */
	    {
	      printf("\n         ");
	      len = 10 + strlen(field[i].name[do_this]) + 5;
	    }
	  printf("\"%s:\"", field[i].name[do_this]);
	  if (i == last_required)
	    {
	      printf(".\n");
	      break;
	    }
	  else 
	    printf(", ");
	  if (required > 1 && i + 1 == last_required)
	    {
	      if ((len += 4) > 75)
		{
		  if (notes > 1)
		    {
		      printf("\n          ");
		    }
		  else
		    {
		      printf("\n      ");
		    }
		}
	      printf("et ");
	    }
	}
    }
  if (optional)
    {
      ++notes_printed;
      printf("\n      *  ");
      len = 39;
      printf("%s pour ",
	     (no_of_fields - required == 1?"Une r�ponse est facultative":"Des r�ponses sont facultatives"));
      for (i = 0; i < no_of_fields; i++)
	{
	  if (field[i].required == YES)
	    continue;
	  if ((len += strlen(field[i].name[do_this]) + 5) >= 75) /* colon +
								   comma +
								   space */
	    {
	      printf("\n         ");
	      len = 10 + strlen(field[i].name[do_this]) + 5;
	    }
	  printf("\"%s:\"", field[i].name[do_this]);
	  if (i == last_optional)
	    {
	      printf(".\n");
	      break;
	    }
	  else 
	    printf(", ");
	  if (optional > 1 && i + 1 == last_optional)
	    {
	      if ((len += 4) > 75)
		{
		  if (notes > 1)
		    {
		      printf("\n          ");
		    }
		  else
		    {
		      printf("\n      ");
		    }
		}
	      printf("et ");
	    }
	}
    }
  if (numbers)
    {
      printf("\n      *  ");
      printf("Veuillez remplacer les 9 par les valeurs correctes.\n");
    }
  if (letters)
    {
      printf("\n      *  ");
      printf("Veuillez remplacer les X par les lettres correctes.\n");
    }
  return notes_printed;
}
/************************************************************/
void
report_vote(void)
{
  int i;
  for (i = 0; i < no_pet_votes; i++)
    {
      printf("\n%d. %s", i+1, (pet_vote[i] == 0 ? "Je ne sais pas" : 
			       (pet_vote[i] == 1
				? "Oui" : "Non")));
    }
}
/*********************************************************
 *     Stores message text and email address.
 **********************************************************/ 
void
store_signature(ITEM_INFO * p_item)
{
  FILE * fp, *fp_tmp;
  char *fname;
  time_t when;
  if (now == 0L)
    {
      fprintf(stderr,"0 stamp in store_signature");
      time(&now);
    }
  push_time(p_item, now);
  when = pull_time(p_item);
  if (when == 0L)
    {
      fprintf(stderr,"0 from pull after push in store_signature");
    }
  fname = pet_signers_fname(p_item, now);
  if ((fp_tmp = lock_to_tmp(fname)) == NULL)
    {
      sprintf(error_msg,"\n\nERREUR! %s \n\nImpossible d'ouvrir un fichier verrou temporaire pour enregistrer\nvotre signature pour \"%s\".\nVeuillez r�essayer plus tard.\n",
	      fname, subject);
      bounce_petition_error(SENDER | OWNER | ADMIN);
    }
  if ((fp = fopen(fname, "a")) == NULL 
     || write_signature(fp, YES, YES) != OK)
    {
      sprintf(error_msg, "\neVote ne peut pas enregistrer votre signature pour %s maintenant.\n\nVeuillez r�essayer plus tard\n", subject);
      bounce_petition_error(SENDER | OWNER | ADMIN);
    }
  if (unlock_to_tmp(NO) != OK)
    {
      fprintf(stderr,"\nERREUR! Impossible d'effacer le fichier temporaire: %sT.\n",
	      fname);
    }			
  fclose(fp);
  chmod(fname, 00660);
  return;
}
/************************************************************/
static void
vote_the_petition(void)
{
  int i;
  char * tmp;
  short old_vote;
  tmp = subject;
  subject = extra_subject;
  copy_poll(YES);
  if (p_item_copy->eVote.vstatus == CLOSED)
    {
      sprintf(error_msg,"\nIl est trop tard pour %s sur:\n\n\t%s\n\n%s.\n",
	      (no_pet_votes
	       ? (confirm_this.status == VERIFIED ? "confirmer v�tre vote" : "voter")
	       : (confirm_this.status == VERIFIED ? "confirmer v�tre signature" 
		  : "signer la p�tition")),
	      original_subject,
	      (no_pet_votes ? "Le scrutin est ferm�" : "La p�tition est finie"));
      bounce_info_error();
    }
  for (i = 0; i < no_pet_votes; i++)
    {
      check(send_vote(p_item_copy + 1 + i, pet_vote[i], &old_vote));
    }
  subject = tmp;
  copy_poll(YES);
  petition = YES;
}
/************************************************************/
static void
finish_unsign(char * name, int * pcc, YESorNO from_owner)
{
  time_t when;
  short old_vote;
  char t_str[40];
  unsigned long  readers;
  char vote_str[10] = "  - ";
  char result[10] = "  - ";
  int i;
  if (have_i_voted(p_item_copy) == NO)
    {
      sprintf(error_msg,"\n%s n'a pas %s \"%s\".\n",
	      name, (no_pet_votes ? "vot� dans" : "sign�" ), subject);
      i_am_leaving();
      *pcc = NOT_OK;
      return;
    }
  if (confirm_this.status == STARTING && !from_owner)
    {
      char buf[1000];
      if (collect_confirm(FOR_SIG) == NULL)
	{
	  bounce_petition_error(SENDER | OWNER | ADMIN);
	}
      sprintf(buf, "Confirmez: %s", confirm_this.key);
      gen_header(SENDER, buf, NO);
      printf("Merci pour votre message \"d'effacement\" sur\n\n\t%s", 
	     original_subject);
      printf("\n\nPour v�rifier que votre message provient bien de vous, veuillez confirmer \nce message par \"renvoi\" (Reply-to). Il ne vous sera pas n�cessaire de \ntaper quoi que ce soit dans ce message de confirmation. eVote ne lira que \nla ligne sujet et votre adresse.  Tout le reste sera ignor�. C'est\nn�cessaire pour assurer l'int�grit� de toutes les p�titions eVote.");
      printf("\n\nIl faudra envoyer votre renvoi avant que la p�tition ne finisse ou\n%s.\n", (no_pet_votes ? "vos votes ne seront pas �ffac�s" : "votre signature ne sera pas �ffac�e"));
      big_petition_finish(0);
    }
  when = pull_time(p_item_copy);
  if ((*pcc = check(send_vote(p_item_copy, READ, &old_vote))) == NOT_OK)
    return;
  if (no_pet_votes)
    {
      char * hold;
      hold = subject;
      subject = extra_subject;
      copy_poll(YES);
      pet_vote = malloc(sizeof(int) * no_pet_votes);
      if (have_i_voted(p_item_copy+1) != NO)
	{
	  if (pet_vote != NULL)
	    {
	      for (i = 1; i <= no_pet_votes ; i++)
		{
		  get_mail_stats((p_item_copy+i)->dropping_id,
				 &readers, vote_str, result);
		  pet_vote[i-1] = atoi(vote_str);
		}
	    }
	}
      check(send_vote(p_item_copy + 1, READ, &old_vote));
      subject = hold;
      petition = YES;
      copy_poll(YES);
    }
  strcpy(t_str, time_str);  /* i_am_leaving wipes it out */
  i_am_leaving();
  if (current_action == SIGNER)
    {
      drop_mail_voter(lclist, YES);
    }
  if (!from_owner)
    {
      strcpy(time_str, t_str);  /* for message to user */
      gen_header(SENDER, "eVote Rcpt:", YES);
      time_str[0] = '\0';  /* wipe it out to prevent another i_am_leaving */
      printf("\n%s de \"%s\".",
	     (no_pet_votes? "Vos votes ont �t� enlev�s" : "Votre signature a �t� enlev�e"), original_subject);
      if (pet_vote != NULL)
	{
	  printf("\n\nLes votes qui ont �t� retir�s sont:\n");
	  report_vote();
	}
      printf("\n\nLe commentaire qui a �t� enlev� est:\n");
    }
  if (from_owner)
    {
      printf("\n%s a retir� sa signature.\nLe texte retir� �tait:\n", name);
    }
  drop_signature(mail_voter, stdout, when);
  *pcc = OK;
  return;
}
