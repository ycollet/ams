QT_BASE_DIR=/usr/lib/qt3
QT_LIB_DIR=$(QT_BASE_DIR)/lib
QT_BIN_DIR=$(QT_BASE_DIR)/bin
QT_INCLUDE_DIR=$(QT_BASE_DIR)/include
X11_LIB_DIR=/usr/X11R6/lib
LADSPA_PATH?=/usr/lib/ladspa:/usr/local/lib/ladspa
VERSION=1.7.1

#CXXFLAGS=-DQT_THREAD_SUPPORT -I$(QT_INCLUDE_DIR) -DLADSPA_PATH=\"$(LADSPA_PATH)\" -I/usr/X11R6/include -I. -O2 -g -Wall
CXXFLAGS=-DQT_THREAD_SUPPORT -I$(QT_INCLUDE_DIR) -DLADSPA_PATH=\"$(LADSPA_PATH)\" -I/usr/X11R6/include -I. -O2 -Wall

AMS_O = synthdata.o module.o module.moc.o \
	configdialog.o configdialog.moc.o \
	port.o port.moc.o \
	midiguicomponent.o midiguicomponent.moc.o \
	midislider.o midislider.moc.o \
	intmidislider.o intmidislider.moc.o \
	floatintmidislider.o floatintmidislider.moc.o \
	midicombobox.o midicombobox.moc.o \
	midicheckbox.o midicheckbox.moc.o \
	midipushbutton.o midipushbutton.moc.o \
	midicontroller.o midicontroller.moc.o \
	midicontrollerlist.o midicontrollerlist.moc.o \
	midiwidget.o midiwidget.moc.o \
	guiwidget.o guiwidget.moc.o \
	ladspadialog.o ladspadialog.moc.o \
	envelope.o envelope.moc.o \
	multi_envelope.o multi_envelope.moc.o \
	scopescreen.o scopescreen.moc.o \
	spectrumscreen.o spectrumscreen.moc.o \
	function.o function.moc.o \
	canvas.o canvas.moc.o \
	canvasfunction.o canvasfunction.moc.o \
	m_sh.o m_sh.moc.o \
	m_vcswitch.o m_vcswitch.moc.o \
	m_vcorgan.o m_vcorgan.moc.o \
	m_dynamicwaves.o m_dynamicwaves.moc.o \
	m_vco.o m_vco.moc.o \
	m_vca.o m_vca.moc.o \
	m_vcf.o m_vcf.moc.o \
        m_lfo.o m_lfo.moc.o \
        m_noise.o m_noise.moc.o \
	m_delay.o m_delay.moc.o \
	m_seq.o m_seq.moc.o \
	m_mcv.o m_mcv.moc.o \
	m_advmcv.o m_advmcv.moc.o \
	m_scmcv.o m_scmcv.moc.o \
	m_env.o m_env.moc.o \
	m_vcenv.o m_vcenv.moc.o \
	m_advenv.o m_advenv.moc.o \
	m_ringmod.o m_ringmod.moc.o \
	m_inv.o m_inv.moc.o \
	m_conv.o m_conv.moc.o \
	m_cvs.o m_cvs.moc.o \
	m_slew.o m_slew.moc.o \
	m_quantizer.o m_quantizer.moc.o \
	m_scquantizer.o m_scquantizer.moc.o \
	m_mix.o m_mix.moc.o \
	m_function.o m_function.moc.o \
	m_stereomix.o m_stereomix.moc.o \
	m_ladspa.o m_ladspa.moc.o \
	m_pcmout.o m_pcmout.moc.o \
	m_pcmin.o m_pcmin.moc.o \
	m_wavout.o m_wavout.moc.o \
	m_midiout.o m_midiout.moc.o \
	m_scope.o m_scope.moc.o \
	m_spectrum.o m_spectrum.moc.o \
	textedit.o textedit.moc.o \
	modularsynth.o modularsynth.moc.o \
	main.o alsa_driver.o

ams:	$(AMS_O)
	gcc -g -o ams $(AMS_O) \
	-L$(QT_LIB_DIR) -L$(X11_LIB_DIR) \
	-lqt-mt -ljack -lasound -lsrfftw -lsfftw -lm

clean:
	/bin/rm -f *.o *.moc.cpp *~

