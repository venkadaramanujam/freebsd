divert(-1)
#
# Copyright (c) 1998 Sendmail, Inc.  All rights reserved.
# Copyright (c) 1994 Eric P. Allman.  All rights reserved.
# Copyright (c) 1994
#	The Regents of the University of California.  All rights reserved.
#
# By using this file, you agree to the terms and conditions set
# forth in the LICENSE file which can be found at the top level of
# the sendmail distribution.
#
#

# Support for DYNIX/ptx 2.x.

divert(0)
VERSIONID(`@(#)ptx2.m4	8.11 (Berkeley) 5/19/98')
ifdef(`QUEUE_DIR',, `define(`QUEUE_DIR', /usr/spool/mqueue)')dnl
define(`ALIAS_FILE', /usr/lib/aliases)dnl
ifdef(`HELP_FILE',,`define(`HELP_FILE', /usr/lib/sendmail.hf)')dnl
ifdef(`STATUS_FILE',,`define(`STATUS_FILE', /usr/lib/sendmail.st)')dnl
define(`LOCAL_MAILER_PATH', `/bin/mail')dnl
define(`LOCAL_MAILER_FLAGS', `fmn9')dnl
define(`LOCAL_SHELL_FLAGS', `eu')dnl
define(`confEBINDIR', `/usr/lib')dnl
