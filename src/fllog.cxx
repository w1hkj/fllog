// ----------------------------------------------------------------------------
//      fllog.cxx
//
// Copyright (C) 2012
//               Dave Freese, W1HKJ
//
// This is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// fldigi is distributed in the hope that it will be useful,
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
// ----------------------------------------------------------------------------
#include "config.h"

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/x.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Menu_Item.H>

#ifdef WIN32
#  include "fllogrc.h"
#  include "compat.h"
#  define dirent fl_dirent_no_thanks
#endif

#include <FL/filename.H>
#ifdef __MINGW32__
#	if FLLOG_FLTK_API_MAJOR == 1 && FLLOG_FLTK_API_MINOR < 3
#		undef dirent
#		include <dirent.h>
#	endif
#else
#	include <dirent.h>
#endif

#include <FL/x.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Image.H>

#include "support.h"
#include "status.h"
#include "debug.h"
#include "util.h"
#include "gettext.h"
#include "fllog_icon.cxx"
#include "fileselect.h"
#include "logbook.h"
#include "lgbook.h"
#include "colorsfonts.h"

#include "XmlRpcClient.h"

int parse_args(int argc, char **argv, int& idx);

Fl_Double_Window *mainwindow = NULL;
string LogHomeDir;
string TempDir;
string defFileName;
string title;

//pthread_t *serial_thread = 0;
//pthread_t *digi_thread = 0;

//pthread_mutex_t mutex_serial = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t mutex_xmlrpc = PTHREAD_MUTEX_INITIALIZER;

bool Log_DEBUG = 0;

//----------------------------------------------------------------------
// main dialog colors, fonts & sizing
//----------------------------------------------------------------------

inline void inp_font_pos(Fl_Input2* inp, int x, int y, int w, int h)
{
	inp->textsize(progStatus.LOGBOOKtextsize);
	inp->textfont(progStatus.LOGBOOKtextfont);
	inp->textcolor(progStatus.LOGBOOKtextcolor);
	inp->color(progStatus.LOGBOOKcolor);
	int ls = progStatus.LOGBOOKtextsize;
	inp->labelsize(ls < 14 ? ls : 14);
	inp->redraw_label();
	inp->resize(x, y, w, h);
}

inline void out_font_pos(Fl_Output* wid, int x, int y, int w, int h)
{
	wid->textsize(progStatus.LOGBOOKtextsize);
	wid->textfont(progStatus.LOGBOOKtextfont);
	wid->textcolor(progStatus.LOGBOOKtextcolor);
	wid->color(progStatus.LOGBOOKcolor);
	int ls = progStatus.LOGBOOKtextsize;
	wid->labelsize(ls < 14 ? ls : 14);
	wid->redraw_label();
	wid->resize(x, y, w, h);
}

inline void date_font_pos(Fl_DateInput* inp, int x, int y, int w, int h)
{
	inp->textsize(progStatus.LOGBOOKtextsize);
	inp->textfont(progStatus.LOGBOOKtextfont);
	inp->textcolor(progStatus.LOGBOOKtextcolor);
	inp->color(progStatus.LOGBOOKcolor);
	int ls = progStatus.LOGBOOKtextsize;
	inp->labelsize(ls < 14 ? ls : 14);
	inp->redraw_label();
	inp->resize(x, y, w, h);
}

