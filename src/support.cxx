#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>

#include "support.h"
#include "debug.h"
#include "gettext.h"
#include "rig_io.h"
#include "rig.h"
#include "dialogs.h"
#include "rigbase.h"
#include "xml_io.h"
#include "ptt.h"

using namespace std;

rigbase *selrig = rigs[0];

extern bool test;
int freqval = 0;

extern void resetWatchDog();

FREQMODE vfoB = {7070000, 0, 0};
FREQMODE vfoA = {14070000, 0, 0};
int transceiver_bw = 0;
int transceiver_mode = 0;

bool localptt = false;
bool remoteptt = false;

char szVfoB[20];

FREQMODE oplist[LISTSIZE];
int  numinlist = 0;
vector<string> rigmodes_;
vector<string> rigbws_;

Cserial RigSerial;
Cserial AuxSerial;
Cserial SepSerial;

bool using_buttons = false;

//=============================================================================
// loop for serial i/o thread
// runs continuously until program is closed
// only accesses the serial port if it has been successfully opened
//=============================================================================

bool bypass_serial_thread_loop = true;
bool run_serial_thread = true;

bool PTT = false;
int  powerlevel = 0;

// the following functions are ONLY CALLED by the serial loop

// read current vfo frequency
void read_vfo()
{
	if (RigSerial.IsOpen() == false) return;
	static long  freq = 0;
		freq = selrig->get_vfoA();
	if (freq != vfoA.freq) {
		vfoA.freq = freq;
		Fl::awake(setFreqDisp);//, (void*)0);
	}
}

// read current signal level
void read_smeter()
{
	if (RigSerial.IsOpen() == false) return;
	int  sig;
		sig = selrig->get_smeter();
	if (sig == -1) return;
	Fl::awake(updateSmeter, (void*)sig);
}

// read power out
void read_power_out()
{
	if (RigSerial.IsOpen() == false) return;
//	if (!selrig->has_power_control) return;
	int sig;
		sig = selrig->get_power_out();
	Fl::awake(updateFwdPwr, (void*)sig);
}

// read swr
void read_swr()
{
	if (RigSerial.IsOpen() == false) return;
//	if (!selrig->has_swr_control) return;
	int sig;
		sig = selrig->get_swr();
	Fl::awake(updateSWR, (void*)sig);
}

// alc
void read_alc()
{
	if (RigSerial.IsOpen() == false) return;
	if (!selrig->has_alc_control) return;
	int sig;
		sig = selrig->get_alc();
	Fl::awake(updateALC, (void*)sig);
}

static bool resetrcv = true;
static bool resetxmt = true;

void * serial_thread_loop(void *d)
{
//  static int  loopcount = 0;
	for(;;) {
		if (!run_serial_thread) break;

		MilliSleep(progStatus.serloop_timing / 2);

		if (bypass_serial_thread_loop) goto serial_bypass_loop;
// rig specific data reads
		if (!PTT) {
			if (resetrcv) {
				Fl::awake(zeroXmtMeters, 0);
				resetrcv = false;
			}
			resetxmt = true;

			pthread_mutex_lock(&mutex_serial);
				read_vfo();
			pthread_mutex_unlock(&mutex_serial);

			MilliSleep(progStatus.serloop_timing / 2);

			pthread_mutex_lock(&mutex_serial);
				read_smeter();
			pthread_mutex_unlock(&mutex_serial);

/*
			switch (loopcount) {
				case 0: read_volume(); loopcount++; break;
				case 1: read_mode(); loopcount++; break;
				case 2: read_bandwidth(); loopcount++; break;
				case 3: read_notch(); loopcount++; break;
				case 4: read_ifshift(); loopcount++; break;
				case 5: read_power_control(); loopcount++; break;
				case 6: read_preamp_att(); loopcount++; break;
				case 7: read_mic_gain(); loopcount++; break;
				case 8: read_squelch(); loopcount++; break;
				case 9: read_rfgain(); loopcount = 0;
			}
*/
		} else {
			if (resetxmt) {
				Fl::awake(updateSmeter, (void *)(0));
				resetxmt = false;
			}
			resetrcv = true;

			pthread_mutex_lock(&mutex_serial);
				read_power_out();
			pthread_mutex_unlock(&mutex_serial);

			MilliSleep(progStatus.serloop_timing / 2);

			pthread_mutex_lock(&mutex_serial);
				read_swr();
				read_alc();
			pthread_mutex_unlock(&mutex_serial);

		}
serial_bypass_loop: ;
	}
	return NULL;
}

//=============================================================================
static bool nofocus = false;

void setFocus()
{
	if (nofocus) return;
	if (Fl::focus() != FreqDisp)
		Fl::focus(FreqDisp);
}

void setBW()
{
//	wait_query = true;
	pthread_mutex_lock(&mutex_serial);
		vfoA.iBW = opBW->index();
		selrig->set_bandwidth(vfoA.iBW);
	pthread_mutex_unlock(&mutex_serial);
	send_bandwidth_changed();
//	wait_query = false;
}

bool bws_changed = false;
const char **old_bws = NULL;

