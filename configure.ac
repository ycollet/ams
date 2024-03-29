#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([ams],[2.2.1],[alsamodular-devel@lists.sourceforge.net])
AC_CANONICAL_TARGET
AC_CONFIG_SRCDIR([src/box.h])
AC_CONFIG_HEADERS([src/config.h])
AM_INIT_AUTOMAKE([dist-bzip2 dist-xz])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB

# If macro PKG_PROG_PKG_CONFIG is not available, Autoconf generates
# a misleading error message, so check for existence first, and
# otherwise provide helpful advice.
#
m4_ifndef([PKG_PROG_PKG_CONFIG], [m4_fatal(
    [Macro PKG_PROG_PKG_CONFIG is not available. It is usually
defined in file pkg.m4 provided by package pkg-config.]
)])

PKG_PROG_PKG_CONFIG

# Checks for header files.
# Autoupdate added the next two lines to ensure that your configure
# script's behavior did not change.  They are probably safe to remove.
AC_CHECK_INCLUDES_DEFAULT
AC_PROG_EGREP


# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SSIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STAT

dnl set basic compiler flags
AMS_CORE_CXXFLAGS="-Wall"

dnl set compiler optimization flags
AC_MSG_CHECKING(which processor class to optimize for)
case "$target" in
    i486-*-*)
         AMS_OPT_CXXFLAGS="-march=i486"
         AC_MSG_RESULT(i486)
         ;;
    i586-*-*)
         AMS_OPT_CXXFLAGS="-march=pentium"
         AC_MSG_RESULT(i586)
         ;;
    i686-*-*)
         AMS_OPT_CXXFLAGS="-march=pentiumpro -msse -mfpmath=sse -ffast-math"
         AC_MSG_RESULT(i686)
         ;;
    i786-*-*)
         AMS_OPT_CXXFLAGS="-march=pentium4 -msse -mfpmath=sse -ffast-math"
         AC_MSG_RESULT(i786)
         ;;
    x86_64-*-*)
         AMS_OPT_CXXFLAGS="-m64 -msse -mfpmath=sse -ffast-math"
         AC_MSG_RESULT(x86_64)
         ;;
    *)
         AC_MSG_RESULT(i386)
         AC_MSG_WARN(This is probably not what you want, use --target)
         ;;
esac

if test "x$AMS_OPT_CXXFLAGS" = "x"; then
    AMS_CXXFLAGS=$AMS_CORE_CXXFLAGS
else
    AC_MSG_NOTICE([Compiler optimization... $AMS_OPT_CXXFLAGS])
    AMS_CXXFLAGS="$AMS_CORE_CXXFLAGS $AMS_OPT_CXXFLAGS"
fi
AC_SUBST(AMS_CXXFLAGS)


# Checks for libraries.
AC_CHECK_LIB([asound], [snd_pcm_open], ,
             [AC_MSG_ERROR(asound is required)])
AC_CHECK_LIB([jack], [jack_client_new], ,
             [AC_MSG_ERROR(jack is required)])
AC_CHECK_LIB([pthread], [pthread_create], ,
             [AC_MSG_ERROR(pthread is required)])
AC_CHECK_LIB([m], [roundf], ,
             [AC_MSG_ERROR([required libm missing])])
AC_CHECK_LIB([dl], [dlopen], ,
             [AC_MSG_ERROR(libdl is required)])
dnl for vocoder module
AC_CHECK_LIB([fftw3], [fftw_execute], ,
             [AC_MSG_ERROR(libfftw3 is required)])

dnl first check for libzita-alsa-pcmi, if not found check for libclalsadrv
AC_CHECK_LIB([zita-alsa-pcmi], [main], ,
             [AC_MSG_WARN([libzita-alsa-pcmi not found, trying libclalsadrv...])])
if test "x$ac_cv_lib_zita_alsa_pcmi_main" = "xno"; then
AC_CHECK_LIB([clalsadrv], [main], ,
             [AC_MSG_ERROR(clalsadrv is required)])

dnl check for clalsadrv library version
AC_LANG_PUSH(C++)
saved_libs="$LDFLAGS"
LDFLAGS="$LDFLAGS -lclalsadrv"
AC_MSG_CHECKING([for API 2.0.0 version of libclalsadrv])
AC_LINK_IFELSE(
    [AC_LANG_PROGRAM([#include <clalsadrv.h>],
    [Alsa_driver ad = Alsa_driver("play", "capt","ctrl", 0, 0, 0)])],
    [HAVE_CLALSADRV_API2=1] [AC_SUBST(HAVE_CLALSADRV_API2)]
    [AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no])])

