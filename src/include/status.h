#ifndef _status_H
#define _status_H

#include <string>
#include <FL/Fl.H>
#include <FL/Enumerations.H>

#include "fllog.h"


enum sorttype {NONE, SORTCALL, SORTDATE, SORTFREQ, SORTMODE};

struct status {
// dialog characteristics
	int  		mainX;
	int  		mainY;
	int  		mainW;
	int  		mainH;

	int  		LOGBOOKtextsize;
	Fl_Font		LOGBOOKtextfont;
	Fl_Color	LOGBOOKtextcolor;
	Fl_Color	LOGBOOKcolor;

	bool        BrowseTimeOFF;
// logbook entries
	std::string logbookfilename;
	std::string mycall;
	std::string opcall;
	std::string opname;
	std::string my_staqth;
	std::string my_staloc;

	sorttype	lastsort;
	bool callfwd;
	bool datefwd;
	bool modefwd;
	bool freqfwd;

	std::string server_port;

	int  SelectCall;
	int  SelectName;
	int  SelectFreq;
	int  SelectBand;
	int  SelectMode;
	int  SelectQSOdateOn;
	int  SelectQSOdateOff;
	int  SelectTimeON;
	int  SelectTimeOFF;
	int  SelectTX_pwr;
	int  SelectRSTsent;
	int  SelectRSTrcvd;
	int  SelectQth;
	int  SelectLOC;
	int  SelectState;
	int  SelectAge;

	int  SelectStaCall;
	int  SelectStaGrid;
	int  SelectStaCity;
	int  SelectOperator;
	int  SelectProvince;
	int  SelectCountry;
	int  SelectNotes;
	int  SelectQSLrcvd;
	int  SelectQSLsent;
	int  SelecteQSLrcvd;
	int  SelecteQSLsent;
	int  SelectLOTWrcvd;
	int  SelectLOTWsent;
	int  SelectQSL_VIA;
	int  SelectSerialIN;
	int  SelectSerialOUT;

	int  SelectCheck;

	int  SelectXchgIn;
	int  SelectMyXchg;

	int  SelectCNTY;
	int  SelectCONT;
	int  SelectCQZ;
	int  SelectDXCC;
	int  SelectIOTA;
	int  SelectITUZ;

	int  SelectClass;
	int  SelectSection;

	int  Select_cwss_serno;
	int  Select_cwss_prec;
	int  Select_cwss_check;
	int  Select_cwss_section;

	int  Select_1010;
	int  SelectJOTA;

	int  use_nulines;  // separate each reported field with a new line character
	int  event_log_open;

	void saveLastState();
	void loadLastState();
	void save_defaults();
	void load_defaults();
};

extern status progStatus;

#endif