void updateBandwidthControl()
{
	bws_changed = false;
	if (selrig->has_bandwidth_control) {
		int newbw = selrig->adjust_bandwidth(vfoA.imode);
		if (newbw != -1) {
			if (old_bws != selrig->bandwidths_) {
				bws_changed = true;
				old_bws = selrig->bandwidths_;
				opBW->clear();
				rigbws_.clear();
				for (int i = 0; selrig->bandwidths_[i] != NULL; i++) {
					rigbws_.push_back(selrig->bandwidths_[i]);
					opBW->add(selrig->bandwidths_[i]);
				}
			}
			opBW->index(newbw);
			vfoA.iBW = newbw;
		}
	}
}

void setMode()
{
//	wait_query = true;
	pthread_mutex_lock(&mutex_serial);
		if (selrig->restore_mbw)
			selrig->set_bandwidth(selrig->last_bw);
		vfoA.imode = opMODE->index();
		selrig->set_mode(vfoA.imode);
		updateBandwidthControl();
		if (selrig->restore_mbw) {
			vfoA.iBW = selrig->last_bw = selrig->get_bandwidth();
			opBW->index(vfoA.iBW);
		} else
			selrig->set_bandwidth(vfoA.iBW);
	pthread_mutex_unlock(&mutex_serial);

	send_mode_changed();
	send_sideband();
	if (bws_changed) send_bandwidths();
	send_bandwidth_changed();
//	wait_query = false;
}

void sortList() {
	if (!numinlist) return;
	FREQMODE temp;
	for (int i = 0; i < numinlist - 1; i++)
		for (int j = i + 1; j < numinlist; j++)
			if (oplist[i].freq > oplist[j].freq) {
					temp = oplist[i];
					oplist[i] = oplist[j];
					oplist[j] = temp;
			}
}

void clearList() {
	if (!numinlist) return;
	for (int i = 0; i < LISTSIZE; i++) {
		oplist[i].freq = 0;
		oplist[i].imode = USB;
		oplist[i].iBW = 0;
	}
	FreqSelect->clear();
	numinlist = 0;
}

void updateSelect() {
	char szFREQMODE[20];
	if (!numinlist) return;
	sortList();
	FreqSelect->clear();
	for (int n = 0; n < numinlist; n++) {
		snprintf(szFREQMODE, sizeof(szFREQMODE),
			"%13.3f%7s", oplist[n].freq / 1000.0,
			selrig->get_modename_(oplist[n].imode));
		FreqSelect->add (szFREQMODE);
	}
}

void addtoList(int val, int imode, int iBW) {
	if (numinlist < LISTSIZE) {
		oplist[numinlist].imode = imode;
		oplist[numinlist].freq = val;
		oplist[numinlist++].iBW = iBW;
	}
}

void readFile() {
	ifstream iList(defFileName.c_str());
	if (!iList) {
		fl_message ("Could not open %s", defFileName.c_str());
		return;
	}
	clearList();
	int i = 0, mode, bw;
	long freq;
	while (!iList.eof()) {
		freq = 0L; mode = -1;
		iList >> freq >> mode >> bw;
		if (freq && (mode > -1)) {
			oplist[i].freq = freq;
			oplist[i].imode = mode;
			oplist[i].iBW = (bw == -1 ? 0 : bw);
			i++;
		}
	}
	iList.close();
	numinlist = i;
	updateSelect();
}

void buildlist() {
	defFileName = RigHomeDir;
	defFileName.append(selrig->name_);
	defFileName.append(".arv");
	FILE *fh = fopen(defFileName.c_str(), "r");
	if (fh != NULL) {
		fclose (fh);
		readFile();
		return;
	}
	clearList();
}

int movFreq() {
	pthread_mutex_lock(&mutex_serial);
		vfoA.freq = FreqDisp->value();
		selrig->set_vfoA(vfoA.freq);
	pthread_mutex_unlock(&mutex_serial);
	send_new_freq();
	return 1;
}

void cbABactive()
{
	if (!vfoB.freq) return;
	static  FREQMODE temp;

	wait_query = true;

	pthread_mutex_lock(&mutex_serial);
		temp = vfoA;
		vfoA = vfoB;
		vfoB = temp;

		FreqDisp->value(vfoA.freq);

		snprintf(szVfoB, sizeof(szVfoB), "%13.3f", vfoB.freq / 1000.0);
		txtInactive->label(szVfoB);
		txtInactive->redraw_label();

		selrig->set_vfoA(vfoA.freq);
		if (vfoA.imode != vfoB.imode) {
			opMODE->index(vfoA.imode);
			selrig->set_mode(vfoA.imode);
			updateBandwidthControl();
		}
		if (vfoA.iBW != vfoB.iBW) {
			opBW->index(vfoA.iBW);
			selrig->set_bandwidth(vfoA.iBW);
		}
	pthread_mutex_unlock(&mutex_serial);

	send_new_freq();
	if (vfoA.imode != vfoB.imode)
		send_mode_changed();
		send_sideband();
	if (bws_changed) {
		send_bandwidths();
		send_bandwidth_changed();
	}

	wait_query = false;

}

void cbA2B()
{
	if (Fl::event_button() == FL_RIGHT_MOUSE) {
		cbABactive();
		return;
	}
	vfoB.freq = FreqDisp->value();
	vfoB.imode = opMODE->index();
	vfoB.iBW = opBW->index();
	snprintf(szVfoB, sizeof(szVfoB), "%13.3f", vfoB.freq / 1000.0);
	txtInactive->label(szVfoB);
	txtInactive->redraw_label();
}