tarball:	clean
	cd ..; /bin/rm -f ams-$(VERSION).tar.bz2; tar cvf ams-$(VERSION).tar ams-$(VERSION); bzip2 ams-$(VERSION).tar


synthdata.o: synthdata.cpp synthdata.h module.h port.h m_env.h m_advenv.h m_vcenv.h main.h
modularsynth.o: modularsynth.cpp modularsynth.h module.h port.h textedit.h \
	synthdata.h midicontroller.h midicontrollerlist.h midiwidget.h guiwidget.h m_vcf.h m_sh.h \
	m_vco.h m_vca.h m_lfo.h m_delay.h m_ringmod.h m_inv.h m_mix.h \
	m_stereomix.h m_ladspa.h m_wavout.h m_conv.h \
	m_mcv.h m_advmcv.h m_seq.h m_env.h m_slew.h m_quantizer.h ladspadialog.h m_cvs.h m_midiout.h m_vcenv.h \
	m_vcorgan.h m_dynamicwaves.h m_advenv.h m_scope.h m_spectrum.h m_vcswitch.h m_pcmout.h m_pcmin.h \
	m_scmcv.h m_scquantizer.h m_function.h main.h
modularsynth.moc.o: modularsynth.moc.cpp modularsynth.h module.h port.h textedit.h \
	synthdata.h midicontroller.h midicontrollerlist.h midiwidget.h guiwidget.h m_vcf.h m_sh.h \
	m_vco.h m_vca.h m_lfo.h m_delay.h m_ringmod.h m_inv.h m_mix.h \
	m_stereomix.h m_ladspa.h m_wavout.h m_conv.h \
	m_mcv.h m_advmcv.h m_seq.h m_env.h m_slew.h m_quantizer.h ladspadialog.h m_cvs.h m_midiout.h m_vcenv.h \
	m_vcorgan.h m_dynamicwaves.h m_advenv.h m_scope.h m_spectrum.h m_vcswitch.h m_pcmout.h m_pcmin.h \
	m_scmcv.h m_scquantizer.h m_function.h
modularsynth.moc.cpp: modularsynth.h modularsynth.cpp
	$(QT_BIN_DIR)/moc modularsynth.h -o modularsynth.moc.cpp
main.o: main.cpp modularsynth.h main.h
configdialog.o: configdialog.cpp configdialog.h midicontroller.h envelope.h multi_envelope.h function.h \
	midicontrollerlist.h midiwidget.h guiwidget.h midislider.h intmidislider.h floatintmidislider.h midicombobox.h midicheckbox.h synthdata.h \
	scopescreen.h spectrumscreen.h midipushbutton.h
configdialog.moc.o: configdialog.moc.cpp configdialog.h midicontroller.h envelope.h multi_envelope.h function.h \
	midicontrollerlist.h midiwidget.h guiwidget.h midislider.h intmidislider.h floatintmidislider.h midicombobox.h midicheckbox.h synthdata.h \
	scopescreen.h spectrumscreen.h midipushbutton.h
configdialog.moc.cpp: configdialog.h configdialog.cpp
	$(QT_BIN_DIR)/moc configdialog.h -o configdialog.moc.cpp
module.o: module.cpp module.h synthdata.h configdialog.h port.h main.h
module.moc.o: module.moc.cpp module.h synthdata.h configdialog.h port.h
module.moc.cpp: module.h module.cpp
	$(QT_BIN_DIR)/moc module.h -o module.moc.cpp
port.o: port.cpp port.h synthdata.h main.h
port.moc.o: port.moc.cpp port.h synthdata.h
port.moc.cpp: port.h port.cpp
	$(QT_BIN_DIR)/moc port.h -o port.moc.cpp
midiguicomponent.o: midiguicomponent.cpp midiguicomponent.h midicontroller.h midicontrollerlist.h
midiguicomponent.moc.o: midiguicomponent.moc.cpp midiguicomponent.h midicontroller.h midicontrollerlist.h
midiguicomponent.moc.cpp: midiguicomponent.h midiguicomponent.cpp
	$(QT_BIN_DIR)/moc midiguicomponent.h -o midiguicomponent.moc.cpp
