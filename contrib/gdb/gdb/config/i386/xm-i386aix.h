/* Macro defintions for AIX PS/2 (i386)
   Copyright 1986, 1987, 1989, 1992, 1993 Free Software Foundation, Inc.

This file is part of GDB.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/*
 * Changed for IBM AIX ps/2 by Minh Tran Le (tranle@intellicorp.com)
 * Revision:	23-Oct-92 17:42:49
 */

#include "i386/xm-i386v.h"

#undef HAVE_TERMIO
#define HAVE_SGTTY

#include <limits.h>

/* Use setpgid instead of setpgrp on AIX */
#define NEED_POSIX_SETPGID