void setLower()
{
}

void setUpper()
{
}

void selectFreq() {
	long n = FreqSelect->value();
	if (!n) return;

	n--;
	pthread_mutex_lock(&mutex_serial);
		wait_query = true;

		vfoA.freq  = oplist[n].freq;
		vfoA.imode = oplist[n].imode;
		vfoA.iBW   = oplist[n].iBW;

		FreqDisp->value(vfoA.freq);
		selrig->set_vfoA(vfoA.freq);

		opMODE->index(vfoA.imode);
		selrig->set_mode(vfoA.imode);

		updateBandwidthControl();
		opBW->index(vfoA.iBW);
		selrig->set_bandwidth(vfoA.iBW);

		send_new_freq();
		send_mode_changed();
		if (bws_changed) send_bandwidths();
		send_bandwidth_changed();
		send_sideband();
		wait_query = false;

	pthread_mutex_unlock(&mutex_serial);
}

void delFreq() {
	if (FreqSelect->value()) {
		long n = FreqSelect->value() - 1;
		for (int i = n; i < numinlist; i ++)
			oplist[i] = oplist[i+1];
		oplist[numinlist - 1].imode = USB;
		oplist[numinlist - 1].freq = 0;
		oplist[numinlist - 1].iBW = 0;
		numinlist--;
		updateSelect();
	}
}

void addFreq() {
	long freq = FreqDisp->value();
	if (!freq) return;
	int mode = opMODE->index();
	int bw = opBW->index();
	for (int n = 0; n < numinlist; n++)
		if (freq == oplist[n].freq && mode == oplist[n].imode) {
			oplist[n].iBW = bw;
			return;
		}
	addtoList(freq, mode, bw);
	updateSelect();
	FreqDisp->visual_beep();
}

void cbRIT()
{
}

void cbXIT()
{
}

void cbBFO()
{
}

void cbAttenuator()
{
	pthread_mutex_lock(&mutex_serial);
		selrig->set_attenuator(btnAttenuator->value());
	pthread_mutex_unlock(&mutex_serial);
}

void setAttControl(void *d)
{
	int val = (long)d;
	btnAttenuator->value(val);
}

void cbPreamp()
{
	pthread_mutex_lock(&mutex_serial);
		selrig->set_preamp(btnPreamp->value());
	pthread_mutex_unlock(&mutex_serial);
}

void setPreampControl(void *d)
{
	int val = (long)d;
	btnPreamp->value(val);
}

void cbNoise()
{
	pthread_mutex_lock(&mutex_serial);
		selrig->set_noise(btnNOISE->value());
	pthread_mutex_unlock(&mutex_serial);
}

void cbNR()
{
	pthread_mutex_lock(&mutex_serial);
		selrig->set_noise_reduction(btnNR->value());
	pthread_mutex_unlock(&mutex_serial);
}

void setNR()
{
	pthread_mutex_lock(&mutex_serial);
		selrig->set_noise_reduction_val(sldrNR->value());
	pthread_mutex_unlock(&mutex_serial);
}

void setNoiseControl(void *d)
{
	btnNOISE->value((long)d);
}

void cbbtnNotch()
{
	pthread_mutex_lock(&mutex_serial);
	if (btnNotch->value() == 0) {
		selrig->set_notch(false, 0);
	} else {
		selrig->set_notch(true, sldrNOTCH->value());
	}
	pthread_mutex_unlock(&mutex_serial);
}

void setNotchButton(void *d)
{
	btnNotch->value((bool)d);
}

void setNotchControl(void *d)
{
	int val = (long)d;
	if (sldrNOTCH->value() != val) sldrNOTCH->value(val);
}

void setNotch()
{
	if (btnNotch->value()) {
		pthread_mutex_lock(&mutex_serial);
			selrig->set_notch(true, sldrNOTCH->value());
		pthread_mutex_unlock(&mutex_serial);
	}
}

void setIFshiftButton(void *d)
{
	bool b = (bool)d;
	if (b && !btnIFsh->value()) btnIFsh->value(1);
	else if (!b && btnIFsh->value()) btnIFsh->value(0);
}

void setIFshiftControl(void *d)
{
	int val = (long)d;
	if (sldrIFSHIFT->value() != val) sldrIFSHIFT->value(val);
}

void setIFshift()
{
	pthread_mutex_lock(&mutex_serial);
		selrig->set_if_shift(sldrIFSHIFT->value());
	pthread_mutex_unlock(&mutex_serial);
}

void cbIFsh()
{
	if (btnIFsh->value() == 1) {
		sldrIFSHIFT->value(0);
		sldrIFSHIFT->activate();
	} else {
		sldrIFSHIFT->value(0);
		sldrIFSHIFT->deactivate();
	}
	setIFshift();
}

void cbEventLog()
{
	debug::show();
}

void setVolume()
{
	pthread_mutex_lock(&mutex_serial);
		selrig->set_volume_control(sldrVOLUME->value());
		progStatus.volume = sldrVOLUME->value();
	pthread_mutex_unlock(&mutex_serial);
}

