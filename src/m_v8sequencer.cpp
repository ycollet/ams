/* 
  V8 sequencer - derived from m_vcdelay.cpp

  Copyright (C) 2011 Bill Yerazunis <yerazunis@yahoo.com>

  This file is part of ams.

  ams is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2 as
  published by the Free Software Foundation.

  ams is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with ams.  If not, see <http://www.gnu.org/licenses/>.
*/

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
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "m_v8sequencer.h"
#include "port.h"

M_v8sequencer::M_v8sequencer(QWidget* parent) 
  : Module(M_type_v8sequencer, 16, parent, tr("V8 Seq"))
{
  QString qs;

  setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_V8SEQUENCER_WIDTH, MODULE_V8SEQUENCER_HEIGHT);

  port_M_step = new Port(tr("Step"), PORT_IN, 0, this);
  port_M_direction = new Port(tr("Direction"), PORT_IN, 1, this);
  port_M_forward = new Port(tr("+ 1"), PORT_IN, 2, this);
  port_M_backward = new Port(tr("- 1"), PORT_IN, 3, this);
  port_M_daisy0 = new Port (tr("Daisy 0"), PORT_IN, 4, this);
  port_M_daisy1 = new Port (tr("Daisy 1"), PORT_IN, 5, this);
  port_M_seqstate = new Port(tr("StateIn"), PORT_IN, 6, this);
  port_M_seqtrig = new Port(tr("TrigIn"), PORT_IN, 7, this);
  port_M_s0 = new Port(tr("S 0"), PORT_IN, 8, this);
  port_M_s1 = new Port(tr("S 1"), PORT_IN, 9, this);
  port_M_s2 = new Port(tr("S 2"), PORT_IN, 10, this);
  port_M_s3 = new Port(tr("S 3"), PORT_IN, 11, this);
  port_M_s4 = new Port(tr("S 4"), PORT_IN, 12, this);
  port_M_s5 = new Port(tr("S 5"), PORT_IN, 13, this);
  port_M_s6 = new Port(tr("S 6"), PORT_IN, 14, this);
  port_M_s7 = new Port(tr("S 7"), PORT_IN, 15, this);

  cv.out_off = 35;
  port_out0 = new Port(tr("Out 0"), PORT_OUT, 0, this);          
  port_out1 = new Port(tr("Out 1"), PORT_OUT, 1, this);          
  port_out2 = new Port(tr("Out 2"), PORT_OUT, 2, this);          
  port_out3 = new Port(tr("Out 3"), PORT_OUT, 3, this);          
  port_aux0 = new Port(tr("Aux 0"), PORT_OUT, 4, this);          
  port_aux1 = new Port(tr("Aux 1"), PORT_OUT, 5, this);          
  port_seqstate = new Port(tr("State"), PORT_OUT, 6, this);          
  port_seqtrig = new Port(tr("Trig"), PORT_OUT, 7, this);          
  port_state0 = new Port(tr("S 0"), PORT_OUT, 8, this);          
  port_state1 = new Port(tr("S 1"), PORT_OUT, 9, this);          
  port_state2 = new Port(tr("S 2"), PORT_OUT, 10, this);          
  port_state3 = new Port(tr("S 3"), PORT_OUT, 11, this);          
  port_state4 = new Port(tr("S 4"), PORT_OUT, 12, this);          
  port_state5 = new Port(tr("S 5"), PORT_OUT, 13, this);          
  port_state6 = new Port(tr("S 6"), PORT_OUT, 14, this);          
  port_state7 = new Port(tr("S 7"), PORT_OUT, 15, this);          

  //    Now the joy of setting up a config screen.
  //    There is one line per sequencer state
  //
  //    Analog1   Analog2   Digi1   Sticky1    Digi2    Sticky2
  QHBoxLayout *hbox;
  int i;
  hbox = configDialog->addHBox();
  configDialog->addCheckBox (tr("Wrap from max to zero"), wrapmaxtozero, hbox);
  configDialog->addCheckBox (tr("Wrap from zero to max"), wrapzerotomax, hbox);
  configDialog->addIntSlider (tr("Max States"), maxstate, 1, 64, hbox);
  QStringList stateFormats;
  stateFormats << tr("1v/step from 0v");
  stateFormats << tr("1v/12 steps from 0v");
  stateFormats << tr("2v/Max States steps from -1v");
  stateFormats << tr("2v/MaxS from -1v, daisychain");
  configDialog->addComboBox (tr("State IO Format"), 
			     stateIOFormat, stateFormats, hbox);
  hbox = configDialog->addHBox();
  QStringList AuxSrcs;
  AuxSrcs << tr("Out0");
  AuxSrcs << tr("Out1");
  AuxSrcs << tr("Out2");
  AuxSrcs << tr("Out3");
  //  AuxSrcs << tr("Out2 ignore hold");
  //  AuxSrcs << tr("Out3 ignore hold");
  QStringList AuxOffsets;
  AuxOffsets << tr("0");
  AuxOffsets << tr("1");
  AuxOffsets << tr("2");
  AuxOffsets << tr("3");
  AuxOffsets << tr("4");
  AuxOffsets << tr("5");
  AuxOffsets << tr("6");
  AuxOffsets << tr("7");
  QStringList AuxBehaviors;
  AuxBehaviors << tr("0,1,2,3,4,5,6,7");
  AuxBehaviors << tr("0,1,2,3,0,1,2,3");
  AuxBehaviors << tr("0,2,4,6,0,2,4,6");
  AuxBehaviors << tr("0,1,0,1,0,1,0,1");
  AuxBehaviors << tr("0,2,0,2,0,2,0,2");
  AuxBehaviors << tr("0,3,0,3,0,3,0,3");
  AuxBehaviors << tr("7,6,5,4,3,2,1,0");
  AuxBehaviors << tr("3,2,1,0,3,2,1,0");
  AuxBehaviors << tr("6,4,2,0,6,4,2,0");
  configDialog->addComboBox(tr("Aux 0 source"), aux0src, AuxSrcs, hbox);
  configDialog->addComboBox(tr("Aux 0 offset"), aux0offset, AuxOffsets, hbox);
  configDialog->addComboBox(tr("Aux 0 mapping"), aux0map, AuxBehaviors, hbox);
  configDialog->addComboBox(tr("Aux 1 source"), aux1src, AuxSrcs, hbox);
  configDialog->addComboBox(tr("Aux 1 offset"), aux1offset, AuxOffsets, hbox);
  configDialog->addComboBox(tr("Aux 1 mapping"), aux1map, AuxBehaviors, hbox);
  
  for (i = 0; i < MODULE_V8SEQUENCER_STATES; i++)
    {      
      hbox = configDialog->addHBox();
      qs = tr ("Out 0 S%1").arg(i);
      configDialog->addSlider (qs, out0[i], -1, 1, false, hbox);
      out0[i] = 0.0;
      qs = tr ("Out 1 S%1").arg(i);
      configDialog->addSlider (qs, out1[i], -1, 1, false, hbox);
      out1[i] = 0.0;
      qs = tr ("Update Out 2");
      configDialog->addCheckBox (qs, sticky2[i], hbox);
      sticky2[i] = 0.0;
      qs = tr ("Out 2 S%1").arg(i);
      configDialog->addSlider (qs, out2[i], -8, 8, false, hbox);
      out2[i] = 0.0;
      qs = tr ("Update Out 3");
      configDialog->addCheckBox (qs, sticky3[i], hbox);
      sticky3[i] = 0.0;
      qs = tr ("Out 3 S%1").arg(i);
      configDialog->addSlider (qs, out3[i], -8, 8, false, hbox);
      out3[i] = 0.0;
    };
}

