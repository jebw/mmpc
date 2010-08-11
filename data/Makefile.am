desktopdir 	= $(datadir)/applications/hildon
desktop_in_files = mmpc.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

servicedir = $(datadir)/dbus-1/services/
service_DATA = mmpc.service

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gmpc.pc

EXTRA_DIST = 		\
	mmpc.desktop 	\
	gmpc.pc.in