void LOGBOOK_colors_font()
{
	if (!mainwindow) return;

// input / output / date / text fields
	fl_font(progStatus.LOGBOOKtextfont, progStatus.LOGBOOKtextsize);
	int wh = fl_height() + 6;
	int width_date = fl_width("20190828") + wh;
	int width_time = fl_width("23:59:599");
	int width_freq = fl_width("99.9999999");
	int width_rst  = fl_width("5999");
	int width_pwr  = fl_width("0000");
	int width_loc  = fl_width("XX88XXX");
	int width_mode = fl_width("CONTESTIA");

	int dlg_width =	inpDate_log->x() +
					width_date + 2 +
					wh + 2 +
					width_time + 2 +
					width_freq + 2 +
					width_mode + 2 +
					width_pwr + 2 +
					width_rst + 2;
//					+ width_date + 2;

	if (progStatus.mainW > dlg_width) {
		width_date = (int)(1.0 * width_date * progStatus.mainW / dlg_width);
		width_time = (int)(1.0 * width_time * progStatus.mainW / dlg_width);
		width_freq = (int)(1.0 * width_freq * progStatus.mainW / dlg_width);
		width_rst  = (int)(1.0 * width_rst * progStatus.mainW / dlg_width);
		width_pwr  = (int)(1.0 * width_pwr * progStatus.mainW / dlg_width);
		width_loc  = (int)(1.0 * width_loc * progStatus.mainW / dlg_width);
		width_mode = (int)(1.0 * width_mode * progStatus.mainW / dlg_width);
		width_freq = (progStatus.mainW -
						width_date - width_time - width_mode -
						width_pwr - width_rst - 14);
		dlg_width = progStatus.mainW;
	}

	int newheight = 24 + 4*(wh + 20) + 3*wh + 4 + bNewSave->h() + 4 + wBrowser->h() + 2;

	if (dlg_width > progStatus.mainW)
		progStatus.mainW = dlg_width;
	else
		dlg_width = progStatus.mainW;
	if (newheight > progStatus.mainH)
		progStatus.mainH = newheight;
	else
		newheight = progStatus.mainH;
	mainwindow->resize(progStatus.mainX, progStatus.mainY, progStatus.mainW, progStatus.mainH);

// row1
	int ypos = inpDate_log->y();
	int xpos = inpDate_log->x();
	int xwidth = width_date;

	date_font_pos(inpDate_log, xpos, ypos, xwidth, wh);

	xpos += width_date + 2;
	xwidth = width_time;
	inp_font_pos(inpTimeOn_log, xpos, ypos, width_time, wh);

	xpos += width_time + 2;
	btnSetTimeOn->resize(xpos, ypos, wh, wh);

	xpos += wh + 2;
	xwidth = width_freq;
	inp_font_pos(inpCall_log, xpos, ypos, width_freq, wh);

	xpos = dlg_width - 2 - width_rst;
	xwidth = width_rst;
	inp_font_pos(inpRstR_log, xpos, ypos, width_rst, wh);

	xpos = inpCall_log->x() + width_freq + 2;
	xwidth = inpRstR_log->x() - 2 - xpos;
	inp_font_pos(inpName_log, xpos, ypos, xwidth, wh);

// row2
	ypos += wh + 20;
	xpos = inpDateOff_log->x();

	date_font_pos(inpDateOff_log, xpos, ypos, width_date, wh);

	xpos += width_date + 2;
	inp_font_pos(inpTimeOff_log, xpos, ypos, width_time, wh);

	xpos += width_time + 2;
	btnSetTimeOff->resize(xpos, ypos, wh, wh);

	xpos += wh + 2;
	inp_font_pos(inpFreq_log, xpos, ypos, width_freq, wh);

	xpos = dlg_width - 2 - width_rst;
	inp_font_pos(inpRstS_log, xpos, ypos, width_rst, wh);

	xpos = inpRstS_log->x() - 2 - width_pwr;
	inp_font_pos(inpTX_pwr_log, xpos, ypos, width_pwr, wh);

	xpos = inpFreq_log->x() + width_freq + 2;
	xwidth = inpTX_pwr_log->x() - 2 - xpos;
	inp_font_pos(inpMode_log, xpos, ypos, xwidth, wh);

// row 3
	ypos += 20 + wh;
	xpos = dlg_width - 2 - width_loc;
	inp_font_pos(inpLoc_log, xpos, ypos, width_loc, wh);

	xpos = inpLoc_log->x() - 2 - inpCountry_log->w();
	inp_font_pos(inpCountry_log, xpos, ypos, inpCountry_log->w(), wh);

	xpos = inpCountry_log->x() - 2 - inpVE_Prov_log->w();
	inp_font_pos(inpVE_Prov_log, xpos, ypos, inpVE_Prov_log->w(), wh);

	xpos = inpVE_Prov_log->x() - 2 - inpState_log->w();
	inp_font_pos(inpState_log, xpos, ypos, inpState_log->w(), wh);

	xpos = inpQth_log->x();
	xwidth = inpState_log->x() - 2 - inpQth_log->x();
	inp_font_pos(inpQth_log, xpos, ypos, xwidth, wh);

	ypos += wh + 4;
	xpos = Tabs->x();
	Tabs->resize(xpos, ypos, Tabs->w(), Tabs->h());

	Fl_Input2* extras[] = {
		inpCNTY_log, inpIOTA_log, inpCQZ_log,
		inpCONT_log, inpITUZ_log, inpDXCC_log,
		inp_log_sta_call,
		inp_log_op_call, inp_log_sta_qth,
		inp_log_sta_loc, inpSerNoOut_log,
		inpMyXchg_log, inpSerNoIn_log,
		inpXchgIn_log, inpClass_log,
		inpSection_log, inpBand_log,
		inp_age_log, inp_1010_log, inp_check_log,
		inp_log_cwss_serno, inp_log_cwss_sec,
		inp_log_cwss_prec, inp_log_cwss_chk,
		inp_log_troop_s, inp_log_troop_r,
		inp_log_scout_s, inp_log_scout_r,
		inpSearchString

	};
	for (size_t i = 0; i < sizeof(extras)/sizeof(*extras); i++) {
		inp_font_pos(extras[i], extras[i]->x(), extras[i]->y(), extras[i]->w(), wh);
	}
	inp_font_pos(inpNotes_log, inpNotes_log->x(), inpNotes_log->y(),
				inpNotes_log->w(), inpNotes_log->h());

	date_font_pos(inpQSLrcvddate_log,
				  inpQSLrcvddate_log->x(), inpQSLrcvddate_log->y(),
				  width_date, wh);

	date_font_pos(inpQSLsentdate_log,
				  inpQSLsentdate_log->x(), inpQSLsentdate_log->y(),
				  width_date, wh);

	inpEQSLrcvddate_log->position(inpQSLrcvddate_log->x() + inpQSLrcvddate_log->w() + 2,
								  inpQSLrcvddate_log->y());
	inpEQSLsentdate_log->position(inpEQSLrcvddate_log->x(), inpEQSLsentdate_log->y());
	date_font_pos(inpEQSLrcvddate_log,
				  inpEQSLrcvddate_log->x(), inpEQSLrcvddate_log->y(),
				  width_date, wh);
	date_font_pos(inpEQSLsentdate_log,
				  inpEQSLsentdate_log->x(), inpEQSLsentdate_log->y(),
				  width_date, wh);

	inpLOTWrcvddate_log->position(inpEQSLrcvddate_log->x() + inpEQSLrcvddate_log->w() + 2,
								  inpLOTWrcvddate_log->y());
	inpLOTWsentdate_log->position(inpLOTWrcvddate_log->x(), inpLOTWsentdate_log->y());
	date_font_pos(inpLOTWrcvddate_log,
				  inpLOTWrcvddate_log->x(), inpLOTWrcvddate_log->y(),
				  width_date, wh);
	date_font_pos(inpLOTWsentdate_log,
				  inpLOTWsentdate_log->x(), inpLOTWsentdate_log->y(),
				  width_date, wh);

	int via_x = inpLOTWrcvddate_log->x() + inpLOTWrcvddate_log->w() + 2;
	int via_y = inpQSL_VIA_log->y();
	int via_w = Tabs->w() + Tabs->x() - via_x;
	int via_h = inpQSL_VIA_log->h();

	inpQSL_VIA_log->resize( via_x, via_y, via_w, via_h);
	inpQSL_VIA_log->textsize(progStatus.LOGBOOKtextsize);
	inpQSL_VIA_log->textfont(progStatus.LOGBOOKtextfont);
	inpQSL_VIA_log->textcolor(progStatus.LOGBOOKtextcolor);
	inpQSL_VIA_log->color(progStatus.LOGBOOKcolor);

	int srchwidth = dlg_width - 8 - Tabs->w();
	int srchx = dlg_width - srchwidth - 2;

	inp_font_pos(txtNbrRecs_log, srchx, ypos + 22, srchwidth, wh);

	int srchy = Tabs->y() + Tabs->h() - 2* wh - 2;
	inp_font_pos(inpSearchString, srchx, srchy, srchwidth, wh);

	int srchbtnwidth = (srchwidth - 2) / 2;
	srchy += wh + 2;
	bSearchPrev->resize( srchx, srchy, srchbtnwidth, wh);
	bSearchNext->resize( srchx + 2 + srchbtnwidth, srchy, srchbtnwidth, wh);

	ypos = Tabs->y() + Tabs->h() + 4;

	out_font_pos(txtLogFile, txtLogFile->x(), ypos, txtLogFile->w(), txtLogFile->h());

	Fl_Button* btns[] = { bNewSave, bUpdateCancel, bDelete };
	for (size_t i = 0; i < sizeof(btns)/sizeof(*btns); i++) {
		btns[i]->resize(btns[i]->x(), ypos, btns[i]->w(), btns[i]->h());
		btns[i]->redraw();
	}

// browser (table)
	ypos += btns[0]->h() + 4;

	wBrowser->rowSize(wh-6); // this is going to determine the font size used
	wBrowser->headerSize(wh-6);
	wBrowser->font(progStatus.LOGBOOKtextfont);
	wBrowser->color(progStatus.LOGBOOKcolor);
	wBrowser->selection_color(FL_SELECTION_COLOR);
	wBrowser->allowHscroll(never);

	browser_group->resize(
		browser_group->x(), ypos, 
		dlg_width - 2*browser_group->x(), mainwindow->h() - 2 - ypos);

	mainwindow->init_sizes();
	mainwindow->damage();
	mainwindow->redraw();

	debug::font(progStatus.LOGBOOKtextfont);
	debug::font_size(progStatus.LOGBOOKtextsize);
}

