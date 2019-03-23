// ----------------------------------------------------------------------------
// globals.cxx  --  constants, variables, arrays & functions that need to be
//                  outside of any thread
//
// Copyright (C) 2006-2018
//		Dave Freese, W1HKJ
// Copyright (C) 2007-2009
//		Stelios Bounanos, M0GLD
//
// This file is part of fllog
//
// Fldigi is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Fldigi is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fldigi.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------

#include <config.h>
#include <iosfwd>
#include <iomanip>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cerrno>
#include <cstdio>
#include <cstring>

#include "globals.h"
#include "util.h"

using namespace std;

// ************ Elements are in enum trx_mode order. ************
// N.B. it is not valid to use an _("NLS") string in this table!!
// ... doing so will break the Fl_menu_item table 'menu_'.  -Kamal

// Last value (true/false) determines if it's used with the KISS interface.
// It must have 8 bit support. Current selection based on the modems used in FLAMP.

const struct mode_info_t mode_info[NUM_MODES] = {

{MODE_CW,"CW","CW","CW",""},

{MODE_CONTESTIA,"CTSTIA","CONTESTI","CONTESTI",""},

{MODE_DOMINOEXMICRO,"DOMEX Micro","DOMINO","DOMINO",""},
{MODE_DOMINOEX4,"DOMEX4","DOMINO","DOMINO","DOMINO"},
{MODE_DOMINOEX5,"DOMEX5","DOMINO","DOMINO","DOMINO"},
{MODE_DOMINOEX8,"DOMEX8","DOMINO","DOMINO","DOMINO"},
{MODE_DOMINOEX11,"DOMX11","DOMINO","DOMINO","DOMINO"},
{MODE_DOMINOEX16,"DOMX16","DOMINO","DOMINO","DOMINO"},
{MODE_DOMINOEX22,"DOMX22","DOMINO","DOMINO","DOMINO"},
{MODE_DOMINOEX44,"DOMX44","DOMINO","DOMINO","DOMINO"},
{MODE_DOMINOEX88,"DOMX88","DOMINO","DOMINO","DOMINO"},

{MODE_FELDHELL,"FELDHELL","HELL","HELL",""},
{MODE_SLOWHELL,"SLOWHELL","HELL","HELL",""},
{MODE_HELLX5,"HELLX5","HELL","HELL",""},
{MODE_HELLX9,"HELLX9","HELL","HELL",""},
{MODE_FSKHELL,"FSKHELL","FMHELL","HELL","FSKHELL"},
{MODE_FSKH105,"FSKH105","FMHELL","FMHELL",""},
{MODE_HELL80,"HELL80","HELL80","HELL","HELL80"},

{MODE_MFSK8,"MFSK8","MFSK8","MFSK","MFSK8"},
{MODE_MFSK16,"MFSK16","MFSK16","MFSK","MFSK16"},
{MODE_MFSK32,"MFSK32","MFSK32","MFSK","MFSK32"},
{MODE_MFSK4,"MFSK4","MFSK4","MFSK","MFSK4"},
{MODE_MFSK11,"MFSK11","MFSK11","MFSK","MFSK11"},
{MODE_MFSK22,"MFSK22","MFSK22","MFSK","MFSK22"},
{MODE_MFSK31,"MFSK31","MFSK31","MFSK","MFSK31"},
{MODE_MFSK64,"MFSK64","MFSK64","MFSK","MFSK64"},
{MODE_MFSK128,"MFSK128","MFSK128","MFSK","MFSK128"},
{MODE_MFSK64L,"MFSK64L","MFSK64L","MFSK","MFSK64"},
{MODE_MFSK128L,"MFSK128L","MFSK128L","MFSK","MFSK128"},
{MODE_WEFAX_576,"WEFAX576","FAX","FAX","FAX"},
{MODE_WEFAX_288,"WEFAX288","FAX","FAX","FAX"},

{MODE_NAVTEX,"NAVTEX","TOR","NAVTEX",""},
{MODE_SITORB,"SITORB","TOR","SITORB",""},

{MODE_MT63_500S,"MT63-500S","MT63","MT63",""},
{MODE_MT63_500L,"MT63-500L","MT63","MT63",""},
{MODE_MT63_1000S,"MT63-1KS","MT63","MT63",""},
{MODE_MT63_1000L,"MT63-1KL","MT63","MT63",""},
{MODE_MT63_2000S,"MT63-2KS","MT63","MT63",""},
{MODE_MT63_2000L,"MT63-2KL","MT63","MT63",""},

{MODE_PSK31,"BPSK31","PSK31","PSK","PSK31"},
{MODE_PSK63,"BPSK63","PSK63","PSK","PSK63"},
{MODE_PSK63F,"BPSK63F","PSK63F","PSK","PSK63F"},
{MODE_PSK125,"BPSK125","PSK125","PSK","PSK125"},
{MODE_PSK250,"BPSK250","PSK250","PSK","PSK250"},
{MODE_PSK500,"BPSK500","PSK500","PSK","PSK500"},
{MODE_PSK1000,"BPSK1000","PSK1000","PSK","PSK1000"},
{MODE_12X_PSK125,"PSK125C12","PSK125C12","PSK",""},
{MODE_6X_PSK250,"PSK250C6","PSK250C6","PSK",""},
{MODE_2X_PSK500,"PSK500C2","PSK500C2","PSK",""},
{MODE_4X_PSK500,"PSK500C4","PSK500C4","PSK",""},
{MODE_2X_PSK800,"PSK800C2","PSK800C2","PSK",""},
{MODE_2X_PSK1000,"PSK1000C2","PSK1000C2","PSK",""},

{MODE_QPSK31,"QPSK31","QPSK31","PSK","QPSK31"},
{MODE_QPSK63,"QPSK63","QPSK63","PSK","QPSK63"},
{MODE_QPSK125,"QPSK125","QPSK125","PSK","QPSK125"},
{MODE_QPSK250,"QPSK250","QPSK250","PSK","QPSK250"},
{MODE_QPSK500,"QPSK500","QPSK500","PSK","QPSK500"},

{MODE_8PSK125,"8PSK125","8PSK125","PSK",""},
{MODE_8PSK125FL,"8PSK125FL","8PSK125FL","PSK",""},
{MODE_8PSK125F,"8PSK125F","8PSK125F","PSK",""},
{MODE_8PSK250,"8PSK250","8PSK250","PSK",""},
{MODE_8PSK250FL,"8PSK250FL","8PSK250FL","PSK",""},
{MODE_8PSK250F,"8PSK250F","8PSK250F","PSK",""},
{MODE_8PSK500,"8PSK500","8PSK500","PSK",""},
{MODE_8PSK500F,"8PSK500F","8PSK500F","PSK",""},
{MODE_8PSK1000,"8PSK1000","8PSK1000","PSK",""},
{MODE_8PSK1000F,"8PSK1000F","8PSK1000F","PSK",""},
{MODE_8PSK1200F,"8PSK1200F","8PSK1200F","PSK",""},

{MODE_OLIVIA,"OLIVIA","OLIVIA","OLIVIA",""},
{MODE_OLIVIA_4_250,"Olivia-4-250","OLIVIA","OLIVIA","OLIVIA 4/250"},
{MODE_OLIVIA_8_250,"Olivia-8-250","OLIVIA","OLIVIA","OLIVIA 8/250"},
{MODE_OLIVIA_4_500,"Olivia-4-500","OLIVIA","OLIVIA","OLIVIA 4/500"},
{MODE_OLIVIA_8_500,"Olivia-8-500","OLIVIA","OLIVIA","OLIVIA 8/500"},
{MODE_OLIVIA_16_500,"Olivia-16-500","OLIVIA","OLIVIA","OLIVIA 16/500"},
{MODE_OLIVIA_8_1000,"Olivia-8-1K","OLIVIA","OLIVIA","OLIVIA 8/1000"},
{MODE_OLIVIA_16_1000,"Olivia-16-1K","OLIVIA","OLIVIA","OLIVIA 16/1000"},
{MODE_OLIVIA_32_1000,"Olivia-32-1K","OLIVIA","OLIVIA","OLIVIA 32/1000"},
{MODE_OLIVIA_64_2000,"Olivia-64-2K","OLIVIA","OLIVIA","OLIVIA 64/2000"},

{MODE_RTTY,"RTTY","RTTY","RTTY",""},

{MODE_THORMICRO,"THOR Micro","THOR","THOR",""},
{MODE_THOR4,"THOR4","THOR","THOR",""},
{MODE_THOR5,"THOR5","THOR","THOR",""},
{MODE_THOR8,"THOR8","THOR","THOR",""},
{MODE_THOR11,"THOR11","THOR","THOR",""},
{MODE_THOR16,"THOR16","THOR","THOR",""},
{MODE_THOR22,"THOR22","THOR","THOR",""},
{MODE_THOR25x4,"THOR25x4","THOR","THOR",""},
{MODE_THOR50x1,"THOR50x1","THOR","THOR",""},
{MODE_THOR50x2,"THOR50x2","THOR","THOR",""},
{MODE_THOR100,"THOR100","THOR","THOR",""},
{MODE_THROB1,"THROB1","THRB","THRB",""},
{MODE_THROB2,"THROB2","THRB","THRB",""},
{MODE_THROB4,"THROB4","THRB","THRB",""},
{MODE_THROBX1,"THRBX1","THRBX","THRB","THRBX"},
{MODE_THROBX2,"THRBX2","THRBX","THRB","THRBX"},
{MODE_THROBX4,"THRBX4","THRBX","THRB","THRBX"},

{MODE_PSK125R,"PSK125R","PSK125R","PSK",""},
{MODE_PSK250R,"PSK250R","PSK250R","PSK",""},
{MODE_PSK500R,"PSK500R","PSK500R","PSK",""},
{MODE_PSK1000R,"PSK1000R","PSK1000R","PSK",""},
{MODE_4X_PSK63R,"PSK63RC4","PSK63RC4","PSK",""},
{MODE_5X_PSK63R,"PSK63RC5","PSK63RC5","PSK",""},
{MODE_10X_PSK63R,"PSK63RC10","PSK63RC10","PSK",""},
{MODE_20X_PSK63R,"PSK63RC20","PSK63RC20","PSK",""},
{MODE_32X_PSK63R,"PSK63RC32","PSK63RC32","PSK",""},
{MODE_4X_PSK125R,"PSK125RC4","PSK125RC4","PSK",""},
{MODE_5X_PSK125R,"PSK125RC5","PSK125RC5","PSK",""},
{MODE_10X_PSK125R,"PSK125RC10","PSK125RC10","PSK",""},
{MODE_12X_PSK125R,"PSK125RC12","PSK125RC12","PSK",""},
{MODE_16X_PSK125R,"PSK125RC16","PSK125RC16","PSK",""},
{MODE_2X_PSK250R,"PSK250RC2","PSK250RC2","PSK",""},
{MODE_3X_PSK250R,"PSK250RC3","PSK250RC3","PSK",""},
{MODE_5X_PSK250R,"PSK250RC5","PSK250RC5","PSK",""},
{MODE_6X_PSK250R,"PSK250RC6","PSK250RC6","PSK",""},
{MODE_7X_PSK250R,"PSK250RC7","PSK250RC7","PSK",""},
{MODE_2X_PSK500R,"PSK500RC2","PSK500RC2","PSK",""},
{MODE_3X_PSK500R,"PSK500RC3","PSK500RC3","PSK",""},
{MODE_4X_PSK500R,"PSK500RC4","PSK500RC4","PSK",""},
{MODE_2X_PSK800R,"PSK800RC2","PSK800RC2","PSK",""},
{MODE_2X_PSK1000R,"PSK1000RC2","PSK1000RC2","PSK",""},

{MODE_FSQ,"FSQ","FSQ","FSQ",""},
{MODE_IFKP,"IFKP","IFKP","IFKP",""},

{MODE_SSB,"SSB","SSB","SSB",""}

};

