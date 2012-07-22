# Copyright (c) 2008 Dave Freese, W1HKJ (w1hkj AT w1hkj DOT com)

bin_PROGRAMS = fllog

fllog_CPPFLAGS = @FLLOG_BUILD_CPPFLAGS@
fllog_CXXFLAGS = @FLLOG_BUILD_CXXFLAGS@
fllog_CFLAGS = $(fllog_CXXFLAGS)
fllog_LDFLAGS = @FLLOG_BUILD_LDFLAGS@
fllog_LDADD = @FLLOG_BUILD_LDADD@

MINGW32_SRC = \
	fllog-rc.rc \
	include/fllogrc.h \
	misc/mingw.c \
	compat/regex.c \
	include/compat.h


# We distribute these but do not always compile them
EXTRA_fllog_SOURCES = $(FLLOG_WIN32_RES_SRC) $(MINGW32_SRC)

fllog_SOURCES =

if MINGW32
if HAVE_WINDRES
  fllog_SOURCES += $(MINGW32_SRC)
endif
endif

########################################################################


FLLOG_VERSION_MAJOR = @FLLOG_VERSION_MAJOR@
FLLOG_VERSION_MINOR = @FLLOG_VERSION_MINOR@
FLLOG_VERSION_PATCH = @FLLOG_VERSION_PATCH@
FLLOG_VERSION = @FLLOG_VERSION@

.EXPORT_ALL_VARIABLES: nsisinst appbundle

# Sources that are generated,
BUILT_SOURCES =
# not distributed,
nodist_fllog_SOURCES = $(BUILT_SOURCES)
# and deleted by the clean targets
CLEANFILES = $(BUILT_SOURCES)
CLEAN_LOCAL =

if MINGW32
if HAVE_WINDRES
.rc.o:
	$(WINDRES) -I$(srcdir)/include -I$(srcdir)/../data/win32 $< -O coff $@
endif
endif

install-data-local:
	if test -f $(srcdir)/../data/fllog.xpm; then \
	    $(mkinstalldirs) $(DESTDIR)/$(datadir)/pixmaps; \
	    $(INSTALL_DATA) $(srcdir)/../data/fllog.xpm $(DESTDIR)/$(datadir)/pixmaps; \
	fi
	if test -f $(srcdir)/../data/fllog.desktop; then \
	    $(mkinstalldirs) $(DESTDIR)/$(datadir)/applications; \
	    $(INSTALL_DATA) $(srcdir)/../data/fllog.desktop $(DESTDIR)/$(datadir)/applications; \
	fi

uninstall-local:
	rm -f $(DESTDIR)/$(datadir)/pixmaps/fllog.xpm
	rm -f $(DESTDIR)/$(datadir)/applications/fllog.desktop

FLLOG_FL_SRC = logbook/lgbook.fl logbook/colorsfonts.fl