void setConfigItems()
{
	if (!dlgConfigItems) make_config_items();
	dlgConfigItems->show();
}

void setDefaultExport()
{
}

void DefaultExport()
{
	if (Fl::event_button() == FL_RIGHT_MOUSE) {
		progStatus.SelectCall = btnSelectCall->value();
		progStatus.SelectName = btnSelectName->value();
		progStatus.SelectFreq = btnSelectFreq->value();
		progStatus.SelectBand = btnSelectBand->value();
		progStatus.SelectMode = btnSelectMode->value();
		progStatus.SelectQSOdateOn = btnSelectQSOdateOn->value();
		progStatus.SelectQSOdateOff = btnSelectQSOdateOff->value();
		progStatus.SelectTimeON = btnSelectTimeON->value();
		progStatus.SelectTimeOFF = btnSelectTimeOFF->value();
		progStatus.SelectQth = btnSelectQth->value();
		progStatus.SelectLOC = btnSelectLOC->value();
		progStatus.SelectState = btnSelectState->value();
		progStatus.SelectProvince = btnSelectProvince->value();
		progStatus.SelectCountry = btnSelectCountry->value();
		progStatus.SelectQSLrcvd = btnSelectQSLrcvd->value();
		progStatus.SelectQSLsent = btnSelectQSLsent->value();
		progStatus.SelectSerialIN = btnSelectSerialIN->value();
		progStatus.SelectSerialOUT = btnSelectSerialOUT->value();
		progStatus.SelectXchgIn = btnSelectXchgIn->value();
		progStatus.SelectRSTsent = btnSelectRSTsent->value();
		progStatus.SelectRSTrcvd = btnSelectRSTrcvd->value();
		progStatus.SelectIOTA = btnSelectIOTA->value();
		progStatus.SelectDXCC = btnSelectDXCC->value();
		progStatus.SelectCNTY = btnSelectCNTY->value();
		progStatus.SelectCONT = btnSelectCONT->value();
		progStatus.SelectCQZ = btnSelectCQZ->value();
		progStatus.SelectITUZ = btnSelectITUZ->value();
		progStatus.SelectTX_pwr = btnSelectTX_pwr->value();
		progStatus.SelectNotes = btnSelectNotes->value();
		progStatus.SelectQSL_VIA = btnSelectQSL_VIA->value();
		progStatus.SelectOperator = btnSelectOperator->value();
		progStatus.SelectStaCall = btnSelectStaCall->value();
		progStatus.SelectStaGrid = btnSelectStaGrid->value();
		progStatus.SelectStaCity = btnSelectStaCity->value();

		progStatus.SelectClass = btnSelectClass->value();
		progStatus.SelectSection = btnSelectSection->value();

		progStatus.Select_cwss_serno = btnSelect_cwss_serno->value();
		progStatus.Select_cwss_prec = btnSelect_cwss_prec->value();
		progStatus.Select_cwss_check = btnSelect_cwss_check->value();
		progStatus.Select_cwss_section = btnSelect_cwss_section->value();

		progStatus.SelectJOTA = btnSelectJOTA->value();
	} else {
		btnSelectCall->value(progStatus.SelectCall);
		btnSelectName->value(progStatus.SelectName);
		btnSelectFreq->value(progStatus.SelectFreq);
		btnSelectBand->value(progStatus.SelectBand);
		btnSelectMode->value(progStatus.SelectMode);
		btnSelectQSOdateOn->value(progStatus.SelectQSOdateOn);
		btnSelectQSOdateOff->value(progStatus.SelectQSOdateOff);
		btnSelectTimeON->value(progStatus.SelectTimeON);
		btnSelectTimeOFF->value(progStatus.SelectTimeOFF);
		btnSelectQth->value(progStatus.SelectQth);
		btnSelectLOC->value(progStatus.SelectLOC);
		btnSelectState->value(progStatus.SelectState);
		btnSelectProvince->value(progStatus.SelectProvince);
		btnSelectCountry->value(progStatus.SelectCountry);
		btnSelectQSLrcvd->value(progStatus.SelectQSLrcvd);
		btnSelectQSLsent->value(progStatus.SelectQSLsent);
		btnSelectSerialIN->value(progStatus.SelectSerialIN);
		btnSelectSerialOUT->value(progStatus.SelectSerialOUT);
		btnSelectXchgIn->value(progStatus.SelectXchgIn);
		btnSelectRSTsent->value(progStatus.SelectRSTsent);
		btnSelectRSTrcvd->value(progStatus.SelectRSTrcvd);
		btnSelectIOTA->value(progStatus.SelectIOTA);
		btnSelectDXCC->value(progStatus.SelectDXCC);
		btnSelectCNTY->value(progStatus.SelectCNTY);
		btnSelectCONT->value(progStatus.SelectCONT);
		btnSelectCQZ->value(progStatus.SelectCQZ);
		btnSelectITUZ->value(progStatus.SelectITUZ);
		btnSelectTX_pwr->value(progStatus.SelectTX_pwr);
		btnSelectNotes->value(progStatus.SelectNotes);
		btnSelectQSL_VIA->value(progStatus.SelectQSL_VIA);
		btnSelectOperator->value(progStatus.SelectOperator);
		btnSelectStaCall->value(progStatus.SelectStaCall);
		btnSelectStaGrid->value(progStatus.SelectStaGrid);
		btnSelectStaCity->value(progStatus.SelectStaCity);

		btnSelectClass->value(progStatus.SelectClass);
		btnSelectSection->value(progStatus.SelectSection);

		btnSelect_cwss_serno->value(progStatus.Select_cwss_serno);
		btnSelect_cwss_prec->value(progStatus.Select_cwss_prec);
		btnSelect_cwss_check->value(progStatus.Select_cwss_check);
		btnSelect_cwss_section->value(progStatus.Select_cwss_section);

		btnSelectJOTA->value(progStatus.SelectJOTA);
	}
}

