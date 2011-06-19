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
	"",			// string	logbookfilename
	SORTDATE,
	true,
	true,
	true,
	true,
	"8421"
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
	spref.set("lastsort", lastsort);
	spref.set("callfwd", callfwd);
	spref.set("datefwd", datefwd);
	spref.set("modefwd", modefwd);
	spref.set("freqfwd", freqfwd);
	spref.set("server_port", server_port.c_str());
}

void status::loadLastState()
{
	Fl_Preferences spref(LogHomeDir.c_str(), "w1hkj.com", PACKAGE_TARNAME);

	if (spref.entryExists("version")) {
		char defbuffer[200];
		int i = lastsort;
		spref.get("mainx", mainX, mainX);
		spref.get("mainy", mainY, mainY);
		spref.get("mainw", mainW, mainW);
		spref.get("mainh", mainH, mainH);
		spref.get("logbook_filename", defbuffer, logbookfilename.c_str(), 199); logbookfilename = defbuffer;
		if (spref.get("lastsort", i, i)) lastsort = (sorttype) i;
		if (spref.get("callfwd", i, i)) callfwd = i;
		if (spref.get("datefwd", i, i)) datefwd = i;
		if (spref.get("modefwd", i, i)) modefwd = i;
		if (spref.get("freqfwd", i, i)) freqfwd = i;
		spref.get("server_port", defbuffer, server_port.c_str(), 199); server_port = defbuffer;
	}
}