midislider.o: midislider.cpp midislider.h intmidislider.h synthdata.h midiwidget.h guiwidget.h \
              midicontroller.h midiguicomponent.h
midislider.moc.o: midislider.moc.cpp midislider.h intmidislider.h synthdata.h midiwidget.h guiwidget.h \
              midicontroller.h midiguicomponent.h
midislider.moc.cpp: midislider.h midislider.cpp
	$(QT_BIN_DIR)/moc midislider.h -o midislider.moc.cpp
intmidislider.o: intmidislider.cpp intmidislider.h synthdata.h midiwidget.h guiwidget.h \
                 midicontroller.h midiguicomponent.h
intmidislider.moc.o: intmidislider.moc.cpp intmidislider.h synthdata.h midiwidget.h guiwidget.h \
                 midicontroller.h midiguicomponent.h
intmidislider.moc.cpp: intmidislider.h intmidislider.cpp
	$(QT_BIN_DIR)/moc intmidislider.h -o intmidislider.moc.cpp
floatintmidislider.o: floatintmidislider.cpp floatintmidislider.h synthdata.h midiwidget.h guiwidget.h \
                 midicontroller.h midiguicomponent.h
floatintmidislider.moc.o: floatintmidislider.moc.cpp floatintmidislider.h synthdata.h midiwidget.h guiwidget.h \
                 midicontroller.h midiguicomponent.h
floatintmidislider.moc.cpp: floatintmidislider.h floatintmidislider.cpp
	$(QT_BIN_DIR)/moc floatintmidislider.h -o floatintmidislider.moc.cpp
midicombobox.o: midicombobox.cpp midicombobox.h synthdata.h midiwidget.h guiwidget.h \
                midicontroller.h midiguicomponent.h
midicombobox.moc.o: midicombobox.moc.cpp midicombobox.h synthdata.h midiwidget.h guiwidget.h \
                midicontroller.h midiguicomponent.h
midicombobox.moc.cpp: midicombobox.h midicombobox.cpp
	$(QT_BIN_DIR)/moc midicombobox.h -o midicombobox.moc.cpp
midicheckbox.o: midicheckbox.cpp midicheckbox.h synthdata.h midiwidget.h guiwidget.h \
                midicontroller.h midiguicomponent.h
midicheckbox.moc.o: midicheckbox.moc.cpp midicheckbox.h synthdata.h midiwidget.h guiwidget.h midicontroller.h
midicheckbox.moc.cpp: midicheckbox.h midicheckbox.cpp
	$(QT_BIN_DIR)/moc midicheckbox.h -o midicheckbox.moc.cpp
midipushbutton.o: midipushbutton.cpp midipushbutton.h synthdata.h midiwidget.h guiwidget.h \
                midicontroller.h midiguicomponent.h
midipushbutton.moc.o: midipushbutton.moc.cpp midipushbutton.h synthdata.h midiwidget.h midicontroller.h guiwidget.h
midipushbutton.moc.cpp: midipushbutton.h midipushbutton.cpp
	$(QT_BIN_DIR)/moc midipushbutton.h -o midipushbutton.moc.cpp
envelope.o: envelope.cpp envelope.h synthdata.h
envelope.moc.o: envelope.moc.cpp envelope.h synthdata.h
envelope.moc.cpp: envelope.h envelope.cpp
	$(QT_BIN_DIR)/moc envelope.h -o envelope.moc.cpp
function.o: function.cpp function.h synthdata.h canvasfunction.h canvas.h
function.moc.o: function.moc.cpp function.h synthdata.h canvasfunction.h canvas.h
function.moc.cpp: function.h function.cpp
	$(QT_BIN_DIR)/moc function.h -o function.moc.cpp
canvasfunction.o: canvasfunction.cpp canvasfunction.h
canvasfunction.moc.o: canvasfunction.moc.cpp canvasfunction.h
canvasfunction.moc.cpp: canvasfunction.h canvasfunction.cpp
	$(QT_BIN_DIR)/moc canvasfunction.h -o canvasfunction.moc.cpp
canvas.o: canvas.cpp canvas.h
canvas.moc.o: canvas.moc.cpp canvas.h
canvas.moc.cpp: canvas.h canvas.cpp
	$(QT_BIN_DIR)/moc canvas.h -o canvas.moc.cpp
