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

    0,                      // bool BrowseTimeOFF

	"",						// logbookfilename
	"",						// mycall
	"",						// opcall
	"",						// opname
	SORTDATE,
	true,
	true,
	true,
	true,
	"8421",

	1,	//bool SelectCall
	1,	//bool SelectName
	1,	//bool SelectFreq
	1,	//bool SelectBand
	1,	//bool SelectMode
	1,	//bool SelectQSOdateOn
	1,	//bool SelectQSOdateOff
	1,	//bool SelectTimeON
	1,	//bool SelectTimeOFF
	0,	//bool SelectQth
	0,	//bool SelectLOC
	0,	//bool SelectState
	0,	//bool SelectProvince
	0,	//bool SelectCountry
	0,	//bool SelectQSLrcvd
	0,	//bool SelectQSLsent
	0,	//bool SelectSerialIN
	0,	//bool SelectSerialOUT
	0,	//bool SelectXchgIn
	0,	//bool SelectMyXchg
	0,	//bool SelectRSTsent
	1,	//bool SelectRSTrcvd
	0,	//bool SelectIOTA
	0,	//bool SelectDXCC
	0,	//bool SelectCNTY
	0,	//bool SelectCONT
	0,	//bool SelectCQZ
	0,	//bool SelectITUZ
	0,	//bool SelectTX_pwr
	0,	//bool SelectNotes
	0,	//bool SelectQSL_VIA
	0,	//bool SelectOperator
	0,	//bool SelectStaCall
	0,	//bool SelectStaGrid
	0	//bool SelectStaCity

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

    spref.set("browsetimeoff", BrowseTimeOFF);

	spref.set("logbook_filename", logbookfilename.c_str());
	spref.set("mycall", mycall.c_str());
	spref.set("opcall", opcall.c_str());
	spref.set("opname", opname.c_str());

	spref.set("lastsort", lastsort);
	spref.set("callfwd", callfwd);
	spref.set("datefwd", datefwd);
	spref.set("modefwd", modefwd);
	spref.set("freqfwd", freqfwd);
	spref.set("server_port", server_port.c_str());

	spref.set("SelectCall", SelectCall);
	spref.set("SelectName", SelectName);
	spref.set("SelectFreq", SelectFreq);
	spref.set("SelectBand", SelectBand);
	spref.set("SelectMode", SelectMode);
	spref.set("SelectQSOdateOn", SelectQSOdateOn);
	spref.set("SelectQSOdateOff", SelectQSOdateOff);
	spref.set("SelectTimeON", SelectTimeON);
	spref.set("SelectTimeOFF", SelectTimeOFF);
	spref.set("SelectQth", SelectQth);
	spref.set("SelectLOC", SelectLOC);
	spref.set("SelectState", SelectState);
	spref.set("SelectProvince", SelectProvince);
	spref.set("SelectCountry", SelectCountry);
	spref.set("SelectQSLrcvd", SelectQSLrcvd);
	spref.set("SelectQSLsent", SelectQSLsent);
	spref.set("SelectSerialIN", SelectSerialIN);
	spref.set("SelectSerialOUT", SelectSerialOUT);
	spref.set("SelectXchgIn", SelectXchgIn);
	spref.set("SelectMyXchg", SelectMyXchg);
	spref.set("SelectRSTsent", SelectRSTsent);
	spref.set("SelectRSTrcvd", SelectRSTrcvd);
	spref.set("SelectIOTA", SelectIOTA);
	spref.set("SelectDXCC", SelectDXCC);
	spref.set("SelectCNTY", SelectCNTY);
	spref.set("SelectCONT", SelectCONT);
	spref.set("SelectCQZ", SelectCQZ);
	spref.set("SelectITUZ", SelectITUZ);
	spref.set("SelectTX_pwr", SelectTX_pwr);
	spref.set("SelectNotes", SelectNotes);
	spref.set("SelectQSL_VIA", SelectQSL_VIA);
	spref.set("SelectOperator", SelectOperator);
	spref.set("SelectStaCall", SelectStaCall);
	spref.set("SelectStaGrid", SelectStaGrid);
	spref.set("SelectStaCity", SelectStaCity);

}

