/* $Id: blurbs.c,v 1.3 2003/01/15 18:54:08 marilyndavis Exp $ */ 
/******************************************************
 *      eVote/src/demo/eVoteui/blurbs.c  - eVote blurbs for the demo.
 ************************************************************
 **********************************************************/
#include "../eVote.h"
#define NO_BLURBS  (41)
YESorNO show_blurbs = YES;
char blurb[NO_BLURBS][NO_LINES][NO_COLS] =
{  
  /*************************************************************************
   *              NULL received -  '?' pressed
   ************************************************************************/ 
  {
    "                                                                            ",
    "      eVote DEMO MENU  -  You are in the | conference.                      ",
    "                          The following commands are available:             ",
    "                                                                            ",
    "          c  -  to see the contents of the current conference.              ",
    "          #  -  where # is a number - allows you to read item number #.     ",
    "          v  -  to vote on the item you just read - or to perform other     ",
    "                eVote functions.                                            ",
    "                                                                            ",
    "          a  -  to add a new item to the | conference.                      ",
    "          d  -  to drop an item from the | conference.                      ",
    "                                                                            ",
    "          g  -  to leave this conference and go to another.  If you ask     ",
    "                to go to a conference that does not exist, it will be       ",
    "                created.                                                    ",
    "          l  -  to list the available conferences.                          ",
    "                                                                            ",
    "          q  -  to quit.                                                    ",
    "          t  -  to access the teacher.                                      ",
    "          x  -  to get an eXplanation of some subject.                      ",
    "          ?  -  to see your current choices.                                ",
    "                                                                            ",
  }, 
  /*************************************************************************
   *  1   opening screen
   ************************************************************************/
  {
    "                                            VV                              ",
    "                                           VV                               ",
    "                                          VV                                ",
    "                                         VV                                 ",
    "                                        VV                                  ",
    "                 VVV                   VV            VV                     ",
    "                   VV                 VV             VV                     ",
    "          VVVVVV    VV               VV  VVVVVV    VVVVVV    VVVVVV         ",
    "         VV    VV    VV             VV  VV    VV     VV     VV    VV        ",
    "        VV      VV    VV           VV  VV      VV    VV    VV      VV       ",
    "        VVVVVVVVVV     VV         VV   VV      VV    VV    VVVVVVVVVV       ",
    "        VV              VV       VV    VV      VV    VV    VV               ",
    "         VV    VV        VV     VV      VV    VV     VV     VV    VV        ",
    "          VVVVVV          VV   VV        VVVVVV      VV      VVVVVV         ",
    "                           VV VV                                            ",
    "                            VVV                                             ",
    "                             V                                              ",
    "                                                                            ",
    "        Copyright (c) 1994...2015 Deliberate.com Patented                    ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
  }, 
  /*************************************************************************
   *  2    - second opening screen
   *************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "        eVote                                                               ",
    "                                                                            ",
    "          *   is a tool for direct democracy.                               ",
    "                                                                            ",
    "          *   gives us face-to-face meetings -- online.                     ",
    "                                                                            ",
    "          *   allows us to invent vote questions for our                    ",
    "              online community.                                             ",
    "                                                                            ",
    "          *   collects, tabulates and reports our votes.                    ",
    "                                                                            ",
    "                                                                            ",
    "   This demo contains a teacher.  Enter `t' at any prompt to use it.        ",
    "                                                                            ",
    "   You can always enter `?' to get information about your current           ",
    "   choices.  Enter `?' again for more help.                                 ",
    "                                                                            ",
    "   For an explanation of any word in eVote's vocabulary, enter              ",
    "   `x the-word' and eVote will eXplain the-word.                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   * 3
   **************************************************************************/
  {
    " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *",
    " *                                                                         *",
    " *   After entering item number ~, as you just did, you expect to read     *",
    " *   some text about ^@.^                                                  *",
    " *                                                                         *",
    " *   However, there is no conferencing system attached to this eVote       *",
    " *   demo program so user-contributed text is not available.               *",
    " *                                                                         *",
    " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *",
    ".                                                                           ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "16                                                                          ",
    "17                                                                          ",
    "18                                                                          ",
    "19                                                                          ",
    "20                                                                          ",
    "21                                                                          "
  }, 
  /*************************************************************************
   *  4
   ***************************************************************************/
  {
    " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *    ",
    " *                                                                     *    ",
    " *   There is no text about ^@.^                                       *    ",
    " *                                                                     *    ",
    " *   Also, you can't contribute text about item #~.                    *    ",
    " *                                                                     *    ",
    " *   However, when eVote is integrated into your conferencing system   *    ",
    " *   or BBS, you will be able to read and contribute text about the    *    ",
    " *   items listed in the conferences.                                  *    ",
    " *                                                                     *    ",
    " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *    ",
    ".                                                                           ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   *   5
   ************************************************************************/
  {
    " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *      ",
    " *                                                                   *      ",
    " *   You can compose new items for the ^|^ conference.               *      ",
    " *                                                                   *      ",
    " *   Although you can't discuss your items, you can set up a vote    *      ",
    " *   on them by using the (a)dd feature.                             *      ",
    " *                                                                   *      ",
    " *   Be sure to add some items.  Only then will you experience the   *      ",
    " *   full power of eVote.                                            *      ",
    " *                                                                   *      ",
    " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *      ",
    ".                                                                           ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    ".                                                                           ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   * 6
   ************************************************************************/
  {
    "                                                                            ",
    " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *    ",
    " *                                                                     *    ",
    " *         A conference without eVote is a just a discussion.          *    ",
    " *                                                                     *    ",
    " *         eVote without a conference is halfway between a             *    ",
    " *               democracy and a plebiscite.                           *    ",
    " *                                                                     *    ",
    " *  [ A plebiscite, in political theory terms, is a limited form of ]  *    ",
    " *  [ democracy where the voters respond to some choices presented  ]  *    ",
    " *  [ to them by someone with superior power, the power to invent   ]  *    ",
    " *  [ vote questions.  There is no substantive discussion.          ]  *    ",
    " *                                                                     *    ",
    " *             A conference with eVote is REAL DEMOCRACY.              *    ",
    " *                                                                     *    ",
    " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *    ",
    ".                                                                           ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
  }, 
  /*************************************************************************
   * 7
   ************************************************************************/
  {
    "                                                                            ",
    " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *        ",
    " *                                                                 *        ",
    " *   Brian Fay, a political theorist, has said about democracy     *        ",
    " *   that what ^is most significant is the involvement of the      *        ",
    " *   citizens in the process of determining their own collective   *        ",
    " *   identity.^  Thus, the primary activity of a real democracy    *        ",
    " *   is discussion.  Voting is the secondary activity, the icing   *        ",
    " *   on the cake.  eVote is icing for the cyberspace cake.         *        ",
    " *                                                                 *        ",
    " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *        ",
    ".                                                                           ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   *   EIGHT
   *************************************************************************/
  {
    "                                                                            ",
    " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *          ",
    " *                                                               *          ",
    " *  Brand X Electronic Democracy involves taking a big vote on   *          ",
    " *  issues determined by powerful people.  That is plebiscitism. *          ",
    " *                                                               *          ",
    " *  Brand X was criticized in 1982 by Jean Betheke Elshtain, a   *          ",
    " *  political scientist, as being an ^interactive shell game     *          ",
    " *  [that] cons us into believing that we are participating when *          ",
    " *  we are really simply performing as the responding 'end' of a *          ",
    " *  prefabricated system of external stimuli.^                   *          ",
    " *                                                               *          ",
    " *  ^In a plebiscitary system, the views of the majority, ... ,  *          ",
    " *  swamp minority or unpopular views.  Plebiscitism is compat-  *          ",
    " *  ible with authoritarian politics carried out under the guise *          ",
    " *  of, or with the connivance of, majority views.  That opinion *          ",
    " *  can be registered by easily manipulated, ritualistic plebis- *          ",
    " *  cites, so there is no need for debate on substantive         *          ",
    " *  questions.^                                                  *          ",
    " *                                                               *          ",
    " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *          ",
    "                                                                            ",
  }, 
  /*************************************************************************
   * NINE
   **************************************************************************/
  {
    "                                                                            ",
    " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *              ",
    " *                                                           *              ",
    " *   Electronic Democracy, eVote-style, is real democracy.   *              ",
    " *                                                           *              ",
    " *     +   You can always raise a vote question.             *              ",
    " *                                                           *              ",
    " *     +   You can always add a comment when you vote.       *              ",
    " *                                                           *              ",
    " *     +   You can discuss and discuss and discuss.          *              ",
    " *                                                           *              ",
    " *     +   You can change your vote.                         *              ",
    " *                                                           *              ",
    " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *              ",
    ".                                                                           ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   *  TEN
   ***************************************************************************/
  {
    ".                                                                           ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   *  ELEVEN
   ***************************************************************************/
  {
    "                                                                            ",
    ".                                                                           ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   *  TWELVE
   ***************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   *   THIRTEEN
   ************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   *  FOURTEEN
   ************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   * FIFTEEN
   *************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   * SIXTEEN
   **************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   *   SEVENTEEEN
   ***************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   *  EIGHTEEN
   ***************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   *  NINTEEN
   ***************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   *  TWENTY
   ***************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   *   TWENTY ONE
   ************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   *   TWENTY TWO
   *************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   *  23
   **************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   *  24
   ***************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   * 25
   ***************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   * 26
   ***************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   *  27
   ***************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   * 28
   ***************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   *29
   *************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   * 30
   **************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   * 31
   ***************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   *  32
   ***************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   * 33
   ***************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   * 34
   ***************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   * 35
   ***************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   * 36
   ***************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   * 37
   **************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   * 38
   ***************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   * 39
   ***************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }, 
  /*************************************************************************
   * 40
   ***************************************************************************/
  {
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "
  }};
/****************************************************************************/
char * 
do_blurb(ITEM_INFO *p_item)
{
  OKorNOT try_poetry(ITEM_INFO *p_item,char **new_input);
  static short visits = 2;
  void show_it(short index, ITEM_INFO *p_item);
  static short nulls = 0;
  char *new_input;
  
  if(strcmp(current_conf,"Poetry") == 0 && visits > 3)
    {
      if(try_poetry(p_item, &new_input) == OK)
	return new_input;
    }
  
  if (p_item == NULL)
    {
      if (++nulls > 2)
	show_it(0, p_item);
      else
	show_it(nulls, p_item);
      return NULL;
    }
#ifdef TITLE
  printf("\n%s Conference: Item # %d --> %s",current_conf,p_item->dropping_id,
	 p_item->eVote.title);
#endif
  if (blurb[++visits][0][0] == '.')
    {
      visits = 2;
      if(show_blurbs == YES)
	{
	  printf("\n * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *            ");
	  printf("\n *                                                             *            ");
	  printf("\n *   That's the end of the lecture on democracy.  The lecture  *            ");
	  printf("\n *   will start over unless you enter 's' for Silence! at the  *            ");
	  printf("\n *   Conf? prompt.                                             *            ");
	  printf("\n *                                                             *            ");
	  printf("\n * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *            ");
	}
      show_it(-1,p_item);
    }
  else
    if(show_blurbs == YES)
      show_it(visits, p_item);
    else
      show_it(-1,p_item);
  return NULL;
}
/****************************************************************************
 *     This is called when the user presses '?' at the Conf? prompt
 *     and she is not currently in a conf.  Shortened menu:
 ****************************************************************************/
#define LINES_IN_BLURBETTE 16
void 
do_blurbette(void)
{
  char blurbette[LINES_IN_BLURBETTE][NO_COLS+1] ={ 
    
    "                                                                            ",
    "      eVote DEMO MENU  -  You are not in a conference right now.            ",
    "                          The following commands are available:             ",
    "                                                                            ",
    "          l  -  to list the available conferences.                          ",
    "          g  -  to leave this conference and go to another.  If you ask     ",
    "                to go to a conference that does not exist, it will be       ",
    "                created for you.                                            ",
    "                                                                            ",
    "          q  -  to quit.                                                    ",
    "          ?  -  to see your current choices.                                ",
    "          x  -  to get an eXplanation of some subject.                      ",
    "          t  -  to access the teacher.                                      ",
    "                                                                            ",
    "                                                                            ",
    "                                                                            "};
  short i;
  for(i = 0; i < LINES_IN_BLURBETTE; i++)
    {
      printf("\n");
      printf("\n%s",blurbette[i]);
    }
}
/**************************************************************************/
void 
show_it(short index, ITEM_INFO* p_item)
{
  short line, ic;
  int ch;
  short extra = 0;
  int i;
#ifdef TITLE
  char strip_title[TITLEN + 1];
  
  if(p_item != NULL)
    {
      strcpy(strip_title,p_item->eVote.title);
      i = TITLEN;
      while(strip_title[i] == ' ')
	i--;
      strip_title[i++] = '\0';
    }
#endif
  
  for (line = 0; line < NO_LINES; line++)
    {
      printf("\n");
      if(index == -1 || blurb[index][line][0] == '.')
	{
#ifdef TITLE
	  if ( line + 2 < NO_LINES && p_item != NULL)
	    printf("\n(Pretend you just read some text about %c%s%c.)\n",
		   34,strip_title,34);
#endif
	  break;
	}
      extra = 0;
      for (ic = 0; ic < NO_COLS; ic++)
	{
	  ch = (int)blurb[index][line][ic];
	  switch (ch)
	    {
	    case 0:
	      break;
	    case '~':
	      printf("%d",p_item->dropping_id);
	      if(p_item->dropping_id > 9)
		extra++;
	      if(p_item->dropping_id > 99)
		extra++;
	      if(p_item->dropping_id > 999)
		extra++;
	      break;
	    case '|':
	      printf("%s",current_conf);
	      extra += (short)strlen(current_conf);
	      break;
	    case '^':
	      printf("%c",34);
	      break;
	    case '@':
#ifdef TITLE
	      printf("%s",strip_title);
	      extra += (short)strlen(strip_title);
#endif
	      break;
	    case ' ':
	      if (extra > 0)
		{
		  for (i = ic; i < NO_COLS; i++)
		    {
		      if (blurb[index][line][i] == ' ')
			continue;
		      break;
		    }
		  if(i == NO_COLS)
		    break;
		  if(i-ic >= 2)
		    {
		      if (i-ic-1 >= extra)
			{
			  ic+= extra - 1;
			  extra = 0;
			}
		      else
			{
			  extra -= (i-ic-1);
			  ic += (i-ic-1);
			}
		    }
		}
	      printf(" ");
	      break;
	    default:
	      printf("%c",blurb[index][line][ic]);
	      break;
	    }
	}
    }
}
/********************************************************
 *     Turns off the blurbs if they're on, and turns
 *     them on if they're off.
 *******************************************************/
void 
toggle_blurbs(void)
{
  if(show_blurbs == NO)
    show_blurbs = YES;
  else
    show_blurbs = NO;
}