if HAVE_FLUID
flgen: $(FLLOG_FL_SRC)
	(cd $(srcdir)/include; \
	for f in $(FLLOG_FL_SRC); do \
	    c=$${f%.fl}.cxx; \
		h=$${f%.fl}.h; \
		h=$${h##*/}; \
	    $(FLUID) -c -o ../$$c -h $$h ../$$f; \
	done)
endif


INSTALLER_FILE = fllog-$(FLLOG_VERSION)_setup.exe
APPBUNDLE=fllog-$(FLLOG_VERSION)
APPBUNDLE_NOLIBS=$(APPBUNDLE)-nolibs

if HAVE_NSIS
nsisinst: $(bin_PROGRAMS)
	sh $(srcdir)/../scripts/mknsisinst.sh "$(srcdir)/../data" .
    CLEANFILES += $(INSTALLER_FILE)
endif

if DARWIN
appbundle: $(bin_PROGRAMS)
	sh $(srcdir)/../scripts/mkappbundle.sh "$(srcdir)/../data" .
    CLEAN_LOCAL += $(APPBUNDLE_NOLIBS) $(APPBUNDLE) $(APPBUNDLE)*.dmg
endif

#if HAVE_ASCIIDOC
#$(builddir)/../doc/guide.html: $(builddir)/../doc/guide.txt
#	$(MAKE) -C $(builddir)/../doc $(AM_MAKEFLAGS) guide.html

#$(builddir)/dialogs/guide.cxx: $(builddir)/../doc/guide.html
#	mkdir -p $(builddir)/dialogs
#	sed 's/"/\\"/g; s/$$/\\n\\/g; 1 s/.*/const char* szBeginner = "&/; $$ s/.*/&n";/' $< > $@
#dialogs/htmlstrings.cxx: $(builddir)/dialogs/guide.cxx

#distclean-local:
#	@if test "$(builddir)" != "$(srcdir)"; then \
#	    rm -f $(builddir)/dialogs/guide.cxx; \
#        fi
#endif

clean-local:
	-rm -rf $(CLEAN_LOCAL)

# Sources that we build. It is OK to have headers here.
fllog_SOURCES += \
	fllog.cxx \
	logbook/adif_io.cxx \
	logbook/colorsfonts.cxx \
	logbook/logsupport.cxx \
	logbook/lgbook.cxx \
	logbook/logbook.cxx \
	logbook/qso_db.cxx \
	logbook/support.cxx \
	logbook/textio.cxx \
	widgets/calendar.cxx \
	widgets/combo.cxx \
	widgets/date.cxx \
	widgets/fileselect.cxx \
	widgets/flinput2.cxx \
	widgets/flslider2.cxx \
	widgets/Fl_Native_File_Chooser.cxx \
	widgets/icons.cxx \
	widgets/pixmaps.cxx \
	widgets/table.cxx \
	widgets/ValueSlider.cxx \
	misc/debug.cxx \
	misc/font_browser.cxx \
	misc/re.cxx \
	misc/status.cxx \
	misc/util.cxx \
	xmlrpccpp/XmlRpcClient.cpp \
	xmlrpccpp/XmlRpcServer.cpp \
	xmlrpccpp/XmlRpcThread.cpp \
	xmlrpccpp/XmlRpcDispatch.cpp \
	xmlrpccpp/XmlRpcServerMethod.cpp \
	xmlrpccpp/XmlRpcThreadedServer.cpp \
	xmlrpccpp/XmlRpcMutex.cpp \
	xmlrpccpp/XmlRpcSocket.cpp \
	xmlrpccpp/XmlRpcUtil.cpp \
	xmlrpccpp/XmlRpcServerConnection.cpp \
	xmlrpccpp/XmlRpcSource.cpp \
	xmlrpccpp/XmlRpcValue.cpp

# Additional source files that are distributed
EXTRA_DIST = \
	config.h \
	compat/regex.c \
	compat/regex.h \
	widgets/flnfc_common.cxx \
	widgets/Fl_Native_File_Chooser_FLTK.cxx \
	widgets/Fl_Native_File_Chooser_MAC.cxx \
	widgets/Fl_Native_File_Chooser_WIN32.cxx \
	fllog_icon.cxx \
	logbook/lgbook.fl \
	include/adif_def.h \
	include/adif_io.h \
	include/colorsfonts.h \
	include/calendar.h \
	include/date.h \
	include/field_def.h \
	include/font_browser.h \
	include/fontdef.h \
	include/lgbook.h \
	include/log.h \
	include/logbook.h \
	include/logger.h \
	include/logsupport.h \
	include/textio.h \
	include/qso_db.h \
	include/table.h \
	include/combo.h \
	include/debug.h \
	include/fileselect.h \
	include/fllogrc.h \
	include/flslider2.h \
	include/font_browser.h \
	include/gettext.h \
	include/mingw.h \
	include/fllog.h \
	include/status.h \
	include/support.h \
	include/threads.h \
	include/re.h \
	include/util.h \
	include/ValueSlider.h \
	include/flinput2.h \
	include/pixmaps.h \
	include/icons.h \
	include/fileselect.h \
	include/FL/Fl_Native_File_Chooser.H \
	include/FL/Fl_Native_File_Chooser_FLTK.H \
	include/FL/Fl_Native_File_Chooser_MAC.H \
	include/FL/Fl_Native_File_Chooser_WIN32.H \
	include/base64.h \
	include/XmlRpcSource.h \
	include/XmlRpcServerConnection.h \
	include/XmlRpcClient.h \
	include/XmlRpcServer.h \
	include/XmlRpcThreadedServer.h \
	include/XmlRpcDispatch.h \
	include/XmlRpcThread.h \
	include/XmlRpcException.h \
	include/XmlRpcServerMethod.h \
	include/XmlRpc.h \
	include/XmlRpcUtil.h \
	include/XmlRpcSocket.h \
	include/XmlRpcMutex.h \
	include/XmlRpcValue.h

# Additional non-source files that are distributed
# Additional source files that support non-Linux cross compilation
EXTRA_DIST += \
	$(srcdir)/../data/fllog.desktop \
	$(srcdir)/../data/fllog.xpm \
	$(srcdir)/../data/win32/fl_app.nsi \
	$(srcdir)/../data/win32/fllog.ico \
	$(srcdir)/../scripts/mknsisinst.sh \
	$(srcdir)/../scripts/build4mingw.sh \
	$(srcdir)/../data/mac/Info.plist.in \
	$(srcdir)/../data/mac/fllog.icns \
	$(srcdir)/../scripts/mkappbundle.sh \
	$(FLLOG_WIN32_SRC) \
	$(FLLOG_FL_SRC)
