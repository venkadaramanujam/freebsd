#!/bin/sh
# $FreeBSD$

desc="rmdir returns EPERM if the parent directory of the named file has its immutable or append-only flag set"

dir=`dirname $0`
. ${dir}/../misc.sh

require chflags

echo "1..30"

n0=`namegen`
n1=`namegen`

expect 0 mkdir ${n0} 0755

expect 0 mkdir ${n0}/${n1} 0755
expect 0 chflags ${n0} SF_IMMUTABLE
expect EPERM rmdir ${n0}/${n1}
expect 0 chflags ${n0} none
expect 0 rmdir ${n0}/${n1}

expect 0 mkdir ${n0}/${n1} 0755
expect 0 chflags ${n0} UF_IMMUTABLE
expect EPERM rmdir ${n0}/${n1}
expect 0 chflags ${n0} none
expect 0 rmdir ${n0}/${n1}

expect 0 mkdir ${n0}/${n1} 0755
expect 0 chflags ${n0} SF_APPEND
expect EPERM rmdir ${n0}/${n1}
expect 0 chflags ${n0} none
expect 0 rmdir ${n0}/${n1}

expect 0 mkdir ${n0}/${n1} 0755
expect 0 chflags ${n0} UF_APPEND
expect EPERM rmdir ${n0}/${n1}
expect 0 chflags ${n0} none
expect 0 rmdir ${n0}/${n1}

expect 0 mkdir ${n0}/${n1} 0755
expect 0 chflags ${n0} SF_NOUNLINK
expect 0 rmdir ${n0}/${n1}
expect 0 chflags ${n0} none

expect 0 mkdir ${n0}/${n1} 0755
expect 0 chflags ${n0} UF_NOUNLINK
expect 0 rmdir ${n0}/${n1}
expect 0 chflags ${n0} none

expect 0 rmdir ${n0}
