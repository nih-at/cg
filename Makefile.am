INCLUDES=-I$(srcdir)/libmap

SUBIDRS=libmap

bin_PROGRAMS=cg

cg_SOURCES=checkgroup.c sockets.c ranges.c parserc.l util.c newsrc.c \
	decode.c tables.c mime.c header.c symbol.c
cg_LDADD=libmap/libmap.a @LIBOBJS@ @LEXLIB@

noinst_HEADER=ranges.h util.h newsrc.h globals.h getopt.h mime.h header.h decode.h symbol.h
