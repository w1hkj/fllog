#ifndef _FT950_H
#define _FT950_H

#include "rigbase.h"

class RIG_FT950 : public rigbase {
private:
	bool notch_on;
	void update_bandwidths();
protected:
	int  preamp_level;
	int  atten_level;
public:
	RIG_FT950();
	~RIG_FT950(){}

	void initialize();
	long get_vfoA();
	void set_vfoA(long);
	int  get_smeter();
	int  get_swr();
	int  get_power_out();
	int  get_power_control();
	void set_volume_control(int val);
	int  get_volume_control();
	void set_power_control(double val);
	void set_PTT_control(int val);
	void tune_rig();
	void set_attenuator(int val);
	int  get_attenuator();
	void set_preamp(int val);
	int  get_preamp();
	void set_mode(int val);
	int  get_mode();
	int  get_modetype(int n);
	void set_bandwidth(int val);
	int  get_bandwidth();
	int  adjust_bandwidth();
	void set_if_shift(int val);
	bool get_if_shift(int &val);
	void get_if_min_max_step(int &min, int &max, int &step);
	void set_notch(bool on, int val);
	bool get_notch(int &val);
	void get_notch_min_max_step(int &min, int &max, int &step);
	void set_noise(bool b);
	void set_mic_gain(int val);
	int  get_mic_gain();
	void get_mic_min_max_step(int &min, int &max, int &step);

	void set_rf_gain(int val);
	int  get_rf_gain();
	void get_rf_min_max_step(int &min, int &max, int &step);

};


#endif