LDFLAGS="$saved_libs"
AC_LANG_POP(C++)
fi

dnl option to enable Qt4 instead of default Qt5 library 
AC_ARG_ENABLE(qt4, AS_HELP_STRING([--enable-qt4],
              [enable Qt4 instead of Qt5 library]),,enable_qt5=yes)
AS_IF([test x$enable_qt4 = xyes],
dnl Check for Qt4 library files
[
PKG_CHECK_MODULES(Qt4, QtCore QtGui QtNetwork >= 4.2.0,
                  HAVE_QT4="yes", HAVE_QT4="no")
if test "x$HAVE_QT4" = "xyes"; then
  dnl Check needed because in some cases the QtGui includedir
  dnl doesn't contain the subsystem dir.
  QT_INCLUDE_DIR=`$PKG_CONFIG --variable=includedir QtGui`
  QT_CXXFLAGS=`$PKG_CONFIG --cflags QtCore QtGui QtOpenGL`
  EXTRA_QT_INCLUDE_DIR="$QT_INCLUDE_DIR/Qt"
  AC_CHECK_FILE([$QT_INCLUDE_DIR/QWidget],
    AC_MSG_NOTICE([No extra QT_INCLUDE_DIR needed]),
      AC_CHECK_FILE([$EXTRA_QT_INCLUDE_DIR/QWidget],
        QT_CPPFLAGS="$QT_CPPFLAGS -I$EXTRA_QT_INCLUDE_DIR",
          AC_MSG_WARN([QWidget not found])))
    
  AC_MSG_NOTICE([Set QT_CXXFLAGS... $QT_CXXFLAGS])
  AC_SUBST(QT_CXXFLAGS)
  AC_DEFINE([HAVE_QT4], [1],[Define to 1 if you have Qt4.])

  AC_CHECK_LIB([QtCore], [main], ,
             [AC_MSG_ERROR(QtCore is required)])
  AC_CHECK_LIB([QtGui], [main], ,
             [AC_MSG_ERROR(QtGui is required)])
  AC_CHECK_LIB([QtOpenGL], [main], ,
             [AC_MSG_ERROR(QtOpenGL is required)])

else
  AC_MSG_ERROR([cannot find Qt4 library >= 4.2])
fi
ac_qtversion=qt4],
dnl ... or use Qt5
[
PKG_CHECK_MODULES(Qt5, Qt5Core Qt5Gui Qt5Widgets >= 5.0,
                  HAVE_QT5="yes", HAVE_QT5="no")
if test "x$HAVE_QT5" = "xyes"; then
  dnl Check needed because in some cases the QtGui includedir
  dnl doesn't contain the subsystem dir.
  QT_INCLUDE_DIR=`$PKG_CONFIG --variable=includedir Qt5Gui`
  QT_CXXFLAGS=`$PKG_CONFIG --cflags Qt5Core Qt5Gui Qt5OpenGL Qt5Widgets`
  QT_CXXFLAGS="$QT_CXXFLAGS -fPIC"
  EXTRA_QT_INCLUDE_DIR="$QT_INCLUDE_DIR/Qt"
  AC_CHECK_FILE([$QT_INCLUDE_DIR/QtWidgets/QWidget],
    AC_MSG_NOTICE([No extra QT_INCLUDE_DIR needed]),
      AC_CHECK_FILE([$EXTRA_QT_INCLUDE_DIR/QtWidgets/QWidget],
        QT_CPPFLAGS="$QT_CPPFLAGS -I$EXTRA_QT_INCLUDE_DIR",
          AC_MSG_WARN([QWidget not found])))
    
  AC_MSG_NOTICE([Set QT_CXXFLAGS... $QT_CXXFLAGS])
  AC_SUBST(QT_CXXFLAGS)
  AC_DEFINE([HAVE_QT5], [1],[Define to 1 if you have Qt5.])

AC_CHECK_LIB([Qt5Core], [main], ,
             [AC_MSG_ERROR(QtCore is required)])
AC_CHECK_LIB([Qt5Gui], [main], ,
             [AC_MSG_ERROR(QtGui is required)])
AC_CHECK_LIB([Qt5Widgets], [main], ,
             [AC_MSG_ERROR(QtWidget is required)])
AC_CHECK_LIB([Qt5OpenGL], [main], ,
             [AC_MSG_ERROR(Qt5OpenGL is required)])

else
  AC_MSG_ERROR([cannot find Qt5 library >= 5.0])
fi
ac_qtversion=qt5
])
dnl endo of qt version test