std::string adif2export(std::string adif)
{
	std::string test = adif;
	for (size_t n = 0; n < test.length(); n++) test[n] = toupper(test[n]);
	for (int n = 0; n < NUM_MODES; n++) {
		if (test == mode_info[n].adif_name)
			return mode_info[n].export_mode;
	}
	return test;
}

std::string adif2submode(std::string adif)
{
	std::string test = adif;
	for (size_t n = 0; n < test.length(); n++) test[n] = toupper(test[n]);
	for (int n = 0; n < NUM_MODES; n++) {
		if (test == mode_info[n].adif_name)
			return mode_info[n].export_submode;
	}
	return "";
}

std::ostream& operator<<(std::ostream& s, const qrg_mode_t& m)
{
	return s << m.rfcarrier << ' '
			 << m.rmode << ' '
			 << m.carrier << ' '
			 << mode_info[m.mode].sname << ' '
			 << m.usage;
}

std::istream& operator>>(std::istream& s, qrg_mode_t& m)
{
	string sMode;
	char temp[255];
	int mnbr;
	s >> m.rfcarrier >> m.rmode >> m.carrier >> sMode;

	s.getline(temp, 255);
	m.usage = temp;
	while (m.usage[0] == ' ') m.usage.erase(0,1);

// handle case for reading older type of specification string
	if (sscanf(sMode.c_str(), "%d", &mnbr)) {
		m.mode = mnbr;
		return s;
	}
	m.mode = MODE_PSK31;
	for (mnbr = MODE_CW; mnbr < NUM_MODES; mnbr++)
		if (sMode == mode_info[mnbr].sname) {
			m.mode = mnbr;
			break;
		}
	return s;
}