multi_envelope.o: multi_envelope.cpp multi_envelope.h synthdata.h
multi_envelope.moc.o: multi_envelope.moc.cpp multi_envelope.h synthdata.h
multi_envelope.moc.cpp: multi_envelope.h multi_envelope.cpp
	$(QT_BIN_DIR)/moc multi_envelope.h -o multi_envelope.moc.cpp
scopescreen.o: scopescreen.cpp scopescreen.h synthdata.h
scopescreen.moc.o: scopescreen.moc.cpp scopescreen.h synthdata.h
scopescreen.moc.cpp: scopescreen.h scopescreen.cpp
	$(QT_BIN_DIR)/moc scopescreen.h -o scopescreen.moc.cpp
spectrumscreen.o: spectrumscreen.cpp spectrumscreen.h synthdata.h
spectrumscreen.moc.o: spectrumscreen.moc.cpp spectrumscreen.h synthdata.h
spectrumscreen.moc.cpp: spectrumscreen.h spectrumscreen.cpp
	$(QT_BIN_DIR)/moc spectrumscreen.h -o spectrumscreen.moc.cpp
midicontroller.o: midicontroller.cpp midicontroller.h
midicontroller.moc.o: midicontroller.moc.cpp midicontroller.h
midicontroller.moc.cpp: midicontroller.h midicontroller.cpp
	$(QT_BIN_DIR)/moc midicontroller.h -o midicontroller.moc.cpp
midicontrollerlist.o: midicontrollerlist.cpp midicontrollerlist.h midicontroller.h
midicontrollerlist.moc.o: midicontrollerlist.moc.cpp midicontrollerlist.h midicontroller.h
midicontrollerlist.moc.cpp: midicontrollerlist.h midicontrollerlist.cpp
	$(QT_BIN_DIR)/moc midicontrollerlist.h -o midicontrollerlist.moc.cpp
midiwidget.o: midiwidget.cpp midiwidget.h midicontroller.h midicontrollerlist.h synthdata.h guiwidget.h \
	midislider.h intmidislider.h floatintmidislider.h midicombobox.h midicheckbox.h midipushbutton.h
midiwidget.moc.o: midiwidget.moc.cpp midiwidget.h midicontroller.h midicontrollerlist.h synthdata.h guiwidget.h \
	midislider.h intmidislider.h floatintmidislider.h midicombobox.h midicheckbox.h midipushbutton.h
midiwidget.moc.cpp: midiwidget.h midiwidget.cpp
	$(QT_BIN_DIR)/moc midiwidget.h -o midiwidget.moc.cpp
guiwidget.o: guiwidget.cpp guiwidget.h midicontroller.h midicontrollerlist.h synthdata.h \
	midislider.h intmidislider.h floatintmidislider.h midicombobox.h midicheckbox.h midipushbutton.h
guiwidget.moc.o: guiwidget.moc.cpp guiwidget.h midicontroller.h midicontrollerlist.h synthdata.h \
	midislider.h intmidislider.h floatintmidislider.h midicombobox.h midicheckbox.h midipushbutton.h
guiwidget.moc.cpp: guiwidget.h guiwidget.cpp
	$(QT_BIN_DIR)/moc guiwidget.h -o guiwidget.moc.cpp
ladspadialog.o: ladspadialog.cpp ladspadialog.h synthdata.h
ladspadialog.moc.o: ladspadialog.moc.cpp ladspadialog.h synthdata.h
ladspadialog.moc.cpp: ladspadialog.h ladspadialog.cpp
	$(QT_BIN_DIR)/moc ladspadialog.h -o ladspadialog.moc.cpp
m_vco.o: m_vco.cpp m_vco.h synthdata.h module.h port.h
m_vco.moc.o: m_vco.moc.cpp m_vco.h synthdata.h module.h port.h
m_vco.moc.cpp: m_vco.h m_vco.cpp
	$(QT_BIN_DIR)/moc m_vco.h -o m_vco.moc.cpp
m_vca.o: m_vca.cpp m_vca.h synthdata.h module.h port.h
m_vca.moc.o: m_vca.moc.cpp m_vca.h synthdata.h module.h port.h
m_vca.moc.cpp: m_vca.h m_vca.cpp
	$(QT_BIN_DIR)/moc m_vca.h -o m_vca.moc.cpp