dnl check for qtchooser
AC_PATH_PROGS(QTCHOOSER, qtchooser, no, [$PATH:$QTDIR/bin])
if test "x$QTCHOOSER" = "xno" ; then
dnl qtchooser not found, try to find Qt tools by old search style 

dnl check for moc
AC_PATH_PROGS(MOC, moc-$ac_qtversion moc, no, [$PATH:$QTDIR/bin])
if test "x$MOC" = "xno" ; then
  AC_MSG_ERROR(
    [cannot find 'moc', please install the Qt development tools package.])
fi

dnl check for lupdate
AC_PATH_PROGS(LUPDATE, lupdate-$ac_qtversion lupdate, no, [$PATH:$QTDIR/bin])
if test "x$LUPDATE" = "xno" ; then
  AC_MSG_ERROR(
    [cannot find 'lupdate', please install the Qt development tools package.])
fi

dnl check for lrelease
AC_PATH_PROGS(LRELEASE, lrelease-$ac_qtversion lrelease, no, [$PATH:$QTDIR/bin])
if test "x$LRELEASE" = "xno" ; then
  AC_MSG_ERROR(
    [cannot find 'lrelease', please install the Qt development tools package.])
fi
dnl end of alternative Qt tools check
else
  dnl qtchooser found, set tool calls manually 
  MOC="$QTCHOOSER -run-tool=moc -qt=$ac_qtversion"
  AC_SUBST(MOC)
  AC_MSG_RESULT(moc: $MOC)
  LUPDATE="$QTCHOOSER -run-tool=lupdate -qt=$ac_qtversion"
  AC_SUBST(LUPDATE)
  AC_MSG_RESULT(lupdate: $LUPDATE)
  LRELEASE="$QTCHOOSER -run-tool=lrelease -qt=$ac_qtversion"
  AC_SUBST(LRELEASE)
  AC_MSG_RESULT(lrelease: $LRELEASE)
fi


# Checks for special library functions.
AC_CHECK_FUNCS([floor ftruncate getcwd memset mkdir pow rint select sqrt])

# Check for ladspa
AC_CHECK_HEADER([ladspa.h], ,
                [AC_MSG_ERROR(essential header file is missing)])

# Check for ladspa plugins path
AC_ARG_WITH(ladspa-path,
  [AS_HELP_STRING([--with-ladspa-path=DIR],[directory for ladspa plugins])],
  [ if test "x$withval" != "x" ; then
     LADSPA_PATH="$withval"
    fi ],
  [LADSPA_PATH="/usr/lib/ladspa:/usr/lib64/ladspa:/usr/local/lib/ladspa:/usr/local/lib64/ladspa"])
AC_SUBST(LADSPA_PATH)

dnl JACK session
AC_ARG_ENABLE(jack-session,
    [AS_HELP_STRING(--disable-jack-session, [Disable JACK session support])],
    [jack_session=$enableval],
    [jack_session=yes])

if test "$jack_session" != "no"; then
    AC_CHECK_HEADER(jack/session.h, jack_session_found="yes",
                    jack_session_found="no")
    if test "$jack_session_found" = "yes"; then
        AC_DEFINE(JACK_SESSION, 1,
                  [Define to enable JACK session support])
        fi
    fi

# NSM support
dnl NSM
AC_ARG_ENABLE(nsm,
    [AS_HELP_STRING(--disable-nsm, [Disable Non Session Manager support])],
    [nsm=$enableval],
    [nsm=yes])

if test "$nsm" != "no"; then
    PKG_CHECK_MODULES(LIBLO, liblo >= 0.26, [have_liblo=yes], [have_liblo=no])
    if test "$have_liblo" = "yes"; then
        AC_DEFINE(NSM_SUPPORT, 1,
                  [Define to enable NSM support])
        LIBSsave="$LIBS"
        CFLAGSsave="$CFLAGS"
        CFLAGS="$LIBLO_CFLAGS"
        LIBS="$LIBLO_LIBS"
        AC_SUBST(LIBLO_CFLAGS)
        AC_SUBST(LIBLO_LIBS)
        LIBS+=" $LIBSsave"
        CFLAGS+="$CFLAGSsave"
    fi
fi


AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([demos/Makefile])
AC_CONFIG_FILES([instruments/Makefile])
AC_CONFIG_FILES([tutorial/Makefile])
AC_CONFIG_FILES([colors/Makefile])
AC_CONFIG_FILES([pixmaps/Makefile])
AC_CONFIG_FILES([man/Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_OUTPUT
