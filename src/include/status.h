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

// logbook entries
	string logbookfilename;
	string mycall;

	sorttype	lastsort;
	bool callfwd;
	bool datefwd;
	bool modefwd;
	bool freqfwd;

	string server_port;

	void saveLastState();
	void loadLastState();
};

extern status progStatus;

#endif