void cbMute()
{
	if (btnVol->value() == 0) {
		pthread_mutex_lock(&mutex_serial);
			sldrVOLUME->deactivate();
			selrig->set_volume_control(0.0);
		pthread_mutex_unlock(&mutex_serial);
	} else {
		pthread_mutex_lock(&mutex_serial);
			sldrVOLUME->activate();
			selrig->set_volume_control(progStatus.volume);
		pthread_mutex_unlock(&mutex_serial);
	}
}

void setMicGain()
{
	pthread_mutex_lock(&mutex_serial);
		selrig->set_mic_gain(sldrMICGAIN->value());
	pthread_mutex_unlock(&mutex_serial);
}

void cbbtnMicLine()
{
	pthread_mutex_lock(&mutex_serial);
		selrig->set_mic_line(btnMicLine->value());
	pthread_mutex_unlock(&mutex_serial);
}

void setMicGainControl(void* d)
{
	sldrMICGAIN->value((long)d);
}

void set_power_controlImage(double pwr)
{
	if (pwr < 26.0) {
		scalePower->image(image_p25);
		sldrFwdPwr->maximum(25.0);
		sldrFwdPwr->minimum(0.0);
	}
	else if (pwr < 51.0) {
		scalePower->image(image_p50);
		sldrFwdPwr->maximum(50.0);
		sldrFwdPwr->minimum(0.0);
	}
	else if (pwr < 101.0) {
		scalePower->image(image_p100);
		sldrFwdPwr->maximum(100.0);
		sldrFwdPwr->minimum(0.0);
	}
	else {
		scalePower->image(image_p200);
		sldrFwdPwr->maximum(200.0);
		sldrFwdPwr->minimum(0.0);
	}
	scalePower->redraw();
	return;
}

void setPower()
{
	double pwr = sldrPOWER->value();
	pthread_mutex_lock(&mutex_serial);
		powerlevel = (int)pwr;
		selrig->set_power_control(pwr);
	pthread_mutex_unlock(&mutex_serial);
}

void reset_power_controlImage( void *d )
{
	int val = (long)d;
	sldrPOWER->value(val);
}

void cbTune()
{
	pthread_mutex_lock(&mutex_serial);
		selrig->tune_rig();
	pthread_mutex_unlock(&mutex_serial);
}

void cbPTT()
{
	if (btnPTT->value() == 1) {
		PTT = true;
		localptt = true;
	} else {
		PTT = false;
		localptt = false;
	}

	if (localptt) {
		wait_query = true;
		rigPTT(PTT);
		wait_query - false;
	} else {
		wait_query = true;
		send_ptt_changed(PTT);
		rigPTT(PTT);
		wait_query = false;
	}

}

void setSQUELCH()
{
	pthread_mutex_lock(&mutex_serial);
		selrig->set_squelch((int)sldrSQUELCH->value());
	pthread_mutex_unlock(&mutex_serial);
}

void setRFGAIN()
{
	pthread_mutex_lock(&mutex_serial);
		selrig->set_rf_gain((int)sldrRFGAIN->value());
	pthread_mutex_unlock(&mutex_serial);
}


void updateALC(void * d)
{
	double data = (long)d;
	Fl_Image *img = btnALC_SWR->image();
	if (img == &image_alc) {
		sldrALC_SWR->value(data);
		sldrALC_SWR->redraw();
	}
}

void updateSWR(void * d)
{
	double data = (long)d;
	Fl_Image *img = btnALC_SWR->image();
	if (img == &image_swr) {
		sldrALC_SWR->value(data);
		sldrALC_SWR->redraw();
	}
}

float fp_ = 0.0, rp_ = 0.0;

void updateFwdPwr(void *d)
{
	double power = (long)d;
	if (!sldrFwdPwr->visible()) {
		sldrRcvSignal->hide();
		sldrFwdPwr->show();
	}
	sldrFwdPwr->value(power);
	sldrFwdPwr->redraw();
}

void updateSquelch(void *d)
{
	sldrSQUELCH->value((long)d);
	sldrSQUELCH->redraw();
}

void updateRFgain(void *d)
{
	sldrRFGAIN->value((long)d);
	sldrRFGAIN->redraw();
}

void zeroXmtMeters(void *d)
{
	sldrFwdPwr->aging(1);
	updateFwdPwr(0);
	updateALC(0);
	updateSWR(0);
	sldrFwdPwr->aging(5);
}

void setFreqDisp(void *d)
{
	FreqDisp->value(vfoA.freq);
	FreqDisp->redraw();
	send_new_freq();
}

void updateSmeter(void *d) // 0 to 100;
{
	double swr = (long)d;
	if (!sldrRcvSignal->visible()) {
		sldrFwdPwr->hide();
		sldrRcvSignal->show();
	}
	sldrRcvSignal->value(swr);
	sldrRcvSignal->redraw();
}

void saveFreqList()
{
	if (!numinlist) {
		remove(defFileName.c_str());
		return;
	}
	ofstream oList(defFileName.c_str());
	if (!oList) {
		fl_message ("Could not write to %s", defFileName.c_str());
		return;
	}
	for (int i = 0; i < numinlist; i++)
		oList << oplist[i].freq << " " << oplist[i].imode << " " << oplist[i].iBW << endl;
	oList.close();
}

void setPTT( void *d)
{
	int val = (long)d;
	PTT = d;
	btnPTT->value(val);
	rigPTT(val);
}