//----------------------------------------------------------------------

const char *server_addr = "127.0.0.1";

//----------------------------------------------------------------------
void visit_URL(void* arg)
{
	const char* url = reinterpret_cast<const char *>(arg);
#ifndef __WOE32__
	const char* browsers[] = {
#  ifdef __APPLE__
		getenv("FLDIGI_BROWSER"), // valid for any OS - set by user
		"open"                    // OS X
#  else
		"fl-xdg-open",            // Puppy Linux
		"xdg-open",               // other Unix-Linux distros
		getenv("FLDIGI_BROWSER"), // force use of spec'd browser
		getenv("BROWSER"),        // most Linux distributions
		"sensible-browser",
		"firefox",
		"mozilla"                 // must be something out there!
#  endif
	};
	switch (fork()) {
	case 0:
#  ifndef NDEBUG
		unsetenv("MALLOC_CHECK_");
		unsetenv("MALLOC_PERTURB_");
#  endif
		for (size_t i = 0; i < sizeof(browsers)/sizeof(browsers[0]); i++)
			if (browsers[i])
				execlp(browsers[i], browsers[i], url, (char*)0);
		exit(EXIT_FAILURE);
	case -1:
		fl_alert(_("Could not run a web browser:\n%s\n\n"
			 "Open this URL manually:\n%s"),
			 strerror(errno), url);
	}
#else
	if ((int)ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL) <= 32)
		fl_alert(_("Could not open url:\n%s\n"), url);
