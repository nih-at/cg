AUTOMAKE_OPTIONS=dist-bzip2

INCLUDES=-I$(srcdir)/libmap

SUBDIRS=libmap regress

bin_PROGRAMS=cg
man_MANS=cg.1
noinst_PROGRAMS=testdec

EXTRA_DIST=cg.mdoc cg.man

cg_SOURCES=checkgroup.c crc.c decode.c header.c mime.c newsrc.c \
	output.c parserc.l ranges.c sockets.c stream.c stream_article.c \
	stream_cat.c stream_decode.c stream_file.c stream_quot_print.c \
	stream_msg_multi.c stream_msg_partial.c stream_section.c \
	stream_uuextract.c stream_yenc.c symbol.c tables.c token.c util.c 

cg_LDADD=libmap/libmap.a @LIBOBJS@ @LEXLIB@
testdec_LDADD=@LIBOBJS@

testdec_SOURCES=testdec.c crc.c decode.c util.c mime.c tables.c header.c \
	stream_fcat.c stream_decode.c stream.c stream_msg_multi.c token.c \
	stream_file.c stream_section.c output.c stream_uuextract.c \
	stream_msg_partial.c stream_article.c stream_quot_print.c \
	stream_yenc.c symbol.c

#stest_SOURCES=stest.c util.c stream.c token.c stream_file.c stream_section.c

noinst_HEADERS=checkgroup.h crc.h decode.h getopt.h globals.h header.h mime.h \
	newsrc.h output.h ranges.h sockets.h stream.h stream_types.h symbol.h \
	util.h

SUFFIXES=.man .mdoc .html
.mdoc.man:
	mdoc2man $< > $@.$$$$ && diff -I NiH $@.$$$$ $@ || mv $@.$$$$ $@; \
	rm -f $@.$$$$

.mdoc.html:
	nroff -mdoc2html $< | sed -e "s,../html1/,," > $@.$$$$ && mv $@.$$$$ $@
