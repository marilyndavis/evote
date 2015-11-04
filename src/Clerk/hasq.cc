/* $Id: hasq.cc,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// hasq.cc processes all incoming instructions for clients
// that already have an outgoing queue.
/*********************************************************
 **********************************************************/
#include"evotedef.h"
#include"instruct.h"
#include"qlist.h"
#include "conflist.h"
extern ConfList conferences;
#include"conf.h"
#include"voter.h"
#include"item.h"
#include"memlist.h"
#include"wholist.h"
extern WhoList wholist;
extern MemList memlist;
extern QList qlist;
#include <fstream.h>
extern time_t now;
extern ofstream *logger;
extern ostream *dlogger;
extern int edebug;
char * uid_string(unsigned long uid);  /* in util.cc */
// ********************************
//   AboutItem:: AboutItem  - abstract class for instructions
//    re an established item - i_read, i_vote, send_stats,
//		who_voted, how_voted, change_vstatus.
AboutItem::AboutItem(char *input, int pid, ITYPE itype)
  :HasQ(input, pid, itype)
{
  if (_status == NOT_OK)
    return;
  sscanf(input, FNE_ABOUT_ITEM, NEE_ABOUT_ITEM);
  _piid = new ItemID((STATIC_ID*)(input+4));		
  if ((_p_conf = conferences.find(_p_outq->conf_name())) == NULL
      && (_p_conf = conferences.fetch(_p_outq->conf_name())) == NULL)
    {
      _status = NOT_OK;
      qlist.send(NO_CONF, _qid, 1);
      return;
    }
  if ((_p_item = _p_conf->items().wheres(*_piid)) == NULL)
    {
      clerklog << uid_string(_uid) << " tried to access an unknown item " 
	       << *_piid 
	       << " for voter " << _voter << " on " << get_itype(itype);
      _status = NOT_OK;
      qlist.send(NO_ITEM, _qid, 1);
    }
  _rest = input + sizeof(STATIC_ID) + 4;
}
AboutItem::~AboutItem(void)
{
  delete _piid;
}
//  *********************************************
//    takes the conf on-line but so that it will be picked up again
//    the next time someone enters it.
ChangeAction::ChangeAction(char *input, int pid, ITYPE itype)
  :HasQ(input, pid, itype)
{
  ACTION new_action;
  if (_status == NOT_OK)
    return;
  sscanf(input, FNE_CHANGE_ACTION, NEE_CHANGE_ACTION);
  switch (_p_voter->change_action(new_action))
    {
    case OK:
      qlist.send(GOOD, _qid);
      break;
    case UNDECIDED:
      qlist.send(NO_CHANGE, _qid);
      break;
    default:
      break;
    }
}
// ***********************************************************
ChangeVStatus::ChangeVStatus(char * input, int pid, ITYPE itype)
  :AboutItem(input, pid, itype)
{
  VSTATUS new_vstatus;
  unsigned long uid_asking;
  if (_status == NOT_OK)
    return;
  sscanf(_rest, FNE_CHANGE_VSTATUS, NEE_CHANGE_VSTATUS);
  switch (_p_voter->p_conf()->change_vstatus(_p_item->dropping_id(),
					     new_vstatus, _voter))
    {
    case OK:
      qlist.send(DONE, _qid);
      break;
    case PROBLEM:
      qlist.send(NO_CHANGE, _qid);
      break;
    case UNDECIDED:
      qlist.send(REDUNDANT, _qid);
      break;
    case NOT_OK:
      qlist.send(NOT_GOOD, _qid);
      break;
    default:
      break;
    }
  return;
}
// ***********************************************************
CreateItems::CreateItems(char *input, int pid, ITYPE itype, int len)
  :HasQ(input, pid, itype)
{
  int no_items;
  char *get_name(unsigned int);
  if (_status == NOT_OK)
    return;
  switch (_p_voter->p_conf()->create_items(input, len,
					   _uid, &no_items))
    {		
    case OK:
      qlist.send(NEW_STAT, _qid, sizeof(ITEM_STAT)*no_items);
      break;
    case NOT_OK:  // some item is already there  
      qlist.send(REDUNDANT, _qid, 1);
      break;
    case PROBLEM:  // no system resources
      qlist.send(NO_ITEM, _qid, 1);
      break;
    case UNDECIDED:
      qlist.send(RESEND, _qid, 1);
      break;
    default:
      break;
    }
}		
// ***********************************************************
extern int new_stuff;  /* sneaking a return back into main */
DropItems::DropItems(char *input, int pid, ITYPE itype, int len)
  :HasQ(input, pid, itype)
{
  if (_status == NOT_OK)
    return;
  _p_voter->p_conf()->drop_items(input, len, _uid);
  qlist.send(GOOD, _qid);
#ifdef PETITION
  if (strncmp(_p_voter->p_conf()->name(),"petition", 8) == 0)
    {
      new_stuff += _p_voter->p_conf()->drop_pending_non_signers();
    }
#endif
  return;
}
// ***********************************************************
Move::Move(char *input, int pid, ITYPE itype)
  :HasQ(input, pid, itype)
{
  char was[200];
  char is[200];
  if (_status == NOT_OK)
    return;
#ifdef EDEBUG
  if (edebug & QUEUES)
    dlog << "Asked " << uid_string(_uid) << " to drop " << _qid 
	 << " : pid = " << pid << ", for " << get_itype(_itype);
#endif
  sscanf(input, FNE_MOVE, NEE_MOVE);
  switch (wholist.move(was, is))
    {
    case OK:  // moved the voter
      qlist.send(DEL_DONE, _qid, 1);
      break;
    case PROBLEM: // Both addresses exist
      qlist.send(DEL_NOT_GOOD, _qid, 1);
      break;
    case UNDECIDED:  // already done?
      qlist.send(DEL_REDUNDANT, _qid, 1);
      break;
    case NOT_OK:  // Neither name in list or bad new name
      qlist.send(DEL_NO_VOTER, _qid, 1);
      break;
    case STOP:      // programmer problem
      qlist.send(DEL_NO_CONF, _qid, 1);
      break;
    default:
      break;
    }
}
// ****************************************************
//   Returns the vote of the ask-for user.
HowVoted::HowVoted(char *input, int pid, ITYPE itype)
  :AboutItem(input, pid, itype)
{
  unsigned long how_uid;
  if (_status == NOT_OK)
    return;
  sscanf(_rest, FNE_HOW_VOTED, NEE_HOW_VOTED);
  switch (_p_item->how_voted(qlist.output, _p_conf, how_uid, _voter))
    {
    case OK:
      qlist.send(GOOD, _qid);
      break;
    case UNDECIDED:
      qlist.send(STRING_OUT, _qid, 0);
      break;
    case NOT_OK:
      qlist.send(NO_VOTER, _qid);
      break;
    default:
      break;
    }
  return;
}
//  *******************************************************
//   records the fact that the user has now read the item
//   and sends back a new stat if the user hadn't read the
//   item before.
IRead::IRead(char *input, int pid, ITYPE itype)
  :AboutItem(input, pid, itype)
{
  short old_entry;
  long len;
  if (_status == NOT_OK)
    return;
  len = _p_voter->read(_p_item, &old_entry);
  switch (old_entry)
    {
    case NOT_READ: 		
    case UNREAD_MIN:
      qlist.send(NEW_STAT, _qid, len);
      break;
    default:  // read before or even voted on before
      qlist.send(NO_CHANGE, _qid, len);
      break;
    }
}		
//  **********************************************
//     records the users vote and sends back a stat refresh
//     if the vote changed.
IVote::IVote(char * input, int pid, ITYPE itype)
  :AboutItem(input, pid, itype)
{
  RTYPE cc = GOOD;
  short vote, old_vote;
  long len;
  if (_status == NOT_OK)
    return;
  sscanf(_rest, FNE_SEND_VOTE, NEE_SEND_VOTE);
  len = _p_voter->vote(&cc, _p_item, vote, &old_vote);
  switch (cc)
    {
    case MORE_STATS:
      // first vote on a grouped item
      len = ((GroupedItem*)_p_item)->send_stats((ITEM_STAT*)qlist.output, 
						_p_voter->p_ballot(), YES);
      do
	{
	  qlist.send(MORE_STATS, _qid, len);
	  len = (int)((GroupedItem*)_p_item)->send_stats((ITEM_STAT*)qlist.output, 
							 _p_voter->p_ballot(), NO);
	}
      while (len > 0);
      len = (int)_p_item->report_stat((ITEM_STAT*)qlist.output, WITH_VOTE,
				      _p_voter->p_ballot(), old_vote);
    case GOOD:
    case NEW_STAT:
      qlist.send(NEW_STAT, _qid, len);
      break;
    default:  // all the errors are in this bunch
      qlist.send(cc, _qid, len);
      break;
    }
}
// ***************************************************
//  drops the associated queue and the associated voter.
LeaveAdmin::LeaveAdmin(char * input, int pid, ITYPE itype)
  :HasQ(input, pid, itype)
{
  YESorNO user_dropped = NO;
  YESorNO &no = user_dropped;
  if (_status == NOT_OK)
    return;
  _p_conf->expose();
  delete _p_voter;
  switch (qlist.drop_q(_p_outq, no, NO))
    {
    case NOT_OK:
#ifdef EDEBUG
      if (edebug & QUEUES)
	dlog << "LeaveAdmin:couldn't drop q for pid " << pid;
#endif
      clerklog << "LeaveAdmin:couldn't drop q for pid " << pid;
      break;
    case UNDECIDED:
#ifdef EDEBUG
      if (edebug & QUEUES)
	dlog << "LeaveAdmin:couldn't drop busy q for pid " << pid;
#endif
      break;
    case OK:
      break;
    default:
      // impossible
      break;
    }
}
// ***************************************************
//  drops the associated queue and the associated voter.
Leaving::Leaving(char * input, int pid, ITYPE itype)
  :HasQ(input, pid, itype)
{
  char * get_name(unsigned long uid);
  YESorNO user_dropped = NO;
  YESorNO &no = user_dropped;
  if (_status == NOT_OK)
    return;
  if (_p_voter != NULL)
    {
#ifdef EDEBUG
      if (edebug & VOTERS)
	dlog << get_name(_p_voter->uid()) << ":" << _p_voter->uid() 
	     << " leaving "<< _p_voter->p_conf()->name() << ".";
#endif
      _p_voter->p_conf()->community().forget(_p_voter);
    }
  switch (qlist.drop_q(_p_outq, no, NO))
    {
    case NOT_OK:
#ifdef EDEBUG
      if (edebug & QUEUES)
	dlog << "Leaving:couldn't drop q for pid " << pid;
#endif
      clerklog << "Leaving:couldn't drop q for pid " << pid;
      break;
    case UNDECIDED:
#ifdef EDEBUG
      if (edebug & QUEUES)
	dlog << "Leaving:couldn't drop busy q for pid " << pid;
#endif
      break;
    case OK:
      break;
    default:
      // impossible
      break;
    }
}
//  **************************************************
MidDropped::MidDropped(char* input, int pid, ITYPE itype)
  :HasQ(input, pid, itype)
{
  int mid;
  if (_status != OK)
    return;
  sscanf(input, FNE_MID_DROPPED, NEE_MID_DROPPED);
  memlist.mid_dropped(mid);
}
// ***************************************************
//  PullTime - Gets the timestamp and offset on a TIMESTAMP
//             vote type.
PullTime::PullTime(char *input, int pid, ITYPE itype)
  :AboutItem(input, pid, itype)
{
  time_t time_stamp;
  streampos offset;
  if (_status == NOT_OK)
    return;
  time_stamp = _p_voter->pull_time(_p_item, &offset);
  sprintf(qlist.output, FEN_PULL_TIME, EEN_PULL_TIME);
  qlist.send(GOOD, _qid);
}
// ***************************************************
//  PushTime - Stores a timestamp and offset on a TIMESTAMP
//             vote type.
//             Returns the old timestamp and offset.
PushTime::PushTime(char *input, int pid, ITYPE itype)
  :AboutItem(input, pid, itype)
{
  time_t new_time, old_time;
  streampos new_offset =0L, old_offset;
  if (_status == NOT_OK)
    return;
  sscanf(_rest, FNE_PUSH_TIME, NEE_PUSH_TIME);
  old_time = _p_voter->push_time(_p_item, new_time, new_offset, &old_offset);
  sprintf(qlist.output, FEN_PUSH_TIME, EEN_PUSH_TIME);
  qlist.send(GOOD, _qid);
}
// ****************************************************
//   Returns the vote of the ask-for user.
SendStamp::SendStamp(char *input, int pid, ITYPE itype)
  :AboutItem(input, pid, itype)
{
  unsigned long how_uid;
  time_t when;
  if (_status == NOT_OK)
    return;
  sscanf(_rest, FNE_SEND_STAMP, NEE_SEND_STAMP);
  when = ((TimeStampItem*)_p_item)->send_stamp(_p_conf, how_uid, _voter);
  sprintf(qlist.output, FEN_SEND_STAMP, EEN_SEND_STAMP);
  qlist.send(GOOD, _qid);
}
// ***************************************************
//  SendStats - gathers statistics for the index screen
SendStats::SendStats(char *input, int pid, ITYPE itype)
  :AboutItem(input, pid, itype)
{
  int len;
  int no_to_send;
  if (_status == NOT_OK)
    return;
  sscanf(_rest, FNE_SEND_STATS, NEE_SEND_STATS);
  len = (int)_p_conf->send_stats(_p_voter, _p_item, no_to_send);
  qlist.send(NEW_STAT, _qid, len);
}
// *********************************************************
UidExist::UidExist(char *input, int pid, ITYPE itype)
  :HasQ(input, pid, itype)
{
  unsigned long uid_to_check;
  if (_status == NOT_OK)
    return;
  sscanf(input, FNE_UID_EXIST, NEE_UID_EXIST);
  _p_conf = _p_voter->p_conf();
  if (_p_conf->does_uid_exist(uid_to_check) == YES)
    qlist.send(GOOD, _qid, 1);
  else
    qlist.send(NOT_GOOD, _qid, 1);
  return;
}
// ************************************************************
//  This instruction sends back a uid list of who voted on the
//  petition item and includes the timestamp for each signer
WhoSigned::WhoSigned(char * input, int pid, ITYPE itype)
  :AboutItem(input, pid, itype)
{
  int len;
  short sdummy;
  YESorNO cont = NO;
  if (_status == NOT_OK)
    return;
  if (_p_item->type() != 'S')
    {
      sprintf(qlist.output, "WhoSigned can only be used on petitions.");
      qlist.send(STRING_OUT, _qid, 0);
      return;
    }
  sscanf(_rest, FNE_WHO_SIGNED, NEE_WHO_SIGNED);
  do
    {
      len = (int)((TimeStampItem*)_p_item)->who_signed(qlist.output, _p_conf, &cont);
      if (len == -1)
	qlist.send(STRING_OUT, _qid, 0);
      else if (cont == YES)
	qlist.send(UID_LIST_MORE, _qid, len);
      else
	qlist.send(UID_LIST, _qid, len);
    }
  while (cont == YES);
}
// ************************************************************
//  This instruction processes questions like who voted <2 on this
//  item or who voted >= 8.
WhoVoted::WhoVoted(char * input, int pid, ITYPE itype)
  :AboutItem(input, pid, itype)
{
  char question[QUESTLEN];
  int len;
  short sdummy;
  YESorNO cont = NO;
  if (_status == NOT_OK)
    return;
  sscanf(_rest, FNE_WHO_VOTED, NEE_WHO_VOTED);
  do
    {
      len = (int)_p_item->who_voted(qlist.output, _p_conf, question, &cont);
      if (len == -1)
	qlist.send(STRING_OUT, _qid, 0);
      else if (cont == YES)
	qlist.send(UID_LIST_MORE, _qid, len);
      else
	qlist.send(UID_LIST, _qid, len);
    }
  while (cont == YES);
}
