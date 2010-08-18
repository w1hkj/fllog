#ifndef _status_H
#define _status_H

#include <string>
#include <FL/Fl.H>
#include <FL/Enumerations.H>

#include "fllog.h"

using namespace std;

struct status {
	int		mainX;
	int		mainY;
	int		mainW;
	int		mainH;

// logbook entries
	string logbookfilename;

	void saveLastState();
	void loadLastState();
};

extern status progStatus;

#endif
