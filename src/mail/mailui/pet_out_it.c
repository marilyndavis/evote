/* $Id: pet_out_it.c,v 1.5 2003/10/20 17:07:34 marilyndavis Exp $ */ 
/**********************************************************
 *  ../eVote/src/mail/mailui/pet_out.c
 *         has code that needs translating
 **********************************************************
  **********************************************************
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
static void vote_the_petition(void);
extern YESorNO give_email;
/*************************************************************
 *  Change this LOCAL_LANGUAGE from ENGLISH to your LANGUAGE *
 *                       \|/                               ***/
#define LOCAL_LANGUAGE ITALIANO
/*************************************************************
 *              |     After translating this file, change the 
 *              |     name of this function to be translate_xx 
 *              |     where xx is the flag for your language.
 **************\|/******************************************/
void
translate_it(LANGUAGE which_language, WHICH_FUNCTION function, 
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
    }
}
extern YESorNO new_signer = NO;
static void bounce_info_error(void);
static OKorNOT check(RTYPE rtype);
static int check_form(int cc);
static void display_howto(LANGUAGE);
static void display_priv_type(ITEM_INFO *);
static void display_remove(ITEM_INFO *, LANGUAGE);
static int explain_form(LANGUAGE);
static void list_petitions(void);
extern void leave();  /* in petition.c */
static int parse_vote(int * cc, char * *);
static void remove_signer(int reports);
static void report_vote(void);
extern void too_late();  /* in petition.c */
extern char (*answer)[FIELD_LEN + 1];  /* in form.c */
extern int no_petitions;
extern int no_publics;
extern int no_privates;
static int * pet_vote;
/************************************************************
 *   void big_petition_finish(int exit_code)
 ************************************************************/
void
big_petition_finish(int exit_code)
{
  printf("\n  ==================    INIZIO DEL MESSAGGIO  ====================\n\n");
  dump_message(stdout, NO, NO, NO);
  printf("\n\n  ===================    FINE DEL MESSAGGIO   ====================\n");
  finish(exit_code);
}
/*******************************************************
 *   void bounce_petition_error(int whom)
 ********************************************************/
static void
bounce_petition_error(int whom)
{
  int i;
  leave();
  /* This call to send splits the process and comes
     back the parent with the child's stdin sucking
     from this stdout */
  /* generate a message header on stdout */
  gen_header(whom, "Errore:", YES);
  printf("\nIn risposta al vostro messaggio che inizia con:\n");
  print_tokens(YES);
  printf("\n----\n");
  printf("%s", error_msg);
  if (list != NULL && list[0] != '\0')
    {
      if ((whom & OWNER && whom != OWNER) 
	 || (whom & APPROVAL && whom != APPROVAL))
	{
	  printf("\nQuesto messaggio viene anche spedito a owner-%s", list);
	  if (whereami != NULL && whereami[0] != '\0')
	    printf("@%s\n", whereami);
	}
    }
  if (whom & ADMIN)
    {
      printf("\nQuesto messaggio viene anche spedito a owner-majordomo");
      if (whereami != NULL && whereami[0] != '\0')
	printf("@%s\n", whereami);
    }
#ifdef EVOTE_ERRORS
  if (whom & DEVELOPER)
    printf("\nQuesto messaggio viene anche spedito a %s.\n", 
	   EVOTE_ERRORS);
#endif
  big_finish(0);
}
/************************************************************
 *    int collect_votes(void)
 *    returns the last cc of get_token or -1 on error
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
      sprintf(error_msg,"\n%s non e` in grado di ricevere al momento. Vi preghiamo di ri provare a mandare il vostro messaggio piu` tardi.\n", whereami);
      return -1;
    }
  for (no = 0; no < no_pet_votes; no++)
    pet_vote[no] = -10;
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
	      sprintf(error_msg,"eVote aspettatevi una risposta al voto come \"1. Si\" a questo punto.\n");
	      return -1;
	    }
	  for (i = 0; i < no_pet_votes; i++)
	    {
	      if (pet_vote[i] == -10)
		pet_vote[i] = 0;
	    }
	  return cc;
	}
      if (no > no_pet_votes)
	{
	  sprintf(error_msg,"Ci sono solo %d domande in questa petizione.\n",
		  no_pet_votes);
	  return -1;
	}
      if (pet_vote[no-1] != -10)
	{
	  sprintf(error_msg,"\nPotete votare solo una volta per la domanda %d.\n",
		  no);
	  return -1;
	}
      if ((pet_vote[no-1] = parse_vote(&cc, &answer)) == -10)
	{ /* parse_vote puts an error_msg in */
	  sprintf(error_msg, "\neVote non riconosce la vostra risposta alla domanda %d.\nAvete risposto \"%s\".  Dovete rispondere\n\"Si\", \"No\", oppure \"Non saprei.\"\n", 
		  no, answer);
	  return -1;
	}
      some_vote = YES;
      cc = get_token();
    }
  return cc;
}
/********************************************************
 *   void display_howto(LANGUAGE)
 *     Displays information about how to sign.
 *********************************************************/
