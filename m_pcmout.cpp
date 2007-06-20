#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>   
#include <qcheckbox.h>  
#include <qlabel.h>


#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qpainter.h>
#include <qtimer.h>
#include "synthdata.h"
#include "m_pcmout.h"
#include "module.h"
#include "port.h"


M_pcmout::M_pcmout(QWidget* parent, const char *name, int port) 
              : Module(0, parent, name)
{
  QString qs;

  M_type = M_type_pcmout;
  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_PCMOUT_WIDTH, MODULE_PCMOUT_HEIGHT);
  gain = 0.5;
  mixer_gain[0] = 0.5;
  mixer_gain[1] = 0.5;
  cycleSize=synthdata->cyclesize * sizeof(float);
  polyroot = sqrt((double)synthdata->poly);
  agc = 0;
  qs.sprintf (" -> Out %2d", port);
  port_in[0] = new Port(qs, PORT_IN, 0, this);          
  port_in[0]->move(0, 35);
  port_in[0]->outTypeAcceptList.append(outType_audio);
  portList.append(port_in[0]);
  qs.sprintf(" -> Out %2d", port + 1);
  port_in[1] = new Port(qs, PORT_IN, 1, this);          
  port_in[1]->move(0, 55);
  port_in[1]->outTypeAcceptList.append(outType_audio);
  portList.append(port_in[1]);
  qs.sprintf("Alsa / Pcm Out  ID %d", moduleID);
  configDialog->setWindowTitle(qs);
  configDialog->addSlider(0, 1, gain, "Gain", &gain, false);
  configDialog->addSlider(0, 1, mixer_gain[0], "Volume 1", &mixer_gain[0], false);
  configDialog->addSlider(0, 1, mixer_gain[1], "Volume 2", &mixer_gain[1], false);
  QStringList agcNames;
  agcNames << "Disabled" << "Enabled";
  configDialog->addComboBox(agc, "Automatic Gain Control", &agc, agcNames.count(), &agcNames);
  pcmdata[0] = new float[synthdata->periodsize];
  pcmdata[1] = new float[synthdata->periodsize];
}

M_pcmout::~M_pcmout()
{
  delete[] pcmdata[0];
  delete[] pcmdata[1];
}

void M_pcmout::generateCycle()
{
    int l1, l2, l3;
    float max, mixgain, **indata;
    
    
    for (l1 = 0; l1 < 2; l1++)
    {
        memset(pcmdata[l1], 0, cycleSize);  
        indata = port_in [l1]->getinputdata ();

        if (indata != synthdata->zeroModuleData)
        {
            mixgain = gain * mixer_gain[l1] / polyroot;
            for (l2 = 0; l2 < synthdata->cyclesize; l2++)
            {
                for (l3 = 0; l3 < synthdata->poly; l3++)
                {
                    pcmdata[l1][l2] += mixgain * indata [l3][l2]; 
                }
            }
            if (agc)
            {
                max = 0.0f;
                for (l2 = 0; l2 < synthdata->cyclesize; l2++)
                {
                    if (max < fabs(pcmdata[l1][l2])) max = fabs(pcmdata[l1][l2]);
                }    
                if (max > 0.9f)
                {
                    max = 0.9f / max;
                    for (l2 = 0; l2 < synthdata->cyclesize; l2++) pcmdata[l1][l2] *= max;
		}
	    }
	}
    }
}

void M_pcmout::showConfigDialog() {
}