std::string qrg_mode_t::str(void)
{
	ostringstream s;
	s << setiosflags(ios::fixed)
	  << setprecision(3) << rfcarrier/1000.0 << '|'
	  << rmode << '|'
	  << (mode < NUM_MODES ? mode_info[mode].sname : "NONE") << '|'
//	  << carrier;
	  << carrier << '|'
	  << usage;
	return s.str();
}

band_t band(long long freq_hz)
{
	switch (freq_hz / 1000000LL) {
		case 0: case 1: return BAND_160M;
		case 3: return BAND_80M;
		case 4: return BAND_75M;
		case 5: return BAND_60M;
		case 7: return BAND_40M;
		case 10: return BAND_30M;
		case 14: return BAND_20M;
		case 18: return BAND_17M;
		case 21: return BAND_15M;
		case 24: return BAND_12M;
		case 28 ... 29: return BAND_10M;
		case 50 ... 54: return BAND_6M;
		case 70 ... 71: return BAND_4M;
		case 144 ... 148: return BAND_2M;
		case 222 ... 225: return BAND_125CM;
		case 420 ... 450: return BAND_70CM;
		case 902 ... 928: return BAND_33CM;
		case 1240 ... 1325: return BAND_23CM;
		case 2300 ... 2450: return BAND_13CM;
		case 3300 ... 3500: return BAND_9CM;
		case 5650 ... 5925: return BAND_6CM;
		case 10000 ... 10500: return BAND_3CM;
		case 24000 ... 24250: return BAND_125MM;
		case 47000 ... 47200: return BAND_6MM;
		case 75500 ... 81000: return BAND_4MM;
		case 119980 ... 120020: return BAND_2P5MM;
		case 142000 ... 149000: return BAND_2MM;
		case 241000 ... 250000: return BAND_1MM;
	}

	return BAND_OTHER;
}

