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
PROGNAME = ams

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
LIBS = -l$(QT) -lpthread -ldl -lrt -ljack -lasound -lsrfftw -lsfftw -lm


SRC = *.cpp
CFLAGS =  -DQT_THREAD_SUPPORT  $(INCLUDEDIRS) -Wall
LDFLAGS =  $(CFLAGS) $(LIBS)

ifeq ($(DEBUG),yes)
CFLAGS    += -g
else
CFLAGS    += -O3
endif


MOCS += moc_actionmidibinding.cpp
MOCS += moc_actionparameter.cpp
MOCS += moc_actionparametereditor.cpp
MOCS += moc_boolmidibinding.cpp
MOCS += moc_boolparameter.cpp
MOCS += moc_boolparametereditor.cpp
MOCS += moc_configdialog.cpp
MOCS += moc_enummidibinding.cpp
MOCS += moc_enumparameter.cpp
MOCS += moc_enumparametereditor.cpp
MOCS += moc_envelope.cpp
MOCS += moc_filter.cpp
MOCS += moc_floatmidibinding.cpp
MOCS += moc_floatparameter.cpp
MOCS += moc_floatparametereditor.cpp
MOCS += moc_intmidibinding.cpp
MOCS += moc_intparameter.cpp
MOCS += moc_intparametereditor.cpp
MOCS += moc_ladspadialog.cpp
MOCS += moc_m_advenv.cpp
MOCS += moc_m_cvs.cpp
MOCS += moc_m_delay.cpp
MOCS += moc_m_dynamicwaves.cpp
MOCS += moc_m_env.cpp
MOCS += moc_m_in.cpp
MOCS += moc_m_inv.cpp
MOCS += moc_m_jackin.cpp
MOCS += moc_m_jackout.cpp
MOCS += moc_m_ladspa.cpp
MOCS += moc_m_lfo.cpp
MOCS += moc_m_mcv.cpp
MOCS += moc_m_midiout.cpp
MOCS += moc_m_mix.cpp
MOCS += moc_m_noise.cpp
MOCS += moc_m_out.cpp
MOCS += moc_m_quantizer.cpp
MOCS += moc_m_ringmod.cpp
MOCS += moc_m_seq.cpp
MOCS += moc_m_sh.cpp
MOCS += moc_m_slew.cpp
MOCS += moc_m_vca.cpp
MOCS += moc_m_vcf.cpp
MOCS += moc_m_vco.cpp
MOCS += moc_m_vcorgan.cpp
MOCS += moc_m_vcswitch.cpp
MOCS += moc_m_wavout.cpp
MOCS += moc_midibinding.cpp
MOCS += moc_midicontrolcenter.cpp
MOCS += moc_midicontroller.cpp
MOCS += moc_modularsynth.cpp
MOCS += moc_module.cpp
MOCS += moc_multi_envelope.cpp
MOCS += moc_parameter.cpp
MOCS += moc_parametereditor.cpp
MOCS += moc_parameterpanel.cpp
MOCS += moc_port.cpp
#MOCS += moc_stylechooser.cpp
MOCS += moc_textedit.cpp
OBJS += actionmidibinding.o
OBJS += actionparameter.o
OBJS += actionparametereditor.o
OBJS += boolmidibinding.o
OBJS += boolparameter.o
OBJS += boolparametereditor.o
OBJS += capture.o
OBJS += configdialog.o
OBJS += enummidibinding.o
OBJS += enumparameter.o
OBJS += enumparametereditor.o
OBJS += envelope.o
OBJS += filter.o
OBJS += floatmidibinding.o
OBJS += floatparameter.o
OBJS += floatparametereditor.o
OBJS += intmidibinding.o
OBJS += intparameter.o
OBJS += intparametereditor.o
OBJS += ladspadialog.o
OBJS += m_advenv.o
OBJS += m_cvs.o
OBJS += m_delay.o
OBJS += m_dynamicwaves.o
OBJS += m_env.o
OBJS += m_in.o
OBJS += m_inv.o
OBJS += m_jackin.o
OBJS += m_jackout.o
OBJS += m_ladspa.o
OBJS += m_lfo.o
OBJS += m_mcv.o
OBJS += m_midiout.o
OBJS += m_mix.o
OBJS += m_noise.o
OBJS += m_out.o
OBJS += m_quantizer.o
OBJS += m_ringmod.o
OBJS += m_seq.o
OBJS += m_sh.o
OBJS += m_slew.o
OBJS += m_vca.o
OBJS += m_vcf.o
OBJS += m_vco.o
OBJS += m_vcorgan.o
OBJS += m_vcswitch.o
OBJS += m_wavout.o
OBJS += main.o
OBJS += midibinding.o
OBJS += midicontrolcenter.o
OBJS += midicontroller.o
OBJS += midicontrollerlist.o
OBJS += modularsynth.o
OBJS += module.o
OBJS += multi_envelope.o
OBJS += objectlistviewitem.o
OBJS += parameter.o
OBJS += parametereditor.o
OBJS += parametereditorfactory.o
OBJS += parameterpanel.o
OBJS += port.o
#OBJS += stylechooser.o
OBJS += synth.o
OBJS += synthdata.o
OBJS += textedit.o
OBJS += moc_actionmidibinding.o
OBJS += moc_actionparameter.o
OBJS += moc_actionparametereditor.o
OBJS += moc_boolmidibinding.o
OBJS += moc_boolparameter.o
OBJS += moc_boolparametereditor.o
OBJS += moc_configdialog.o
OBJS += moc_enummidibinding.o
OBJS += moc_enumparameter.o
OBJS += moc_enumparametereditor.o
OBJS += moc_envelope.o
OBJS += moc_filter.o
OBJS += moc_floatmidibinding.o
OBJS += moc_floatparameter.o
OBJS += moc_floatparametereditor.o
OBJS += moc_intmidibinding.o
OBJS += moc_intparameter.o
OBJS += moc_intparametereditor.o
OBJS += moc_ladspadialog.o
OBJS += moc_m_advenv.o
OBJS += moc_m_cvs.o
OBJS += moc_m_delay.o
OBJS += moc_m_dynamicwaves.o
OBJS += moc_m_env.o
OBJS += moc_m_in.o
OBJS += moc_m_inv.o
OBJS += moc_m_jackin.o
OBJS += moc_m_jackout.o
OBJS += moc_m_ladspa.o
OBJS += moc_m_lfo.o
OBJS += moc_m_mcv.o
OBJS += moc_m_midiout.o
OBJS += moc_m_mix.o
OBJS += moc_m_noise.o
OBJS += moc_m_out.o
OBJS += moc_m_quantizer.o
OBJS += moc_m_ringmod.o
OBJS += moc_m_seq.o
OBJS += moc_m_sh.o
OBJS += moc_m_slew.o
OBJS += moc_m_vca.o
OBJS += moc_m_vcf.o
OBJS += moc_m_vco.o
OBJS += moc_m_vcorgan.o
OBJS += moc_m_vcswitch.o
OBJS += moc_m_wavout.o
OBJS += moc_midibinding.o
OBJS += moc_midicontrolcenter.o
OBJS += moc_midicontroller.o
OBJS += moc_modularsynth.o
OBJS += moc_module.o
OBJS += moc_multi_envelope.o
OBJS += moc_parameter.o
OBJS += moc_parametereditor.o
OBJS += moc_parameterpanel.o
OBJS += moc_port.o
#OBJS += moc_stylechooser.o
OBJS += moc_textedit.o






#---------------------------------------
#Rules:
#
all: target
include .depend

target: $(MOCS) $(OBJS)
	@echo linking
	$(CC) $(LDFLAGS) $(OBJS) -o $(PROGNAME)

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