#endif
}

void about()
{
	string msg = "\
%s\n\
Version %s\n\
copyright W1HKJ, 2009-11\n\
w1hkj@@w1hkj.com";
	fl_message(msg.c_str(), PACKAGE_TARNAME, PACKAGE_VERSION);
}

void on_line_help()
{
	visit_URL((void *)"http://www.w1hkj.com/fllog-help");
}

//----------------------------------------------------------------------

void fllog_terminate() {
	std::cerr << "terminating" << std::endl;
	fl_message("Closing fllog");
	cbExit();
}

void showEvents(void *)
{
	debug::show();
}


#if defined(__WIN32__) && defined(PTW32_STATIC_LIB)
static void ptw32_cleanup(void)
{
	(void)pthread_win32_process_detach_np();
}

void ptw32_init(void)
{
	(void)pthread_win32_process_attach_np();
	atexit(ptw32_cleanup);
}
#endif // __WIN32__

#define KNAME "fllog"
#if !defined(__WIN32__) && !defined(__APPLE__)
Pixmap  Log_icon_pixmap;

void make_pixmap(Pixmap *xpm, const char **data)
{
	Fl_Window w(0,0, KNAME);
	w.xclass(KNAME);
	w.show();
	w.make_current();
	Fl_Pixmap icon(data);
	int maxd = (icon.w() > icon.h()) ? icon.w() : icon.h();
	*xpm = fl_create_offscreen(maxd, maxd);
	fl_begin_offscreen(*xpm);
	fl_color(FL_BACKGROUND_COLOR);
	fl_rectf(0, 0, maxd, maxd);
	icon.draw(maxd - icon.w(), maxd - icon.h());
	fl_end_offscreen();
}