void
display_howto(LANGUAGE which)
{
  printf("\n");
  highlight((no_pet_votes ? "PER VOTARE": "PER SOTTOSCRIVERE QUESTA PETIZIONE"));
  printf("\n1.  Mandare un messaggio a:");
  printf("\n\n\teVote@%s", whereami);
  printf("\n");
  printf("\n2.  Il vostro soggetto deve essere:");
  printf("\n\n\t%s", get_trans(subject, which, YES));
  printf("\n");
  if (!form_exists && !no_pet_votes)
    {
      printf("\n3.  Se il vostro messaggio e` vuoto, solo il vostro indirizzo email\n    sara` registrato oppure\n\n    Scrivete il vostro nome, affiliazione e localita`.\n");
    }
  else  /* also explains votes */
    {
      explain_form(which);
    }
  printf("\n4.  Se il vostro messaggio contiene un file con la firma, o qualsisasi\n    altro tipo di testo che non volete che sia incluso, aggiungete una\n    riga con la dicitura \"fine\".\n");
}
/************************************************************
 *    	display_petition_info(LANGUAGE which_language, 
 *                           ITEM_INFO * p_item, YESorNO just_checking, 
 *                           YESorNO new, YESorNO do_header)
 ************************************************************/
void
display_petition_info(LANGUAGE which_language,
			   ITEM_INFO * p_item, YESorNO just_checking, 
                           YESorNO new_pet, YESorNO do_header)
{
  int reports;
  int fields;   /* also in no_of_fields, external */
  if (do_header)
    {
      char head[200];
      char uppers[100];
      printf("\n");
      sprintf(head, "ISTRUZIONI PER %s", 
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
/*******************************************************
 *    void display_petition_results(ITEM_INFO *p_item)
 *******************************************************/
void
display_petition_results(ITEM_INFO *p_item)
{
  unsigned long voters;
  voters = get_mail_voters(p_item->dropping_id);
  printf("\n");
  highlight("RISULTATI");
  if (no_pet_votes)
    {
      printf("\n%ld votante%s hanno participato in questa votazione.\n",
	     voters, (voters == 1L? "" : "s"));
    }
  else
    printf("\nQuesta petizione ha raccolto %ld firma %s.\n",
	   voters, (voters == 1L? "" : "s"));
}
/*********************************************************
 *   void display_petition_text(ITEM_INFO * p_item, 
 *                             YESorNO just_checking,
 *                             LANGUAGE which)
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
  printf("\n");
  highlight((no_pet_votes ? "TESTO DELLA VOTAZIONE" : "TESTO DELLA PETIZIONE"));
  default_name = pet_fname("text", p_item);
  sprintf(fname, "%s/%s", default_name, tongue[which].name);
  while ((fp = fopen(fname, "r")) == NULL)
    {
      sprintf(fname, "%s/%s", default_name, tongue[DEFAULT_LANGUAGE].name);
      if (++tries < 2)
	continue;
      printf("\n%s", file_error);
      printf("\n\t%s \n\n", fname);
      printf("per leggere il testo di questa petizione.\n\nPer favore speditelo a %s.\n\n",
	     eVote_mail_to);
      fprintf(stderr,"%s\n\t%s \n\nper leggere il testo di questa petizione.\n\n", 
	      file_error, fname);
      perror("");
      return;
    }
  if ((ch = fgetc(fp)) == EOF)
    {
      char er[] = "\n\nERRORE! File vuoto: \n\n";
      char er2[] = "\n\nImpossibile leggere il testo di questa votazione/petizione.\n\n";
      printf("%s%s%s Per favore inoltrate questo a %s.\n\n", 
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
 *    void display_priv_type(ITEM_INFO *p_item)
 *    Displays messages about the privacy type.
 *******************************************************/
void
display_priv_type(ITEM_INFO *p_item)
{
  switch (p_item->eVote.priv_type)
    {
    case PRIVATE:
      printf("\n");
      highlight("PETIZIONE PRIVATA");
      printf("\nLe firme per questa petizione possono essere recuperate \n%s e dall' amministratore di sistema di questo sito \nSolamente loro sono responsabili per l'uso delle informazioni.\n",
	     author_name);
      break;
    default:
      printf("\n");
      highlight((no_pet_votes ? "VOTAZIONE PUBBLICA" : "PETIZIONE PUBBLICA"));
      if (same(subject,"Kopilli Ketzalli"))
	{
	  printf("\nQuesta e` una *petizione pubblica*.Pubblicheremo il vostro nome, \npaese e commenti (ma non il vostro indirizzo emal) sulla pagina web.\nIn futuro, i vostro amici saranno in grado di interrogare il sistema\n(se sono a conoscenza del vostro indirizzo email) per vedere il vostro\nmessaggio /petizione.\n\nCi riserviamo il diritto di cancellare qualsiasi contributo\nconsiderato inappropriato.  Potete cancellare il vostro contributo in\nqualunque momento.  In ogni caso, se la vostra firma rimane una\nsettimana, potete cancellarla dai dati web ma potrebbe essere gia`\nstata conteggiata in Austria.\n\nUtilizziamo il vostro indirizzo email per essere sicuri che vi e`\nsolo una firma per ogni indirizzo email e per nessun altro motivo.\n",
		 whereami);
	}
      else
	{
	  printf("\nQuesta e` una petizione *pubblica*. Le informazioni da voi fornite\npotranno essere lette da chiunque firmi la petizione e da coloro che\nfiugrano nella lista email %s@%s.\n", list, whereami);
	  printf("\nQuesto significa che le informazioni sono pubbliche e che non c'e`\ncontrollo sul loro uso.\n");
	}
    }
}
/*******************************************************
 *    void display_remove(ITEM_INFO *p_item, LANGUAGE lang)
 *******************************************************/
void
display_remove(ITEM_INFO *p_item, LANGUAGE lang)
{
  printf("\n");
  highlight((no_pet_votes ? "STIAMO CANCELLANDO IL VOSTRO VOTO" : "CANCELLARE LA VOSTRA FIRMA"));
  printf("\nSe pensate di %s sia un errore, per favore rimuovete il vostro %s:",
	 (no_pet_votes ? "votato" : "firmato"),
	 (no_pet_votes ? "votato" : "firmato"));
  printf("\n\n1.  Mandare un messaggio a:");
  printf("\n\n\teVote@%s", whereami);
  printf("\n\n2.  Il vostro soggetto deve essere:");
  printf("\n\n\t%s", get_trans(subject, lang, YES));
  printf("\n\n3.  Il vostro messaggio dovrebbe essere:");
  printf("\n\n\tcancellare\n");
#ifdef ROSA
  if (same(subject,"Kopilli Ketzalli"))
    {
      printf("\nSe volete rimuovere la vostra firma, per favore fatelo tempestivamente.\nTra una settimana la vostra settimana sara` inoltrata a Vienna e \nconteggiata.\n");
      printf("\n");
      highlight("NOTIZIE");
      printf("\nper ricevere il bollettino di informazioni email per i Yanakuikanahuak\nper favore:\n");
      printf("\n1.  Mandare un messaggio a:");
      printf("\n\n\tmajordomo@deliberate.com");
      printf("\n\n2.  Non tenere conto della linea del soggetto.");
      printf("\n\n3.  Il vostro messaggio dovrebbe essere:");
      printf("\n\n\tsubscribe");
      printf(" anahuak-en");
      printf("\n\tend");
      printf("\n\nCi sono anche bollettini in, \"anahuak-es\", tedesco, \"anahuak-de\", \ne francese, \"anahuak-fr\" a cui siete invitati ad iscrivervi.\n");
      printf("\n");
      highlight("RAGGRUPPAMENTO");
      printf("\nPer entrare a far parte del gruppo di discussione email u Kopilli\nKetzalli e il suo ritorno in Messico:\n");
      printf("\n1.  Mandare un messaggio a:");
      printf("\n\n\tmajordomo@deliberate.com");
      printf("\n\n2.  Non tenere conto della linea del soggetto.");
      printf("\n\n3.  Il vostro messaggio dovrebbe essere:");
      printf("\n\n\tsubscribe");
      printf(" kopilli-en");
      printf("\n\tend");
      printf("\n\nCi sono anche raggruppamenti in spagnolo, \"kopilli-es\", tedesco, \n\"kopilli-de\", and francese, \"kopilli-fr\" a cui siete invitati ad iscrivervi.\n");
      printf("\n");
      highlight("BILINGUE?");
      printf("\nSe scrivete in tedesco, spagnolo, francese o inglese e volete dare \nun aiuto nella traduzione per i Yankuikanahuak:");
      printf("\n\n1.  Mandare un messaggio a:");
      printf("\n\n\tmajordomo@%s", whereami);
      printf("\n\n2.  Non tenere conto della linea del soggetto.");
      printf("\n\n3.  Il vostro messaggio dovrebbe essere:");
      printf("\n\n\tsubscribe");
      printf(" es");
      printf("\n\tend");
      printf("\n\nper mettersi in contatto con traduttori di lingua spagnola. Ci sono\nanche gruppi di traduttori per il tedesco, \"de\", e francese, \"fr\", che\nsaranno lieti di ospitarvi.\n");
      printf("\nPotete mandare piu` istruzioni a majordomo@deliberate.com in un solo messaggio:");
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
      printf("\n\nvi fara` sottoscrivere a cinque liste.\n");
    }
#endif
}
/*******************************************************
 *    void do_petition(void)
 *     Processes a message when it came into the system
 *     through the petition facility.
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
  cc = get_token();
  if (same(token,"eVote"))
    {
      sprintf(error_msg,"\nPer utilizzare comandi eVote dovete essere membri della lista \n\n  %s@%s\n\ne i comandi eVote devono essere spediti direttamente all'indirizzo\ndella lista.",
	      list, whereami);
      bounce_error(SENDER);
    }
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
  if (set_up_eVote() != OK)  /* Identifies the list, and user */
    /* if we get this far, communication to the list is working
       the list exists and, if mail_voter != 0, we're into
       the database.  */
    {
      char *str;
      if (remove_sig)
	{
	  sprintf(error_msg, "\nNon avete partecipato a eVoting in %s.\n",
		  whereami);
	}
      enter_signer();
    }
  if (subject == NULL || subject[0] == '\0')
    {
      sprintf(error_msg, "\nDovete specificare il titolo di una petizione come soggetto del vosrto messaggio.\n");
      list_petitions();
    }
  if (current_action == READ_ONLY)
    {
      sprintf(error_msg,"\nNon potete partecipare a eVoting. Siete solo un membro read-only della\n%s.\n", list);
      bounce_petition_error(SENDER | OWNER);
    }
  copy_poll(YES); /* If dropping_id != 0, we have a polled
		     subject */
  if (dropping_id == 0 || petition != YES 
     || (p_item_copy->eVote.type != TALLIEDY 
	 && p_item_copy->eVote.type != TIMESTAMP))
    {
      gen_header(SENDER, "ERRORE:", NO);
      printf("\nNon c'e` petizione allegata al soggetto, \"%s\".\n\nPer favore controllate lo spelling nella riga del soggetto del vostro \nmessaggio.\n", 
	     original_subject);
      list_petitions();
      leave();
      big_petition_finish(0);
    }
  if (error_msg[0] != '\0')  /* remove_sig and never eVoted */
    {
      bounce_info_error();
    }
  if (same(tongue[the_language].help, token)
     || same(tongue[the_language].info, token))
    send_petition_info(SENDER, p_item_copy, NO, NO, NO);
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
	  sprintf(error_msg,"\nSiete arrivati troppo tardi per rimuovere il vostro %s in:\n\n        %s\n\nNon e` piu` operativa.\n", (no_pet_votes ? "voti" : "signfirmaature"),
		  original_subject);
	}
      else
	{
	  sprintf(error_msg,"\nSiete arrivati troppo tardi per %s in:\n\n        %s\n\nNon e` piu` operativa.\n",
		  (no_pet_votes ? "votare" : "firmare la petizione"),
		  original_subject);
	}
      too_late();
    }
  reports  = read_report_instructions(p_item_copy);
  fields = read_form_template(p_item_copy);
  /* remove_sig is set in strip_subject() if [R] in subject
     line -- good for javascript */
  if (remove_sig == YES)
    {
      remove_signer(reports);
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
      sprintf(error_msg, "\nProblemi di sistema in %s: Il vostro %s non e` stato registrato.\n", 
	      whereami,
	      (no_pet_votes ? "voto" : "firma"));
      bounce_petition_error(SENDER | ADMIN | OWNER);
      break;
    case NO_MODS:
      sprintf(error_msg,"\nSiete arrivati troppo tardi per %s in:\n\n        %s\n\nNon e` piu` operativa.\n",
	      (no_pet_votes ? "votare" : "firmare la petizione"),
	      original_subject);
      list_petitions();   /* never returns */
      break;
    case NO_CHANGE:
      sprintf(error_msg, "\nGrazie.  Avete gia` %s in \n\n\t%s\n\nNon potete %s due volte.\n", 
	      (no_pet_votes ? "votato" : "firmato la petizione"),
	      original_subject, 
	      (no_pet_votes ? "votato" : "firmato la petizione"));
      bounce_info_error();
      break;
    case GOOD:  
      get_mail_stats(p_item_copy->dropping_id,
		     &readers, vote_str, result);
      voters= atoul(vote_str);
      store_signature(p_item_copy);
      if (no_pet_votes)
	vote_the_petition();
      gen_header(SENDER, "eVote Ricevuta:", NO);
      printf("\nGrazie per il vostro %s oppure\n\n\t%s", 
	     (no_pet_votes == 0 ? "firma" : (no_pet_votes == 1 ?
					     "voto" : "voti")),
	     original_subject);
      printf("\n\nFino ad oggi, %lu %s %s.", voters,
	     (voters == 1? "persona ha" : "persone hanno"),
	     (no_pet_votes ? "votato" : "firmato la petizione"));
      display_priv_type(p_item_copy);
      display_remove(p_item_copy, the_language);
      printf("\n");
      highlight(no_pet_votes == 0 ? "LA VOSTRA FIRMA" : 
		 (no_pet_votes == 1 ? "IL VOSTRO VOTO" : "I VOSTRI VOTI"));
      if (no_pet_votes)
	{
	  printf("\nI vostri voti sono stati registrati come:\n");
	  report_vote();
	  printf("\n");
	  highlight("I VOSTRI COMMENTI");
	}
      printf("\nIl vostro %s stato registrato come:", (no_pet_votes ? "commenti sono" : 
						       "firma e"));
      if (write_signature(stdout, NO, NO) != OK)
	{
	  sprintf(error_msg, "\neVote impossibile archiviare il vostro %s per %s ora.\n\nPer favore provate piu` tardi\n", 
		  (no_pet_votes? "vote" : "signature"), original_subject);
	  bounce_petition_error(SENDER | OWNER | ADMIN);
	}
      display_petition_text(p_item_copy, NO, the_language);
      break;
    }
  /*   For testing the timeout
       {
       long i;
       for (i = 0; i >= 0; i++)
       fprintf(stderr, "%d", i);
       }
  */
  if (reports > 0)
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
 *    void list_petitions()
 *           Sends the list of open petitions
 *           the sender.  
 **********************************************************/
void
list_petitions(void)
{
  void read_translations(void);
  gen_header(SENDER, "Re:", YES);
  if (error_msg[0] != '\0')
    {
      printf("%s", error_msg);
    }
  printf("\nLe petizioni disponibili attualmente presso %s sono \nelencate qui sotto.", 
	 whereami);
  printf("\n\nPer informazioni circa una particolare petizione:");
  printf("\n\n1.  Mandare un messaggio a:");
  printf("\n\n\teVote@%s\n\n", whereami);
  printf("2.  La vostra linea del soggetto deve cpmbaciare con il titolo della petizione.");
  printf("\n\n3.  Il vostro messaggio dovrebbe essere: ");
  printf("\n\n\tinfo");
  printf("\n\n    per ricevere il testo della petizione e le istruzioni per sottoscriverla.\n");
  lowlight("Privacy");
  printf("\nnella lista seguente:\n\nPUBBLICO le petizioni sono aperte a tutti %s@%s\n         lista per recuperare le firme annesse alla.\n         Inoltre, chiunque firmi la petizione puo` recuperare le \n         firme.  Non ci sono controlli sull'uso della firma.\n\nPRIVATI  le petizioni permettono solo all'iniziatore della petizione \n         e all'amministratore di sistema %s di recuperare \n         le firme.\n",
	 list, whereami, whereami);
  if (no_petitions == -1)
    read_translations();
  if (no_publics == 0)
    printf("\nNon ci sono petizioni PUBBLICHE.\n\n");
  else if (no_publics == 1)
    {
      printf("\nC'e` una petizione PUBBLICA:\n\n");
      print_petitions(YES);
    }
  else
    {
      printf("\npetizioni PUBBLICHE:\n\n");
      print_petitions(YES);
    }
  if (no_privates == 0)
    printf("\nNon ci sono petizioni PRIVATE.\n\n");
  else if (no_privates == 1)
    {
      printf("\nC'e` una petizione PRIVATA:\n\n");
      print_petitions(NO);
    }
  else
    {
      printf("\npetizioni PRIVATE:\n\n");
      print_petitions(NO);
    }
  leave();
  big_petition_finish(0);
}
/*************************************************************
 *   int parse_vote(int * cc)
 ************************************************************/
static int
parse_vote(int * pcc, char **ans)
{
  static char answer[200];
  char delimiter[2] = " ";
  int cc, i, j;
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
  for (i = 0; i < no_of_answers; i++)
    {
      if (same(answer_list[i].str, answer))
	return answer_list[i].answer;
    }
  return -10;
}
static OKorNOT
check(RTYPE rtype)
{
  switch (rtype)
    {
    case FAILURE:
      sprintf(error_msg, "\nProblemi di sistema in %s: Il vostro %s non e` stato cancellato.\n", 
	      whereami, (no_pet_votes ? "voto" : "firma"));
      return NOT_OK;
      break;
    case NO_MODS:
      sprintf(error_msg, "\nErrore di programmazione in pet_out.c:check.\n");
      return NOT_OK;
      break;
    case NO_CHANGE:  /* first vote makes all others 0 */
    case GOOD:  
      return OK;
    }
}
/************************************************************
 *  void remove_signer(int reports)
 ************************************************************/
static void
remove_signer(int reports)
{
  unsigned long  readers, voters;
  char vote_str[10] = "  - ";
  char result[10] = "  - ";
  time_t when;
  int i, cc;
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
/**********************************************************
 *    void start_petition_displays(ITEM_INFO * p_item, LANGUAGE which)
 **********************************************************/
void
start_petition_displays(ITEM_INFO * p_item, YESorNO new_pet,
			     LANGUAGE which)
{
  unsigned long voters;
  printf("\nIn %s%s annesso a %s a questo soggetto:\n\n\t \"%s\"\n",
	 date_str(p_item->eVote.open), author_name, 
	 (no_pet_votes? "votazione" : "petizione"),
	 get_trans(subject, which, YES));
  if (new_pet)
    voters = 0L;
  else
    voters = get_mail_voters(p_item->dropping_id);
  if (p_item->eVote.vstatus == CLOSED) 
    {
      printf("\nQuestoa %s e` stata iniziata il %s ma e` stata chiusa dal %s",
	     (no_pet_votes ? "votazione" : "petizione"),
	     date_str(p_item->eVote.open), date_str(p_item->eVote.close));
      printf("\n%hd %s%s raccolti.\n",
	     voters, (no_pet_votes ? "vote" : "signature"),
	     (voters == 1L ? " era" : "s erano"));
    }
  else if (!new_pet)
    {
      printf("\n%hd %s%s sono stati raccolti fino ad ora.\n",
	     voters, (no_pet_votes ? "vote" : "signature"),
	     (voters == 1L ? " ha" : "s hanno"));
    }
}
/**********     FORMS  ******************/
/****************************************************
 *   void bounce_info_error()
 *        called from check_form when the user doesn't
 *        fill it in properly.
 *****************************************************/
void
bounce_info_error(void)
{
  int i;
  leave();
  /* generate a message header on stdout */
  gen_header(SENDER, "Errore:", NO);
  printf("\nIn risposta al vostro messaggio:\n");
  print_tokens(NO);
  printf("%s", error_msg);
  printf("\nForse le istruzione per \"%s\" saranno di aiuto.\n",
	 original_subject);
  send_petition_info(SENDER, NULL, NO, NO, NO);
  printf("\n\nSegue il vostro messaggio completo:\n");
  big_finish(0);
}
/************************************************************
 *  int check_form(int cc)
 *       This checks the answers to the form that are supplied
 *       by the signer.  If there is an error, it bounces the
 *       message.
 *       The int returned and cc are the delimiter for the
 *       current token.
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
      sprintf(error_msg, "\nNon ci sono risorse disponibili per processare il vostro %s\nin \"%s\".  Per favore provate piu` tardi.\n",
	      (no_pet_votes? "voto" : "firma"), original_subject);
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
			  "\nDovete dare il vostro \"%s:\"\nprima di fare un commento nel sottoscrivere la petizione per \n\n        \"%s\"\n", force_name(i),
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
	      sprintf(error_msg, "\n\"%s\" non e` parte della formula in:\n\n\t\"%s\".\n",
		      name, original_subject);
	      bounce_info_error();
	    }
	}
      if (answer[i][0] != '\0')
	{
	  sprintf(error_msg, "\nPer favore date il vostro %s solo una volta quando firmate la petizione per\n\"%s\".\n", force_name(i), original_subject);
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
	      sprintf(error_msg, "\nPer favore limitate la vostra risposta a \"%s:\" a %%d caratteri.\n", field[i].name[the_language]);
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
	 && !same(answer[i], "No") && !same(answer[i], "Non"))
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
	  sprintf(error_msg, "\nA %s in\n\n\t\"%s\"\n\ndovete includere il vostro \"%s\"", 
		  (no_pet_votes ? "voto" : "firmare la petizione"),
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
			  strcat(error_msg, " e ");
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
	  sprintf(instruction, "\nquando fornite il vostro \"%s, \" per\nfavore fate si` che la vostra risposta abbia questo formato\n\n        %s: %s\n\n", field[i].name[the_language], field[i].name[the_language], field[i].format);
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
	      sprintf(error_msg,"%srimpiazzare tutte le X con le lettere corrette.\n", instruction);
	    }
	  else if (xes == NO)
	    {
	      sprintf(error_msg,"%srimpiazzre tutti i 9 con il numero corretto.\n", instruction);
	    }
	  else
	    {
	      sprintf(error_msg,"%srimpiazzare tutti i 9 con il numero corretto e tutte le X \ncon le lettere corrette.\n", instruction);
	    }
	  bounce_info_error();
	}
    }
  return cc;
}
static YESorNO
could_match(char * right_name, char * try_name, int len)
{
  if (strNcmp(right_name, try_name, len) == 0
     && (len == strlen(right_name) || right_name[len] == ' '))
    {
      if (len == strlen(right_name))
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
static char
*force_name(int j)
{
  return (field[j].name[the_language][0] == '\0'?
	  field[j].name[default_language] :
	  field[j].name[the_language]);
}
/************************************************************
 *   int explain_form(LANGUAGE which)
 *       Called from display_howto when there is a form.
 *       Returns the number of notes printed.
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
  printf("\n3.  Per favore fate in modo che il vostro messaggio appaia come: \n");
  printf("\n---  tagliare qui --- \n");
  for (i = 1; i <= no_pet_votes; i++)
    {
      printf("\n%d. Il vostro voto", i);
    }
  if (no_of_fields > 0)
    {
      if (field[0].name[which][0] == '\0')
	do_this = DEFAULT_LANGUAGE;
      for (i = 0; i < no_of_fields; i++)
	{
	  if (strncmp(field[i].name[do_this],
		     tongue[do_this].comment, 
		     strlen(tongue[do_this].comment)) == 0)
	    comment = YES;
	  if (strncmp(field[i].name[do_this],
		     "nome", 4) == 0)
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
      printf("\nPotete aggiungere un %s qui.",
	     tongue[do_this].comment);
    }
  else if (!comment && !name_field)
    {
      printf("\nPotete aggiungere il vostro nome e/o un %s qui.",
	     tongue[do_this].comment);
    }
  else if (comment && !name_field)
    {
      printf("\nPotete aggiungere il vostro nome qui.");
    }
  printf("\n\n---  tagliare qui --- \n");
  if (no_pet_votes)
    {
      printf("\n    dove ogni \"1.\", \"2.\", ecc. rappresenta un numero di domanda. Invece di \"il \n    vosrto voto\", potete rispondere \"Si\", \"No\", oppure \"Non saprei.\"\n");
    } 
  notes = (required ? 1 : 0) + (optional ? 1 : 0)
    + letters + numbers;
  if (notes == 0)
    return notes;
  printf("\n      %s:\n", (notes > 1 ? "NOTES" : "NOTE"));
  if (required)
    {
      printf("\n      *  ", ++notes_printed);
      len = 39;
      printf("Dovete dare %s per ", (required == 1?"una risposta":"risposte"));
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
	      printf("e ");
	    }
	}
    }
  if (optional)
    {
      printf("\n      *  ", ++notes_printed);
      len = 39;
      printf("%s opzionale per ",
	     (no_of_fields - required == 1?"una risposta �":"Le risposte sono"));
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
	      printf("e ");
	    }
	}
    }
  if (numbers)
    {
      printf("\n      *  ");
      printf("Per favore rimpiazzate tutti i 9 con i numeri corretti.\n");
    }
  if (letters)
    {
      printf("\n      *  ");
      printf("Per favore rimpiazzate tutte le X con le lettere.\n");
    }
  return notes_printed;
}
/************************************************************
 *  void report_vote(void)
 ************************************************************/
void
report_vote(void)
{
  int i;
  for (i = 0; i < no_pet_votes; i++)
    {
      printf("\n%d. %s", i+1, (pet_vote[i] == 0 ? "Non saprei" 
			       : (pet_vote[i] == 1 ? "Si" : "No")));
    }
}
/************************************************************
 *   void set_comment(void)
 ************************************************************/
static void
set_comment(void)
{
  tongue[LOCAL_LANGUAGE].comment = "commento";
}
/*********************************************************
 *   void store_signature(ITEM_INFO * p_item)
 *     Stores message text and email address.
 **********************************************************/ 
void
store_signature(ITEM_INFO * p_item)
{
  FILE * fp, *fp_tmp;
  char *fname;
  int no_items;
  push_time(p_item, now);
  fname = pet_signers_fname(p_item, now);
  if ((fp_tmp = lock_to_tmp(fname)) == NULL)
    {
      sprintf(error_msg,"\n\nERRORE! %s\n\nImpossibile aprire il file temporaneo per archiviare la vostra firma in \"%s\".\nPer favore provate piu` tardi.\n",
	      fname, subject);
      bounce_petition_error(SENDER | OWNER | ADMIN);
    }
  if ((fp = fopen(fname, "a")) == NULL 
     || write_signature(fp, YES, YES) != OK)
    {
      sprintf(error_msg, "\neVote impossibile archiviare la vostra firma per %s ora.\n\nPer favore provate piu` tardi\n", subject);
      bounce_petition_error(SENDER | OWNER | ADMIN);
    }
  if (unlock_to_tmp(NO) != OK)
    {
      fprintf(stderr,"\nERRORE! Impossibile rimuovere file tm: %sT.\n",
	      fname);
    }			
  fclose(fp);
  chmod(fname, 00660);
  return;
}
/************************************************************
 *  void vote_the_petition(void)
 ************************************************************/
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
      sprintf(error_msg,"\nSiete arrivati troppo tardi per %s in:\n\n        %s\n\nNon e` piu` operativa.\n",
	      (no_pet_votes ? "votare" : "firmare la petizione"),
	      original_subject);
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
/************************************************************
 *   void finish_unsign(char * name, int * pcc, YESorNO from_owner)
 ***********************************************************/
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
      sprintf(error_msg,"\n%s non ha %s \"%s\".\n",
	      name, (no_pet_votes ? "voted in" : "signed" ), subject);
      i_am_leaving();
      *pcc = NOT_OK;
      return;
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
  if (current_action == VOTE_ONLY)
    {
      drop_mail_voter(lclist, YES);
    }
  if (!from_owner)
    {
      strcpy(time_str, t_str);  /* for message to user */
      gen_header(SENDER, "eVote Ricevuta:", YES);
      time_str[0] = '\0';  /* wipe it out to prevent another i_am_leaving */
      printf("\nIl vostro %sstata rimossa da \n\"%s\".",
	     (no_pet_votes? "voti sono" : "firma e`"), original_subject);
      if (pet_vote != NULL)
	{
	  printf("\n\nI voto cancellati sono:\n");
	  report_vote();
	}
      printf("\n\nIl commento cancellato e`:\n");
    }
  if (from_owner)
    {
      printf("\n%s non e` stato firmato.\nIl testo e` stato rimosso da:\n", name);
    }
  drop_signature(name, stdout, when);
  *pcc = OK;
  return;
}
