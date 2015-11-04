#! /usr/bin/env python
#
# Copyright (C) 1998,1999,2000 by the Free Software Foundation, Inc.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software 
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

"""Clone a member address.

Cloning a member address means that a new member will be added who has all the
same options and passwords as the original member address.  Note that this
operation is fairly trusting of the user who runs it -- it does no
verification to the new address, it does not send out a welcome message, etc.

The existing member's subscription is usually not modified in any way.  If you
want to remove the old address, use the -r flag.  If you also want to change
any list admin addresses, use the -a flag.

Usage:
    clone_member [options] fromoldaddr tonewaddr

Where:

    --listname=listname
    -l listname
        Check and modify only the named mailing lists.  If -l is not given,
        then all mailing lists are scanned from the address.  Multiple -l
        options can be supplied.

    --remove
    -r
        Remove the old address from the mailing list after it's been cloned.

    --admin
    -a
        Scan the list admin addresses for the old address, and clone or change
        them too.

    --quiet
    -q
        Do the modifications quietly.

    --nomodify
    -n
        Print what would be done, but don't actually do it.  Inhibits the
        --quiet flag.

    --help
    -h
        Print this help message and exit.

 fromoldaddr (`from old address') is the old address of the user.  tonewaddr
 (`to new address') is the new address of the user.

"""

import sys
import string
import getopt

import paths
from Mailman import MailList
from Mailman import Utils
from Mailman import Errors



def usage(status, msg=''):
    print __doc__ % globals()
    if msg:
        print msg
    sys.exit(status)


def dolist(mlist, options):
    if not options.quiet:
        print 'processing mailing list:', mlist.internal_name()

    # scan the list owners.  TBD: mlist.owner keys should be lowercase?
    oldowners = mlist.owner[:]
    oldowners.sort()
    if options.admintoo:
        if not options.quiet:
            print '    scanning list owners:', string.join(oldowners, ' '),
        newowners = {}
        foundp = 0
        for owner in mlist.owner:
            if options.lfromaddr == string.lower(owner):
                foundp = 1
                if options.remove:
                    continue
            newowners[owner] = 1
        if foundp:
            newowners[options.toaddr] = 1 
        newowners = newowners.keys()
        newowners.sort()
        if options.modify:
            mlist.owner = newowners
        if not options.quiet:
            if newowners <> oldowners:
                print
                print '    new list owners:', string.join(newowners, ' ')
            else:
                print '(no change)'

    # see if the fromaddr is a digest member or regular member
    if options.lfromaddr in mlist.GetDigestMembers():
        digest = 1
    elif options.lfromaddr in mlist.GetMembers():
        digest = 0
    else:
        if not options.quiet:
            print '    address "%s" not found' % options.fromaddr
        return

    # Get the user's current options and password.  TBD: Ugly hack: if a
    # user's options would have been zero, then Mailman saves room by deleting
    # the entry for the user from the user_options dictionary.  Note that
    # /really/ it would be better if GetUserOption() and SetUserOption()
    # supported an interface to get the entire option value.
    flags = mlist.user_options.get(options.lfromaddr, 0)
    password = mlist.passwords[options.lfromaddr]

    # now add the new user
    try:
        if options.modify:
            mlist.ApprovedAddMember(options.toaddr, password,
                                    digest, 0, eVote_notif=0)
        if not options.quiet:
            print '    clone address added:', options.toaddr
    except Errors.MMAlreadyAMember:
        if not options.quiet:
            print '    clone address is already a member:', options.toaddr

    # hack the account flags
    ltoaddr = string.lower(options.toaddr)
    if not flags:
        try:
            del mlist.user_options[ltoaddr]
        except KeyError:
            # the user's options were already zero
            pass
    else:
        mlist.user_options[ltoaddr] = flags

    # perhaps remove the original address
    if options.remove:
        if options.modify:
            mlist.DeleteMember(options.fromaddr, userack=0, eVote_notif=0)
        print '    original address removed:', options.fromaddr

def main():
    # default options
    class Options:
        listnames = None
        remove = 1
        admintoo = 1
        quiet = 0
        modify = 1

    # scan sysargs
    try:
        opts, args = getopt.getopt(
            sys.argv[1:], 'arl:qnh',
            ['admin', 'remove', 'listname=', 'quiet', 'nomodify', 'help'])
    except getopt.error, msg:
        usage(1, msg)

    options = Options()
    for opt, arg in opts:
        if opt in ('-h', '--help'):
            usage(0)
        elif opt in ('-q', '--quiet'):
            options.quiet = 0
        elif opt in ('-n', '--nomodify'):
            options.modify = 0
        elif opt in ('-a', '--admin'):
            options.admintoo = 1
        elif opt in ('-r', '--remove'):
            options.remove = 1
        elif opt in ('-l', '--listname'):
            if options.listnames is None:
                options.listnames = []
            options.listnames.append(string.lower(arg))

    # further options and argument processing
    if not options.modify:
        options.quiet = 0

    if len(args) <> 2:
        usage(1)
    fromaddr = args[0]
    toaddr = args[1]
        
    # validate and normalize the target address
    try:
        Utils.ValidateEmail(toaddr)
    except Errors.EmailAddressError:
        usage(1, 'Not a valid email address: %s' % toaddr)
    lfromaddr = string.lower(fromaddr)
    options.toaddr = toaddr
    options.fromaddr = fromaddr
    options.lfromaddr = lfromaddr

    if options.listnames is None:
        options.listnames = Utils.list_names()

    for listname in options.listnames:
        try:
            mlist = MailList.MailList(listname)
        except Errors.MMListError, e:
            print 'Error opening list "%s": %s... skipping.' % (listname, e)
            continue
        try:
            dolist(mlist, options)
        finally:
            mlist.Save()
            mlist.Unlock()


if __name__ == '__main__':
    main()

def eVote(mlist):
    print "in"

def PrintSubjectPrefix(mlist):
    print mlist.subject_prefix

def ListMembers(mlist):
    from Mailman import mm_cfg
    rmembers = mlist.GetDeliveryMembers()
    rmembers = rmembers + mlist.GetDigestDeliveryMembers()
    for addr in rmembers:
        print addr, mlist.GetUserOption(addr, mm_cfg.DisableDelivery)

def IsAdminPassword(mlist, pw):
    if pw and mlist.ValidAdminPassword(pw):
        print "yes"
    else:
        print "no"




    
