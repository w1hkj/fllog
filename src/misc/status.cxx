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
	"",						// string my_staqth;
	"",						// string my_staloc;

	SORTDATE,
	true,
	true,
	true,
	true,
	"8421",

	1,	//int  SelectCall;
	1,	//int  SelectName;
	1,	//int  SelectFreq;
	1,	//int  SelectBand;
	1,	//int  SelectMode;
	1,	//int  SelectQSOdateOn;
	1,	//int  SelectQSOdateOff;
	1,	//int  SelectTimeON;
	1,	//int  SelectTimeOFF;
	0,	//int  SelectTX_pwr;
	0,	//int  SelectRSTsent;
	1,	//int  SelectRSTrcvd;
	0,	//int  SelectQth;
	0,	//int  SelectLOC;
	0,	//int  SelectState;
	0,	//int  SelectAge;

	0,	//int  SelectStaCall;
	0,	//int  SelectStaGrid;
	0,	//int  SelectStaCity;
	0,	//int  SelectOperator;
	0,	//int  SelectProvince;
	0,	//int  SelectCountry;
	0,	//int  SelectNotes;
	0,	//int  SelectQSLrcvd;
	0,	//int  SelectQSLsent;
	0,	//int  SelecteQSLrcvd;
	0,	//int  SelecteQSLsent;
	0,	//int  SelectLOTWrcvd;
	0,	//int  SelectLOTWsent;
	0,	//int  SelectQSL_VIA
	0,	//int  SelectSerialIN;
	0,	//int  SelectSerialOUT;

	0,	//int  SelectCheck;

	0,	//int  SelectXchgIn;
	0,	//int  SelectMyXchg;

	0,	//int  SelectCNTY;
	0,	//int  SelectCONT;
	0,	//int  SelectCQZ;
	0,	//int  SelectDXCC;
	0,	//int  SelectIOTA;
	0,	//int  SelectITUZ;

	0,	//int  SelectClass;
	0,	//int  SelectSection;

	0,	//int  Select_cwss_serno;
	0,	//int  Select_cwss_prec;
	0,	//int  Select_cwss_check;
	0,	//int  Select_cwss_section;

	0,	//int  Select_1010;
	0,	//int  SelectJOTA;

	1,	//int  use_nulines;
	1	//int  event_log_open;
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
	spref.set("my_staqth", my_staqth.c_str());
	spref.set("my_staloc", my_staloc.c_str());

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
	spref.set("SelectTX_pwr", SelectTX_pwr);
	spref.set("SelectRSTsent", SelectRSTsent);
	spref.set("SelectRSTrcvd", SelectRSTrcvd);
	spref.set("SelectQth", SelectQth);
	spref.set("SelectLOC", SelectLOC);
	spref.set("SelectState", SelectState);
	spref.set("SelectAge", SelectAge);

	spref.set("SelectStaCall", SelectStaCall);
	spref.set("SelectStaGrid", SelectStaGrid);
	spref.set("SelectStaCity", SelectStaCity);
	spref.set("SelectOperator", SelectOperator);
	spref.set("SelectProvince", SelectProvince);
	spref.set("SelectCountry", SelectCountry);
	spref.set("SelectNotes", SelectNotes);
	spref.set("SelectQSLrcvd", SelectQSLrcvd);
	spref.set("SelectQSLsent", SelectQSLsent);
	spref.set("SelecteQSLrcvd", SelecteQSLrcvd);
	spref.set("SelecteQSLsent", SelecteQSLsent);
	spref.set("SelectLOTWrcvd", SelectLOTWrcvd);
	spref.set("SelectLOTWsent", SelectLOTWsent);
	spref.set("SelectQSL_VIA", SelectQSL_VIA);
	spref.set("SelectSerialIN", SelectSerialIN);
	spref.set("SelectSerialOUT", SelectSerialOUT);

	spref.set("SelectCheck", SelectCheck);

	spref.set("SelectXchgIn", SelectXchgIn);
	spref.set("SelectMyXchg", SelectMyXchg);

	spref.set("SelectCNTY", SelectCNTY);
	spref.set("SelectCONT", SelectCONT);
	spref.set("SelectCQZ", SelectCQZ);
	spref.set("SelectDXCC", SelectDXCC);
	spref.set("SelectIOTA", SelectIOTA);
	spref.set("SelectITUZ", SelectITUZ);

	spref.set("SelectClass", SelectClass);
	spref.set("SelectSection", SelectSection);

	spref.set("Select_cwss_serno", Select_cwss_serno);
	spref.set("Select_cwss_prec", Select_cwss_prec);
	spref.set("Select_cwss_check", Select_cwss_check);
	spref.set("Select_cwss_section", Select_cwss_section);

	spref.set("Select_1010", Select_1010);
	spref.set("SelectJOTA", SelectJOTA);

	spref.set("use_nulines", use_nulines);

	int visible = debug_visible();
	spref.set("event_log_open", visible);
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

		spref.get("my_staqth", defbuffer, my_staqth.c_str(), 199);
		my_staqth = defbuffer;

		spref.get("my_staloc", defbuffer, my_staloc.c_str(), 199);
		my_staloc = defbuffer;

		if (spref.get("lastsort", i, lastsort)) lastsort = (sorttype) i;
		if (spref.get("callfwd", i, callfwd)) callfwd = i;
		if (spref.get("datefwd", i, datefwd)) datefwd = i;
		if (spref.get("modefwd", i, modefwd)) modefwd = i;
		if (spref.get("freqfwd", i, freqfwd)) freqfwd = i;

		spref.get("server_port", defbuffer, server_port.c_str(), 199); server_port = defbuffer;

	spref.get("SelectCall", SelectCall, SelectCall);
	spref.get("SelectName", SelectName, SelectName);
	spref.get("SelectFreq", SelectFreq, SelectFreq);
	spref.get("SelectBand", SelectBand, SelectBand);
	spref.get("SelectMode", SelectMode, SelectMode);
	spref.get("SelectQSOdateOn", SelectQSOdateOn, SelectQSOdateOn);
	spref.get("SelectQSOdateOff", SelectQSOdateOff, SelectQSOdateOff);
	spref.get("SelectTimeON", SelectTimeON, SelectTimeON);
	spref.get("SelectTimeOFF", SelectTimeOFF, SelectTimeOFF);
	spref.get("SelectTX_pwr", SelectTX_pwr, SelectTX_pwr);
	spref.get("SelectRSTsent", SelectRSTsent, SelectRSTsent);
	spref.get("SelectRSTrcvd", SelectRSTrcvd, SelectRSTrcvd);
	spref.get("SelectQth", SelectQth, SelectQth);
	spref.get("SelectLOC", SelectLOC, SelectLOC);
	spref.get("SelectState", SelectState, SelectState);
	spref.get("SelectAge", SelectAge, SelectAge);

	spref.get("SelectStaCall", SelectStaCall, SelectStaCall);
	spref.get("SelectStaGrid", SelectStaGrid, SelectStaGrid);
	spref.get("SelectStaCity", SelectStaCity, SelectStaCity);
	spref.get("SelectOperator", SelectOperator, SelectOperator);
	spref.get("SelectProvince", SelectProvince, SelectProvince);
	spref.get("SelectCountry", SelectCountry, SelectCountry);
	spref.get("SelectNotes", SelectNotes, SelectNotes);
	spref.get("SelectQSLrcvd", SelectQSLrcvd, SelectQSLrcvd);
	spref.get("SelectQSLsent", SelectQSLsent, SelectQSLsent);
	spref.get("SelecteQSLrcvd", SelecteQSLrcvd, SelecteQSLrcvd);
	spref.get("SelecteQSLsent", SelecteQSLsent, SelecteQSLsent);
	spref.get("SelectLOTWrcvd", SelectLOTWrcvd, SelectLOTWrcvd);
	spref.get("SelectLOTWsent", SelectLOTWsent, SelectLOTWsent);
	spref.get("SelectQSL_VIA", SelectQSL_VIA, SelectQSL_VIA);
	spref.get("SelectSerialIN", SelectSerialIN, SelectSerialIN);
	spref.get("SelectSerialOUT", SelectSerialOUT, SelectSerialOUT);

	spref.get("SelectCheck", SelectCheck, SelectCheck);

	spref.get("SelectXchgIn", SelectXchgIn, SelectXchgIn);
	spref.get("SelectMyXchg", SelectMyXchg, SelectMyXchg);

	spref.get("SelectCNTY", SelectCNTY, SelectCNTY);
	spref.get("SelectCONT", SelectCONT, SelectCONT);
	spref.get("SelectCQZ", SelectCQZ, SelectCQZ);
	spref.get("SelectDXCC", SelectDXCC, SelectDXCC);
	spref.get("SelectIOTA", SelectIOTA, SelectIOTA);
	spref.get("SelectITUZ", SelectITUZ, SelectITUZ);

	spref.get("SelectClass", SelectClass, SelectClass);
	spref.get("SelectSection", SelectSection, SelectSection);

	spref.get("Select_cwss_serno", Select_cwss_serno, Select_cwss_serno);
	spref.get("Select_cwss_prec", Select_cwss_prec, Select_cwss_prec);
	spref.get("Select_cwss_check", Select_cwss_check, Select_cwss_check);
	spref.get("Select_cwss_section", Select_cwss_section, Select_cwss_section);

	spref.get("Select_1010", Select_1010, Select_1010);
	spref.get("SelectJOTA", SelectJOTA, SelectJOTA);

	spref.get("use_nulines", use_nulines, use_nulines);

	spref.get("event_log_open", event_log_open, event_log_open);

	}
}