#endif

static void checkdirectories(void)
{
	struct {
		string& dir;
		const char* suffix;
		void (*new_dir_func)(void);
	} dirs[] = {
		{ LogHomeDir, 0, 0 }
	};

	int r;
	for (size_t i = 0; i < sizeof(dirs)/sizeof(*dirs); i++) {
		if (dirs[i].suffix)
			dirs[i].dir.assign(LogHomeDir).append(dirs[i].suffix).append("/");

		if ((r = mkdir(dirs[i].dir.c_str(), 0777)) == -1 && errno != EEXIST) {
			cerr << _("Could not make directory") << ' ' << dirs[i].dir
				 << ": " << strerror(errno) << '\n';
			exit(EXIT_FAILURE);
		}
		else if (r == 0 && dirs[i].new_dir_func)
			dirs[i].new_dir_func();
	}
}

void exit_main(Fl_Widget *w)
{
	if (Fl::event_key() == FL_Escape)
		return;
	cbExit();
}

static std::string pname = "";

int main (int argc, char *argv[])
{
	pname = argv[0];
	size_t pn = pname.rfind("/");
	if (pn != std::string::npos) pname.erase(0, pn + 1);
	pn = pname.rfind("\\");
	if (pn != std::string::npos) pname.erase(0, pn + 1);
	XmlRpc::set_pname(pname);

	Fl::visual(FL_RGB); // insure 24 bit color operation

	Fl::set_fonts(0);

	std::set_terminate(fllog_terminate);

	int arg_idx;

	char dirbuf[FL_PATH_MAX + 1];
#ifdef __WIN32__
	fl_filename_expand(dirbuf, sizeof(dirbuf) - 1, "$USERPROFILE/fllog.files/");
#else
	fl_filename_expand(dirbuf, sizeof(dirbuf) - 1, "$HOME/.fllog/");
#endif
	LogHomeDir = dirbuf;
	Fl::args(argc, argv, arg_idx, parse_args);

	create_logbook_dialogs();

	mainwindow = dlgLogbook;

	mainwindow->callback(exit_main);

	checkdirectories();

	try {
		debug::start(string(LogHomeDir).append("status_log.txt").c_str());
		time_t t = time(NULL);
		LOG(debug::INFO_LEVEL, debug::LOG_OTHER, _("%s log started on \n%s"), PACKAGE_STRING, ctime(&t));
	}
	catch (const char* error) {
		cerr << error << '\n';
		debug::stop();
		exit(1);
	}

	start_server(atoi(progStatus.server_port.c_str()));

	Fl::lock();

#if defined(__WIN32__) && defined(PTW32_STATIC_LIB)
	ptw32_init();
#endif

	progStatus.loadLastState();

	LOGBOOK_colors_font();

	mainwindow->xclass(KNAME);

	Fl::add_handler(main_handler);

#if defined(__WOE32__)
#  ifndef IDI_ICON
#    define IDI_ICON 101
#  endif
	mainwindow->icon((char*)LoadIcon(fl_display, MAKEINTRESOURCE(IDI_ICON)));
	mainwindow->show (argc, argv);
#elif !defined(__APPLE__)
	make_pixmap(&Log_icon_pixmap, fllog_icon);
	mainwindow->icon((char *)Log_icon_pixmap);
	mainwindow->show(argc, argv);
#else
	mainwindow->show(argc, argv);
#endif

	debug::resize(mainwindow->x(), mainwindow->y() + mainwindow->h() + 26,
				  mainwindow->w(), 200);

	if (progStatus.event_log_open) debug::show();

	start_logbook();

	FSEL::create();

	return Fl::run();

}

int parse_args(int argc, char **argv, int& idx)
{
	if (strcasecmp("--help", argv[1]) == 0) {
		printf("Usage: \n\
  --help this help text\n\
  --version\n\
  --config-dir [pathname]\n\
  --debug\n");
		exit(0);
	}
	if (strcasecmp("--version", argv[1]) == 0) {
		printf("Version: " VERSION "\n");
		exit (0);
	}
	if (strcasecmp("--debug", argv[1]) == 0) {
		Log_DEBUG = 1;
		idx++;
		return 1;
	}
	if (strcasecmp("--config-dir", argv[1]) == 0) {
		LogHomeDir = argv[2];
		if (LogHomeDir[LogHomeDir.length() -1] != '/')
			LogHomeDir += '/';
		idx += 2;
		return 1;
	}
	return 0;
}