m_vcf.o: m_vcf.cpp m_vcf.h synthdata.h module.h port.h
m_vcf.moc.o: m_vcf.moc.cpp m_vcf.h synthdata.h module.h port.h
m_vcf.moc.cpp: m_vcf.h m_vcf.cpp
	$(QT_BIN_DIR)/moc m_vcf.h -o m_vcf.moc.cpp
m_lfo.o: m_lfo.cpp m_lfo.h synthdata.h module.h port.h
m_lfo.moc.o: m_lfo.moc.cpp m_lfo.h synthdata.h module.h port.h
m_lfo.moc.cpp: m_lfo.h m_lfo.cpp
	$(QT_BIN_DIR)/moc m_lfo.h -o m_lfo.moc.cpp
m_noise.o: m_noise.cpp m_noise.h synthdata.h module.h port.h
m_noise.moc.o: m_noise.moc.cpp m_noise.h synthdata.h module.h port.h
m_noise.moc.cpp: m_noise.h m_noise.cpp
	$(QT_BIN_DIR)/moc m_noise.h -o m_noise.moc.cpp
m_delay.o: m_delay.cpp m_delay.h synthdata.h module.h port.h
m_delay.moc.o: m_delay.moc.cpp m_delay.h synthdata.h module.h port.h
m_delay.moc.cpp: m_delay.h m_delay.cpp
	$(QT_BIN_DIR)/moc m_delay.h -o m_delay.moc.cpp
m_env.o: m_env.cpp m_env.h synthdata.h module.h port.h
m_env.moc.o: m_env.moc.cpp m_env.h synthdata.h module.h port.h
m_env.moc.cpp: m_env.h m_env.cpp
	$(QT_BIN_DIR)/moc m_env.h -o m_env.moc.cpp
m_vcenv.o: m_vcenv.cpp m_vcenv.h synthdata.h module.h port.h
m_vcenv.moc.o: m_vcenv.moc.cpp m_vcenv.h synthdata.h module.h port.h
m_vcenv.moc.cpp: m_vcenv.h m_vcenv.cpp
	$(QT_BIN_DIR)/moc m_vcenv.h -o m_vcenv.moc.cpp
m_advenv.o: m_advenv.cpp m_advenv.h synthdata.h module.h port.h
m_advenv.moc.o: m_advenv.moc.cpp m_advenv.h synthdata.h module.h port.h
m_advenv.moc.cpp: m_advenv.h m_advenv.cpp
	$(QT_BIN_DIR)/moc m_advenv.h -o m_advenv.moc.cpp
m_mcv.o: m_mcv.cpp m_mcv.h synthdata.h module.h port.h
m_mcv.moc.o: m_mcv.moc.cpp m_mcv.h synthdata.h module.h port.h
m_mcv.moc.cpp: m_mcv.h m_mcv.cpp
	$(QT_BIN_DIR)/moc m_mcv.h -o m_mcv.moc.cpp
m_advmcv.o: m_advmcv.cpp m_advmcv.h synthdata.h module.h port.h
m_advmcv.moc.o: m_advmcv.moc.cpp m_advmcv.h synthdata.h module.h port.h
m_advmcv.moc.cpp: m_advmcv.h m_advmcv.cpp
	$(QT_BIN_DIR)/moc m_advmcv.h -o m_advmcv.moc.cpp
m_scmcv.o: m_scmcv.cpp m_scmcv.h synthdata.h module.h port.h
m_scmcv.moc.o: m_scmcv.moc.cpp m_scmcv.h synthdata.h module.h port.h
m_scmcv.moc.cpp: m_scmcv.h m_scmcv.cpp
	$(QT_BIN_DIR)/moc m_scmcv.h -o m_scmcv.moc.cpp
m_seq.o: m_seq.cpp m_seq.h synthdata.h module.h port.h
m_seq.moc.o: m_seq.moc.cpp m_seq.h synthdata.h module.h port.h
m_seq.moc.cpp: m_seq.h m_seq.cpp
	$(QT_BIN_DIR)/moc m_seq.h -o m_seq.moc.cpp