void status::save_defaults()
{
	SelectCall = btnSelectCall->value();
	SelectName = btnSelectName->value();
	SelectFreq = btnSelectFreq->value();
	SelectBand = btnSelectBand->value();
	SelectMode = btnSelectMode->value();
	SelectQSOdateOn = btnSelectQSOdateOn->value();
	SelectQSOdateOff = btnSelectQSOdateOff->value();
	SelectTimeON = btnSelectTimeON->value();
	SelectTimeOFF = btnSelectTimeOFF->value();
	SelectTX_pwr = btnSelectTX_pwr->value();
	SelectRSTsent = btnSelectRSTsent->value();
	SelectRSTrcvd = btnSelectRSTrcvd->value();
	SelectQth = btnSelectQth->value();
	SelectLOC = btnSelectLOC->value();
	SelectState = btnSelectState->value();
	SelectAge = btnSelectAge->value();

	SelectStaCall = btnSelectStaCall->value();
	SelectStaGrid = btnSelectStaGrid->value();
	SelectStaCity = btnSelectStaCity->value();
	SelectOperator = btnSelectOperator->value();
	SelectProvince = btnSelectProvince->value();
	SelectCountry = btnSelectCountry->value();
	SelectNotes = btnSelectNotes->value();
	SelectQSLrcvd = btnSelectQSLrcvd->value();
	SelectQSLsent = btnSelectQSLsent->value();
	SelecteQSLrcvd = btnSelecteQSLrcvd->value();
	SelecteQSLsent = btnSelecteQSLsent->value();
	SelectLOTWrcvd = btnSelectLOTWrcvd->value();
	SelectLOTWsent = btnSelectLOTWsent->value();
	SelectQSL_VIA = btnSelectQSL_VIA->value();
	SelectSerialIN = btnSelectSerialIN->value();
	SelectSerialOUT = btnSelectSerialOUT->value();

	SelectCheck = btnSelectCheck->value();

	SelectXchgIn = btnSelectXchgIn->value();
	SelectMyXchg = btnSelectMyXchg->value();

	SelectCNTY = btnSelectCNTY->value();
	SelectCONT = btnSelectCONT->value();
	SelectCQZ = btnSelectCQZ->value();
	SelectDXCC = btnSelectDXCC->value();
	SelectIOTA = btnSelectIOTA->value();
	SelectITUZ = btnSelectITUZ->value();

	SelectClass = btnSelectClass->value();
	SelectSection = btnSelectSection->value();

	Select_cwss_serno = btnSelect_cwss_serno->value();
	Select_cwss_prec = btnSelect_cwss_prec->value();
	Select_cwss_check = btnSelect_cwss_check->value();
	Select_cwss_section = btnSelect_cwss_section->value();

	Select_1010 = btnSelect_1010->value();
	SelectJOTA = btnSelectJOTA->value();

}

