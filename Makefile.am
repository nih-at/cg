INCLUDES=-I$(srcdir)/libmap

SUBIDRS=libmap

bin_PROGRAMS=checkgroup

checkgroup_SOURCES=checkgroup.c sockets.c ranges.c parserc.l util.c
checkgroup_LDADD=libmap/libmap.a @LIBOBJS@ @LEXLIB@

noinst_HEADER=ranges.h util.h