void cbExit()
{
	pthread_mutex_lock(&mutex_serial);
		if (selrig->has_mode_control)
			selrig->set_mode(transceiver_mode);
		if (selrig->has_bandwidth_control) {
			updateBandwidthControl();
			selrig->set_bandwidth(transceiver_bw);
		}
	pthread_mutex_unlock(&mutex_serial);

	run_serial_thread = false;
	pthread_join(*serial_thread, NULL);
	RigSerial.ClosePort();

	progStatus.rig_nbr = rig_nbr;

	progStatus.freq = vfoA.freq;
	progStatus.opMODE = vfoA.imode;
	progStatus.opBW = vfoA.iBW;

	progStatus.freq_B = vfoB.freq;
	progStatus.imode_B = vfoB.imode;
	progStatus.iBW_B = vfoB.iBW;

	progStatus.mute = btnVol->value();
	progStatus.volume = sldrVOLUME->value();
	progStatus.power_level = sldrPOWER->value();
	progStatus.mic_gain = sldrMICGAIN->value();
	progStatus.notch = btnNotch->value();
	progStatus.notch_val = sldrNOTCH->value();
	progStatus.shift = btnIFsh->value();
	progStatus.shift_val = sldrIFSHIFT->value();
	progStatus.noise_reduction = btnNR->value();
	progStatus.noise_reduction_val = sldrNR->value();
	progStatus.noise = btnNOISE->value();
	progStatus.attenuator = btnAttenuator->value();
	progStatus.preamp = btnPreamp->value();

	progStatus.saveLastState();

	saveFreqList();

	if (dlgDisplayConfig && dlgDisplayConfig->visible())
		dlgDisplayConfig->hide();
	if (dlgXcvrConfig && dlgXcvrConfig->visible())
		dlgXcvrConfig->hide();
	if (dlgMemoryDialog && dlgMemoryDialog->visible())
		dlgMemoryDialog->hide();
	debug::stop();

	exit(0);
}

void cbALC_SWR()
{
	Fl_Image *img = btnALC_SWR->image();
	if (img == &image_swr)
		btnALC_SWR->image(image_alc);
	else
		btnALC_SWR->image(image_swr);
	btnALC_SWR->redraw();
}

void about()
{
	string msg = "\
%s\n\
Version %s\n\
copyright W1HKJ, 2009\n\
w1hkj@@w1hkj.com";
	fl_message(msg.c_str(), PACKAGE_TARNAME, PACKAGE_VERSION);
}

void adjust_control_positions()
{
	int y = 118;
	if (selrig->has_rit || selrig->has_xit || selrig->has_bfo) {
		y += 20;
		cntRIT->position( cntRIT->x(), y );
		cntXIT->position( cntXIT->x(), y );
		cntBFO->position( cntBFO->x(), y );
		cntRIT->redraw();
		cntXIT->redraw();
		cntBFO->redraw();
	}
	if (selrig->has_volume_control) {
		y += 20;
		sldrVOLUME->position( sldrVOLUME->x(), y );
		btnVol->position( btnVol->x(), y);
		sldrVOLUME->redraw();
		btnVol->redraw();
	}
	if (selrig->has_rf_control) {
		y += 20;
		sldrRFGAIN->position( sldrRFGAIN->x(), y );
		sldrRFGAIN->redraw();
	}
	if (selrig->has_sql_control) {
		y += 20;
		sldrSQUELCH->position( sldrSQUELCH->x(), y);
		sldrSQUELCH->redraw();
	}
	if (selrig->has_noise_reduction_control) {
		y += 20;
		sldrNR->position( sldrNR->x(), y);
		btnNR->position( btnNR->x(), y);
		sldrNR->redraw();
		btnNR->show();
	}
	if (selrig->has_ifshift_control) {
		y += 20;
		sldrIFSHIFT->position( sldrIFSHIFT->x(), y);
		btnIFsh->position( btnIFsh->x(), y);
		sldrIFSHIFT->redraw();
		btnIFsh->show();
	}
	if (selrig->has_notch_control) {
		y += 20;
		sldrNOTCH->position( sldrNOTCH->x(), y);
		btnNotch->position( btnNotch->x(), y);
		sldrNOTCH->redraw();
		btnNotch->show();
	}
	if (selrig->has_micgain_control) {
		y += 20;
		sldrMICGAIN->position( sldrMICGAIN->x(), y);
		sldrMICGAIN->redraw();
	}
	if (selrig->has_power_control) {
		y += 20;
		sldrPOWER->position( sldrPOWER->x(), y);
		sldrPOWER->redraw();
	}
	y += 20;
	btnAttenuator->position( btnAttenuator->x(), y);
	btnAttenuator->redraw();
	btnPreamp->position( btnPreamp->x(), y);
	btnPreamp->redraw();
	btnNOISE->position( btnNOISE->x(), y);
	btnNOISE->redraw();
	btnTune->position( btnTune->x(), y);
	btnTune->redraw();

	int use_AuxPort = (progStatus.aux_serial_port != "NONE");
	if (use_AuxPort) {
		btnPTT->resize(btnPTT->x(), y, btnPTT->w(), 38);
		btnPTT->redraw();
		y += 20;
		boxControl->position(boxControl->x(), y);
		btnAuxRTS->position(btnAuxRTS->x(), y);
		btnAuxDTR->position(btnAuxDTR->x(), y);
		btnAuxRTS->value(progStatus.aux_rts);
		btnAuxDTR->value(progStatus.aux_dtr);
		boxControl->show();
		btnAuxRTS->show();
		btnAuxDTR->show();
	} else {
		boxControl->hide();
		btnAuxRTS->hide();
		btnAuxDTR->hide();
		btnPTT->resize(btnPTT->x(), y, btnPTT->w(), 18);
		btnPTT->redraw();
	}
	mainwindow->size( mainwindow->w(), y + 20);
	mainwindow->redraw();

	if (progStatus.tooltips) {
		Fl_Tooltip::enable(1);
		mnuTooltips->set();
	} else {
		mnuTooltips->clear();
		Fl_Tooltip::enable(0);
	}

}

