CFLAGS=	-g

RANLIB=	true

all: libmap.a

OBJS=	free-map.o map-delete.o map-insert.o map-iter.o map-lookup.o \
	new-map.o priv-map-lookup.o

libmap.a: $(OBJS)
	ar r libmap.a $(OBJS)
	$(RANLIB) libmap.a