m_ringmod.o: m_ringmod.cpp m_ringmod.h synthdata.h module.h port.h
m_ringmod.moc.o: m_ringmod.moc.cpp m_ringmod.h synthdata.h module.h port.h
m_ringmod.moc.cpp: m_ringmod.h m_ringmod.cpp
	$(QT_BIN_DIR)/moc m_ringmod.h -o m_ringmod.moc.cpp
m_inv.o: m_inv.cpp m_inv.h synthdata.h module.h port.h
m_inv.moc.o: m_inv.moc.cpp m_inv.h synthdata.h module.h port.h
m_inv.moc.cpp: m_inv.h m_inv.cpp
	$(QT_BIN_DIR)/moc m_inv.h -o m_inv.moc.cpp
m_conv.o: m_conv.cpp m_conv.h synthdata.h module.h port.h
m_conv.moc.o: m_conv.moc.cpp m_conv.h synthdata.h module.h port.h
m_conv.moc.cpp: m_conv.h m_conv.cpp
	$(QT_BIN_DIR)/moc m_conv.h -o m_conv.moc.cpp
m_cvs.o: m_cvs.cpp m_cvs.h synthdata.h module.h port.h
m_cvs.moc.o: m_cvs.moc.cpp m_cvs.h synthdata.h module.h port.h
m_cvs.moc.cpp: m_cvs.h m_cvs.cpp
	$(QT_BIN_DIR)/moc m_cvs.h -o m_cvs.moc.cpp
m_sh.o: m_sh.cpp m_sh.h synthdata.h module.h port.h
m_sh.moc.o: m_sh.moc.cpp m_sh.h synthdata.h module.h port.h
m_sh.moc.cpp: m_sh.h m_sh.cpp
	$(QT_BIN_DIR)/moc m_sh.h -o m_sh.moc.cpp
m_vcswitch.o: m_vcswitch.cpp m_vcswitch.h synthdata.h module.h port.h
m_vcswitch.moc.o: m_vcswitch.moc.cpp m_vcswitch.h synthdata.h module.h port.h
m_vcswitch.moc.cpp: m_vcswitch.h m_vcswitch.cpp
	$(QT_BIN_DIR)/moc m_vcswitch.h -o m_vcswitch.moc.cpp
m_vcorgan.o: m_vcorgan.cpp m_vcorgan.h synthdata.h module.h port.h
m_vcorgan.moc.o: m_vcorgan.moc.cpp m_vcorgan.h synthdata.h module.h port.h
m_vcorgan.moc.cpp: m_vcorgan.h m_vcorgan.cpp
	$(QT_BIN_DIR)/moc m_vcorgan.h -o m_vcorgan.moc.cpp
m_dynamicwaves.o: m_dynamicwaves.cpp m_dynamicwaves.h synthdata.h module.h port.h
m_dynamicwaves.moc.o: m_dynamicwaves.moc.cpp m_dynamicwaves.h synthdata.h module.h port.h
m_dynamicwaves.moc.cpp: m_dynamicwaves.h m_dynamicwaves.cpp
	$(QT_BIN_DIR)/moc m_dynamicwaves.h -o m_dynamicwaves.moc.cpp
m_slew.o: m_slew.cpp m_slew.h synthdata.h module.h port.h
m_slew.moc.o: m_slew.moc.cpp m_slew.h synthdata.h module.h port.h
m_slew.moc.cpp: m_slew.h m_slew.cpp
	$(QT_BIN_DIR)/moc m_slew.h -o m_slew.moc.cpp
m_quantizer.o: m_quantizer.cpp m_quantizer.h synthdata.h module.h port.h
m_quantizer.moc.o: m_quantizer.moc.cpp m_quantizer.h synthdata.h module.h port.h
m_quantizer.moc.cpp: m_quantizer.h m_quantizer.cpp
	$(QT_BIN_DIR)/moc m_quantizer.h -o m_quantizer.moc.cpp
m_scquantizer.o: m_scquantizer.cpp m_scquantizer.h synthdata.h module.h port.h
m_scquantizer.moc.o: m_scquantizer.moc.cpp m_scquantizer.h synthdata.h module.h port.h
m_scquantizer.moc.cpp: m_scquantizer.h m_scquantizer.cpp
	$(QT_BIN_DIR)/moc m_scquantizer.h -o m_scquantizer.moc.cpp
