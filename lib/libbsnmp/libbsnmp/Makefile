# $FreeBSD$
#
# Author: Harti Brandt <harti@freebsd.org>

CONTRIB= ${.CURDIR}/../../../contrib/bsnmp/lib
.PATH: ${CONTRIB}

LIB	= bsnmp
SHLIBDIR ?= /lib
WARNS	?= 6

CFLAGS+= -I${CONTRIB} -DHAVE_ERR_H -DHAVE_GETADDRINFO -DHAVE_STRLCPY
CFLAGS+= -DHAVE_STDINT_H -DHAVE_INTTYPES_H -DQUADFMT='"llu"' -DQUADXFMT='"llx"'

SRCS=	asn1.c snmp.c snmpagent.c snmpclient.c support.c
INCS=	asn1.h snmp.h snmpagent.h snmpclient.h
MAN=	asn1.3 bsnmplib.3 bsnmpclient.3 bsnmpagent.3

.include <bsd.lib.mk>
