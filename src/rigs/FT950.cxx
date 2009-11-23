/*
 * Yaesu FT-950 drivers
 * 
 * a part of flrig
 * 
 * Copyright 2009, Dave Freese, W1HKJ
 * 
 */


#include "FT950.h"
#include "debug.h"
#include "support.h"

static const char FT950name_[] = "FT-950";

static const char *FT950modes_[] = {
"LSB", "USB", "CW", "FM", "AM", "RTTY-L",
"CW-R", "PKT-L", "RTTY-U", "PKT-FM",
"FM-N", "PKT-U", "AM-N", NULL};
static const char FT950_mode_chr[] =  { '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D' };
static const char FT950_mode_type[] = { 'L', 'U', 'U', 'U', 'U', 'L', 'L', 'L', 'U', 'U', 'U', 'U', 'U' };

static const char *FT950_widths_SSB[] = {
"200", "400", "600", "850", "1100", "1350", "1500", "1650", "1800",
"1950", "2100", "2250", "2400", "2450", "2500", "2600", "2700",
"2800", "2900", "3000", NULL };
static int FT950_wvals_SSB[] = {
1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};

static const char *FT950_widths_CW[] = {
"100", "200", "300", "400", "500",
"800", "1200", "1400", "1700", "2000", "2400", NULL };
static int FT950_wvals_CW[] = {
3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };

RIG_FT950::RIG_FT950() {
// base class values	
	name_ = FT950name_;
	modes_ = FT950modes_;
	bandwidths_ = FT950_widths_SSB;
	bw_vals_ = FT950_wvals_SSB;
	comm_baudrate = BR38400;
	stopbits = 1;
	comm_retries = 2;
	comm_wait = 5;
	comm_timeout = 50;
	comm_rtscts = true;
	comm_rtsplus = false;
	comm_dtrplus = false;
	comm_catptt = true;
	comm_rtsptt = false;
	comm_dtrptt = false;
	mode_ = 1;
	bw_ = 2;
	def_mode = 10;
	defbw_ = 2;
	deffreq_ = 14070000;


	has_power_control =
	has_volume_control =
	has_rf_control =
	has_micgain_control =
	has_mode_control =
	has_noise_control =
	has_bandwidth_control =
	has_notch_control =
	has_attenuator_control =
	has_preamp_control =
	has_ifshift_control =
	has_ptt_control =
	has_tune_control =
	has_swr_control = true;
	
// derived specific
	notch_on = false;
	atten_level = 0;
	preamp_level = 0;
}

void RIG_FT950::initialize()
{
	get_preamp();
	get_attenuator();
}


long RIG_FT950::get_vfoA ()
{
	cmd = "FA;";
	if (sendCommand(cmd, 11, false)) {
		int f = 0;
		for (size_t n = 2; n < 10; n++)
			f = f*10 + replybuff[n] - '0';
		freq_ = f;
	}
	return freq_;
}

void RIG_FT950::set_vfoA (long freq)
{
	freq_ = freq;
	cmd = "FA00000000;";
	for (int i = 9; i > 1; i--) {
		cmd[i] += freq % 10;
		freq /= 10;
	}
	sendCommand(cmd, 0, false);
}

int RIG_FT950::get_smeter()
{
	cmd = "SM0;";
	if(sendCommand(cmd, 7, false)) {
		replybuff[6] = 0;
		int mtr = atoi(&replybuff[3]);
		mtr = mtr * 100.0 / 256.0;
		return mtr;
	}
	return 0;
}

int RIG_FT950::get_swr()
{
	cmd = "RM6;";
	if (sendCommand(cmd, 7, false)) {
		replybuff[6] = 0;
		int mtr = atoi(&replybuff[3]);
		return mtr / 2.56;
	}
	return 0;
}

int RIG_FT950::get_power_out()
{
	cmd = "RM5;";
	if (sendCommand(cmd, 7, false)) {
		replybuff[6] = 0;
		double mtr = (double)(atoi(&replybuff[3]));
		mtr = -6.6263535 + .11813178 * mtr + .0013607405 * mtr * mtr;
		return (int)mtr;
	}
	return 0;
}

// Transceiver power level
int RIG_FT950::get_power_control()
{
	cmd = "PC;";
	if (sendCommand(cmd, 6, false)) {
		replybuff[5] = 0;
		int mtr = atoi(&replybuff[2]);
		return mtr;
	}
	return 0;
}

