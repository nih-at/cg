INCLUDES=-I$(srcdir)/libmap

SUBDIRS=libmap

bin_PROGRAMS=cg testdec

cg_SOURCES=checkgroup.c decode.c header.c hstrerror.c mime.c newsrc.c \
	output.c parserc.l ranges.c sockets.c stream.c stream_article.c \
	stream_decode.c \
	stream_msg_multi.c stream_msg_partial.c stream_section.c \
	stream_uuextract.c symbol.c tables.c token.c util.c 

cg_LDADD=libmap/libmap.a @LIBOBJS@ @LEXLIB@

testdec_SOURCES=testdec.c decode.c util.c mime.c tables.c header.c symbol.c \
	stream_fcat.c stream_decode.c stream.c stream_msg_multi.c token.c \
	stream_file.c stream_section.c output.c stream_uuextract.c \
	stream_msg_partial.c

#stest_SOURCES=stest.c util.c stream.c token.c stream_file.c stream_section.c

noinst_HEADERS=ranges.h util.h newsrc.h globals.h getopt.h mime.h header.h decode.h symbol.h
