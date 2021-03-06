# Process this file with autoconf to produce a configure script.
AC_INIT(src)
AM_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([mmpc], [0.2.1],holger@homac.de)

# Checks for programs.

AC_PROG_INTLTOOL([0.21])
AC_PROG_LIBTOOL
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

AC_SUBST(GLIB_REQUIRED)
AC_SUBST(GTK_REQUIRED)

# Split this out, because pkgconfig macro doesn't return nicely what is missing

# hildon
PKG_CHECK_MODULES([hildon], hildon-1)
AC_SUBST(hildon_LIBS)
AC_SUBST(hildon_CFLAGS)

# glib
PKG_CHECK_MODULES([glib], glib-2.0 >= 2.4)
AC_SUBST(glib_LIBS)
AC_SUBST(glib_CFLAGS)

# gobject
PKG_CHECK_MODULES([gobject], gobject-2.0 >= 2.4)
AC_SUBST(gobject_LIBS)
AC_SUBST(gobject_CFLAGS)

#gtk
PKG_CHECK_MODULES([gtk], gtk+-2.0 >= 2.4)
AC_SUBST(gtk_LIBS)
AC_SUBST(gtk_CFLAGS)

#gmodule
PKG_CHECK_MODULES([gmodule], gmodule-2.0 >= 2.4)
AC_SUBST(gmodule_LIBS)
AC_SUBST(gmodule_CFLAGS)

# libmpd
PKG_CHECK_MODULES([libmpd], libmpd >= 0.12.0)
AC_SUBST(libmpd_LIBS)
AC_SUBST(libmpd_CFLAGS)

# libxml
PKG_CHECK_MODULES([libxml], libxml-2.0)
AC_SUBST(libxml_LIBS)
AC_SUBST(libxml_CFLAGS)

# libglade
PKG_CHECK_MODULES([libglade], libglade-2.0)
AC_SUBST(libglade_LIBS)
AC_SUBST(libglade_CFLAGS)

# gthread
PKG_CHECK_MODULES([gthread], gthread-2.0)
AC_SUBST(gthread_LIBS)
AC_SUBST(gthread_CFLAGS)

# installation paths
AC_MSG_CHECKING(prefix)

if test "x${prefix}" = "xNONE"; then
	PACKAGE_PREFIX="${ac_default_prefix}"
else
	PACKAGE_PREFIX="${prefix}"
fi
AC_MSG_RESULT($PACKAGE_PREFIX)

# i18n support
dnl please keep them in alphabetical order
ALL_LINGUAS="fr ru"

GETTEXT_PACKAGE=gmpc
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE",[GMPC gettext package])
AM_GLIB_GNU_GETTEXT


#AM_GNU_GETTEXT([external])
# setting correct paths
PACKAGE_LOCALE_DIR="${PACKAGE_PREFIX}/${DATADIRNAME}/locale"
AC_DEFINE_UNQUOTED(PACKAGE_LOCALE_DIR,"$PACKAGE_LOCALE_DIR",[The package's locale path for gettext])


AC_SUBST(PACKAGE_LOCALE_DIR)
AC_SUBST(PACKAGE_LIBS)

EXTRA_CFLAGS=
EXTRA_LDFLAGS=
case $host in
  *-*-mingw32* | *-*-windows)
    EXTRA_CFLAGS="-mno-cygwin -mwindows -mms-bitfields"
    EXTRA_LDFLAGS="-export-all-symbols -mwindows -mms-bitfields" ;;
  *-*-cygwin*)
    EXTRA_LDFLAGS="-export-all-symbols -mms-bitfields" ;;
esac
AC_SUBST(EXTRA_CFLAGS)
AC_SUBST(EXTRA_LDFLAGS)

AC_OUTPUT(Makefile src/Makefile pixmaps/Makefile glade/Makefile data/Makefile po/Makefile.in data/gmpc.pc)