void status::load_defaults()
{
	btnSelectCall->value(SelectCall);
	btnSelectName->value(SelectName);
	btnSelectFreq->value(SelectFreq);
	btnSelectBand->value(SelectBand);
	btnSelectMode->value(SelectMode);
	btnSelectQSOdateOn->value(SelectQSOdateOn);
	btnSelectQSOdateOff->value(SelectQSOdateOff);
	btnSelectTimeON->value(SelectTimeON);
	btnSelectTimeOFF->value(SelectTimeOFF);
	btnSelectTX_pwr->value(SelectTX_pwr);
	btnSelectRSTsent->value(SelectRSTsent);
	btnSelectRSTrcvd->value(SelectRSTrcvd);
	btnSelectQth->value(SelectQth);
	btnSelectLOC->value(SelectLOC);
	btnSelectState->value(SelectState);
	btnSelectAge->value(SelectAge);

	btnSelectStaCall->value(SelectStaCall);
	btnSelectStaGrid->value(SelectStaGrid);
	btnSelectStaCity->value(SelectStaCity);
	btnSelectOperator->value(SelectOperator);
	btnSelectProvince->value(SelectProvince);
	btnSelectCountry->value(SelectCountry);
	btnSelectNotes->value(SelectNotes);
	btnSelectQSLrcvd->value(SelectQSLrcvd);
	btnSelectQSLsent->value(SelectQSLsent);
	btnSelecteQSLrcvd->value(SelecteQSLrcvd);
	btnSelecteQSLsent->value(SelecteQSLsent);
	btnSelectLOTWrcvd->value(SelectLOTWrcvd);
	btnSelectLOTWsent->value(SelectLOTWsent);
	btnSelectQSL_VIA->value(SelectQSL_VIA);
	btnSelectSerialIN->value(SelectSerialIN);
	btnSelectSerialOUT->value(SelectSerialOUT);

	btnSelectCheck->value(SelectCheck);

	btnSelectXchgIn->value(SelectXchgIn);
	btnSelectMyXchg->value(SelectMyXchg);

	btnSelectCNTY->value(SelectCNTY);
	btnSelectCONT->value(SelectCONT);
	btnSelectCQZ->value(SelectCQZ);
	btnSelectDXCC->value(SelectDXCC);
	btnSelectIOTA->value(SelectIOTA);
	btnSelectITUZ->value(SelectITUZ);

	btnSelectClass->value(SelectClass);
	btnSelectSection->value(SelectSection);

	btnSelect_cwss_serno->value(Select_cwss_serno);
	btnSelect_cwss_prec->value(Select_cwss_prec);
	btnSelect_cwss_check->value(Select_cwss_check);
	btnSelect_cwss_section->value(Select_cwss_section);

	btnSelect_1010->value(Select_1010);
	btnSelectJOTA->value(SelectJOTA);
}