m_mix.o: m_mix.cpp m_mix.h synthdata.h module.h port.h
m_mix.moc.o: m_mix.moc.cpp m_mix.h synthdata.h module.h port.h
m_mix.moc.cpp: m_mix.h m_mix.cpp
	$(QT_BIN_DIR)/moc m_mix.h -o m_mix.moc.cpp
m_function.o: m_function.cpp m_function.h synthdata.h module.h port.h function.h
m_function.moc.o: m_function.moc.cpp m_function.h synthdata.h module.h port.h function.h
m_function.moc.cpp: m_function.h m_function.cpp
	$(QT_BIN_DIR)/moc m_function.h -o m_function.moc.cpp
m_stereomix.o: m_stereomix.cpp m_stereomix.h synthdata.h module.h port.h
m_stereomix.moc.o: m_stereomix.moc.cpp m_stereomix.h synthdata.h module.h port.h
m_stereomix.moc.cpp: m_stereomix.h m_stereomix.cpp
	$(QT_BIN_DIR)/moc m_stereomix.h -o m_stereomix.moc.cpp
m_ladspa.o: m_ladspa.cpp m_ladspa.h synthdata.h module.h port.h
m_ladspa.moc.o: m_ladspa.moc.cpp m_ladspa.h synthdata.h module.h port.h
m_ladspa.moc.cpp: m_ladspa.h m_ladspa.cpp
	$(QT_BIN_DIR)/moc m_ladspa.h -o m_ladspa.moc.cpp
m_pcmout.o: m_pcmout.cpp m_pcmout.h synthdata.h module.h port.h
m_pcmout.moc.o: m_pcmout.moc.cpp m_pcmout.h synthdata.h module.h port.h
m_pcmout.moc.cpp: m_pcmout.h m_pcmout.cpp
	$(QT_BIN_DIR)/moc m_pcmout.h -o m_pcmout.moc.cpp
m_pcmin.o: m_pcmin.cpp m_pcmin.h synthdata.h module.h port.h
m_pcmin.moc.o: m_pcmin.moc.cpp m_pcmin.h synthdata.h module.h port.h
m_pcmin.moc.cpp: m_pcmin.h m_pcmin.cpp
	$(QT_BIN_DIR)/moc m_pcmin.h -o m_pcmin.moc.cpp
m_wavout.o: m_wavout.cpp m_wavout.h synthdata.h module.h port.h
m_wavout.moc.o: m_wavout.moc.cpp m_wavout.h synthdata.h module.h port.h
m_wavout.moc.cpp: m_wavout.h m_wavout.cpp
	$(QT_BIN_DIR)/moc m_wavout.h -o m_wavout.moc.cpp
m_midiout.o: m_midiout.cpp m_midiout.h synthdata.h module.h port.h
m_midiout.moc.o: m_midiout.moc.cpp m_midiout.h synthdata.h module.h port.h
m_midiout.moc.cpp: m_midiout.h m_midiout.cpp
	$(QT_BIN_DIR)/moc m_midiout.h -o m_midiout.moc.cpp
m_scope.o: m_scope.cpp m_scope.h synthdata.h module.h port.h
m_scope.moc.o: m_scope.moc.cpp m_scope.h synthdata.h module.h port.h
m_scope.moc.cpp: m_scope.h m_scope.cpp
	$(QT_BIN_DIR)/moc m_scope.h -o m_scope.moc.cpp
m_spectrum.o: m_spectrum.cpp m_spectrum.h synthdata.h module.h port.h
m_spectrum.moc.o: m_spectrum.moc.cpp m_spectrum.h synthdata.h module.h port.h
m_spectrum.moc.cpp: m_spectrum.h m_spectrum.cpp
	$(QT_BIN_DIR)/moc m_spectrum.h -o m_spectrum.moc.cpp
textedit.o: textedit.cpp textedit.h synthdata.h
textedit.moc.o: textedit.moc.cpp textedit.h synthdata.h
textedit.moc.cpp: textedit.h textedit.cpp
	$(QT_BIN_DIR)/moc textedit.h -o textedit.moc.cpp
alsa_driver.o:	alsa_driver.h
