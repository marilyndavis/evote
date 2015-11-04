/* $Id: pet_out_es.c,v 1.7 2003/10/20 17:07:34 marilyndavis Exp $ */ 
/**********************************************************
 *  ../eVote/src/mail/mailui/pet_out.c
 *         has code that needs translating
 *********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
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
#define LOCAL_LANGUAGE ESPANOL
/*************************************************************
 *              |     After translating this file, change the 
 *              |     name of this function to be translate_xx 
 *              |     where xx is the flag for your language.
 ************* \|/ ******************************************/
void
translate_es(LANGUAGE which_language, WHICH_FUNCTION function, 
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
  printf("\n  ===============    INICIO DEL MENSAJE RECIBIDO ==================\n\n");
  dump_message(stdout, NO, NO, NO);
  printf("\n\n  ===================     FIN DEL MENSAJE   =======================\n");
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
  gen_header(whom, "Error:", YES);
  printf("\nEn respuesta a su mensjae que empieza:\n");
  /*  print_tokens(YES); */
  print_first_line();
  printf("\n----\n");
  printf("%s", error_msg);
  if (list != NULL && list[0] != '\0')
    {
      if ((whom & OWNER && whom != OWNER) 
	 || (whom & APPROVAL && whom != APPROVAL))
	{
	  printf("\nEste mensaje tambie'n se envia al owner-%s", list);
	  if (whereami != NULL && whereami[0] != '\0')
	    printf("@%s\n", whereami);
	}
    }
  if (whom & ADMIN)
    {
      printf("\nEste mensaje tambien se envia al owner-majordomo");
      if (whereami != NULL && whereami[0] != '\0')
	printf("@%s\n", whereami);
    }
#ifdef EVOTE_ERRORS
  if (whom & DEVELOPER)
    printf("\nEste mensaje tambie'n se envia a %s.\n", 
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
      sprintf(error_msg, "\nSu mensaje sobre:\n\n\t%s\n\nperece como un respuesta de confirmacio'n para \"%s\" \ncon el llave de comfirmacio'n \"%s\".  Sin embargo, no hay registro \nde este llave de confirmacio'n para Vd. para \"%s\".\n", original_subject, subject, confirm_this.key, subject);
      if (cc == NO)
	{
	  bounce_info_error();
	  break;
	}
      /* case MAYBE: found a file_key but wrong one */
      sprintf(buf, "Confirme: %s ", file_key);
      gen_header(SENDER, buf, NO);
      printf(error_msg);
      printf("\nSin embargo, Vd. tiene un llave de confirmacio'n \"%s\" \nque esta esperando para %s.\n\nNota que eVote guarda solo su intencio'n mas rece'n.  Lo siguente es\nel mensaje que esta esperando para confirmacio'n.  Para confirmar lo,\nmanda Ud. un mensaje usando \"reply-to\" con este mensaje.\n\nSi Ud. quiere mandar un mensaje nuevo para %s,\ntrata de nuevo, sino no escribe \"Confirma: %s\" en la li'nea\nde \"Subject\".\n",
	     file_key, subject, subject, confirm_this.key);
      printf("\n\n ----- SU MENSAJE ESPERANDO PARA CONFIRMACIO'N -----\n\n");
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
      sprintf(error_msg,"\n%s no tiene recursos disponibles ahorita. Favor de intentar ma's tarde.\n", whereami);
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
	      sprintf(error_msg,"\neVote a este punto espera un respuesta en el voto como la siguiente \"1. Si'\".\n");
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
	  sprintf(error_msg,"Hay solemente %d preguntas en esta urna.\n",
		  no_pet_votes);
	  return -1;
	}
      if (pet_vote[no-1] != UNKNOWN)
	{
	  sprintf(error_msg,"\nSolo puede votar una vez en la pregunta nu'mero %d.\n",
		  no);
	  return -1;
	}
      if ((pet_vote[no-1] = parse_vote(&cc, &answer, no-1)) == UNKNOWN)
	{ /* parse_vote puts an error_msg in */
	  if ((p_first_vote_item+no)->eVote.min == -1
	     && (p_first_vote_item+no)->eVote.max == 1)
	    {
	      sprintf(error_msg, "\neVote no reconose su respuesta para la pregunta %d.\nUsted respondi'o \"%s\". Usted debe de contestar \n\"Si'\", \"No\" o \"No Se'.\"\n", 
		      no, answer);
	    }
	  else
	    {
	      sprintf(error_msg, "\neVote no reconose su respuesta para la pregunta %d.\nUsted respondi'o \"%s\". Usted debe de contestar entre %d y %d.\n",
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
  highlight((no_pet_votes ? "PARA VOTAR": "PARA FIRMAR ESTA PETICIO'N"));
  printf("\n1.  Envie un mensaje a:");
  printf("\n\n\teVote@%s", whereami);
  printf("\n");
  printf("\n2.  En \"Subject\" tiene que escribir:");
  printf("\n\n\t%s", get_trans(subject, which, YES));
  printf("\n");
  if (!form_exists && !no_pet_votes)
    {
      printf("\n3.  Si esta' en blanco su mensaje, solamente se grabara' su direccio'n\n    de correo electro'nico\n\n    o\n\n    Escriba su nombre, afiliacio'n y ubicacio'n.\n");
    }
  else  /* also explains votes */
    {
      explain_form(which);
    }
  printf("\n4.  Si su mensaje tiene firma automatizada, o cualquier texto que\n    no quiera incluir, escriba \"fin\".\n");
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
      sprintf(head, "INSTRUCCIONES PARA %s", 
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
  highlight("RESULTADOS");
  if (no_pet_votes)
    {
      printf("\n%lu votante%s participado en esta urna.\n",
	     voters, (voters == 1L? " ha" : "s han"));
    }
  else
    printf("\nEsta peticio'n recopilo %lu firma%s.\n",
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
  highlight((no_pet_votes ? "TEXTO DE LA URNA" : "TEXTO DE LA PETICIO'N"));
  default_name = pet_fname("text", p_item);
  sprintf(fname, "%s/%s", default_name, tongue[which].name);
  while ((fp = fopen(fname, "r")) == NULL)
    {
      sprintf(fname, "%s/%s", default_name, tongue[DEFAULT_LANGUAGE].name);
      if (++tries < 2)
	continue;
      printf("\n%s", file_error);
      printf("\n\t%s \n\n", fname);
      printf("para leer el texto de esta peticio'n.\n\nFavor de envirar e'ste a %s.\n\n",
	     eVote_mail_to);
      fprintf(stderr,"%s\n\t%s\n\npara leer el texto de esta peticio'n.\n\n", 
	      file_error, fname);
      perror("");
      return;
    }
  if ((ch = fgetc(fp)) == EOF)
    {
      char er[] = "\n\nERROR! Archivo vaci'o: \n\n";
      char er2[] = "\n\nNo se puede leer el texto de esta urna/peticio'n.\n\n";
      printf("%s%s%sFavor de enviar e'sto a\n%s.\n\n", 
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
      highlight("PETICIO'N PRIVADA");
      printf("\nLas firmas para esta peticio'n, y las direccio'nes correos electro'nicos,\nsolamente se pueden recuperar por %s \ny por el administrador de este sitio.  Ellos unicamente son responsables \npor el uso de la informacio'n.\n",
	     author_name);
      break;
    default:
      printf("\n");
      highlight((no_pet_votes ? "URNA PUBLICA" : "PETICIO'N PUBLICA"));
#ifdef ROSA
      if (same(subject,"Kopilli Ketzalli"))
	{
	  printf("\nEsta es una peticio'n *pu'blica*.  Publicaremos su nombre, pai's y\ncomentarios, pero no su direccio'n de correo electro'nico, en la pa'gina\ndel web. En el futuro, sus amigos podra'n preguntar este sistema (si ya\nsaben su direccio'n de correo electro'nico) para ver su mensaje de\npeticio'n.\n\nReservamos el derecho de borrar cualquier contribucio'n que\nconsideramos inapropiada.  Ud. mismo puede borrar su propia\ncontribucion en cualquier momento.  Sin embargo, si su firma se queda\npor una semana lo puede borrar de los datos en el cyberespacio pero ya\npuede haber sido contado en Austria.\n\nUtilizamos su direccio'n de correo electro'nico para asegurar que so'lo\nhaya una firma por cada direccio'n, y para ningun otro propo'sito.\n");
	}
      else
#endif
	{
	  printf("\nEsta es una peticio'n *pu'blica*.  La informacio'n que Ud. contribuye\npuede ser recuperada por cualquier persona que firme la peticio'n y por\ncualquier persona subscrita en la lista %s@%s de\ncorreo electro'nico.\n", list, whereami);
	  printf("\nEsto quiere decir que la informacio'n es bastante pu'blica y que \nno hay controles sobre su uso.\n");
	  printf("\nSin embargo, solamente el adminstrador del sistema en \n%s puede ver su direccio'n correo electro'nico.\n\nUtilizamos su direccio'n de correo electro'nico para asegurar \nque so'lo haya una firma por cada direccio'n, y para ningun \notro propo'sito.\n", whereami);
	}
    }
}
/*******************************************************/
void
display_remove(ITEM_INFO *p_item, LANGUAGE lang)
{
  /*  printf("\n"); */
  highlight((no_pet_votes ? "QUITANDO SU VOTO" : "QUITANDO SU FIRMA"));
  printf("\nSi Ud. piensa que ha %s equivocadamente favor de borrar su %s:",
	 (no_pet_votes ? "votado" : "firmado"),
	 (no_pet_votes ? "votado" : "firmado"));
  printf("\n\n1.  Envie un mensaje a:");
  printf("\n\n\teVote@%s", whereami);
  printf("\n\n2.  En \"Subject\" tiene que escribir:");
  printf("\n\n\t%s", get_trans(subject, lang, YES));
  printf("\n\n3.  Su mensaje debe decir:");
  printf("\n\n\tborrar\n");
#ifdef ROSA
  if (same(subject,"Kopilli Ketzalli"))
    {
      printf("\nSi Ud. quiere borrar su firma, favor de hacerlo pronto.  Despue's de \nuna semana, su firma puede ser enviada a Viena para ser contada.\n");
      printf("\n");
      highlight("BOLETIN");
      printf("\nPara recibir el boleti'n de correo electro'nico para el Yanakuikanahuak\nfavor de:\n");
      printf("\n1.  Envie un mensaje a:");
      printf("\n\n\tmajordomo@deliberate.com");
      printf("\n\n2.  No importa lo que contenga la li'nea de \"Subject\".");
      printf("\n\n3.  Su mensaje debe decir:");
      printf("\n\n\tsubscribe");
      printf(" anahuak-en");
      printf("\n\tend");
      printf("\n\nTambie'n hay boletines en espa~ol, \"anahuak-es\", alema'n, \"anahuak-de\",\ny france's, \"anahuak-fr\" a los cuales Ud. es bienvenido subscribir.\n");
      printf("\n");
      highlight("ENCUENTRO");
      printf("\nPara juntarse con el encuentro de correo electro'nico para discutir \ntemas tocante el Kopillo Ketzalli y su regreso a Me'xico:\n");
      printf("\n1.  Envie un mensaje a:");
      printf("\n\n\tmajordomo@deliberate.com");
      printf("\n\n2.  No importa lo que contenga la li'nea de \"Subject\".");
      printf("\n\n3.  Su mensaje debe decir:");
      printf("\n\n\tsubscribe");
      printf(" kopilli-en");
      printf("\n\tend");
      printf("\n\nTambie'n hay encuentros en espa~ol, \"kopilli-es\", alema'n, \"kopilli-de\",\ny france's, \"kopilli-fr\" a los cuales Ud. es bienvenido subscribir.\n");
      printf("\n");
      highlight("BILINGUE?");
      printf("\nSi Ud. sabe escribir en alema'n, espa~ol o france's tanto como en \ningle's, y si quiere ayudar con traducciones para los Yankuikanahuak:");
      printf("\n\n1.  Envie un mensaje a:");
      printf("\n\n\tmajordomo@%s", whereami);
      printf("\n\n2.  No importa lo que contenga la li'nea de \"Subject\".");
      printf("\n\n3.  Su mensaje debe decir:");
      printf("\n\n\tsubscribe");
      printf(" es");
      printf("\n\tend");
      printf("\n\npara juntarse con inte'rpretes que hablan espa~ol.  Tambie'n hay\nreuniones para inte'repretes en alema'n, \"de\", y france's, \"fr\" a \nlas cuales Ud. es bienvenida juntarse.\n");
      printf("\nUd. puede enviar muchos mandatos a la majordomo@deliberate.com en un\nsolo mensaje:");
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
      printf("\n\nle subscribira' a cinco listas.\n");
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
	  sprintf(error_msg,"\nPara utilizar mandatos de eVote Ud. tiene que ser \nmiembro de la lista\n\n \t%s@%s\n\ny los mandatos de eVote tienen que ser enviados\ndirectamente a la direccio'n de la lista.\n",
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
	  sprintf(error_msg, "\nUsted no ha participado en eVotacio'n por %s.\n",
		  whereami);
	}
      enter_signer();
    }
  if (subject == NULL || subject[0] == '\0')
    {
      sprintf(error_msg, "\nUd. tiene que especificar un ti'tulo de peticio'n como el \"Subject\" \nde su mensaje.\n");
      list_petitions();
    }
  if (current_action == READ_ONLY)
    {
      sprintf(error_msg,"\nUd. no puede participar en eVotando.\nUd. es un miembro de leer-solamente de la lista %s.\n", list);
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
      gen_header(SENDER, "ERROR:", NO);
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
	  sprintf(error_msg,"\nUd. es demasiado tarde para borrar su%s de:\n\n \t%s\n\nLa %s esta' cerrada.\n", 
		  (no_pet_votes ? "s votos" : " firma") ,
		  original_subject, 
		  (no_pet_votes ? "s votos" : " firma") );
	}
      else
	{
	  sprintf(error_msg,"\nUd. es demasiado tarde para %s sobre:\n\n\t%s\n\nLa %s esta' cerrada.\n",
		  (no_pet_votes ? "votar" : "firmar la peticio'n"),
		  original_subject,
		  (no_pet_votes ? "votar" : "firmar la peticio'n"));
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
      sprintf(error_msg, "\nProblemas con el sistema en %s: Su %s no fue grabada.\n", 
	      whereami,
	      (no_pet_votes ? "voto" : "firma"));
      bounce_petition_error(SENDER | ADMIN | OWNER);
      break;
    case NO_MODS:
      sprintf(error_msg,"\nUd. es demasiado tarde para %s sobre:\n\n\t%s\n\nLa %s esta' cerrada.\n",
	      (no_pet_votes 
	       ? (confirm_this.status == VERIFIED ? "confirmar su voto" : "votar")
	       : (confirm_this.status == VERIFIED ? "confirmar su firma" :"firmar la peticio'n")),
	      original_subject,
	      (no_pet_votes ? "urna" : "peticio'n"));
      list_petitions();   /* never returns */
      break;
    case NO_CHANGE:
      sprintf(error_msg, "\nGracias.  Ud. ya %s sobre\n\n\t%s\n\nNo se puede %s dos veces.\n", 
	      (no_pet_votes ? "voto'" : "firmo' la peticio'n"),
	      original_subject, 
	      (no_pet_votes ? "voto'" : "firmo' la peticio'n"));
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
	  gen_header(SENDER, "eVote Recibo:", NO);
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
	  sprintf(buf, "Confirme: %s", confirm_this.key);
	  gen_header(SENDER, buf, NO);
	  printf("\n%s en \n\"%s\" ya.",
	     (no_pet_votes == 0 ? "Su firma no ha sido registrado" : (no_pet_votes == 1 ?
						 "Su vota no ha sido registrado" : "Sus votas no han sido registrados")),
	     original_subject);
	  printf("\n\nPara confirmar que este mensaje en realidad se mando de Ud., por favor\nconfirma este mensaje por usando \"Reply-To\".  No cambia nada de la\nlinea de sujeto.  No necesita escribir nada de nuevo en su mensaje de\nconfirmacio'n.  Esto asegura que todos los peticio'nes de eVote son\nexactos.");
	  printf("\n\nVd. debe que mandar su confirmacio'n \"Reply-To\" antes de la peticio'n se \ncierra o no sera contada.\n");
	}
      printf("\nGracias por %s on\n\n\t%s", 
	     (no_pet_votes == 0 ? "su firma" : (no_pet_votes == 1 ?
						 "so voto" : "sus votos")),
	     original_subject);
      voters= atoul(vote_str);
      printf("\n\nHasta la fecha, %lu %s %s.\n", voters,
	     (voters ==1? "persona ha" : "personas han"),
	     (no_pet_votes ? "votado" : "firmado esta peticio'n"));
      if (confirm_this.status == STARTING)
	{
	  printf("\nDespues de usa \"reply-to\" para este mensaje, lo siguente grabara':\n\n");
	}
      highlight(no_pet_votes == 0 ? "SU FIRMA" : 
		 (no_pet_votes == 1 ? "SU VOTO" : "SUS VOTOS"));
      if (no_pet_votes)
	{
	  printf("\nSus votos se %s grabada como:\n", (confirm_this.status == STARTING ? "quedara'n" : "queda"));
	  report_vote();
	  printf("\n");
	  highlight("SUS COMENTARIOS");
	}
      if (confirm_this.status == STARTING)
	{
	  printf("\nSu %s sera grabado como:", (no_pet_votes ? "comentario" :
					    "firma"));
	  printf("\n\n - - - - principio de la firma - - - - \n");
	  cc = write_signature(stdout, NO, NO);
	  printf("\n - - - - fin de la firma - - - - \n");
	}
      else
	{
	  printf("\nSu %s queda grabado como:", (no_pet_votes ? "commentario" :
					    "firma"));
	  cc = write_signature(stdout, NO, YES);
	  printf("\n - - - - fin de la firma - - - - ");
	}
      if (cc != OK)
	{
	  sprintf(error_msg, "\neVote no puede guardar su %s para %s ahora.\n\nFavor de intentar ma's tarde\n", 
		  (no_pet_votes? "voto" : "firma"), original_subject);
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
  printf("\nLas peticio'nes actualmente disponibles en %s esta'n indicadas\nabajo.", 
	 whereami);
  printf("\n\nPara recibir informacio'n sobre alguna peticio'n en especial:");
  printf("\n\n1.  Envie un mensaje a:");
  printf("\n\n\teVote@%s\n\n", whereami);
  printf("2.  Lo que ud. escribe en la li'nea de \"Subject\" debe ser igual \n    al ti'tulo de la peticio'n.");
  printf("\n\n3.  Su mensaje debe decir: ");
  printf("\n\n\tinfo");
  printf("\n\n    para recibir el texto de la peticio'n y las instrucciones para firmar.\n");
  lowlight("Privacia");
  printf("\nEn la lista que viene abajo:\n\nPUBLICAS Las peticio'nes PUBLICAS permiten a cualquier persona en \n         la lista %s@%s recuperar las firmas, sino\n         no las direccio'nes de correo electro'nico,  que vienen\n         adjuntas a la peticio'n.  Adema's, cualquier persona \n         que firma la peticio'n puede recuperar las firmas.  No hay \n         controles en el uso de su firma. Utilizamos su direccio'n \n         de correo electro'nico para asegurar que so'lo haya una \n         firma por cada direccio'n, y para ningun otro propo'sito.\n\nPRIVADAS Las peticio'nes PRIVADAS solamente permiten a quien inicia \n         la peticio'n y el administrador del sistema en %s \n         a recuperar las firmas.  Tambie'n las dos personas pueden \n         conocer a las direccio'nes de correo electro'nico.\n",
	 list, whereami, whereami);
  if (no_petitions == -1)
    read_translations();
  if (no_publics == 0)
    printf("\nNo hay peticio'nes PUBLICAS.\n\n");
  else if (no_publics == 1)
    {
      printf("\nHay una peticio'n PUBLICA:\n\n");
      print_petitions(YES);
    }
  else
    {
      printf("\nPeticio'Nes PUBLICAS:\n\n");
      print_petitions(YES);
    }
  if (no_privates == 0)
    printf("\nNo hay peticio'nes PRIVADAS.\n\n");
  else if (no_privates == 1)
    {
      printf("\nHay una peticio'n PRIVADA:\n\n");
      print_petitions(NO);
    }
  else
    {
      printf("\nPeticio'Nes PRIVADAS:\n\n");
      print_petitions(NO);
    }
  leave();
  big_petition_finish(0);
}
/************************************************************/
void
no_pet_message(void)
{
  printf("\nNo hay peticio'n adjunta al sujeto, \"%s\".\n\nFavor de revisar la ortografi'a en la li'nea de \"Subject\" de su mensaje.\n", 
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
      sprintf(error_msg, "\nProblemas con el sistema en %s: Su %s no fue borrado.\n", 
	      whereami, (no_pet_votes ? "voto" : "firma"));
      return NOT_OK;
      break;
    case NO_MODS:
      sprintf(error_msg, "\nError de programa en pet_out.c:check.\n");
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
  printf("\nEl %s%s adjunto una %s a este \"Subject\":\n\n          \"%s\"\n",
	 date_str(p_item->eVote.open), author_name, 
	 (no_pet_votes? "urna" : "peticio'n"),
	 get_trans(subject, which, YES));
  if (new_pet)
    voters = 0L;
  else
    voters = get_mail_voters(p_item->dropping_id);
  if (p_item->eVote.vstatus == CLOSED) 
    {
      printf("\nEsta %s fue iniciada en %s pero ha sido cerrado desde el %s",
	     (no_pet_votes ? "urna" : "peticio'n"),
	     date_str(p_item->eVote.open), date_str(p_item->eVote.close));
      printf("\n%lu %s%s recopilada%s.\n",
	     voters, (no_pet_votes ? "voto" : "firma"),
	     (voters == 1L ? " fue" : "s fueron"),
	     (voters == 1L ? "" : "s"));
    }
  else if (!new_pet)
    {
      printf("\n%lu %s%s sido recopilado%s.\n",
	     voters, (no_pet_votes ? "voto" : "firma"),
	     (voters == 1L ? " ha" : "s han"),
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
  gen_header(SENDER, "Error:", NO);
  printf("\nEn respuesta a su mensjae que empieza:\n");
  print_tokens(NO);
  /*  print_first_line(); */
  printf("%s", error_msg);
  error_msg[0] = '\0';
  if (dropping_id && item_info)
    {
	printf("\nQuiz�s las instrucciones para \"%s\" ayudara'n.\n",
	       original_subject);
      send_petition_info(SENDER, NULL, NO, NO, NO);
    }  
  leave();
  printf("\n\nSu mensaje entero sigue:\n");
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
      sprintf(error_msg, "\nNo hay recursos disponibles ahorita para procesar su %s \nen \"%s\".  Favor de intentar ma's tarde.\n",
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
			  "\nUd. tiene que dar su \"%s:\"\nantes de poder dar un comentario cuando esta' firmando la peticio'n para\n\n\t\"%s\"\n", force_name(i),
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
	      sprintf(error_msg, "\n\"%s\" no es parte de la forma para:\n\n\t\"%s\".\n",
		      name, original_subject);
	      bounce_info_error();
	    }
	}
      if (answer[i][0] != '\0')
	{
	  sprintf(error_msg, "\nFavor de dar su %s una vez solamente cuando esta'\nfirmando la peticio'n para \"%s\".\n", force_name(i), original_subject);
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
	      sprintf(error_msg, "\nPor favor, limite su respuesta para \"%s:\" a %d letras.\n", field[i].name[the_language], FIELD_LEN);
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
	  sprintf(error_msg, "\nPara %s sobre\n\n \t\"%s\"\n\nhay que incluir su \"%s\"", 
		  (no_pet_votes ? "votar" : "firmar la peticio'n"),
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
			  strcat(error_msg, " y ");
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
	  sprintf(instruction, "\nCuando Ud. entrega su \"%s,\" favor de\narreglar su respuesta para que se parezca asi:\n\n\t%s: %s\n\n", field[i].name[the_language], field[i].name[the_language], field[i].format);
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
	      sprintf(error_msg,"%sdonde Ud. escribe las letras correctas en el lugar de\ntodas las Xs.\n", instruction);
	    }
	  else if (xes == NO)
	    {
	      sprintf(error_msg,"%sdonde Ud. escribe los nu'meros correctos en el lugar de\ntodos los 9s.\n", instruction);
	    }
	  else
	    {
	      sprintf(error_msg,"%sdonde Ud. escribe los nu'meros correctos en el lugar de\ntodos los 9s y las letras correctas en el lugar de todas las Xs.\n", instruction);
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
  printf("\n3.  Favor de arreglar su mensaje para que se parezca asi: \n");
  printf("\n---  corta aqui --- \n");
  for (i = 1; i <= no_pet_votes; i++)
    {
      printf("\n%d. Su voto", i);
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
		     "nombre", 4) == 0)
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
      printf("\nUsted puede a~adir un comentario aqui'.");
    }
  else if (!comment && !name_field)
    {
      printf("\nUd. puede dar su nombre y/o un %s aqui'.",
	     tongue[do_this].comment);
    }
  else if (comment && !name_field)
    {
      printf("\nUd. puede dar su nombre aqui'.");
    }
  printf("\n\n---  corta aqui --- \n");
  if (no_pet_votes)
    {
      printf("\n    Los n�meros \"1.\", \"2.\", hasta 5 se refieren a las preguntas de la\n    Consulta.  Usted puede contestar \"Si'\", \"No\", o \"No Se'\"\n");
    } 
  notes = (required ? 1 : 0) + (optional ? 1 : 0)
    + letters + numbers;
  if (notes == 0)
    return notes;
  printf("\n      %s:\n", (notes > 1 ? "NOTAS" : "NOTA"));
  if (required)
    {
      ++notes_printed;
      printf("\n      *  ");
      len = 39;
      printf("Ud. tiene que dar %s para ", (required == 1?"una respuesta":"respuestas"));
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
	      printf("y ");
	    }
	}
    }
  if (optional)
    {
      ++notes_printed;
      printf("\n      *  ");
      len = 39;
      printf("%s para ",
	     (no_of_fields - required == 1?"Respuesta es opcional":"Respuestas son opcionales"));
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
	      printf("y ");
	    }
	}
    }
  if (numbers)
    {
      printf("\n      *  ");
      printf("Favor de escribir los nu'meros correctos en el lugar de los 9s.\n");
    }
  if (letters)
    {
      printf("\n      *  ");
      printf("Favor de escribir las letras correctas en el lugar de las Xs.\n");
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
      printf("\n%d. %s", i+1, (pet_vote[i] == 0 ? "No Se'" : 
			       (pet_vote[i] == 1
				? "Si'" : "No")));
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
      sprintf(error_msg,"\n\nERROR! %s\nNo se puede abrir el seguro del archivo temporal para guardar \nsu firma para \"%s\".\nFavor de intentar ma's tarde.\n",
	      fname, subject);
      bounce_petition_error(SENDER | OWNER | ADMIN);
    }
  if ((fp = fopen(fname, "a")) == NULL 
     || write_signature(fp, YES, YES) != OK)
    {
      sprintf(error_msg, "\neVote no puede guardar su firma para %s ahora.\n\nFavor de intentar ma's tarde\n", subject);
      bounce_petition_error(SENDER | OWNER | ADMIN);
    }
  if (unlock_to_tmp(NO) != OK)
    {
      fprintf(stderr,"\nERROR! No se puede borrar el archivo temporal: %sT.\n",
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
      sprintf(error_msg,"\nUd. es demasiado tarde para %s sobre:\n\n\t%s\n\nLa %s esta' cerrada.\n",
	      (no_pet_votes 
	       ? (confirm_this.status == VERIFIED ? "confirmar su voto" : "votar")
	       : (confirm_this.status == VERIFIED ? "confirmar su firma" :"firmar la peticio'n")),
	      original_subject,
	      (no_pet_votes ? "urna" : "peticio'n"));
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
      sprintf(error_msg,"\n%s no %s \"%s\".\n",
	      name, (no_pet_votes ? "ha votado" : "firme" ), subject);
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
      sprintf(buf, "Confirme: %s", confirm_this.key);
      gen_header(SENDER, buf, NO);
      printf("Gracias por su mensaje de \"borrar\" sobre\n\n\t%s", 
	     original_subject);
      printf("\n\nPara confirmar que este mensaje en realidad se mando de Ud., por favor\nconfirma este mensaje por usando \"Reply-To\".  No cambia nada de la\nlinea de sujeto.  No necesita escribir nada de nuevo en su mensaje de\nconfirmacio'n.  Esto asegura que todos los peticio'nes de eVote son\nexactos.");
      printf("\n\nVd. debe que mandar su confirmacio'n \"Reply-To\" antes de la peticio'n se \ncierra o no se %s.\n", (no_pet_votes ? "borrara'n sus votos" : "borrara' su firma"));
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
      gen_header(SENDER, "eVote Recibo:", YES);
      time_str[0] = '\0';  /* wipe it out to prevent another i_am_leaving */
      printf("\nSu %s ha sido borrado de \"%s\".",
	     (no_pet_votes ? "voto" : "firma"), original_subject);
      if (pet_vote != NULL)
	{
	  printf("\n\nEste es el voto que ha sido borrado:\n");
	  report_vote();
	}
      printf("\n\nEl comentario que fue borrado es:\n");
    }
  if (from_owner)
    {
      printf("\n%s ha sido borrado\nEl texto que fue borrado es:\n", name);
    }
  drop_signature(mail_voter, stdout, when);
  *pcc = OK;
  return;
}

