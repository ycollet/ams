#====================================================================
# New Makefile
# This is a very stupid makefile, since i yet have to learn how
# to use autoconf/automake :-)
#
#
# Please have a look at the variables below and adjust them to fit
# your needs.
#====================================================================

#---------------------------------------
#  Some definitions:
#  Please change them to fit your system
#

# Name of executable
PROGNAME = test_autoload

# Path to your QT dirs.
QT_DIR = /usr
QT_INCLUDE_DIR = $(QT_DIR)/include/qt
QT_LIB_DIR = $(QT_DIR)/lib

# Path to your x11 dirs
X11_INCLUDE_DIR = /usr/X11R6/include
X11_LIB_DIR = /usr/X11R6/lib

# GCC
CC = /usr/bin/g++

# moc
MOC = $(QT_DIR)/bin/moc

#Set this to qt or qt-mt
QT = qt-mt

#Set this to yes to disable optimization and include
#debug informaion into the binary. only tested with gcc/gdb
DEBUG = yes

#------------------------------------------------
#  The following settings should only be altered
#  if they seem particulary wrong with your setup.
#


# Include dirs:
INCLUDEDIRS = -I$(QT_INCLUDE_DIR) -I$(X11_INCLUDE_DIR) -I.


# System libs to link with
#LIBS = -l$(QT) -ljack -lasound -lsrfftw -lsfftw -lm
LIBS = -l$(QT) -lpthread -ldl -lrt -ljack -lasound -lsrfftw -lsfftw -lm


SRC = *.cpp

CFLAGS =  -rdynamic -DQT_THREAD_SUPPORT  $(INCLUDEDIRS) -Wall
LDFLAGS =  $(CFLAGS) $(LIBS)

ifeq ($(DEBUG),yes)
CFLAGS    += -g
else
CFLAGS    += -O3
endif


MOCS += moc_testplugin.cpp
OBJS += plugin.o
OBJS += testplugin.o
OBJS += moc_testplugin.o

OBJS += test_autoload.o



#---------------------------------------
#Rules:
#
all: target
include .depend

target: $(MOCS) $(OBJS)
	@echo linking
	$(CC) $(LDFLAGS) -o $(PROGNAME) test_autoload.o plugin.o  
	$(CC) $(LDFLAGS) -shared -Wl,-soname,libtestplugin.so -o libtestplugin.so testplugin.o moc_testplugin.o 
moc_%.cpp:%.h
	@echo mocking $@ from $<
	$(MOC) $< -o $@

%.o:%.cpp
	@echo building $@
	$(CC) $(CFLAGS) -c $< -o $@

depend:
	echo Making depends into .depend
	$(CC) $(CFLAGS) -MM $(SRC)> .depend

clean:
	rm -f $(PROGNAME) $(MOCS) $(OBJS)


