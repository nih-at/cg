INCLUDES=-I$(srcdir)/libmap

SUBDIRS=libmap

bin_PROGRAMS=cg testdec

cg_SOURCES=checkgroup.c sockets.c ranges.c parserc.l util.c newsrc.c \
	decode.c tables.c mime.c header.c symbol.c hstrerror.c
cg_LDADD=libmap/libmap.a @LIBOBJS@ @LEXLIB@

testdec_SOURCES=testdec.c decode.c util.c mime.c tables.c header.c symbol.c

noinst_HEADERS=ranges.h util.h newsrc.h globals.h getopt.h mime.h header.h decode.h symbol.h