band_t band(const char* freq_mhz)
{
	errno = 0;
	double d = strtod(freq_mhz, NULL);
	if (d != 0.0 && errno == 0)
		return band((long long)(d * 1e6));
	else
		return BAND_OTHER;
}

struct band_freq_t {
	const char* band;
	const char* freq;
};

static struct band_freq_t band_names[NUM_BANDS] = {
	{ "160m", "1.8" },
	{ "80m", "3.5" },
	{ "75m", "4.0" },
	{ "60m", "5.3" },
	{ "40m", "7.0" },
	{ "30m", "10.0" },
	{ "20m", "14.0" },
	{ "17m", "18.0" },
	{ "15m", "21.0" },
	{ "12m", "24.0" },
	{ "10m", "28.0" },
	{ "6m", "50.0" },
	{ "4m", "70.0" },
	{ "2m", "144.0" },
	{ "1.25m", "222.0" },
	{ "70cm", "420.0" },
	{ "33cm", "902.0" },
	{ "23cm", "1240.0" },
	{ "13cm", "2300.0" },
	{ "9cm", "3300.0" },
	{ "6cm", "5650.0" },
	{ "3cm", "10000.0" },
	{ "1.25cm", "24000.0" },
	{ "6mm", "47000.0" },
	{ "4mm", "75500.0" },
	{ "2.5mm", "119980.0" },
	{ "2mm", "142000.0" },
	{ "1mm", "241000.0" },
	{ "other", "" }
};

const char* band_name(band_t b)
{
	return band_names[CLAMP(b, 0, NUM_BANDS-1)].band;
}

const char* band_name(const char* freq_mhz)
{
	return band_name(band(freq_mhz));
}

const char* band_freq(band_t b)
{
	return band_names[CLAMP(b, 0, NUM_BANDS-1)].freq;
}

const char* band_freq(const char* band_name)
{
	for (size_t i = 0; i < BAND_OTHER; i++)
		if (!strcmp(band_names[i].band, band_name))
			return band_names[i].freq;

	return "";
}

