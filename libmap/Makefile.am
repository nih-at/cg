AUTOMAKE_OPTIONS=foreign

PACKAGE=@PACKAGE@
VERSION=@VERSION@

lib_LIBRARIES=map

map_SOURCES=new.c new2.c free.c lookup.c insert.c delete.c \
	start.c next.c stop.c globals.c strhash.c lookup-int.c \
	version.c

map_LIBADD=@LIBOBJS@

include_HEADERS=map.h

info_TEXINFOS=libmap.texi

EXTRA_PROGRAMS=testmap
testmap_SOURCES=testmap.c
testmap_LDADD=libmap.a

TESTS=testmap


version.c: configure.in
	echo 'const char map_version[] = "$(PACKAGE) (version $(VERSION))";' \
		> version.c