void initXcvrTab()
{
	if (selrig->has_line_out) cnt_line_out->activate(); else cnt_line_out->deactivate();
	if (selrig->has_agc_level) cbo_agc_level->activate(); else cbo_agc_level->deactivate();
	if (selrig->has_cw_wpm) cnt_cw_wpm->activate(); else cnt_cw_wpm->deactivate();
	if (selrig->has_cw_vol) cnt_cw_vol->activate(); else cnt_cw_vol->deactivate();
	if (selrig->has_cw_spot) cnt_cw_spot->activate(); else cnt_cw_spot->deactivate();
	if (selrig->has_vox_onoff) btn_vox->activate(); else btn_vox->deactivate();
	if (selrig->has_vox_gain) cnt_vox_gain->activate(); else cnt_vox_gain->deactivate();
	if (selrig->has_vox_anti) cnt_anti_vox->activate(); else cnt_anti_vox->deactivate();
	if (selrig->has_vox_hang) cnt_vox_hang->activate(); else cnt_vox_hang->deactivate();
	if (selrig->has_compression) cnt_compression->activate(); else cnt_compression->deactivate();
}

void initRig()
{
	pthread_mutex_lock(&mutex_serial);
	selrig->initialize();
	if (selrig->has_mode_control)
		transceiver_mode = selrig->get_mode();
	if (selrig->has_bandwidth_control) {
		transceiver_bw = selrig->get_bandwidth();
		selrig->last_bw = transceiver_bw;
	}

	rigmodes_.clear();
	opMODE->clear();
	if (selrig->has_mode_control) {
		for (int i = 0; selrig->modes_[i] != NULL; i++) {
			rigmodes_.push_back(selrig->modes_[i]);
			opMODE->add(selrig->modes_[i]);
		}
		opMODE->activate();
		opMODE->index(progStatus.opMODE);
		selrig->set_mode(progStatus.opMODE);
		updateBandwidthControl();
	} else {
		opMODE->add(" ");
		opMODE->index(0);
		opMODE->deactivate();
	}

	rigbws_.clear();
	opBW->clear();
	if (selrig->has_bandwidth_control) {
		selrig->adjust_bandwidth(vfoA.imode);
		old_bws = selrig->bandwidths_;
		for (int i = 0; selrig->bandwidths_[i] != NULL; i++) {
			rigbws_.push_back(selrig->bandwidths_[i]);
				opBW->add(selrig->bandwidths_[i]);
			}
		opBW->activate();
		opBW->index(progStatus.opBW);
		selrig->set_bandwidth(progStatus.opBW);
	} else {
		opBW->add(" ");
		opBW->index(0);
		opBW->deactivate();
	}

	if (selrig->has_rit) {
		cntRIT->activate();
		cntRIT->show();
	} else {
		cntRIT->deactivate();
		cntRIT->hide();
	}

	if (selrig->has_xit) {
		cntXIT->activate();
		cntXIT->show();
	} else {
		cntXIT->deactivate();
		cntXIT->hide();
	}

	if (selrig->has_bfo) {
		cntBFO->activate();
		cntBFO->show();
	} else {
		cntBFO->deactivate();
		cntBFO->hide();
	}
	
	if (selrig->has_volume_control) {
		sldrVOLUME->value(progStatus.volume);
		if (progStatus.mute == 0) {
			btnVol->value(0);
			sldrVOLUME->deactivate();
			selrig->set_volume_control(0);
		} else {
			btnVol->value(1);
			sldrVOLUME->activate();
			selrig->set_volume_control(progStatus.volume);
		}
		btnVol->show();
		sldrVOLUME->show();
	} else {
		btnVol->hide();
		sldrVOLUME->hide();
	}

	if (selrig->has_rf_control) {
		sldrRFGAIN->value(
			progStatus.rfgain = selrig->get_rf_gain());
		sldrRFGAIN->show();
	} else {
		sldrRFGAIN->hide();
	}

	if (selrig->has_sql_control) {
		sldrSQUELCH->value(
			progStatus.squelch = selrig->get_squelch());
		sldrSQUELCH->show();
	} else {
		sldrSQUELCH->hide();
	}

	if (selrig->has_noise_reduction_control) {
		btnNR->show();
		btnNR->value(progStatus.noise_reduction);
		sldrNR->show();
		sldrNR->value(progStatus.noise_reduction_val);
	} else {
		btnNR->hide();
		sldrNR->hide();
	}

	if (selrig->has_ifshift_control) {
		int min, max, step;
		selrig->get_if_min_max_step(min, max, step);
		sldrIFSHIFT->minimum(min);
		sldrIFSHIFT->maximum(max);
		sldrIFSHIFT->step(step);
		if (progStatus.shift) {
			btnIFsh->value(1);
			sldrIFSHIFT->value(progStatus.shift_val);
			selrig->set_if_shift(progStatus.shift_val);
		} else {
			btnIFsh->value(0);
			sldrIFSHIFT->value(0);
			sldrIFSHIFT->deactivate();
			selrig->set_if_shift(0);
		}
		btnIFsh->show();
		sldrIFSHIFT->show();
	} else {
		btnIFsh->hide();
		sldrIFSHIFT->hide();
	}

	if (selrig->has_notch_control) {
		int min, max, step;
		selrig->get_notch_min_max_step(min, max, step);
		sldrNOTCH->minimum(min);
		sldrNOTCH->maximum(max);
		sldrNOTCH->step(step);
		btnNotch->value(progStatus.notch);
		sldrNOTCH->value(progStatus.notch_val);
		selrig->set_notch(progStatus.notch, progStatus.notch_val);
		btnNotch->show();
		sldrNOTCH->show();
	} else {
		btnNotch->hide();
		sldrNOTCH->hide();
	}

	if (selrig->has_micgain_control) {
		int min, max, step;
		selrig->get_mic_min_max_step(min, max, step);
		sldrMICGAIN->minimum(min);
		sldrMICGAIN->maximum(max);
		sldrMICGAIN->step(step);
		sldrMICGAIN->value(progStatus.mic_gain);
		selrig->set_mic_gain(progStatus.mic_gain);
		sldrMICGAIN->show();
	} else {
		sldrMICGAIN->hide();
	}

	if (selrig->has_power_control) {
		selrig->set_power_control(progStatus.power_level);
		sldrPOWER->value(progStatus.power_level);
		sldrPOWER->show();
	} else {
		sldrPOWER->hide();
	}
	set_power_controlImage(selrig->max_power);

	if (selrig->has_attenuator_control) {
		btnAttenuator->label("Att");
		progStatus.attenuator = selrig->get_attenuator();
		btnAttenuator->value(progStatus.attenuator);
		btnAttenuator->show();
	} else {
		btnAttenuator->hide();
	}

	if (selrig->has_preamp_control) {
		btnPreamp->label("Pre");
		progStatus.preamp = selrig->get_preamp();
		btnPreamp->value(progStatus.preamp);
		btnPreamp->show();
	} else {
		btnPreamp->hide();
	}

	if (selrig->has_noise_control) {
		btnNOISE->value(progStatus.noise);
		selrig->set_noise(progStatus.noise);
		btnNOISE->show();
	}
	else {
		btnNOISE->hide();
	}

	if (selrig->has_tune_control) {
		btnTune->show();
	} else {
		btnTune->hide();
	}

	if (selrig->has_ptt_control) {
		btnPTT->show();
	} else {
		btnPTT->hide();
	}

	if (selrig->has_swr_control)
		btnALC_SWR->activate();
	else {
		btnALC_SWR->deactivate();
	}

	if (selrig->has_compON || selrig->has_compression)
		selrig->set_compression();

	vfoB.freq  = vfoA.freq  = selrig->deffreq_;
	vfoB.imode = vfoA.imode = selrig->def_mode;
	vfoB.iBW   = vfoA.iBW   = selrig->defbw_;

	FreqDisp->value( vfoA.freq );
	opMODE->index( vfoA.imode  );
	opBW->index( vfoA.iBW );

	snprintf(szVfoB, sizeof(szVfoB), "%13.3f", vfoB.freq / 1000.0);
	txtInactive->label(szVfoB);
	txtInactive->redraw();

	adjust_control_positions();
	initXcvrTab();
	pthread_mutex_unlock(&mutex_serial);

	buildlist();

}