void status::loadLastState()
{
	Fl_Preferences spref(LogHomeDir.c_str(), "w1hkj.com", PACKAGE_TARNAME);

	if (spref.entryExists("version")) {
		void *variable;
		char defbuffer[200];
		int i = 0;
		spref.get("mainx", mainX, mainX);
		spref.get("mainy", mainY, mainY);
		spref.get("mainw", mainW, mainW);
		spref.get("mainh", mainH, mainH);
		spref.get("textsize", LOGBOOKtextsize, LOGBOOKtextsize);
		spref.get("textfont", (void *&)variable, (void *)&LOGBOOKtextfont, sizeof(Fl_Font));

        spref.get("browsetimeoff", i, i); BrowseTimeOFF = i;

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

		spref.get("opcall", defbuffer, opcall.c_str(), 199);
		opcall = defbuffer;

		spref.get("opname", defbuffer, opname.c_str(), 199);
		opname = defbuffer;

		if (spref.get("lastsort", i, lastsort)) lastsort = (sorttype) i;
		if (spref.get("callfwd", i, callfwd)) callfwd = i;
		if (spref.get("datefwd", i, datefwd)) datefwd = i;
		if (spref.get("modefwd", i, modefwd)) modefwd = i;
		if (spref.get("freqfwd", i, freqfwd)) freqfwd = i;

		spref.get("server_port", defbuffer, server_port.c_str(), 199); server_port = defbuffer;

		spref.get("SelectName", i, i); SelectName = i;
		spref.get("SelectFreq", i, i); SelectFreq = i;
		spref.get("SelectBand", i, i); SelectBand = i;
		spref.get("SelectMode", i, i); SelectMode = i;
		spref.get("SelectQSOdateOn", i, i); SelectQSOdateOn = i;
		spref.get("SelectQSOdateOff", i, i); SelectQSOdateOff = i;
		spref.get("SelectTimeON", i, i); SelectTimeON = i;
		spref.get("SelectTimeOFF", i, i); SelectTimeOFF = i;
		spref.get("SelectQth", i, i); SelectQth = i;
		spref.get("SelectLOC", i, i); SelectLOC = i;
		spref.get("SelectState", i, i); SelectState = i;
		spref.get("SelectProvince", i, i); SelectProvince = i;
		spref.get("SelectCountry", i, i); SelectCountry = i;
		spref.get("SelectQSLrcvd", i, i); SelectQSLrcvd = i;
		spref.get("SelectQSLsent", i, i); SelectQSLsent = i;
		spref.get("SelectSerialIN", i, i); SelectSerialIN = i;
		spref.get("SelectSerialOUT", i, i); SelectSerialOUT = i;
		spref.get("SelectXchgIn", i, i); SelectXchgIn = i;
		spref.get("SelectMyXchg", i, i); SelectMyXchg = i;
		spref.get("SelectRSTsent", i, i); SelectRSTsent = i;
		spref.get("SelectRSTrcvd", i, i); SelectRSTrcvd = i;
		spref.get("SelectIOTA", i, i); SelectIOTA = i;
		spref.get("SelectDXCC", i, i); SelectDXCC = i;
		spref.get("SelectCNTY", i, i); SelectCNTY = i;
		spref.get("SelectCONT", i, i); SelectCONT = i;
		spref.get("SelectCQZ", i, i); SelectCQZ = i;
		spref.get("SelectITUZ", i, i); SelectITUZ = i;
		spref.get("SelectTX_pwr", i, i); SelectTX_pwr = i;
		spref.get("SelectNotes", i, i); SelectNotes = i;
		spref.get("SelectQSL_VIA", i, i); SelectQSL_VIA = i;
		spref.get("SelectOperator", i, i); SelectOperator = i;
		spref.get("SelectStaCall", i, i); SelectStaCall = i;
		spref.get("SelectStaGrid", i, i); SelectStaGrid = i;
		spref.get("SelectStaCity", i, i); SelectStaCity = i;

	}
}