void RIG_FT950::set_power_control(double val)
{
	int ival = (int)val;
	cmd = "PC000;";
	if (val < 5) val = 5;
	if (val > 100) val = 100;
	for (int i = 4; i > 1; i--) {
		cmd[i] += ival % 10;
		ival /= 10;
	}
	sendCommand(cmd, 0, false);
}

// Volume control return 0 ... 100
int RIG_FT950::get_volume_control()
{
	cmd = "AG0;";
	if (sendCommand(cmd, 7, false))  {
		cmd[6] = 0;
		int val = atoi(&replybuff[3]);
		return (int)(val / 2.55);
	}
	return 0;
}

void RIG_FT950::set_volume_control(int val) 
{
	int ivol = (int)(val * 2.55);
	cmd = "AG0000;";
	for (int i = 5; i > 2; i--) {
		cmd[i] += ivol % 10;
		ivol /= 10;
	}
	sendCommand(cmd, 0, false);
}

// Tranceiver PTT on/off
void RIG_FT950::set_PTT_control(int val)
{
	if (val) sendCommand("TX1;", 0, false);
	else	 sendCommand("TX0;", 0, false);
}

void RIG_FT950::tune_rig()
{
	sendCommand("AC002;",0);
}

void RIG_FT950::set_attenuator(int val)
{
	int cmdval = 0;
	if (atten_level == 0) {
		atten_level = 1;
		atten_label("6 dB", false);
		cmdval = 0x06;
	} else if (atten_level == 1) {
		atten_level = 2;
		atten_label("12 dB", false);
		cmdval = 0x12;
	} else if (atten_level == 2) {
		atten_level = 3;
		atten_label("18 dB", false);
		cmdval = 0x18;
	} else if (atten_level == 3) {
		atten_level = 0;
		atten_label("Att", false);
		cmdval = 0x00;
	}
	cmd = "RA0;";
	cmd[2] = '0' + atten_level;
	sendCommand(cmd, 0, false);
}

int RIG_FT950::get_attenuator()
{
	if (sendCommand("RA0;", 5, false)) {
		atten_level = replybuff[3] - '0';
		if (atten_level == 1) {
			atten_label("6 dB", false);
		} else if (atten_level == 2) {
			atten_label("12 dB", false);
		} else if (atten_level == 3) {
			atten_label("18 dB", false);
		} else {
			atten_level = 0;
			atten_label("Att", false);
		}
	}
	return atten_level;
}

void RIG_FT950::set_preamp(int val)
{
	cmd = "PA00;";
	if (preamp_level == 0) {
		preamp_level = 1;
		preamp_label("Pre 1", false);
	} else if (preamp_level == 1) {
		preamp_level = 2;
		preamp_label("Pre 2", false);
	} else if (preamp_level == 2) {
		preamp_level = 0;
		preamp_label("Pre", false);
	}
	cmd[3] = '0' + preamp_level;
	sendCommand (cmd, 0, false);
}

int RIG_FT950::get_preamp()
{
	if (sendCommand("PA0;", 5, false)) {
		return (replybuff[3] == '1' ? 1 : 0);
		preamp_level = replybuff[3] - '0';
		if (preamp_level == 1) {
			preamp_label("Pre 1", false);
		} else if (preamp_level == 2) {
			preamp_label("Pre 2", false);
		} else {
			preamp_label("Pre", false);
			preamp_level = 0;
		}
	}
	return preamp_level;
}


void RIG_FT950::update_bandwidths()
{
	if (mode_ == 2 || mode_ == 5 || mode_ == 6 || mode_ == 8) {
		bandwidths_ = FT950_widths_CW;
		bw_vals_ = FT950_wvals_CW;
		bw_ = 7; // 500 Hz
	} else {
		bandwidths_ = FT950_widths_SSB;
		bw_vals_ = FT950_wvals_SSB;
		bw_ = 13; // 2400 Hz
	}
}

void RIG_FT950::set_mode(int val)
{
	mode_ = val;
	cmd = "MD0";
	cmd += FT950_mode_chr[val];
	cmd += ';';
	sendCommand(cmd, 0, false);
	update_bandwidths();
}

int RIG_FT950::get_mode()
{
	if (sendCommand("MD0;", 5, false))
		mode_ = replybuff[3];
	update_bandwidths();
	return mode_;
}