void init_title()
{
	title = PACKAGE_STRING;
	title += " ";
	title.append(selrig->name_);
	mainwindow->label(title.c_str());
}

void initConfigDialog()
{
	selectCommPort->index(0);
	rigbase *selrig = rigs[selectRig->index()];

	progStatus.loadXcvrState(selrig->name_);

	selectCommPort->value(progStatus.xcvr_serial_port.c_str());
	btnOneStopBit->value( progStatus.stopbits == 1 );
	btnTwoStopBit->value( progStatus.stopbits == 2 );

	mnuBaudrate->index( selrig->comm_baudrate );
	btnOneStopBit->value( selrig->stopbits == 1 );
	btnTwoStopBit->value( selrig->stopbits == 2 );
	cntRigCatRetries->value( selrig->comm_retries );
	cntRigCatTimeout->value( selrig->comm_timeout );
	cntRigCatWait->value( selrig->comm_wait );
	btnRigCatEcho->value( selrig->comm_echo );
	btncatptt->value( selrig->comm_catptt );
	btnrtsptt->value( selrig->comm_rtsptt );
	btndtrptt->value( selrig->comm_dtrptt );
	chkrtscts->value( selrig->comm_rtscts );
	btnrtsplus->value( selrig->comm_rtsplus );
	btndtrplus->value( selrig->comm_dtrplus );
}

