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


version.c: configure.in
	echo 'const char map_version[] = "$PACAKGE (version $VERSION)";' \
		> version.c