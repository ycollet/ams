dnl Most of this file, aswell as the rest of the autoconf/automake setup 
dnl was insidously stolen from other projects, 
dnl namely MusE, as it happens to have similar requirements.
dnl Please forgive me, Werner. :-). L.D.

AC_INIT(Ams, 2.0.0pre1)
AC_CONFIG_SRCDIR(main.cpp)
AM_INIT_AUTOMAKE(1.6)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE
AM_ACLOCAL_INCLUDE(m4)

dnl ***** Check for programs *****
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_LN_S
dnl AC_PROG_RANLIB
###############
### DocBook ###
###############
#PGAC_PROG_JADE
#if test -n "$JADE"; then
#  PGAC_CHECK_DOCBOOK(4.1)
#  if test "$pgac_cv_check_docbook" = "yes"; then
#    PGAC_PATH_DOCBOOK_STYLESHEETS
#    if test -n "$DOCBOOKSTYLE"; then
#      DOCBOOKTARGETS="book1.html"
#    else
#      AC_MSG_WARN([DocBook stylesheets not found or not usable - documentation will not be built])
#    fi
#  else
#    AC_MSG_WARN([DocBook 4.1 DTD not found or not usable - documentation will not be built])
#  fi
#else
#  AC_MSG_WARN([jade not found - documentation will not be built])
#fi
#AC_SUBST(DOCBOOKTARGETS)

###############
### doxygen ###
###############
AC_ARG_VAR(DOXYGEN, Doxygen)
AC_PATH_PROG(DOXYGEN, doxygen, no)

################
### graphviz ###
################
AC_ARG_VAR(DOT, The "dot" program from graphviz)
AC_PATH_PROG(DOT, dot, no)
if test "$DOT" != "no"; then
  HAVEDOT=YES
  DOTPATH=$( dirname "$DOT" )
else
  HAVEDOT=NO
fi
AC_SUBST(HAVEDOT)
AC_SUBST(DOTPATH)

############
### perl ###
############
AC_ARG_VAR(PERL, Perl)
AC_PATH_PROG(PERL, perl, no)
if test "$PERL" == "no"; then
  AC_MSG_WARN([you don't have perl installed!  wierdo!  (and doxygen might not like this)])
fi

dnl ***** Check for libraries *****

#########
### X ###
#########
AC_PATH_XTRA
if test "$X_DISPLAY_MISSING" != ""; then
  AC_MSG_ERROR([X required])
fi

##########
### QT ###
##########
CONFIGURE_QT(3.0.0, , AC_MSG_ERROR([need qt >= 3.0.0]))
if test "$ams_qttest" = "yes"; then
  AC_LANG_PUSH([C++])
dnl  ams_save_CFLAGS="$CFLAGS"
dnl  CFLAGS="$CFLAGS $QT_CFLAGS"
  AC_SUBST(QT_CFLAGS)
dnl  ams_save_CXXFLAGS="$CXXFLAGS"
dnl  CXXFLAGS="$CXXFLAGS $QT_CFLAGS"
dnl  ams_save_LDFLAGS="$LDFLAGS"
dnl  LDFLAGS="$LDFLAGS $QT_LIBS"
  AC_SUBST(QT_LIBS)
  AC_CHECK_LIB(qt-mt, qt_selection_property, [
    true
  ],[
    AC_MSG_ERROR([

  Ams requires thread enabled QT (qt-mt) >= 3.0.0

  You can either compile your own version of QT (making sure you enable
  threading!) or install the appropriate package for your distribution.
])
  ])
  CFLAGS="$ams_save_CFLAGS"
  CXXFLAGS="$ams_save_CXXFLAGS"
  LDFLAGS="$ams_save_LDFLAGS"
  unset ams_save_CFLAGS
  unset ams_save_CXXFLAGS
  unset ams_save_LDFLAGS
  AC_LANG_POP([C++])
fi

############
### ALSA ###
############
AM_PATH_ALSA(0.9.0,
  true,
  AC_MSG_ERROR([latest alsa (0.9.x) is required])
)

#################
### iiwusynth ###
#################
#LIBIIWUSYNTH=
#ams_enable_iiwulib="yes"
#AC_ARG_ENABLE(iiwusynth,
#[  --disable-iiwusynth     disable iiwusynth softsynth plugin],[
#  case "$enableval" in
#    "yes")
#      ;;
#    "no")
#      ams_enable_iiwulib="no"
#      ;;
#    *)
#      AC_MSG_ERROR([must use --enable-iiwusynth(=yes/no) or --disable-iiwusynth])
#      ;;
#  esac
#])
#
#if test "$ams_enable_iiwulib" = "yes"; then
#  AC_CHECK_LIB(iiwusynth, new_iiwu_synth,
#    IIWUSYNTHDIRS="iiwu iiwusynth",
#    AC_MSG_WARN([libiiwusynth not found; disabling iiwusynth plugins])
#  )
#fi
#AC_SUBST(IIWUSYNTHDIRS)

###############
### sndfile ###
###############
PKG_CHECK_MODULES(SNDFILE, sndfile >= 1.0.0,
  true,
  AC_MSG_ERROR([libsndfile >= 1.0.0 is required])
)

############
### JACK ###
############
ams_enable_jack="yes"
AC_ARG_ENABLE(jack,
[  --disable-jack          disable JACK support],[
  case "$enableval" in
    "yes")
      ;;
    "no")
      ams_enable_jack="no"
      ;;
    *)
      AC_MSG_ERROR([must use --enable-jack(=yes/no) or --disable-jack])
      ;;
  esac
])

