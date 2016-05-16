// ---------------------------------------------------------------------
//
// status.cxx
//
// Copyright (C) 2008-2012
//               Dave Freese, W1HKJ
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with the program; if not, write to the
//
//  Free Software Foundation, Inc.
//  51 Franklin Street, Fifth Floor
//  Boston, MA  02110-1301 USA.
//
// ---------------------------------------------------------------------

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
	50,						// int mainX;
	50,						// int mainY;
	580,					// int mainW;
	385,					// int mainH;

	14,						// text size
	FL_HELVETICA,			// text font
	FL_BLACK,				// text color
	FL_BACKGROUND2_COLOR,	// text background

	"",						// logbookfilename
	"",						// mycall
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
	spref.set("textsize", LOGBOOKtextsize);
	spref.set("textfont", (void *)&LOGBOOKtextfont, sizeof(Fl_Font));
	spref.set("textcolor", (void *)&LOGBOOKtextcolor, sizeof(Fl_Color));
	spref.set("bkcolor", (void *)&LOGBOOKcolor, sizeof(Fl_Color));
	spref.set("logbook_filename", logbookfilename.c_str());
	spref.set("mycall", mycall.c_str());
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
		void *variable;
		char defbuffer[200];
		int i = lastsort;
		spref.get("mainx", mainX, mainX);
		spref.get("mainy", mainY, mainY);
		spref.get("mainw", mainW, mainW);
		spref.get("mainh", mainH, mainH);
		spref.get("textsize", LOGBOOKtextsize, LOGBOOKtextsize);
		spref.get("textfont", (void *&)variable, (void *)&LOGBOOKtextfont, sizeof(Fl_Font));
		LOGBOOKtextfont = *(Fl_Font *)variable;
		free(variable);
		spref.get("textcolor", (void *&)variable, (void *)&LOGBOOKtextcolor, sizeof(Fl_Color));
		LOGBOOKtextcolor = *(Fl_Color *)variable;
		free(variable);
		spref.get("bkcolor", (void *&)variable, (void *)&LOGBOOKcolor, sizeof(Fl_Color));
		LOGBOOKcolor = *(Fl_Color *)variable;
		free(variable);

		spref.get("logbook_filename", defbuffer, logbookfilename.c_str(), 199);
		logbookfilename = defbuffer;

		spref.get("mycall", defbuffer, mycall.c_str(), 199);
		mycall = defbuffer;

		if (spref.get("lastsort", i, i)) lastsort = (sorttype) i;
		if (spref.get("callfwd", i, i)) callfwd = i;
		if (spref.get("datefwd", i, i)) datefwd = i;
		if (spref.get("modefwd", i, i)) modefwd = i;
		if (spref.get("freqfwd", i, i)) freqfwd = i;
		spref.get("server_port", defbuffer, server_port.c_str(), 199); server_port = defbuffer;
	}
}