M_v8sequencer::~M_v8sequencer() {

}

void M_v8sequencer::generateCycle() {

  //   For referencing into the output data array "data[l0][l1][l2]"
  //  int l0 // l0 is the output port number
  int l1;    //  l1 is for cyclesize
  unsigned int l2;  //  l2 is for polyphony 

  //   Note that auxMap must be kept in synch with the combo box above!
  int auxMap[9][8] = {{0,1,2,3,4,5,6,7},
		      {0,1,2,3,0,1,2,3},
		      {0,2,4,5,0,2,4,6},
		      {0,1,0,1,0,1,0,1},
		      {0,2,0,2,0,2,0,2},
		      {0,3,0,3,0,3,0,3},
		      {7,6,5,4,3,2,1,0},
		      {3,2,1,0,3,2,1,0},
		      {6,4,2,0,6,4,2,0}};

  m_step = port_M_step->getinputdata();
  m_direction = port_M_direction->getinputdata();
  m_forward = port_M_forward->getinputdata();
  m_backward = port_M_backward->getinputdata();
  m_seqstate = port_M_seqstate->getinputdata();
  m_seqtrig = port_M_seqtrig->getinputdata();
  m_daisy0 = port_M_daisy0->getinputdata();
  m_daisy1 = port_M_daisy1->getinputdata();
  m_s0 = port_M_s0->getinputdata();
  m_s1 = port_M_s1->getinputdata();
  m_s2 = port_M_s2->getinputdata();
  m_s3 = port_M_s3->getinputdata();
  m_s4 = port_M_s4->getinputdata();
  m_s5 = port_M_s5->getinputdata();
  m_s6 = port_M_s6->getinputdata();
  m_s7 = port_M_s7->getinputdata();


  //    Big loop for cycling thru cyclesize, and thru polyphony
  for (l2 = 0; l2 < synthdata->cyclesize; l2++)
    for (l1 = 0; l1 < synthdata->poly; l1++)
      {

	//    Switch state according to the inputs, in the order given
	//    (first via step/direction, then back/forward, then
	//    seqtrig, then Sn.)
	//
	//    Note that step/direction and back/forward have
	//    hysteresis (i.e. must transition from <=0 to >0 to
	//    trigger a transition, while seqstate is merely
	//    level-sensitive on seqtrig, and the S(n) states are
	//    state-sensitive, and highest state number wins.
	oldstate[l1] = state[l1];

	//   Step and direction.
	if ((oldstep[l1] <= 0 ) && (m_step[l1][l2] > 0))
	  {
	    if (m_direction[l1][l2] < 0)
	      { state[l1]--;} 
	    else 
	      { state[l1]++;};
	  };
	oldstep[l1] = m_step[l1][l2];
	
	//   backward/forward
	if ((oldforward[l1] <= 0.0) && (m_forward[l1][l2] > 0.0) ) 
	  {
	    state[l1]++;
	  };
	oldforward[l1] = m_forward[l1][l2];

	if ( (oldbackward[l1] <= 0.0) && (m_backward[l1][l2] > 0.0))
	  {
	    state[l1]--;
	  };
	oldbackward[l1] = m_backward[l1][l2];

	//   triggered sequence number load (for slaving sequencers, etc)
	if ( m_seqtrig[l1][l2] > 0) 
	  {
	    if (stateIOFormat == 0)
	      state[l1] = int (m_seqstate[l1][l2] + 0.49);
	    if (stateIOFormat == 1)
	      state[l1] = int ((m_seqstate[l1][l2]*12.0) + 0.49);
	    if (stateIOFormat == 2)
	      state[l1] = int ((m_seqstate[l1][l2]+1.0)*maxstate*0.5);
	    if (stateIOFormat == 3)
	      state[l1] = int ((m_seqstate[l1][l2]+1.0 - 
				(16/(maxstate+.000001))) //daisychain fix
			       *maxstate*0.5);
	  }
	oldseqtrig[l1] = m_seqtrig[l1][l2];  // not really needed.
	
	//    Direct state setting
	if (m_s0[l1][l2] > 0) state[l1] = 0;
	if (m_s1[l1][l2] > 0) state[l1] = 1;
	if (m_s2[l1][l2] > 0) state[l1] = 2;
	if (m_s3[l1][l2] > 0) state[l1] = 3;
	if (m_s4[l1][l2] > 0) state[l1] = 4;
	if (m_s5[l1][l2] > 0) state[l1] = 5;
	if (m_s6[l1][l2] > 0) state[l1] = 6;
	if (m_s7[l1][l2] > 0) state[l1] = 7;	

	//   Handle wraparounds of state
	if (wrapmaxtozero > 0.5 )
	  if (state[l1] >= maxstate) 
	    state[l1] = 0;
	
	if (wrapzerotomax > 0.5)
	  if (state[l1] < 0) 
	    state[l1] = maxstate;
	
	//    Firewall against impossibility...
	if (state[l1] >= maxstate) 
	  state[l1] = maxstate - 1;
	if (state[l1] < 0) state[l1] = 0;
	
	//     State is now set.  Determine our outputs.
	
	//    Data 0...7 are out1, 2, 3, 4, aux1, 2, state, trig 
	//
	//   Deal with daisychaining on D0 and D1
	if (state[l1] > -0.5 && state[l1] < 7.5 )
	  {
	    data[0][l1][l2] = oldout0[l1] = out0[state[l1]];
	    data[1][l1][l2] = oldout1[l1] = out1[state[l1]];
	  }
	else
	  {
	    data[0][l1][l2] = oldout0[l1] = m_daisy0[l1][l2];
	    data[1][l1][l2] = oldout1[l1] = m_daisy1[l1][l2];
	  };
	  
	//    the rest of the outputs are always local (no daisychain)
	if (sticky2[state[l1]]) 
	  {
	    data[2][l1][l2] = oldout2[l1] = out2[state[l1]%8];
	  }
	else
	  data[2][l1][l2] = oldout2[l1];
	if (sticky3[state[l1]]) 
	  {
	    data[3][l1][l2] = oldout3[l1] = out3[state[l1]%8];
	  }
	else
	  data[3][l1][l2] = oldout3[l1];
	
	//   aux0 and aux1 - 
	int aux0state, aux1state;
	float aux0lcl = 0;
	float aux1lcl = 0;

       	aux0state = (auxMap[aux0map][ state[l1] % 8 ] + aux0offset) % 8;
	switch (aux0src)
	  {
	  case 0:
	    aux0lcl = out0[aux0state];
	    break;
	  case 1:
	    aux0lcl = out1[aux0state];
	    break;
	  case 2:
	    aux0lcl = out2[aux0state];
	    break;
	  case 3:
	    aux0lcl = out3[aux0state];
	    break;
	  };

       	aux1state = (auxMap[aux1map][ state[l1] % 8] + aux1offset) % 8;
	switch (aux1src)
	  {
	  case 0:
	    aux1lcl = out0[aux1state];
	    break;
	  case 1:
	    aux1lcl = out1[aux1state];
	    break;
	  case 2:
	    aux1lcl = out2[aux1state];
	    break;
	  case 3:
	    aux1lcl = out3[aux1state];
	    break;
	  };

	data[4][l1][l2] = aux0lcl;   // aux0
	data[5][l1][l2] = aux1lcl;   // aux1

	//   state and trig - note daisychaining decrement happens on inputs
	    
	//   triggered sequence number load (for slaving sequencers, etc)
	//   case 0 is also the default;
	if (stateIOFormat == 0)
	  data[6][l1][l2] = state[l1];
	if (stateIOFormat == 1)
	  data[6][l1][l2] = state[l1]/12.0;
	if (stateIOFormat == 2)
	  data[6][l1][l2] = (state[l1]*2.0 / maxstate) - 1.0;
	if (stateIOFormat == 3)
	  data[6][l1][l2] = (state[l1]*2.0 / maxstate) - 1.0; 
	//	    - (16/(maxstate+.000001));
	if (stateIOFormat != 3)
	  {
	    data[7][l1][l2] = (state[l1] == oldstate[l1]) ? 0 : 1;
	  }
	else
	  {
	    data[7][l1][l2] = 1;
	  }
	
	//   data[8] thru data[15] are direct state outputs.
	data[8][l1][l2] = (state[l1] == 0) ? 1 : 0;
	data[9][l1][l2] = (state[l1] == 1) ? 1 : 0;
	data[10][l1][l2] = (state[l1] == 2) ? 1 : 0;
	data[11][l1][l2] = (state[l1] == 3) ? 1 : 0;
	data[12][l1][l2] = (state[l1] == 4) ? 1 : 0;
	data[13][l1][l2] = (state[l1] == 5) ? 1 : 0;
	data[14][l1][l2] = (state[l1] == 6) ? 1 : 0;
	data[15][l1][l2] = (state[l1] == 7) ? 1 : 0;
      }
}
