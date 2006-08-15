# -*- Makefile -*- for emacs

NAME:=slogcxx
SHELL:=bash

TOP_LEVEL_FILES := AUTHORS README Makefile VERSION
TOP_LEVEL_FILES += ChangeLog
TOP_LEVEL_FILES += LICENSE.txt
TOP_LEVEL_FILES += TODO

VERSION:=${shell cat VERSION}
DIST:=${NAME}-${VERSION}
DISTDIR:=dist/${DIST}
release:
	@echo Building release for version ${VERSION}
	@echo
	rm -rf ${DISTDIR} dist/${DIST}.tar*
	mkdir -p ${DISTDIR}
	(cd src && make real-clean && doxygen)
	cp -r src examples ${TOP_LEVEL_FILES} ostridge-logo.png ${DISTDIR}/
	mv ${DISTDIR}/src/html ${DISTDIR}/
	(cd ${DISTDIR} && find . -name .svn | xargs rm -rf)
	(cd dist && tar cf ${DIST}.tar ${DIST})
	bzip2 -9 dist/${DIST}.tar
	rm -rf ${DISTDIR}

.PHONY: release dist
dist: release