void initStatusConfigDialog()
{
	rig_nbr = progStatus.rig_nbr;
	selrig = rigs[rig_nbr];

	selectRig->index(rig_nbr);
	mnuBaudrate->index( progStatus.comm_baudrate );

	selectCommPort->value( progStatus.xcvr_serial_port.c_str() );
	selectAuxPort->value( progStatus.aux_serial_port.c_str() );
	selectSepPTTPort->value( progStatus.sep_serial_port.c_str() );
	btnOneStopBit->value( progStatus.stopbits == 1 );
	btnTwoStopBit->value( progStatus.stopbits == 2 );

	cntRigCatRetries->value( progStatus.comm_retries );
	cntRigCatTimeout->value( progStatus.comm_timeout );
	cntRigCatWait->value( progStatus.comm_wait );
	btnRigCatEcho->value( progStatus.comm_echo );

	btncatptt->value( progStatus.comm_catptt );
	btnrtsptt->value( progStatus.comm_rtsptt );
	btndtrptt->value( progStatus.comm_dtrptt );
	chkrtscts->value( progStatus.comm_rtscts );
	btnrtsplus->value( progStatus.comm_rtsplus );
	btndtrplus->value( progStatus.comm_dtrplus );

	btnSepDTRplus->value(progStatus.sep_dtrplus);
	btnSepDTRptt->value(progStatus.sep_dtrptt);
	btnSepRTSplus->value(progStatus.sep_rtsplus);
	btnSepRTSptt->value(progStatus.sep_rtsptt);

	init_title();

	if (!startXcvrSerial()) {
		if (progStatus.xcvr_serial_port.compare("NONE") == 0) {
			LOG_WARN("No comm port ... test mode");
		} else {
			LOG_WARN("%s cannot be accessed", progStatus.xcvr_serial_port.c_str());
			progStatus.xcvr_serial_port = "NONE";
			selectCommPort->value(progStatus.xcvr_serial_port.c_str());
		}
	} else {
		selectCommPort->value(progStatus.xcvr_serial_port.c_str());
	}
	if (!startAuxSerial()) {
		if (progStatus.aux_serial_port.compare("NONE") == 0) {
			LOG_WARN("Aux port not selected");
		} else {
			LOG_WARN("%s cannot be accessed", progStatus.aux_serial_port.c_str());
			progStatus.aux_serial_port = "NONE";
			selectAuxPort->value(progStatus.aux_serial_port.c_str());
		}
	}
	if (!startSepSerial()) {
		if (progStatus.sep_serial_port.compare("NONE") != 0) {
			progStatus.sep_serial_port = "NONE";
			selectSepPTTPort->value(progStatus.sep_serial_port.c_str());
		}
	}

	initRig();

	wait_query = true;

	FreqDisp->value( vfoA.freq = progStatus.freq );

	pthread_mutex_lock(&mutex_serial);
		selrig->set_vfoA(progStatus.freq);
		opMODE->index( vfoA.imode = progStatus.opMODE );
		selrig->set_mode(progStatus.opMODE);

		updateBandwidthControl();
		opBW->index( vfoA.iBW = progStatus.opBW );
		selrig->set_bandwidth(progStatus.opBW);
	pthread_mutex_unlock(&mutex_serial);

	send_name();
	send_modes();
	send_bandwidths();
	send_mode_changed();
	send_sideband();
	send_bandwidth_changed();
	send_new_freq();

	wait_query = false;

	vfoB.freq = progStatus.freq_B;
	vfoB.imode = progStatus.imode_B;
	vfoB.iBW = progStatus.iBW_B;


	snprintf(szVfoB, sizeof(szVfoB), "%13.3f", vfoB.freq / 1000.0);
	txtInactive->label(szVfoB);
	txtInactive->redraw();

	bypass_serial_thread_loop = false;

}

void initRigCombo()
{
	selectRig->clear();
	int i = 0;
	while (rigs[i] != NULL)
		selectRig->add(rigs[i++]->name_);

	selectRig->index(rig_nbr = 0);
}

void preamp_label(const char * l, bool on = false)
{
	btnPreamp->value(on);
	btnPreamp->label(l);
	btnPreamp->redraw_label();
}

void atten_label(const char * l, bool on = false)
{
	btnAttenuator->value(on);
	btnAttenuator->label(l);
	btnAttenuator->redraw_label();
}

void cbAuxPort()
{
	AuxSerial.setRTS(progStatus.aux_rts);
	AuxSerial.setDTR(progStatus.aux_dtr);
}

void cb_line_out()
{
	selrig->set_line_out();
}

void cb_agc_level()
{
	selrig->set_agc_level();
}

void cb_cw_wpm()
{
	selrig->set_cw_wpm();
}

void cb_cw_vol()
{
	selrig->set_cw_vol();
}

void cb_cw_spot()
{
	selrig->set_cw_spot();
}

void cb_vox_gain()
{
	selrig->set_vox_gain();
}

void cb_vox_anti()
{
	selrig->set_vox_anti();
}

void cb_vox_hang()
{
	selrig->set_vox_hang();
}

void cb_vox_onoff()
{
	selrig->set_vox_onoff();
}

void cb_compression()
{
	selrig->set_compression();
}