void RIG_FT950::set_bandwidth(int val)
{
	int bw_indx = bw_vals_[val];
	bw_ = val;
	cmd = "SH0";
	cmd += '0' + bw_indx / 10;
	cmd += '0' + bw_indx % 10;
	cmd += ';';
	sendCommand(cmd, 0, false);
}

int RIG_FT950::get_bandwidth()
{
	int i = 0;
	int limit = sizeof(bw_vals_)/sizeof(int);
	if (sendCommand("SH0;", 6, false)) {
		replybuff[5] = 0;
		int bw_indx = atoi(&replybuff[3]);
		for (i = 0; i < limit; i++)
			if (bw_vals_[i] == bw_indx) break;
	}
	if (i < limit) bw_ = i;
	return bw_;
}

int RIG_FT950::get_modetype(int n)
{
	return FT950_mode_type[n];
}

void RIG_FT950::set_if_shift(int val)
{
	cmd = "IS0+0000;";
	if (val < 0) cmd[3] = '-';
	val = abs(val);
	for (int i = 4; i > 0; i--) {
		cmd[3+i] += val % 10;
		val /= 10;
	}
	sendCommand(cmd, 0, false);
}

bool RIG_FT950::get_if_shift(int &val)
{
	static int oldval = 0;
	sendCommand("IS0;", 9, false);
	replybuff[8] = 0;
	val = atoi(&replybuff[3]);
	if (val != 0 || oldval != val) {
		oldval = val;
		return true;
	}
	oldval = val;
	return false;
}

void RIG_FT950::get_if_min_max_step(int &min, int &max, int &step)
{
	min = -1000;
	max = 1000;
	step = 50;
}

void RIG_FT950::set_notch(bool on, int val)
{
	cmd = "BP00000;";
	if (on == false) {
		sendCommand(cmd, 0, false);
		notch_on = false;
		return;
	}
	if (!notch_on) {
		cmd[6] = '1'; // notch ON
		sendCommand(cmd, 0, false);
		cmd[6] = '0';
		notch_on = true;
	}
	cmd[3] = '1'; // manual NOTCH position
	val /= 10;
	if (val < 1) val = 1;
	if (val > 300) val = 300;
	for (int i = 3; i > 0; i--) {
		cmd[3 + i] += val % 10;
		val /=10;
	}
	sendCommand(cmd, 0, false);
}

bool  RIG_FT950::get_notch(int &val)
{
	bool ison = false;
	cmd = "BP00;";
	if (sendCommand(cmd, 8, false)) {
		if (replybuff[6] == '1') {
			ison = true;
			cmd = "BP01;";
			if (sendCommand(cmd, 8, false)) {
				replybuff[7] = 0;
				val = atoi(&replybuff[4]);
				val *= 10;
			}
		}
	}
	return ison;
}

void RIG_FT950::get_notch_min_max_step(int &min, int &max, int &step)
{
	min = 0;
	max = 3000;
	step = 10;
}

void RIG_FT950::set_noise(bool b)
{
	if (b)
		cmd = "NB01;";
	else
		cmd = "NB00;";
	sendCommand(cmd, 0, false);
}

// val 0 .. 100
void RIG_FT950::set_mic_gain(int val)
{
	cmd = "MG000;";
	val = (int)(val * 2.55); // convert to 0 .. 255
	for (int i = 3; i > 0; i--) {
		cmd[1+i] += val % 10;
		val /= 10;
	}
	sendCommand(cmd, 0, false);
}

int RIG_FT950::get_mic_gain()
{
	sendCommand("MG;", 6, false);
	replybuff[5] = 0;
	int val = atoi(&replybuff[2]);
	return val;
}

void RIG_FT950::get_mic_min_max_step(int &min, int &max, int &step)
{
	min = 0;
	max = 100;
	step = 1;
}

void RIG_FT950::set_rf_gain(int val)
{
	cmd = "RG0000;";
	int rfval = val * 2.55;
	for (int i = 5; i > 2; i--) {
		cmd[i] = rfval % 10 + '0';
		rfval /= 10;
	}
	sendCommand(cmd, 0, false);
}

int  RIG_FT950::get_rf_gain()
{
	int rfval = 0;
	cmd = "RG0;";
	if (sendCommand(cmd, 7, false)) {
		for (int i = 3; i < 6; i++) {
			rfval *= 10;
			rfval += replybuff[i] - '0';
		}
	}
	return (int)(rfval / 2.55);
}

void RIG_FT950::get_rf_min_max_step(int &min, int &max, int &step)
{
	min = 0;
	max = 100;
	step = 1;
}

