# dwm - dynamic window manager
# See LICENSE file for copyright and license details.

include config.mk

SRC = drw.c dwm.c util.c
OBJ = ${SRC:.c=.o}

all: dwm

.c.o:
	${CC} -c ${CFLAGS} $<

${OBJ}: config.h config.mk

config.h:
	cp config.def.h $@

dwm: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f dwm ${OBJ} dwm-${VERSION}.tar.gz

dist: clean
	mkdir -p dwm-${VERSION}
	cp -R LICENSE Makefile README config.def.h config.mk\
		dwm.1 drw.h util.h ${SRC} dwm.pdf transient.c dwm-${VERSION}
	tar -cf dwm-${VERSION}.tar dwm-${VERSION}
	gzip dwm-${VERSION}.tar
	rm -rf dwm-${VERSION}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f dwm ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/dwm
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	sed "s/VERSION/${VERSION}/g" < dwm.1 > ${DESTDIR}${MANPREFIX}/man1/dwm.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/dwm.1

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/dwm\
		${DESTDIR}${MANPREFIX}/man1/dwm.1

callgraph: dwm.c
	${CC} -c ${CFLAGS} -fdump-rtl-expand dwm.c
	python3 cally.py $$(ls dwm.c.*r.expand | tail -1) | dot -Grankdir=LR -Tpdf -o dwm_call_graph.pdf

partial_callgraph:
	@if [ -z "$(FUNC)" ]; then echo "Error: FUNC must be specified, e.g., make partial_callgraph FUNC=main"; exit 1; fi
	for src in ${SRC}; do \
		${CC} -c ${CFLAGS} -fdump-rtl-expand $$src; \
	done
	find . -name '*.expand' | xargs python3 cally.py --caller $(FUNC) --max-depth 6 --exclude "ds_.*|write" | dot -Grankdir=LR -Tpdf -o $(FUNC)_call_graph.pdf

.PHONY: all clean dist install uninstall callgraph partial_callgraph
