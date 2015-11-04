/* $Id: instruct.h,v 1.3 2003/01/15 18:54:07 marilyndavis Exp $ */ 
// instruct.h header for the Instruction hierarchy.
/*********************************************************
 *    Copyright (c) 1994...2015 Deliberate.com Patented.
 *    by Marilyn Davis
 **********************************************************/
#ifndef INSTRUCTIONHPP
#define INSTRUCTIONHPP
#include<stdio.h>
#include "evotedef.h"
class OutQ;
class Voter;
class Conf;
class Item;
class ItemID;
int reset_nice(int);  /* in main.cc */
/* #include "itemid.h" */
//  **********************************************
//   Instruction and some of its children are abstract.  These
//   are listed first.
//  *************************************************
class Instruction
{
 public:
  Instruction(char * input, int pid, ITYPE itype);
  ~Instruction(void);
  char  _conf_name[CONFLEN + 1];
  int _pid;
  OKorNOT _status;
  ITYPE _itype;		
  Voter* _p_voter;		
  char msg_end[3]; // for collecting the end of message Q! 
  time_t _now;
  Conf* _p_conf;
  OutQ* _p_outq;
  int _qid;
  unsigned long _uid;
  unsigned long _voter;
 protected:
  YESorNO _fetched;
 private:
};
class NeedsQ:public Instruction
{
 public:  
  NeedsQ(char *input, int pid, ITYPE itype);
};
class HasQ:public Instruction
{
 public:
  HasQ(char * input, int pid, ITYPE itype);
};
class RespondOnce: public NeedsQ
{
 public:
  RespondOnce(char* input, int pid, ITYPE itype);
  ~RespondOnce(void);
};
class MaybeQ: public Instruction
{
 public:
  MaybeQ(char* input, int pid, ITYPE itype);
  ~MaybeQ(void);
 private:
  YESorNO tmpq;
};
class AboutItem:public HasQ
{
 public:
  AboutItem(char *input, int pid, ITYPE itype);
  ~AboutItem(void);
 protected:
  char* _rest;	/* where to read rest of instruction */ 
  ItemID * _piid;
  STATIC_ID *_static_id;   /* application id */
  Item* _p_item;
};
// *****************************************
//  The  following classes will be instantiated.
//  ************************************************
class AdjournConf:public RespondOnce
{
 public:
  AdjournConf(char* input, int pid, ITYPE itype);
};
class ChangeAction:public HasQ
{
 public:
  ChangeAction(char* input, int pid, ITYPE itype);
};
class ChangeVStatus: public AboutItem
{
 public:
  ChangeVStatus(char* input, int pid, ITYPE itype);
};
class CheckConf: public RespondOnce
{
 public:
  CheckConf(char* input, int pid, ITYPE itype);
};
class CreateConf: public RespondOnce
{
 public:
  CreateConf(char *input, int pid, ITYPE itype);
};
class CreateItems: public HasQ
{
 public:
  CreateItems(char *input, int pid, ITYPE itype, int len);
};
class DoDebug: public Instruction
{
 public:
  DoDebug(char *input, int pid, ITYPE itype);
};
class UidExist: public HasQ
{
 private:
 public:
  UidExist(char * input, int pid, ITYPE itype);
};
class DropConf: public RespondOnce
{
 public:
  DropConf(char * input, int pid, ITYPE itype);
};	
class DropOldQs: public Instruction
{
 public:
  DropOldQs(char *input, int pid, ITYPE itype);
};
class DropItems:public HasQ
{
 private:
 public:
  DropItems(char *input, int pid, ITYPE itype, int len);
};
class DropVoter:public RespondOnce
{
 public:
  DropVoter(char* input, int pid, ITYPE itype);
};
class DownPriority:public Instruction
{
 public:
  DownPriority(char *input, int pid, ITYPE itype);
};
class EnterAdmin: public NeedsQ
{
 public:		
  EnterAdmin(char *input, int pid, ITYPE itype);
};
class Entering: public NeedsQ
{
 public:		
  Entering(char *input, int pid, ITYPE itype);
};
class Exist: public RespondOnce
{
 public:
  Exist(char * input, int pid, ITYPE itype);
};
class Flush: public Instruction
{
 public:
  Flush(char *input, int pid, ITYPE itype);
};
class GrowConf: public Instruction
{
 public:
  GrowConf(char *input, int pid, ITYPE itype);
};
class Hello: public RespondOnce
{
 public:
  Hello(char *input, int pid, ITYPE itype);
};
class HowVoted: public AboutItem
{
 public:
  HowVoted(char *input, int pid, ITYPE itype);
};
class IRead: public AboutItem
{
 public:
  IRead(char *input, int pid, ITYPE itype);
};
class IVote: public AboutItem
{
 public:
  IVote(char *input, int pid, ITYPE itype);
}; 
class Joining: public NeedsQ
{
 public:		
  Joining(char *input, int pid, ITYPE itype);
};
class Leaving: public HasQ
{
 public:
  Leaving(char *input, int pid, ITYPE itype);
};
class LeaveAdmin: public HasQ
{
 public:
  LeaveAdmin(char *input, int pid, ITYPE itype);
};
class Move: public HasQ
{
 public:
  Move(char *input, int pid, ITYPE itype);
};
class MidDropped:public HasQ
{
 public:
  MidDropped(char *input, int pid, ITYPE itype);
};
class NewExe: public Instruction
{
 public:
  NewExe(char *input, int pid, ITYPE itype);
};
class NewLog: public Instruction
{
 public:
  NewLog(char *input, int pid, ITYPE itype);
};
class PullTime: public AboutItem
{
 public:
  PullTime(char *input, int pid, ITYPE itype);
};
class PushTime: public AboutItem
{
 public:
  PushTime(char *input, int pid, ITYPE itype);
};
class Quit:public Instruction
{
 public:
  Quit(char *input, int pid, ITYPE itype);
};
class ReorderConf:public Instruction
{
 public:
  ReorderConf(char *input, int pid, ITYPE itype);
};
class SendStamp: public AboutItem
{
 public:
  SendStamp(char *input, int pid, ITYPE itype);
};
class SendStats:public AboutItem
{
 public:
  SendStats(char *input, int pid, ITYPE itype);
};
class SyncConf:public RespondOnce
{
 public:
  SyncConf(char * input, int pid, ITYPE itype, int len);
};
class UpPriority:public Instruction
{
 public:
  UpPriority(char *input, int pid, ITYPE itype);
};
class WhoDrop:public RespondOnce
{
 public:
  WhoDrop(char* input, int pid, ITYPE itype);
};
class WhoIs:public MaybeQ
{
 public:
  WhoIs(char* input, int pid, ITYPE itype);
};
class WhoNum:public MaybeQ
{
 public:
  WhoNum(char* input, int pid, ITYPE itype);
};
class WhoSync:public RespondOnce
{
 public:
  WhoSync(char *input, int pic, ITYPE itype);
};
class WhoSigned: public AboutItem
{
 public:
  WhoSigned(char *input, int pid, ITYPE itype);
};
class WhoVoted: public AboutItem
{
 public:
  WhoVoted(char *input, int pid, ITYPE itype);
};
class WhosIn:public RespondOnce
{
 public:
  WhosIn(char* input, int pid, ITYPE itype);
};
#endif
