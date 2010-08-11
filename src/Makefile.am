bin_PROGRAMS=mmpc

mmpc_SOURCES=\
	cover-art.c\
	main.c\
	plugin.c\
	mpdinteraction.c\
	mmpc-player.c\
	mmpc.c\
	preferences.c\
	id3info.c\
	misc.c\
	playlist3.c\
	edit_markup.c\
	playlist3-tag-browser.c\
	playlist3-file-browser.c\
	playlist3-current-playlist-browser.c\
	playlist3-artist-browser.c\
	playlist3-find-browser.c\
	qthread.c\
	config1.c\
	cover-art-manager.c\
	TreeSearchWidget.c

INCLUDES= @EXTRA_CFLAGS@ -Wall\
	-DPIXMAP_PATH=\""$(pkgdatadir)"\" \
	-DGLADE_PATH=\""$(pkgdatadir)"\" \
	-DVERSION=\"@VERSION@\"

LIBS = \
	@glib_LIBS@ @LIBS@ \
	@libmpd_LIBS@ \
	@gobject_LIBS@\
	@libxml_LIBS@ \
	@gtk_LIBS@\
	@gmodule_LIBS@\
	@libglade_LIBS@\
	@gthread_LIBS@\
	@hildon_LIBS@

AM_LDFLAGS = \
	@EXTRA_LDFLAGS@

AM_CFLAGS = @CFLAGS@\
	@glib_CFLAGS@\
	@libmpd_CFLAGS@ \
	@gobject_CFLAGS@\
	@libxml_CFLAGS@ \
	@gtk_CFLAGS@\
	@gmodule_CFLAGS@\
	@libglade_CFLAGS@\
	@gthread_CFLAGS@\
	@hildon_CFLAGS@

mmpcinclude_HEADERS = \
		plugin.h	\
		cover-art.h	\
		config1.h	\
		qthread.h
mmpcincludedir =$(includedir)/mmpc/

EXTRA_DIST = 	playlist3.h\
	    	main.h\
		player.h\
		mmpc.h\
		misc.h\
		mpdinteraction.h\
		config-defaults.h\
		playlist3-tag-browser.h\
		playlist3-file-browser.h\
		playlist3-current-playlist-browser.h\
		playlist3-artist-browser.h\
		playlist3-find-browser.h\
		TreeSearchWidget.h