if test "$ams_enable_jack" = "yes"; then
  PKG_CHECK_MODULES(JACK,
    jack >= 0.32.0,
    JACK_FOUND="yes",
    JACK_FOUND="no"
  )
  if test x"$JACK_FOUND" = x"yes"; then
    AC_DEFINE(HAVE_JACK, 1, [whether or not we have (or should be building with) jack])
  else
    AC_MSG_WARN([jack not found - disabling jack support])
  fi
else
  JACK_FOUND="no"
fi
AM_CONDITIONAL(JACK, test "$JACK_FOUND" = "yes")

##############
### LADCCA ###
##############
ams_enable_ladcca="yes"
AC_ARG_ENABLE(ladcca,
[  --disable-ladcca        disable LADCCA support],[
  case "$enableval" in
    "yes")
      ;;
    "no")
      ams_enable_ladcca="no"
      ;;
    *)
      AC_MSG_ERROR([must use --enable-ladcca(=yes/no) or --disable-ladcca])
      ;;
  esac
])

if test "$ams_enable_ladcca" = "yes"; then
  PKG_CHECK_MODULES(LADCCA, ladcca-1.0 >= 0.2, LADCCA_FOUND="yes", LADCCA_FOUND="no")
  AC_SUBST(LADCCA_CFLAGS)
  AC_SUBST(LADCCA_LIBS)

  if test "$LADCCA_FOUND" = "yes"; then
    AC_DEFINE(HAVE_LADCCA, 1, [whether or not we are supporting ladcca])
    LADCCA_VERSION=$( pkg-config --modversion ladcca-1.0 )
    AC_DEFINE_UNQUOTED(LADCCA_VERSION, "$LADCCA_VERSION", [The version of ladcca we're compiling against])
  fi
else
  AC_MSG_WARN([LADCCA support is disabled])
  LADCCA_FOUND="no"
fi
AM_CONDITIONAL(HAVE_LADCCA, test "$LADCCA_FOUND" = "yes")


dnl ***** Check for header files *****

dnl ***** Check for options *****

#################
### givertcap ###
#################
#GIVERTCAP=
#report_rtcap="no"
#AC_ARG_ENABLE(rtcap,
#[  --enable-rtcap          use Tommi Ilmonen's "givertcap" to give
#                          realtime capabilities],[
#  case "$enableval" in
#    "yes")
#      AC_DEFINE(RTCAP, 1, [Use givertcap])
#      AMSCXXFLAGS="$AMSCXXFLAGS -DRTCAP"
#      GIVERTCAP="givertcap"
#      report_rtcap="yes"
#      ;;
#    "no")
#      ;;
#    *)
#      AC_MSG_ERROR([must use --enable-rtcap(=yes/no) or --disable-rtcap])
#  esac
#])
#AC_SUBST(GIVERTCAP)


####################
### suid-install ###
####################
#SUIDINSTALL="no"
#AC_ARG_ENABLE(suid-install,
#[  --enable-suid-install   install MusE setuid root],[
#  case "$enableval" in
#    "yes")
#      SUIDINSTALL="yes"
#      ;;
#    "no")
#      ;;
#    *)
#      AC_MSG_ERROR([must use --enable-suid-install(=yes/no) or --disable-suid-install])
#      ;;
#  esac
#])
#AC_SUBST(SUIDINSTALL)

##################
### suid-build ###
##################
#SUIDBUILD="yes"
#AC_ARG_ENABLE(suid-build,
#[  --disable-suid-build    do not build MusE setuid root],[
#  case "$enableval" in
#    "yes")
#      ;;
#    "no")
#      SUIDBUILD="no"
#      ;;
#    *)
#      AC_MSG_ERROR([must use --enable-suid-build(=yes/no) or --disable-suid-build])
#      ;;
#  esac
#])
#AC_SUBST(SUIDBUILD)


########################
### doxygen-treeview ###
########################
DOXYGEN_TREEVIEW="YES"
report_doxy_treeview="yes";
AC_ARG_ENABLE(doxy-treeview,
[  --disable-doxy-treeview stop Doxygen from generating java-ified
                          treeview menus in its HTML output],[
  case "$enableval" in
    "yes")
      ;;
    "no")
      DOXYGEN_TREEVIEW="NO"
      report_doxy_treeview="no";
      ;;
    *)
      AC_MSG_ERROR([must use --enable-doxy-treeview(=yes/no) or --disable-doxy-treeview])
      ;;
  esac
])
AC_SUBST(DOXYGEN_TREEVIEW)

