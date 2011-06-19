#ifndef _status_H
#define _status_H

#include <string>
#include <FL/Fl.H>
#include <FL/Enumerations.H>

#include "fllog.h"

using namespace std;

enum sorttype {NONE, SORTCALL, SORTDATE, SORTFREQ, SORTMODE};

struct status {
	int		mainX;
	int		mainY;
	int		mainW;
	int		mainH;

// logbook entries
	string logbookfilename;
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
