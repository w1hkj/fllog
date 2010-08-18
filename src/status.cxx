/* -----------------------------------------------------------------------------
 * status structure / methods
 *
 * A part of "rig", a rig control program compatible with fldigi / xmlrpc i/o
 *
 * copyright Dave Freese 2009, w1hkj@w1hkj.com
 *
*/

#include <iostream>
#include <fstream>
#include <string>

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Progress.H>

#include "status.h"
#include "util.h"
#include "config.h"
#include "debug.h"

#include "logsupport.h"

status progStatus = {
	50,			// int mainX;
	50,			// int mainY;
	580,		// int mainW;
	385,		// int mainH;
	""			// string	logbookfilename
};

void status::saveLastState()
{
	Fl_Preferences spref(LogHomeDir.c_str(), "w1hkj.com", PACKAGE_TARNAME);

	int mX = mainwindow->x();
	int mY = mainwindow->y();
	int mH = mainwindow->h();
	int mW = mainwindow->w();

	if (mX >= 0 && mX >= 0) {
		mainX = mX;
		mainY = mY;
	}

	spref.set("version", PACKAGE_VERSION);
	spref.set("mainx", mX);
	spref.set("mainy", mY);
	spref.set("mainw", mW);
	spref.set("mainh", mH);
	spref.set("logbook_filename", logbookfilename.c_str());
}

void status::loadLastState()
{
	Fl_Preferences spref(LogHomeDir.c_str(), "w1hkj.com", PACKAGE_TARNAME);

	if (spref.entryExists("version")) {
		char defbuffer[200];
		spref.get("mainx", mainX, mainX);
		spref.get("mainy", mainY, mainY);
		spref.get("mainw", mainW, mainW);
		spref.get("mainh", mainH, mainH);
		spref.get("logbook_filename", defbuffer, "", 199); logbookfilename = defbuffer;
	}
}

