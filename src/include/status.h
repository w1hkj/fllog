#ifndef _status_H
#define _status_H

#include <string>
#include <FL/Fl.H>
#include <FL/Enumerations.H>

#include "fllog.h"

using namespace std;

enum sorttype {NONE, SORTCALL, SORTDATE, SORTFREQ, SORTMODE};

struct status {
// dialog characteristics
	int			mainX;
	int			mainY;
	int			mainW;
	int			mainH;

	int			LOGBOOKtextsize;
	Fl_Font		LOGBOOKtextfont;
	Fl_Color	LOGBOOKtextcolor;
	Fl_Color	LOGBOOKcolor;

    bool        BrowseTimeOFF;
// logbook entries
	string logbookfilename;
	string mycall;

	sorttype	lastsort;
	bool callfwd;
	bool datefwd;
	bool modefwd;
	bool freqfwd;

	string server_port;

	bool SelectCall;
	bool SelectName;
	bool SelectFreq;
	bool SelectBand;
	bool SelectMode;
	bool SelectQSOdateOn;
	bool SelectQSOdateOff;
	bool SelectTimeON;
	bool SelectTimeOFF;
	bool SelectQth;
	bool SelectLOC;
	bool SelectState;
	bool SelectProvince;
	bool SelectCountry;
	bool SelectQSLrcvd;
	bool SelectQSLsent;
	bool SelectSerialIN;
	bool SelectSerialOUT;
	bool SelectXchgIn;
	bool SelectMyXchg;
	bool SelectRSTsent;
	bool SelectRSTrcvd;
	bool SelectIOTA;
	bool SelectDXCC;
	bool SelectCNTY;
	bool SelectCONT;
	bool SelectCQZ;
	bool SelectITUZ;
	bool SelectTX_pwr;
	bool SelectNotes;
	bool SelectQSL_VIA;
	bool SelectOperator;
	bool SelectStaCall;
	bool SelectStaGrid;
	bool SelectStaCity;

	void saveLastState();
	void loadLastState();
};

extern status progStatus;

#endif
