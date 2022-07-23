LLIBDIR=@LIBDIR@ 
lib_LTLIBRARIES = lib3dc.la
lib3dc_la_SOURCES = lib3dc.h lib3dc.c font_6x4.c font_7x5.c font_8x8.c lcd.c
lib3dc_la_LDFLAGS = -version-info 1:1:0
include_HEADERS= lib3dc.h 
EXTRA_DIST = debian rpm
AM_CFLAGS = -fPIC @LIBUSB_CFLAGS@

dist-hook:
	rm -rf `find $(distdir)/debian -name .svn`
	rm -rf `find $(distdir)/rpm -name .svn`

dist-rpm: dist-bzip2
	rpmbuild -ts $(distdir).tar.bz2

dist-archives:
	$(MAKE) dist-bzip2
	$(MAKE) dist-rpm
