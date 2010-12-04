#!/bin/sh

#
# Copyright (c) 2010 Peter Holm <pho@FreeBSD.org>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
# $FreeBSD: projects/stress2/misc/rename.sh 191485 2009-04-25 10:19:36Z pho $
#

# Variation of truncate2.sh whih FS check added.

[ `id -u ` -ne 0 ] && echo "Must be root!" && exit 1

. ../default.cfg

mount | grep -q "$mntpoint" && umount $mntpoint
mdconfig -l | grep -q $mdstart &&  mdconfig -d -u $mdstart

mdconfig -a -t swap -s 400m -u $mdstart
bsdlabel -w md${mdstart} auto

newfs -U md${mdstart}${part} > /dev/null
mount /dev/md${mdstart}${part} $mntpoint
export RUNDIR=$mntpoint/stressX

here=`pwd`
cd /tmp
sed '1,/^EOF/d' < $here/$0 > truncate3.c
cc -o truncate3 -Wall -O2 truncate3.c
rm -f truncate3.c
[ -d $RUNDIR ] || mkdir -p $RUNDIR
cd $RUNDIR

/tmp/truncate3

cd $here
rm -f /tmp/truncate3

sync;sync;sync
while mount | grep -q $mntpoint; do
	sync;sync;sync
	sleep 1
	umount $mntpoint
done

if fsck -n -t ufs /dev/md${mdstart}${part} | egrep -q "BAD|WRONG"; then
	fsck -n -t ufs /dev/md${mdstart}${part}
fi

mdconfig -d -u $mdstart
exit 0
EOF
#include <fcntl.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define SIZ 512
char buf[SIZ];

void
test(void)
{
	int fd[10], i, j;
	char name[128];
	off_t len;

	for (j = 0; j < 10; j++) {
		sprintf(name, "%05d.%05d", getpid(), j);
		fd[j] = open(name, O_WRONLY | O_CREAT | O_APPEND, 0666);
		if (fd[j] == -1)
			err(1, name);
		unlink(name);
	}

	for (i = 0; i < 100; i++) {
		for (j = 0; j < 10; j++) {
			if (write(fd[j], buf, 2) != 2)
				err(1, "write");
			len = arc4random() % 1024 * 1024;
			if (ftruncate(fd[j], len) == -1)
				err(1, "ftruncate");
		}
	}

	exit(0);
}
int
main(int argc, char **argv)
{
	int i, status;

	for (i = 0; i < 20; i++) {
		if (fork() == 0) 
			test();
	}
	for (i = 0; i < 20; i++)
		wait(&status);

	return (0);
}
