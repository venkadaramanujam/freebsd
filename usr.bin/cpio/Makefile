# $FreeBSD$

.include <bsd.own.mk>

PROG=	bsdcpio
BSDCPIO_VERSION_STRING=2.7.0
SRCS=	cpio.c cmdline.c err.c matching.c pathmatch.c
WARNS?=	6
DPADD=	${LIBARCHIVE} ${LIBZ} ${LIBBZ2}
CFLAGS+= -DBSDCPIO_VERSION_STRING=\"${BSDCPIO_VERSION_STRING}\"
CFLAGS+= -DPLATFORM_CONFIG_H=\"config_freebsd.h\"
.ifdef RELEASE_CRUNCH
# FreeBSD's installer uses cpio in crunched binaries that are
# statically linked, cannot use -lcrypto, and are size sensitive.
CFLAGS+= -DSMALLER
.endif
LDADD+=	-larchive -lz -lbz2 -lmd
.if ${MK_OPENSSL} != "no"
LDADD+= -lcrypto
.endif

SYMLINKS=bsdcpio ${BINDIR}/cpio
MLINKS= bsdcpio.1 cpio.1

.PHONY: check test

check test: $(PROG) bsdcpio.1.gz
	cd ${.CURDIR}/test && make clean test

.include <bsd.prog.mk>