################
### patchbay ###
################
###PATCHBAY="no"
###AC_SUBST(PATCHBAY)
###AM_CONDITIONAL(PATCHBAY, test "$PATCHBAY" == "yes")

AMSCXXFLAGS="-g -Wall -W -D_GNU_SOURCE -D_REENTRANT \
-DQT_CLEAN_NAMESPACE -DQT_NO_COMPAT -DQT_NO_ASCII_CAST \
-I\$(top_srcdir) -I\$(top_srcdir)/widgets \$(QT_CFLAGS)"

AMSOPTCXXFLAGS="-O6 -fomit-frame-pointer -ffast-math -fstrength-reduce \
-funroll-loops -fmove-all-movables"

################
### optimize ###
################
report_optimise="no"
AC_ARG_ENABLE(optimize,
  [  --enable-optimize       ask the compiler for its best optimizations (possibly only i386)],[
    case "$enableval" in
      "yes")
        report_optimise="yes"
        CXXFLAGS="$( echo "$CXXFLAGS" | sed "s/-O.[ ]*//" )"
        AMSCXXFLAGS="$AMSCXXFLAGS $AMSOPTCXXFLAGS"
        ;;
      "no")
        ;;
      *)
        AC_MSG_ERROR([must use --enable-optimize(=yes/no) or --disable-optimize])
        ;;
    esac
  ])

############
### arch ###
############
report_march="none"
AC_ARG_ENABLE(arch,
  [  --enable-arch=ARCH      use -march=ARCH and -mcpu=ARCH argument to CXX
                          choose from: 386, 486, 586, 686, pentium, pentiumpro,
                                       and k6.  gcc 3.0 gives you athlon as well],[
    case "$enableval" in
      "no")
        ;;
      *)
        report_march="$enableval"
        AMSCXXFLAGS="$AMSCXXFLAGS -mcpu=$enableval -march=$enableval"
        ;;
    esac
  ])

AC_DEFINE(AUDIO, 1, [Make audio-specific code])
AM_CONDITIONAL(AUDIO, test "yes" = "yes")
AC_SUBST(AMSCXXFLAGS)

AC_OUTPUT([\
Makefile \
plugins/Makefile \
interfaces/Makefile \
events/Makefile \
trivia/Makefile \
m4/Makefile \
])

######## Show a configuration report ########
#if test -n "$JADE"; then
#  jade_report="$JADE";
#else
#  jade_report="not present"
#fi
AC_MSG_NOTICE([

  Ams configured

  JACK support:        $JACK_FOUND
  LADCCA support:      $LADCCA_FOUND
  doxygen:             $DOXYGEN
  graphviz:            $DOT
  perl:                $PERL
  treeviews in doxygen
  html output:         $report_doxy_treeview

  C compiler:          $CC
  C++ compiler:        $CXX
  optimizing:          $report_optimise
  optimise for arch:   $report_march

  installation prefix: $prefix

])

