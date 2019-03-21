// ---------------------------------------------------------------------
//
//      logsupport.cxx
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

#include <config.h>

#include <string>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#include <FL/Fl.H>
#include <FL/filename.H>
#include <FL/fl_ask.H>

#include "debug.h"
#include "status.h"
#include "date.h"
#include "adif_io.h"
#include "logbook.h"
#include "textio.h"
#include "lgbook.h"
#include "globals.h"
#include "logger.h"
#include "fileselect.h"
#include "icons.h"
#include "gettext.h"
#include "timeops.h"

using namespace std;

cQsoDb		qsodb;
cAdifIO		adifFile;
cTextFile	txtFile;

string		logbook_filename;

// convert to and from "00:00:00" <=> "000000"
const char *timeview(const char *s)
{
	static char ds[9];
	int len = strlen(s);
	strcpy(ds, "00:00:00");
	if (len < 4)
		return ds;
	ds[0] = s[0]; ds[1] = s[1];
	ds[3] = s[2]; ds[4] = s[3];
	if (len < 6)
		return ds;
	ds[6] = s[4];
	ds[7] = s[5];
	return ds;
}

const char *timestring(const char *s)
{
	static char ds[7];
	int len = strlen(s);
	if (len <= 4) return s;
	ds[0] = s[0]; ds[1] = s[1];
	ds[2] = s[3]; ds[3] = s[4];
	if (len < 8) {
		ds[4] = ds[5] = '0';
		ds[6] = 0;
		return ds;
	}
	ds[4] = s[6];
	ds[5] = s[7];
	ds[6] = 0;
	return ds;
}

const char *timeview4(const char *s)
{
	static char ds[6];
	int len = strlen(s);
	strcpy(ds, "00:00");
	if (len < 4)
		return ds;
	ds[0] = s[0]; ds[1] = s[1];
	ds[3] = s[2]; ds[4] = s[3];
	return ds;
}

const char *time4(const char *s)
{
	static char ds[5];
	int len = strlen(s);
	memset(ds, 0, 5);
	if (len < 5)
		return ds;
	ds[0] = s[0]; ds[1] = s[1];
	ds[2] = s[2]; ds[3] = s[3];
	return ds;
}

char *szTime(int typ)
{
	static char szDt[80];
	time_t tmptr;
	tm sTime;
	time (&tmptr);
	switch (typ) {
		case 0:
			localtime_r(&tmptr, &sTime);
			strftime(szDt, 79, "%H%M", &sTime);
			break;
		case 1:
			localtime_r(&tmptr, &sTime);
			strftime(szDt, 79, "%H:%M", &sTime);
			break;
		case 2:
			gmtime_r (&tmptr, &sTime);
			strftime(szDt, 79, "%H%MZ", &sTime);
			break;
		case 3:
			gmtime_r (&tmptr, &sTime);
			strftime(szDt, 79, "%H:%MZ", &sTime);
			break;
		case 4:
			gmtime_r (&tmptr, &sTime);
			strftime(szDt, 79, "%H%M UTC", &sTime);
			break;
		case 5:
			gmtime_r (&tmptr, &sTime);
			strftime(szDt, 79, "%H:%M UTC", &sTime);
			break;
		default:
			localtime_r(&tmptr, &sTime);
			strftime(szDt, 79, "%H%ML", &sTime);
	}
	return szDt;
}

static const char *month_name[] =
{
  "January",
  "Febuary",
  "March",
  "April",
  "May",
  "June",
  "July",
  "August",
  "September",
  "October",
  "November",
  "December"
};

char *szDate(int fmt)
{
	static char szDt[20];
	static char szMonth[10];

	time_t tmptr;
	tm sTime;
	time (&tmptr);
	if ((fmt & 0x80) == 0x80) {
		gmtime_r (&tmptr, &sTime);
	} else {
		localtime_r(&tmptr, &sTime);
	}
	switch (fmt & 0x7F) {
		case 1 :
			snprintf (szDt, sizeof(szDt), "%02d/%02d/%02d",
				sTime.tm_mon + 1,
				sTime.tm_mday,
				sTime.tm_year > 100 ? sTime.tm_year - 100 : sTime.tm_year);
			break;
		case 2 :
			snprintf (szDt, sizeof(szDt), "%4d-%02d-%02d",
				sTime.tm_year + 1900,
				sTime.tm_mon + 1,
				sTime.tm_mday);
			break;
		case 3 :
			snprintf (szDt, sizeof(szDt), "%s %2d, %4d",
				month_name[sTime.tm_mon],
				sTime.tm_mday,
				sTime.tm_year + 1900);
			break;
		case 4 :
			strcpy (szMonth, month_name [sTime.tm_mon]);
			szMonth[3] = 0;
			snprintf (szDt, sizeof(szDt), "%s %2d, %4d",
				szMonth,
				sTime.tm_mday,
				sTime.tm_year + 1900);
			break;
		case 5 :
			strcpy (szMonth, month_name [sTime.tm_mon]);
			szMonth[3] = 0;
			for (int i = 0; i < 3; i++) szMonth[i] = toupper(szMonth[i]);
			snprintf (szDt, sizeof(szDt), "%s %d",
				szMonth,
				sTime.tm_mday);
			break;
		case 6 :
			snprintf (szDt, sizeof(szDt), "%4d%02d%02d",
				sTime.tm_year + 1900,
				sTime.tm_mon + 1,
				sTime.tm_mday);
			break;
		case 0 :
		default :
			snprintf (szDt, sizeof(szDt), "%02d/%02d/%04d",
				sTime.tm_mon + 1,
				sTime.tm_mday,
				sTime.tm_year + 1900);
		break;
	}
	return szDt;
}

void Export_CSV()
{
	if (chkExportBrowser->nchecked() == 0) return;

	cQsoRec *rec;

	string title = _("Export to CSV file");
	string filters = "CSV\t*.csv";
#ifdef __APPLE__
	filters.append("\n");
#endif
	const char* p = FSEL::saveas( title.c_str(), filters.c_str(), "export.csv");

	if (!p) return;
	if (!*p) return;

	for (int i = 0; i < chkExportBrowser->nitems(); i++) {
		if (chkExportBrowser->checked(i + 1)) {
			rec = qsodb.getRec(i);
			rec->putField(EXPORT, "E");
			qsodb.qsoUpdRec (i, rec);
		}
	}
	string sp = p;
	if (sp.find(".csv") == string::npos) sp.append(".csv");
	txtFile.writeCSVFile(sp.c_str(), &qsodb);
}


void Export_TXT()
{
	if (chkExportBrowser->nchecked() == 0) return;

	cQsoRec *rec;
	string title = _("Export to fixed field text file");
	string filters = "TEXT\t*.txt";
#ifdef __APPLE__
	filters.append("\n");
#endif
	const char* p = FSEL::saveas( title.c_str(), filters.c_str(), "export.txt");

	if (!p) return;
	if (!*p) return;

	for (int i = 0; i < chkExportBrowser->nitems(); i++) {
		if (chkExportBrowser->checked(i + 1)) {
			rec = qsodb.getRec(i);
			rec->putField(EXPORT, "E");
			qsodb.qsoUpdRec (i, rec);
		}
	}
	string sp = p;
	if (sp.find(".txt") == string::npos) sp.append(".txt");
	txtFile.writeTXTFile(p, &qsodb);
}

string adif_field(int num, const char *s)
{
	char tempstr[100];
	int slen = strlen(s);
	if (slen == 0) return "";
	int n = snprintf(tempstr, sizeof(tempstr), "<%s:%d>%s", fields[num].name, slen, s);
	if (n == -1) {
		return "";
	}
	return tempstr;
}

bool want_field(int j)
{
	if (fields[j].btn == NULL) return false;
	return (*fields[j].btn)->value();
}

string export_rec(cQsoRec *rec)
{
	string record;
	string notes;
	string temp;

	if (want_field(CALL)) {
		temp = rec->getField(CALL);
		if (temp.empty()) return "";
		record += adif_field(CALL, temp.c_str());
	}
	if (want_field(QSO_DATE)) {
		temp = rec->getField(QSO_DATE);
		if (temp.empty()) return "";
		record += adif_field(QSO_DATE, temp.c_str());
	}
	if (want_field(TIME_ON)) {
		temp = rec->getField(TIME_ON);
		if (temp.empty()) return "";
		record += adif_field(TIME_ON, temp.c_str());
	}
	if (want_field(QSO_DATE_OFF)) {
		temp = rec->getField(QSO_DATE_OFF);
		if (temp.empty()) return "";
		record += adif_field(QSO_DATE_OFF, temp.c_str());
	}
	if (want_field(TIME_OFF)) {
		temp = rec->getField(TIME_OFF);
		if (temp.empty()) return "";
		record += adif_field(TIME_OFF, temp.c_str());
	}
	if (want_field(FREQ))
		record += adif_field(FREQ, rec->getField(FREQ));
	if (want_field(MODE)) {
		record += adif_field(MODE, adif2export(rec->getField(MODE)).c_str());
		string submode = adif2submode(rec->getField(MODE));
		if (!submode.empty())
			record += adif_field(SUBMODE, submode.c_str());
	}
	if (want_field(RST_SENT))
		record += adif_field(RST_SENT, rec->getField(RST_SENT));
	if (want_field(RST_RCVD))
		record += adif_field(RST_RCVD, rec->getField(RST_RCVD));
	if (want_field(TX_PWR))
		record += adif_field(TX_PWR, rec->getField(TX_PWR));
	if (want_field(NAME))
		record += adif_field(NAME, rec->getField(NAME));
	if (want_field(QTH))
		record += adif_field(QTH, rec->getField(QTH));
	if (want_field(STATE))
		record += adif_field(STATE, rec->getField(STATE));
	if (want_field(COUNTRY))
		record += adif_field(VE_PROV, rec->getField(COUNTRY));
	if (want_field(GRIDSQUARE))
		record += adif_field(GRIDSQUARE, rec->getField(GRIDSQUARE));
	if (want_field(STX))
		record += adif_field(STX, rec->getField(STX));
	if (want_field(SRX))
		record += adif_field(SRX, rec->getField(SRX));
	if (want_field(XCHG1))
		record += adif_field(XCHG1, rec->getField(XCHG1));
	if (want_field(MYXCHG))
		record += adif_field(MYXCHG, rec->getField(MYXCHG));
	if (want_field(NOTES)) {
		notes = rec->getField(NOTES);
		for (size_t i = 0; i < notes.length(); i++)
			if (notes[i] == '\n') notes[i] = ';';
		record += adif_field(NOTES, notes.c_str());
	}
	if (want_field(IOTA))
		record += adif_field(IOTA, rec->getField(IOTA));
	if (want_field(DXCC))
		record += adif_field(DXCC, rec->getField(DXCC));
	if (want_field(QSL_VIA))
		record += adif_field(QSL_VIA, rec->getField(QSL_VIA));
	if (want_field(QSLRDATE))
		record += adif_field(QSLRDATE, rec->getField(QSLRDATE));
	if (want_field(QSLSDATE))
		record += adif_field(QSLSDATE, rec->getField(QSLSDATE));
	if (want_field(EQSLRDATE))
		record += adif_field(EQSLRDATE, rec->getField(EQSLRDATE));
	if (want_field(EQSLSDATE))
		record += adif_field(EQSLSDATE, rec->getField(EQSLSDATE));
	if (want_field(LOTWRDATE))
		record += adif_field(LOTWRDATE, rec->getField(LOTWRDATE));
	if (want_field(LOTWSDATE))
		record += adif_field(LOTWSDATE, rec->getField(LOTWSDATE));

	record += "<EOR>\r\n";
	return record;
}

void Export_ADIF()
{
	if (chkExportBrowser->nchecked() == 0) return;

	string title = _("Export to ADIF file");
	string filters;
	string defname;
	filters.assign("ADIF\t*.").append(ADIF_SUFFIX);
#ifdef __APPLE__
	filters.append("\n");
#endif
	defname.assign("export.").append(ADIF_SUFFIX);
	const char* p = FSEL::saveas( title.c_str(), filters.c_str(), defname.c_str());

	if (!p) return;
	if (!*p) return;

	string sp = p;
	string temp = ".";
	temp.append(ADIF_SUFFIX);

	if (sp.find(temp) == string::npos) sp.append(temp);

	string adif;
	char szheader[500];
	const char *header =\
"File: %s\r\n\
<ADIF_VER:%d>%s\r\n\
<PROGRAMID:%d>%s\r\n\
<PROGRAMVERSION:%d>%s\r\n\
<EOH>\r\n";

	snprintf (szheader, sizeof(szheader), header,
			 fl_filename_name(sp.c_str()),
			 strlen(ADIF_VERS), ADIF_VERS,
			 strlen(PACKAGE_NAME), PACKAGE_NAME,
			 strlen(PACKAGE_VERSION), PACKAGE_VERSION);

	adif.assign(szheader);
	for (int i = 0; i < chkExportBrowser->nitems(); i++) {
		if (chkExportBrowser->checked(i + 1)) {
			adif.append(export_rec(qsodb.getRec(i)));
		}
	}

	ofstream ofile(sp.c_str());
	ofile << adif;
	ofile.close();

}

/*
//======================================================================

// refresh_logbook_dialog ONLY called as an Fl::awake process
// insures that logbook dialog LOTWSDATE field is updated by
// FLTK UI thread.
// called by Export_LOTW() and saveRecord()

static string lotwsdate;
void refresh_logbook_dialog(void *)
{
	inpLOTWsentdate_log->value(lotwsdate.c_str());
	inpLOTWsentdate_log->redraw();
}

void Export_LOTW()
{
	if (chkExportBrowser->nchecked() == 0) return;

	cQsoRec *rec;

	if (str_lotw.empty())
		str_lotw = "Fldigi LoTW upload file\n<ADIF_VER:5>2.2.7\n<EOH>\n";
	string adifrec;

	for (int i = 0; i < chkExportBrowser->nitems(); i++) {
		if (chkExportBrowser->checked(i + 1)) {
			rec = qsodb.getRec(i);
			rec->putField(EXPORT, "E");
			rec->putField(LOTWSDATE, zdate());
			qsodb.qsoUpdRec (i, rec);
			lotw_recs_sent.push_back(i);
			if (i == editNbr) {
				lotwsdate = rec->getField(LOTWSDATE);
				Fl::awake(refresh_logbook_dialog);
			}
			adifrec = lotw_rec(*rec);
			if (adifrec.empty()) {
				LOG_INFO("%s", "Invalid LOTW record");
			} else
				str_lotw.append(adifrec);
		}
	}
}

Fl_Double_Window *lotw_review_dialog = 0;
static Fl_Text_Buffer *buff = 0;
static Fl_Text_Editor *disp = 0;
static Fl_Button *lotw_close_review = 0;
static Fl_Button *lotw_save_review = 0;
static Fl_Button *lotw_clear_review = 0;

void cb_lotw_close_review(Fl_Button *, void *)
{
	lotw_review_dialog->hide();
	delete lotw_review_dialog;
	lotw_review_dialog = 0;
	lotw_close_review = 0;
	buff = 0;
	disp = 0;
}

void cb_lotw_save_review(Fl_Button *, void *)
{
	str_lotw = buff->text();
}

void cb_lotw_clear_review(Fl_Button *, void *)
{
	buff->text("");
}

void cb_review_lotw()
{
	if (str_lotw.empty()) return;

	if (!lotw_review_dialog) {
		lotw_review_dialog = new Fl_Double_Window(50,50, 640, 400, _("LoTW Review"));
		lotw_review_dialog->begin();

		buff = new Fl_Text_Buffer();
		disp = new Fl_Text_Editor(4, 4, 632, 364);
		disp->textfont(FL_SCREEN);
		disp->buffer(buff); // attach text buffer to display widget
		lotw_close_review = new Fl_Button(576, 372, 60, 24, _("Close"));
		lotw_close_review->callback((Fl_Callback *)cb_lotw_close_review);

		lotw_clear_review = new Fl_Button(4, 372, 60, 24, _("Clear"));
		lotw_clear_review->callback((Fl_Callback *)cb_lotw_clear_review);

		lotw_save_review = new Fl_Button(lotw_review_dialog->w()/2-30, 372, 60, 24, _("Save"));
		lotw_save_review->callback((Fl_Callback *)cb_lotw_save_review);

		lotw_review_dialog->end();
	}
	buff->text(str_lotw.c_str());

	lotw_review_dialog->show();
}

void cb_send_lotw()
{
	send_to_lotw(NULL);
}
//======================================================================
*/

static savetype export_to = ADIF;

void Export_log()
{
//	if (export_to == LOTW) Export_LOTW();
//	else 
	if (export_to == ADIF) Export_ADIF();
	else if (export_to == CSV) Export_CSV();
	else Export_TXT();
}

void saveLogbook()
{
	if (!qsodb.isdirty()) return;

	if (!fl_choice2(_("Save changed Logbook?"), _("No"), _("Yes"), NULL))
		return;

	qsodb.SortByDate();

	adifFile.writeLog (logbook_filename.c_str(), &qsodb, true);

	restore_sort();

	qsodb.isdirty(0);
}

void cb_mnuNewLogbook()
{
	saveLogbook();

	string title = _("Create new logbook file");
	string filter;
	filter.assign("ADIF\t*.").append(ADIF_SUFFIX);
#ifdef __APPLE__
	filter.append("\n");
#endif

	logbook_filename = LogHomeDir;
	logbook_filename.append("newlog.").append(ADIF_SUFFIX);

	const char* p = FSEL::saveas( title.c_str(), filter.c_str(), logbook_filename.c_str());
	if (!p) return;
	if (!*p) return;

	string temp = p;
	string suffix = ".";
	suffix.append(ADIF_SUFFIX);
	if (temp.find(suffix) == string::npos) temp.append(suffix);

	FILE *testopen = fl_fopen(temp.c_str(), "r");
	if (testopen) {
		string warn = logbook_filename;
		int ans = fl_choice2(
					_("%s exists, overwrite?"),
					_("No"), _("Yes"), NULL,
					temp.c_str());
		if (!ans) return;
		fclose(testopen);
	}

	progStatus.logbookfilename = logbook_filename = temp;

	dlgLogbook->label(fl_filename_name(logbook_filename.c_str()));
	qsodb.deleteRecs();
//	dxcc_entity_cache_clear();
	wBrowser->clear();
	clearRecord();
	qsodb.isdirty(1);
	saveLogbook();
}

void OpenLogbook()
{
	adifFile.readFile (progStatus.logbookfilename.c_str(), &qsodb);
	txtLogFile->value(progStatus.logbookfilename.c_str());
	txtLogFile->redraw();
	Fl::flush();
	qsodb.isdirty(0);
	loadBrowser();
	activateButtons();
}

void adif_read_OK()
{
	if (qsodb.nbrRecs() == 0)
		adifFile.writeFile(logbook_filename.c_str(), &qsodb);
//	dxcc_entity_cache_clear();
//	dxcc_entity_cache_add(qsodb);
	restore_sort();
	activateButtons();
	loadBrowser();
}

void cb_mnuOpenLogbook()
{
	const char* p = FSEL::select(_("Open logbook file"), "ADIF\t*." ADIF_SUFFIX,
					 logbook_filename.c_str());
	Fl::flush();
	if (p) {
		saveLogbook();
		qsodb.deleteRecs();
		logbook_filename = p;
		progStatus.logbookfilename = logbook_filename;
		OpenLogbook();
	}
}

void cb_mnuSaveLogbook() {
	string title = _("Save logbook file");
	string filter;
	filter.assign("ADIF\t*.").append(ADIF_SUFFIX);
#ifdef __APPLE__
	filter.append("\n");
#endif
	std::string deffilename = LogHomeDir;
	deffilename.append(fl_filename_name(logbook_filename.c_str()));

	const char* p = FSEL::saveas( title.c_str(), filter.c_str(), deffilename.c_str());

	if (!p) return;
	if (!*p) return;

	logbook_filename = p;
	string temp = ".";
	temp.append(ADIF_SUFFIX);
	if (logbook_filename.find(temp) == string::npos)
		logbook_filename.append(temp);

	progStatus.logbookfilename = logbook_filename;

	dlgLogbook->label(fl_filename_name(logbook_filename.c_str()));

	qsodb.SortByDate();

	qsodb.isdirty(0);
	restore_sort();
	adifFile.writeLog (logbook_filename.c_str(), &qsodb);

}

/*
//======================================================================
// separate thread for performing the database merger
//
// thread 'merge_thread' is instantiated for a database file merger
// either on failure or successful merger the thread signals the main
// UI thread to close the merge_thread and release all of it's resources
//
// merge_thread is not re-entrant.  Only a single instance of the thread
// is allowed.
//
// the user will be notified if an attempt is made to start a new merger
// while one is already in progress.
//
//======================================================================

pthread_t* MERGE_thread = 0;
pthread_mutex_t MERGE_mutex = PTHREAD_MUTEX_INITIALIZER;

static string mrg_fname;
static string disptxt;
static bool   abort_merger;
static int num_merge_recs;
static float read_secs;

static void merge_announce_1(void *)
{
	static char announce[500];
	snprintf(announce, sizeof(announce),
		"Merging records:\n   File: %s\n", mrg_fname.c_str());
	ReceiveText->addstr(announce);
	ReceiveText->redraw();
	Fl::flush();
}

static void merge_announce_2(void *)
{
	static char announce[500];
	snprintf(announce, sizeof(announce),
		"   Read %d records in %4.1f seconds\n   Merging ... please wait",
		num_merge_recs, read_secs );
	ReceiveText->addstr(announce);
	ReceiveText->redraw();
	Fl::flush();
}

void close_MERGE_thread (void *)
{
	ENSURE_THREAD(FLMAIN_TID);

	if (!MERGE_thread) return;

	pthread_mutex_lock(&MERGE_mutex);
	abort_merger = true;
	pthread_mutex_unlock(&MERGE_mutex);

	pthread_join(*MERGE_thread, NULL);

	delete MERGE_thread;

	MERGE_thread = 0;
	abort_merger = false;

	qsodb.isdirty(1);
	saveLogbook(true); // force the save independent of user settings
	loadBrowser();

	ReceiveText->addstr(disptxt.c_str());
	ReceiveText->redraw();

}

static void *merge_thread(void *args)
{
	SET_THREAD_ID(ADIF_MERGE_TID);

	static char msg1[200];

	sorttype orig_sort = progStatus.lastsort;

	int     orig_reverse = cQsoDb::reverse;

	cQsoDb::reverse = false;

	cQsoDb *db = &qsodb;
	cQsoDb *mrgdb = new cQsoDb;
	cQsoDb *merge_dups = new cQsoDb;
	cQsoDb *orig_dups = new cQsoDb;
	cQsoDb *copy = new cQsoDb(db);

	string mergedir;
	string mrg_dups_name;
	string orig_dups_name;
	string lg_recs_name;
	string fname;
	size_t pname;

	cQsoRec *lastrec = 0;
	cQsoRec *rec_n;
	cQsoRec *rec_m;

	int N;
	int M;
	int n = 0;
	int m = 0;
	int cmp = 0;
	int cmp2 = 0;

	int merged = 0;
	int merge_duplicates = 0;
	int orig_duplicates = 0;

	struct timespec t0, t1, t2;
	float  merger_time = 0;

	Fl::awake(merge_announce_1);

#ifdef _POSIX_MONOTONIC_CLOCK
	clock_gettime(CLOCK_MONOTONIC, &t0);
#else
	clock_gettime(CLOCK_REALTIME, &t0);
#endif

	adifFile.do_readfile (mrg_fname.c_str(), mrgdb);

#ifdef _POSIX_MONOTONIC_CLOCK
	clock_gettime(CLOCK_MONOTONIC, &t1);
#else
	clock_gettime(CLOCK_REALTIME, &t1);
#endif

	N = copy->nbrRecs();
	M = mrgdb->nbrRecs();

	if (M == 0) {
		disptxt.assign("\n================================================\n");
		disptxt.append("Merge file contains no records\n");
		disptxt.append("\n================================================");
		LOG_INFO("%s", disptxt.c_str());
		disptxt.append("\n");
		goto exit_merge_thread;
	}

	read_secs = t1.tv_sec - t0.tv_sec + (t1.tv_nsec- t0.tv_nsec)/1e9;
	num_merge_recs = M;

	Fl::awake(merge_announce_2);

	disptxt.assign("\n================================================\n");

	copy->SortByCall();
	mrgdb->SortByCall();

	for (int i = 0; i < M; i++) {
		mrgdb->getRec(i)->checkBand();
		mrgdb->getRec(i)->checkDateTimes();
	}

//writeLog("copy.adi", copy);
//writeLog("mrgdb.adi", mrgdb);

	db->clearDatabase();

	for (;;) {

		pthread_mutex_lock(&MERGE_mutex);
		if (abort_merger) goto abort;
		pthread_mutex_unlock(&MERGE_mutex);

		rec_n = copy->getRec(n);
		rec_m = mrgdb->getRec(m);

		if (N == 0) {
			if (m == M) break;
			if (lastrec == 0) {
				db->qsoNewRec(lastrec = rec_m);
				merged++;
			} else {
				cmp = comparebycall(lastrec, rec_m);
				if (cmp != 0) {
					db->qsoNewRec(lastrec = rec_m);
					merged++;
				} else {
					merge_dups->qsoNewRec(rec_m);
					merge_duplicates++;
				}
			}
			m++;
			continue;
		}

		if (n == N) {
			if (m == M) break;
			cmp = comparebycall(lastrec, rec_m);
			if (cmp == 0) {
				merge_dups->qsoNewRec(rec_m);
				merge_duplicates++;
			} else {
				db->qsoNewRec(lastrec = rec_m);
				merged++;
			}
			m++;
			continue;
		}
		if (m == M) {
			if (n == N) break;
			cmp = comparebycall(lastrec, rec_n);
			if (cmp == 0) {
				orig_dups->qsoNewRec(rec_n);
				orig_duplicates++;
			} else {
				db->qsoNewRec(lastrec = rec_n);
			}
			n++;
			continue;
		}

		if (lastrec == 0) {
			cmp = comparebycall(rec_n, rec_m);
			if (cmp < 0) {
				db->qsoNewRec(lastrec = rec_n);
				n++;
				continue;
			}
			if (cmp == 0) {
				db->qsoNewRec(lastrec = rec_n);
				n++;
				merge_dups->qsoNewRec(rec_m);
				m++;
				merge_duplicates++;
				continue;
			} // cmp > 0
			db->qsoNewRec(lastrec = rec_m);
			m++;
		} else { // lastrec exists
			cmp = comparebycall(rec_n, rec_m);
			if (cmp == 0) {
				merge_dups->qsoNewRec(rec_m);
				merge_duplicates++;
				m++;
				cmp2 = comparebycall(lastrec, rec_n);
				if (cmp2 == 0) {
					orig_dups->qsoNewRec(rec_n);
					orig_duplicates++;
				} else
					db->qsoNewRec(lastrec = rec_n);
				n++;
				continue;
			}
			if (cmp < 0) {
				cmp2 = comparebycall(lastrec, rec_n);
				if (cmp2 == 0) {
					orig_dups->qsoNewRec(rec_n);
					orig_duplicates++;
				} else
					db->qsoNewRec(lastrec = rec_n);
				n++;
				continue;
			} // cmp > 0
			cmp2 = comparebycall(lastrec, rec_m);
			if (cmp2 == 0) {
				merge_dups->qsoNewRec(rec_m);
				merge_duplicates++;
			} else if (cmp2 < 0) {
				db->qsoNewRec(lastrec = rec_m);
				merged++;
			}
			m++;
		}
	}

	mergedir = logbook_filename;

	fname = fl_filename_name(mergedir.c_str());
	pname = mergedir.find(fname);
	mergedir.erase(pname);

	if (db->nbrRecs())
		db->SortByCall();

	if (merged > 0) {
		snprintf(msg1, sizeof(msg1), "Merged %d records\n", merged);
		disptxt.append(msg1);
	}

	if (merge_duplicates) {
		merge_dups->SortByCall();
		mrg_dups_name = mergedir;
		mrg_dups_name.append("merge_file_dups");
#ifdef __WIN32__
		mrg_dups_name.append(".adi");
#else
		mrg_dups_name.append(".adif");
#endif
		adifFile.writeLog (mrg_dups_name.c_str(), merge_dups, true);

		snprintf(msg1, sizeof(msg1), "Found %d duplicate records\n", merge_duplicates);
		disptxt.append(msg1);
		snprintf(msg1, sizeof(msg1), "Duplicate's saved in %s\n", mrg_dups_name.c_str());
		disptxt.append(msg1);
	}

	if (orig_duplicates) {
		orig_dups->SortByCall();
		orig_dups_name = mergedir;
		orig_dups_name.append("original_file_dups");
#ifdef __WIN32__
		orig_dups_name.append(".adi");
#else
		orig_dups_name.append(".adif");
#endif
		adifFile.writeLog (orig_dups_name.c_str(), orig_dups, true);

		snprintf(msg1,sizeof(msg1), "Original database had %d duplicates\n", orig_duplicates);
		disptxt.append(msg1);
		snprintf(msg1, sizeof(msg1), "Duplicate's saved in %s\n", orig_dups_name.c_str());
		disptxt.append(msg1);
	}

#ifdef _POSIX_MONOTONIC_CLOCK
	clock_gettime(CLOCK_MONOTONIC, &t2);
#else
	clock_gettime(CLOCK_REALTIME, &t2);
#endif

	merger_time = t2.tv_sec - t0.tv_sec + (t2.tv_nsec- t2.tv_nsec)/1e9;

	snprintf(msg1, sizeof(msg1), "Merger took %4.1f seconds\n", merger_time);
	disptxt.append(msg1);

	disptxt.append("================================================");
	LOG_INFO("%s", disptxt.c_str());
	disptxt.append("\n");

exit_merge_thread:

	delete mrgdb;
	delete orig_dups;
	delete merge_dups;
	delete copy;

	progStatus.lastsort = orig_sort;
	cQsoDb::reverse = orig_reverse;

	Fl::awake(close_MERGE_thread);

	return NULL;

abort:

	pthread_mutex_unlock(&MERGE_mutex);

	disptxt.assign("Merger aborted");

	delete mrgdb;
	delete orig_dups;
	delete merge_dups;
	delete copy;

	progStatus.lastsort = orig_sort;
	cQsoDb::reverse = orig_reverse;

	Fl::awake(close_MERGE_thread);

	return NULL;
}

void cb_mnuMergeADIF_log(Fl_Menu_* m, void* d) {

	ENSURE_THREAD(FLMAIN_TID);

	if (MERGE_thread) {
		fl_alert2("Database merger in progress");
		return;
	}

	const char* p = FSEL::select(
			_("Merge ADIF file"),
			"ADIF\t*.{adi,adif}",
			LogsDir.c_str());

	fl_digi_main->redraw();
	Fl::flush();

	if (!p) return;
	if (!*p) return;

	mrg_fname = p;

	abort_merger = false;

	MERGE_thread = new pthread_t;
	if (pthread_create(MERGE_thread, NULL, merge_thread, NULL) != 0) {
		LOG_PERROR("pthread_create");
		return;
	}
	MilliSleep(10);

}

*/

//======================================================================
/*
static string lotw_download_name = "";
static cQsoDb *lotw_db = 0;

void verify_lotw(void *)
{
	lotw_db = new cQsoDb;
	adifFile.do_readfile (lotw_download_name.c_str(), lotw_db);

	string notice;
	char sznote[50];

	if (lotw_db->nbrRecs() == 0) {
		notice = "No records in lotw download file";
		LOG_INFO("%s", notice.c_str());
	} else {

		int matchrec;
		cQsoRec *qrec, *lrec;
		int nverified = 0;
		string date;
		string qdate;

		for (int i = 0; i < lotw_db->nbrRecs(); i++) {
			lrec = lotw_db->getRec(i);
			date = lrec->getField(QSLRDATE);
			matchrec = qsodb.matched( lrec );
			if (matchrec != -1) {
				qrec = qsodb.getRec(matchrec);
				qdate = qrec->getField(LOTWRDATE);
				if (date != qdate) {
					nverified++;
					qrec->putField(STATE, lrec->getField(STATE));
					qrec->putField(GRIDSQUARE, lrec->getField(GRIDSQUARE));
					qrec->putField(CQZ, lrec->getField(CQZ));
					qrec->putField(COUNTRY, lrec->getField(COUNTRY));
					qrec->putField(CNTY, lrec->getField(CNTY));
					qrec->putField(DXCC, lrec->getField(DXCC));
					qrec->putField(DXCC, lrec->getField(DXCC));
					qrec->putField(LOTWRDATE, lrec->getField(QSLRDATE));
				}
			} else {
				notice.append("Could not match ");
				notice.append(lrec->getField(CALL)).append(" on ");
				notice.append(lrec->getField(QSO_DATE)).append("\n");
				LOG_INFO("Could not match %s on %s", lrec->getField(CALL), lrec->getField(QSO_DATE));
			}
		}
		snprintf(sznote, sizeof(sznote),"%d records matched", nverified);
		notice.append(sznote).append("\n");
		if (nverified) qsodb.isdirty(1);
		LOG_INFO("%d records matched", nverified);
	}
	fl_alert2("%s", notice.c_str());

	delete lotw_db;
}

void cb_btn_verify_lotw(Fl_Button *, void *) {

	string deffname = LoTWDir;
	deffname.append("lotwreport.adi");

	ifstream f(deffname.c_str());

//	const char* p = FSEL::select(_("LoTW download file"), "ADIF\t*.{adi,adif}", deffname.c_str());

//	if (!p || !*p) {
	if (!f) {
		fl_alert2("\
Could not find LoTW report file.\n\n\
Download from ARRL's LoTW page after logging in at:\n\n\
https://lotw.arrl.org/lotwuser/default\n\n\
Store the report file to the fldigi LOTW folder,\n\n\
naming the file 'lotwreport.adi'");
		return;
	}
//	lotw_download_name = p;
	f.close();
	lotw_download_name = deffname;
	Fl::awake(verify_lotw);
}
*/
//======================================================================

void cb_export_date_select() {
	if (qsodb.nbrRecs() == 0) return;
	int start = atoi(inp_export_start_date->value());
	int stop = atoi(inp_export_stop_date->value());

	chkExportBrowser->check_none();

	if (!start || !stop) return;
	int chkdate;

	if (!btn_export_by_date->value()) return;

	cQsoRec *rec;
	for (int i = 0; i < qsodb.nbrRecs(); i++) {
		rec = qsodb.getRec (i);
		chkdate = atoi(
			rec->getField(QSO_DATE_OFF));
//				progStatus.sort_date_time_off ? QSO_DATE_OFF : QSO_DATE));
		if (chkdate >= start && chkdate <= stop)
			chkExportBrowser->checked(i+1, 1);
	}

	chkExportBrowser->redraw();
}

void cb_mnuMergeADIF_log() {
	const char* p = FSEL::select(_("Merge ADIF file"), "ADIF\t*." ADIF_SUFFIX);
	Fl::flush();
	if (p) {

		adifFile.readFile (p, &qsodb);
		txtLogFile->value(progStatus.logbookfilename.c_str());
		txtLogFile->redraw();
		Fl::flush();

		restore_sort();
		loadBrowser();

		qsodb.isdirty(1);
	}
}

inline const char *szfreq(const char *freq)
{
	static char szf[11];
	float f = atof(freq);
	snprintf(szf, sizeof(szf), "%10.6f", f);
	return szf;
}

void cb_Export_log() {
	if (qsodb.nbrRecs() == 0) return;
	cQsoRec *rec;
	char line[80];
	chkExportBrowser->clear();
#ifdef __APPLE__
	chkExportBrowser->textfont(FL_SCREEN_BOLD);
	chkExportBrowser->textsize(12);
#else
	chkExportBrowser->textfont(FL_COURIER);
	chkExportBrowser->textsize(12);
#endif
	for( int i = 0; i < qsodb.nbrRecs(); i++ ) {
		rec = qsodb.getRec (i);
		snprintf(line,sizeof(line),"%8s %4s %-10s %-10s %-s",
			rec->getField(QSO_DATE),
			rec->getField(TIME_OFF),//(export_to == LOTW ? TIME_ON : TIME_OFF) ),
			rec->getField(CALL),
			szfreq(rec->getField(FREQ)),
			rec->getField(MODE) );
        chkExportBrowser->add(line);
	}
	cb_export_date_select();
	wExport->show();
}

void cb_mnuExportADIF_log() {
	export_to = ADIF;
	cb_Export_log();
}

void cb_mnuExportCSV_log() {
	export_to = CSV;
	cb_Export_log();
}

void cb_mnuExportTEXT_log() {
	export_to = TEXT;
	cb_Export_log();
}


void cb_mnuShowLogbook()
{
	dlgLogbook->show();
}

void cb_mnuEvents()
{
	wEvents->show();
}

enum State {VIEWREC, NEWREC};
static State logState = VIEWREC;

void activateButtons()
{
	if (logState == NEWREC) {
		bNewSave->label(_("Save"));
		bUpdateCancel->label(_("Cancel"));
		bUpdateCancel->activate();
		bDelete->deactivate ();
		bSearchNext->deactivate ();
		bSearchPrev->deactivate ();
		inpDate_log->take_focus();
		return;
	}
	bNewSave->label(_("New"));
	bUpdateCancel->label(_("Update"));
	if (qsodb.nbrRecs() > 0) {
		bDelete->activate();
		bUpdateCancel->activate();
		bSearchNext->activate ();
		bSearchPrev->activate ();
		wBrowser->take_focus();
	} else {
		bDelete->deactivate();
		bUpdateCancel->deactivate();
		bSearchNext->deactivate();
		bSearchPrev->deactivate();
	}
}

void cb_btnNewSave(Fl_Button* b, void* d) {
	if (logState == VIEWREC) {
		logState = NEWREC;
		clearRecord();
		activateButtons();
	} else {
		saveRecord();
		qsodb.SortByDate();
		loadBrowser();
		logState = VIEWREC;
		activateButtons();
	}
}

void cb_btnUpdateCancel(Fl_Button* b, void* d) {
	if (logState == NEWREC) {
		logState = VIEWREC;
		activateButtons ();
	} else {
		updateRecord();
		wBrowser->take_focus();
	}
}

void cb_btnDelete(Fl_Button* b, void* d) {
	deleteRecord();
	wBrowser->take_focus();
}

void restore_sort()
{
	switch (progStatus.lastsort) {
	case SORTCALL :
		cQsoDb::reverse = progStatus.callfwd;
		qsodb.SortByCall();
		break;
	case SORTDATE :
		cQsoDb::reverse = progStatus.datefwd;
		qsodb.SortByDate();
		break;
	case SORTFREQ :
		cQsoDb::reverse = progStatus.freqfwd;
		qsodb.SortByFreq();
		break;
	case SORTMODE :
		cQsoDb::reverse = progStatus.modefwd;
		qsodb.SortByMode();
		break;
	default: break;
	}
}

void cb_SortByCall (void) {
	if (progStatus.lastsort == SORTCALL)
		progStatus.callfwd = !progStatus.callfwd;
	else {
		progStatus.callfwd = false;
		progStatus.lastsort = SORTCALL;
	}
	cQsoDb::reverse = progStatus.callfwd;
	qsodb.SortByCall();
	loadBrowser();
}

void cb_SortByDate (void) {
	if (progStatus.lastsort == SORTDATE)
		progStatus.datefwd = !progStatus.datefwd;
	else {
		progStatus.lastsort = SORTDATE;
	}
	cQsoDb::reverse = progStatus.datefwd;
	qsodb.SortByDate();
	loadBrowser();
}

void reload_browser()
{
	qsodb.SortByDate();
	loadBrowser();
}

void cb_SortByMode (void) {
	if (progStatus.lastsort == SORTMODE)
		progStatus.modefwd = !progStatus.modefwd;
	else {
		progStatus.modefwd = false;
		progStatus.lastsort = SORTMODE;
	}
	cQsoDb::reverse = progStatus.modefwd;
	qsodb.SortByMode();

	loadBrowser();
}

void cb_SortByFreq (void) {
	if (progStatus.lastsort == SORTFREQ)
		progStatus.freqfwd = !progStatus.freqfwd;
	else {
		progStatus.freqfwd = false;
		progStatus.lastsort = SORTFREQ;
	}
	cQsoDb::reverse = progStatus.freqfwd;
	qsodb.SortByFreq();

	loadBrowser();
}

void DupCheck()
{
}

cQsoRec* SearchLog(const char *callsign)
{
	size_t len = strlen(callsign);
	char* re = new char[len + 3];
	snprintf(re, len + 3, "^%s$", callsign);

	int row = 0, col = 2;
	return wBrowser->search(row, col, !cQsoDb::reverse, re) ? qsodb.getRec(row) : 0;
}

void SearchLastQSO(const char *callsign)
{
}

static const char *adifmt = "<%s:%d>%s";
const char *adif_record(cQsoRec *rec)
{
static char szrec[1000];
static char szfield[100];
string sFld;
	szrec[0] = 0;
	for (int j = 0; j < NUMFIELDS; j++) {
		sFld = rec->getField(fields[j].type);
		if (!sFld.empty()) {
			snprintf(szfield, sizeof(szfield), adifmt,
				fields[j].name, sFld.length(), sFld.c_str());
			strcat(szrec, szfield);
		}
	}
	return (const char *)szrec;
}

static std::string fetch_callsign;
void update_fetch(void *)
{
	int row = 0, col = 2;
	wBrowser->search(row, col, false, fetch_callsign.c_str());
	wBrowser->GotoRow(row);
	wBrowser->redraw();
	mainwindow->redraw();
}

const char *fetch_record(const char *callsign)
{
	cQsoRec *rec = SearchLog(callsign);
	if (rec) {
		fetch_callsign = callsign;
		Fl::awake(update_fetch);
		return adif_record(rec);
	} else
		return "NO_RECORD";
}

void cb_search(Fl_Widget* w, void*)
{
	const char* str = inpSearchString->value();
	if (!*str)
		return;

	bool rev = w == bSearchPrev;
	int col = 2, row = wBrowser->value() + (rev ? -1 : 1);
	row = WCLAMP(row, 0, wBrowser->rows() - 1);

	if (wBrowser->search(row, col, rev, str))
		wBrowser->GotoRow(row);
	wBrowser->take_focus();
}

int log_search_handler(int)
{
	if (!(Fl::event_state() & FL_CTRL))
		return 0;

	switch (Fl::event_key()) {
	case 's':
		bSearchNext->do_callback();
		break;
	case 'r':
		bSearchPrev->do_callback();
		break;
	default:
		return 0;
	}
	return 1;
}

int editNbr = 0;

void clearRecord() 
{
	Date tdy;
	tdy.gmdate(true);
	inpCall_log->value ("");
	inpName_log->value ("");
	inpDate_log->value (tdy.szDate(2));
	inpDateOff_log->value (tdy.szDate(2));
	inpTimeOn_log->value ("");
	inpTimeOff_log->value ("");
	inpRstR_log->value ("");
	inpRstS_log->value ("");
	inpFreq_log->value ("");
	inpBand_log->value ("");
	inpMode_log->value ("");
	inpQth_log->value ("");
	inpState_log->value ("");
	inpVE_Prov_log->value ("");
	inpCountry_log->value ("");
	inpLoc_log->value ("");

	inpQSLrcvddate_log->value ("");
	inpQSLsentdate_log->value ("");

	inpEQSLrcvddate_log->value ("");
	inpEQSLsentdate_log->value ("");

	inpLOTWrcvddate_log->value ("");
	inpLOTWsentdate_log->value ("");

	inpSerNoOut_log->value ("");
	inpSerNoIn_log->value ("");
	inpXchgIn_log->value("");
	inpNotes_log->value ("");
	inpIOTA_log->value("");
	inpDXCC_log->value("");
	inpQSL_VIA_log->value("");
	inpCONT_log->value("");
	inpCNTY_log->value("");
	inpCQZ_log->value("");
	inpITUZ_log->value("");
	inpTX_pwr_log->value("");
	inpSearchString->value ("");

	inp_log_sta_call->value("");
	inp_log_op_call->value("");
	inp_log_sta_qth->value("");
	inp_log_sta_loc->value("");

	inp_log_cwss_serno->value("");
	inp_log_cwss_prec->value("");
	inp_log_cwss_chk->value("");
	inp_log_cwss_sec->value("");

	inp_age_log->value("");
	inp_1010_log->value("");
	inp_check_log->value("");

	inp_log_troop_s->value("");
	inp_log_troop_r->value("");
	inp_log_scout_s->value("");
	inp_log_scout_r->value("");
}

void saveRecord()
{
	cQsoRec rec;
	rec.putField(CALL, inpCall_log->value());
	rec.putField(NAME, inpName_log->value());
	rec.putField(QSO_DATE, inpDate_log->value());
	rec.putField(QSO_DATE_OFF, inpDateOff_log->value());

	string tm = timestring(inpTimeOn_log->value());
	rec.putField(TIME_ON, tm.c_str());
	inpTimeOn_log->value(timeview(tm.c_str()));

	tm = timestring(inpTimeOff_log->value());
	rec.putField(TIME_OFF, tm.c_str());
	inpTimeOff_log->value(timeview(tm.c_str()));

	rec.putField(FREQ, inpFreq_log->value());
	rec.putField(BAND, inpBand_log->value());
	rec.putField(MODE, inpMode_log->value());
	rec.putField(QTH, inpQth_log->value());
	rec.putField(STATE, inpState_log->value());
	rec.putField(VE_PROV, inpVE_Prov_log->value());
	rec.putField(COUNTRY, inpCountry_log->value());
	rec.putField(GRIDSQUARE, inpLoc_log->value());
	rec.putField(NOTES, inpNotes_log->value());

	rec.putField(QSLRDATE, inpQSLrcvddate_log->value());
	rec.putField(QSLSDATE, inpQSLsentdate_log->value());

	rec.putField(EQSLRDATE, inpEQSLrcvddate_log->value());
	rec.putField(EQSLSDATE, inpEQSLsentdate_log->value());

	rec.putField(LOTWRDATE, inpLOTWrcvddate_log->value());
	rec.putField(LOTWSDATE, inpLOTWsentdate_log->value());

	rec.putField(RST_RCVD, inpRstR_log->value ());
	rec.putField(RST_SENT, inpRstS_log->value ());
	rec.putField(SRX, inpSerNoIn_log->value());
	rec.putField(STX, inpSerNoOut_log->value());
	rec.putField(XCHG1, inpXchgIn_log->value());
	rec.putField(CLASS, inpClass_log->value());
	rec.putField(ARRL_SECT, inpSection_log->value());
	rec.putField(MYXCHG, inpMyXchg_log->value());
	rec.putField(CNTY, inpCNTY_log->value());
	rec.putField(IOTA, inpIOTA_log->value());
	rec.putField(DXCC, inpDXCC_log->value());
	rec.putField(DXCC, inpQSL_VIA_log->value());
	rec.putField(CONT, inpCONT_log->value());
	rec.putField(CQZ, inpCQZ_log->value());
	rec.putField(ITUZ, inpITUZ_log->value());
	rec.putField(TX_PWR, inpTX_pwr_log->value());

	rec.putField(STA_CALL, inp_log_sta_call->value());
	rec.putField(OP_CALL, inp_log_op_call->value());
	rec.putField(MY_CITY, inp_log_sta_qth->value());
	rec.putField(MY_GRID, inp_log_sta_loc->value());

	rec.putField(SS_SERNO, inp_log_cwss_serno->value());
	rec.putField(SS_PREC, inp_log_cwss_prec->value());
	rec.putField(SS_CHK, inp_log_cwss_chk->value());
	rec.putField(SS_SEC, inp_log_cwss_sec->value());

	rec.putField(AGE,  inp_age_log->value());
	rec.putField(TEN_TEN, inp_1010_log->value());
	rec.putField(CHECK, inp_check_log->value());

	rec.putField(TROOPS, inp_log_troop_s->value());
	rec.putField(TROOPR, inp_log_troop_r->value());
	rec.putField(SCOUTS, inp_log_scout_s->value());
	rec.putField(SCOUTR, inp_log_scout_r->value());

	qsodb.qsoNewRec (&rec);

	qsodb.isdirty(0);

	reload_browser();

	if (qsodb.nbrRecs() == 1)
		adifFile.writeLog (logbook_filename.c_str(), &qsodb);
	else
		adifFile.writeAdifRec(&rec, logbook_filename.c_str());

}

void updateRecord() {
	cQsoRec rec;
	if (qsodb.nbrRecs() == 0) return;
	rec.putField(CALL, inpCall_log->value());
	rec.putField(NAME, inpName_log->value());
	rec.putField(QSO_DATE, inpDate_log->value());
	rec.putField(QSO_DATE_OFF, inpDateOff_log->value());

	string tm = timestring(inpTimeOn_log->value());
	rec.putField(TIME_ON, tm.c_str());
	inpTimeOn_log->value(timeview(tm.c_str()));

	tm = timestring(inpTimeOff_log->value());
	rec.putField(TIME_OFF, tm.c_str());
	inpTimeOff_log->value(timeview(tm.c_str()));

	rec.putField(FREQ, inpFreq_log->value());
	rec.putField(BAND, inpBand_log->value());
	rec.putField(MODE, inpMode_log->value());
	rec.putField(QTH, inpQth_log->value());
	rec.putField(STATE, inpState_log->value());
	rec.putField(VE_PROV, inpVE_Prov_log->value());
	rec.putField(COUNTRY, inpCountry_log->value());
	rec.putField(GRIDSQUARE, inpLoc_log->value());
	rec.putField(NOTES, inpNotes_log->value());

	rec.putField(QSLRDATE, inpQSLrcvddate_log->value());
	rec.putField(QSLSDATE, inpQSLsentdate_log->value());

	rec.putField(EQSLRDATE, inpEQSLrcvddate_log->value());
	rec.putField(EQSLSDATE, inpEQSLsentdate_log->value());

	rec.putField(LOTWRDATE, inpLOTWrcvddate_log->value());
	rec.putField(LOTWSDATE, inpLOTWsentdate_log->value());

	rec.putField(RST_RCVD, inpRstR_log->value ());
	rec.putField(RST_SENT, inpRstS_log->value ());
	rec.putField(SRX, inpSerNoIn_log->value());
	rec.putField(STX, inpSerNoOut_log->value());
	rec.putField(XCHG1, inpXchgIn_log->value());
	rec.putField(MYXCHG, inpMyXchg_log->value());
	rec.putField(CLASS, inpClass_log->value());
	rec.putField(ARRL_SECT, inpSection_log->value());
	rec.putField(CNTY, inpCNTY_log->value());
	rec.putField(IOTA, inpIOTA_log->value());
	rec.putField(DXCC, inpDXCC_log->value());
	rec.putField(QSL_VIA, inpQSL_VIA_log->value());
	rec.putField(CONT, inpCONT_log->value());
	rec.putField(CQZ, inpCQZ_log->value());
	rec.putField(ITUZ, inpITUZ_log->value());
	rec.putField(TX_PWR, inpTX_pwr_log->value());

	rec.putField(STA_CALL, inp_log_sta_call->value());
	rec.putField(OP_CALL, inp_log_op_call->value());
	rec.putField(MY_CITY, inp_log_sta_qth->value());
	rec.putField(MY_GRID, inp_log_sta_loc->value());

	rec.putField(SS_SERNO, inp_log_cwss_serno->value());
	rec.putField(SS_PREC, inp_log_cwss_prec->value());
	rec.putField(SS_CHK, inp_log_cwss_chk->value());
	rec.putField(SS_SEC, inp_log_cwss_sec->value());

	rec.putField(AGE,  inp_age_log->value());
	rec.putField(TEN_TEN, inp_1010_log->value());
	rec.putField(CHECK, inp_check_log->value());

	rec.putField(TROOPS, inp_log_troop_s->value());
	rec.putField(TROOPR, inp_log_troop_r->value());
	rec.putField(SCOUTS, inp_log_scout_s->value());
	rec.putField(SCOUTR, inp_log_scout_r->value());

	qsodb.qsoUpdRec (editNbr, &rec);

	qsodb.isdirty(0);
	restore_sort();

	loadBrowser(true);

	adifFile.writeLog (logbook_filename.c_str(), &qsodb);

}

void deleteRecord () {
	if (qsodb.nbrRecs() == 0 || fl_choice2(_("Really delete record for \"%s\"?"),
					       _("Yes"), _("No"), NULL, wBrowser->valueAt(-1, 2)))
		return;

	qsodb.qsoDelRec(editNbr);

	qsodb.isdirty(0);
	restore_sort();

	loadBrowser(true);

	adifFile.writeLog (logbook_filename.c_str(), &qsodb);
}

void EditRecord( int i )
{
	cQsoRec *editQSO = qsodb.getRec (i);
	if( !editQSO )
		return;

	inpCall_log->value (editQSO->getField(CALL));
	inpName_log->value (editQSO->getField(NAME));
	inpDate_log->value (editQSO->getField(QSO_DATE));
	inpDateOff_log->value (editQSO->getField(QSO_DATE_OFF));
	inpTimeOn_log->value (timeview(editQSO->getField(TIME_ON)));
	inpTimeOff_log->value (timeview(editQSO->getField(TIME_OFF)));
	inpRstR_log->value (editQSO->getField(RST_RCVD));
	inpRstS_log->value (editQSO->getField(RST_SENT));
	inpFreq_log->value (editQSO->getField(FREQ));
	inpBand_log->value (editQSO->getField(BAND));
	inpMode_log->value (editQSO->getField(MODE));
	inpState_log->value (editQSO->getField(STATE));
	inpVE_Prov_log->value (editQSO->getField(VE_PROV));
	inpCountry_log->value (editQSO->getField(COUNTRY));
	inpQth_log->value (editQSO->getField(QTH));
	inpLoc_log->value (editQSO->getField(GRIDSQUARE));

	inpQSLrcvddate_log->value (editQSO->getField(QSLRDATE));
	inpQSLsentdate_log->value (editQSO->getField(QSLSDATE));

	inpEQSLrcvddate_log->value (editQSO->getField(EQSLRDATE));
	inpEQSLsentdate_log->value (editQSO->getField(EQSLSDATE));

	inpLOTWrcvddate_log->value (editQSO->getField(LOTWRDATE));
	inpLOTWsentdate_log->value (editQSO->getField(LOTWSDATE));

	inpNotes_log->value (editQSO->getField(NOTES));
	inpSerNoIn_log->value(editQSO->getField(SRX));
	inpSerNoOut_log->value(editQSO->getField(STX));
	inpXchgIn_log->value(editQSO->getField(XCHG1));
	inpClass_log->value(editQSO->getField(CLASS));
	inpSection_log->value(editQSO->getField(ARRL_SECT));
	inpMyXchg_log->value(editQSO->getField(MYXCHG));
	inpCNTY_log->value(editQSO->getField(CNTY));
	inpIOTA_log->value(editQSO->getField(IOTA));
	inpDXCC_log->value(editQSO->getField(DXCC));
	inpQSL_VIA_log->value(editQSO->getField(QSL_VIA));
	inpCONT_log->value(editQSO->getField(CONT));
	inpCQZ_log->value(editQSO->getField(CQZ));
	inpITUZ_log->value(editQSO->getField(ITUZ));
	inpTX_pwr_log->value(editQSO->getField(TX_PWR));

	inp_log_sta_call->value(editQSO->getField(STA_CALL));
	inp_log_op_call->value(editQSO->getField(OP_CALL));
	inp_log_sta_qth->value(editQSO->getField(MY_CITY));
	inp_log_sta_loc->value(editQSO->getField(MY_GRID));

	inp_log_cwss_serno->value(editQSO->getField(SS_SERNO));
	inp_log_cwss_prec->value(editQSO->getField(SS_PREC));
	inp_log_cwss_chk->value(editQSO->getField(SS_CHK));
	inp_log_cwss_sec->value(editQSO->getField(SS_SEC));

	inp_age_log->value(editQSO->getField(AGE));
	inp_1010_log->value(editQSO->getField(TEN_TEN));
	inp_check_log->value(editQSO->getField(CHECK));

	inp_log_troop_s->value(editQSO->getField(TROOPS));
	inp_log_troop_r->value(editQSO->getField(TROOPR));
	inp_log_scout_s->value(editQSO->getField(SCOUTS));
	inp_log_scout_r->value(editQSO->getField(SCOUTR));

}

std::string sDate_on = "";
std::string sTime_on = "";
std::string sDate_off = "";
std::string sTime_off = "";

void AddRecord ()
{
}

void cb_browser (Fl_Widget *w, void *data )
{
	Table *table = (Table *)w;
	editNbr = atoi(table->valueAt(-1,6));
	EditRecord (editNbr);
}

void addBrowserRow(cQsoRec *rec, int nbr)
{
	char sNbr[6];
	snprintf(sNbr,sizeof(sNbr),"%d", nbr);
	wBrowser->addRow (7,
		rec->getField(QSO_DATE_OFF),
		timeview4(rec->getField(progStatus.BrowseTimeOFF ? TIME_OFF : TIME_ON)),
		rec->getField(CALL),
		rec->getField(NAME),
		rec->getField(FREQ),
		rec->getField(MODE),
		sNbr);
}

void adjustBrowser(bool keep_pos)
{
	int row = wBrowser->value(), pos = wBrowser->scrollPos();
	if (row >= qsodb.nbrRecs()) row = qsodb.nbrRecs() - 1;
	if (keep_pos && row >= 0) {
		wBrowser->value(row);
		wBrowser->scrollTo(pos);
	}
	else {
		if (cQsoDb::reverse == true)
			wBrowser->FirstRow ();
		else
			wBrowser->LastRow ();
	}
	char szRecs[6];
	snprintf(szRecs, sizeof(szRecs), "%5d", qsodb.nbrRecs());
	txtNbrRecs_log->value(szRecs);
}

void loadBrowser(bool keep_pos)
{
	cQsoRec *rec;
	wBrowser->clear();
	if (qsodb.nbrRecs() == 0)
		return;
	for( int i = 0; i < qsodb.nbrRecs(); i++ ) {
		rec = qsodb.getRec (i);
		addBrowserRow(rec, i);
	}
	adjustBrowser(keep_pos);
}

//=============================================================================
// Cabrillo reporter
//=============================================================================

const char *contests[] =
{   "AP-SPRINT",
	"ARRL-10", "ARRL-160", "ARRL-DX-CW", "ARRL-DX-SSB", "ARRL-SS-CW",
	"ARRL-SS-SSB", "ARRL-UHF-AUG", "ARRL-VHF-JAN", "ARRL-VHF-JUN", "ARRL-VHF-SEP",
	"ARRL-RTTY",
	"BARTG-RTTY", "BARTG-SPRINT",
	"CQ-160-CW", "CQ-160-SSB", "CQ-WPX-CW", "CQ-WPX-RTTY", "CQ-WPX-SSB", "CQ-VHF",
	"CQ-WW-CW", "CQ-WW-RTTY", "CQ-WW-SSB",
	"DARC-WAEDC-CW", "DARC-WAEDC-RTTY", "DARC-WAEDC-SSB",
	"FCG-FQP", "IARU-HF", "JIDX-CW", "JIDX-SSB",
	"NAQP-CW", "NAQP-RTTY", "NAQP-SSB", "NA-SPRINT-CW", "NA-SPRINT-SSB", "NCCC-CQP",
	"NEQP", "OCEANIA-DX-CW", "OCEANIA-DX-SSB", "RDXC", "RSGB-IOTA",
	"SAC-CW", "SAC-SSB", "STEW-PERRY", "TARA-RTTY", 0 };

enum icontest {
	AP_SPRINT,
	ARRL_10, ARRL_160, ARRL_DX_CW, ARRL_DX_SSB, ARRL_SS_CW,
	ARRL_SS_SSB, ARRL_UHF_AUG, ARRL_VHF_JAN, ARRL_VHF_JUN, ARRL_VHF_SEP,
	ARRL_RTTY,
	BARTG_RTTY, BARTG_SPRINT,
	CQ_160_CW, CQ_160_SSB, CQ_WPX_CW, CQ_WPX_RTTY, CQ_WPX_SSB, CQ_VHF,
	CQ_WW_CW, CQ_WW_RTTY, CQ_WW_SSB,
	DARC_WAEDC_CW, DARC_WAEDC_RTTY, DARC_WAEDC_SSB,
	FCG_FQP, IARU_HF, JIDX_CW, JIDX_SSB,
	NAQP_CW, NAQP_RTTY, NAQP_SSB, NA_SPRINT_CW, NA_SPRINT_SSB, NCCC_CQP,
	NEQP, OCEANIA_DX_CW, OCEANIA_DX_SSB, RDXC, RSGB_IOTA,
	SAC_CW, SAC_SSB, STEW_PERRY, TARA_RTTY
};

bool bInitCombo = true;
icontest contestnbr;

void setContestType()
{
	contestnbr = (icontest)cboContest->index();

	btnCabCall->value(true);
	btnCabFreq->value(true);
	btnCabMode->value(true);
	btnCabQSOdate->value(true);
	btnCabQSOdate->value(false);
	btnCabTimeOFF->value(false);
	btnCabRSTsent->value(true);
	btnCabRSTrcvd->value(true);
	btnCabSerialIN->value(true);
	btnCabSerialOUT->value(true);
	btnCabXchgIn->value(true);
	btnCabMyXchg->value(true);

	switch (contestnbr) {
		case ARRL_SS_CW :
		case ARRL_SS_SSB :
			btnCabRSTrcvd->value(false);
			break;
		case BARTG_RTTY :
		case BARTG_SPRINT :
			break;
		case ARRL_UHF_AUG :
		case ARRL_VHF_JAN :
		case ARRL_VHF_JUN :
		case ARRL_VHF_SEP :
		case CQ_VHF :
			btnCabRSTrcvd->value(false);
			btnCabSerialIN->value(false);
			btnCabSerialOUT->value(false);
			break;
		case AP_SPRINT :
		case ARRL_10 :
		case ARRL_160 :
		case ARRL_DX_CW :
		case ARRL_DX_SSB :
		case CQ_160_CW :
		case CQ_160_SSB :
		case CQ_WPX_CW :
		case CQ_WPX_RTTY :
		case CQ_WPX_SSB :
		case RDXC :
		case OCEANIA_DX_CW :
		case OCEANIA_DX_SSB :
			break;
		case DARC_WAEDC_CW :
		case DARC_WAEDC_RTTY :
		case DARC_WAEDC_SSB :
			break;
		case NAQP_CW :
		case NAQP_RTTY :
		case NAQP_SSB :
		case NA_SPRINT_CW :
		case NA_SPRINT_SSB :
			break;
		case RSGB_IOTA :
			break;
		default :
			break;
	}
}

void cb_Export_Cabrillo() {
	if (qsodb.nbrRecs() == 0) return;
	cQsoRec *rec;
	char line[80];
	int indx = 0;

	if (bInitCombo) {
		bInitCombo = false;
		while (contests[indx])  {
			cboContest->add(contests[indx]);
			indx++;
		}
	}
	cboContest->index(0);
	chkCabBrowser->clear();
	chkCabBrowser->textfont(FL_COURIER);
	chkCabBrowser->textsize(12);
	for( int i = 0; i < qsodb.nbrRecs(); i++ ) {
		rec = qsodb.getRec (i);
		memset(line, 0, sizeof(line));
		snprintf(line,sizeof(line),"%8s|%6s|%-10s|%10s|%-s",
			rec->getField(QSO_DATE),
			time4(rec->getField(TIME_OFF)),
			rec->getField(CALL),
			szfreq(rec->getField(FREQ)),
			rec->getField(MODE) );
		chkCabBrowser->add(line);
	}
	wCabrillo->show();
}

void cabrillo_append_qso (FILE *fp, cQsoRec *rec)
{
	char freq[16] = "";
	string rst_in, rst_out, exch_in, exch_out, date, time, mode, mycall, call;
	string qsoline = "QSO: ";
	int rst_len = 3;
	int ifreq = 0;
	size_t len = 0;

	exch_out.clear();

	if (btnCabFreq->value()) {
		ifreq = (int)(1000.0 * atof(rec->getField(FREQ)));
		snprintf(freq, sizeof(freq), "%d", ifreq);
		qsoline.append(freq); qsoline.append(" ");
	}

	if (btnCabMode->value()) {
		mode = rec->getField(MODE);
		if (mode.compare("USB") == 0 || mode.compare("LSB") == 0 ||
			mode.compare("PH") == 0 ) mode = "PH";
		else if (mode.compare("FM") == 0 || mode.compare("CW") == 0 ) ;
		else mode = "RY";
		if (mode.compare("PH") == 0 || mode.compare("FM") == 0 ) rst_len = 2;
		qsoline.append(mode); qsoline.append(" ");
	}

	if (btnCabQSOdate->value()) {
		date = rec->getField(QSO_DATE);
		date.insert(4,"-");
		date.insert(7,"-");
		qsoline.append(date); qsoline.append(" ");
	}

	if (btnCabTimeOFF->value()) {
		time = time4(rec->getField(TIME_OFF));
		qsoline.append(time); qsoline.append(" ");
	}

	if (btnCabQSOdate->value()) {
		date = rec->getField(QSO_DATE_OFF);
		date.insert(4,"-");
		date.insert(7,"-");
		qsoline.append(date); qsoline.append(" ");
	}

	if (btnCabTimeOFF->value()) {
		time = time4(rec->getField(TIME_OFF));
		qsoline.append(time); qsoline.append(" ");
	}

	mycall = progStatus.mycall;
	if (mycall.length() > 13) mycall = mycall.substr(0,13);
	if ((len = mycall.length()) < 13) mycall.append(13 - len, ' ');
	qsoline.append(mycall); qsoline.append(" ");

	if (btnCabRSTsent->value()) {
		rst_out = rec->getField(RST_SENT);
		rst_out = rst_out.substr(0,rst_len);
		exch_out.append(rst_out).append(" ");
	}

	if (btnCabSerialOUT->value()) {
		exch_out.append(rec->getField(STX)).append(" ");
	}

	if (btnCabMyXchg->value()) {
		exch_out.append(rec->getField(MYXCHG)).append(" ");
	}

	if (contestnbr == BARTG_RTTY && exch_out.length() < 11) {
		exch_out.append(rec->getField(TIME_OFF)).append(" ");
	}

	if (exch_out.length() > 14) exch_out = exch_out.substr(0,14);
	if ((len = exch_out.length()) < 14) exch_out.append(14 - len, ' ');

	qsoline.append(exch_out).append(" ");

	if (btnCabCall->value()) {
		call = rec->getField(CALL);
		if (call.length() > 13) call = call.substr(0,13);
		if ((len = call.length()) < 13) call.append(13 - len, ' ');
		qsoline.append(call); qsoline.append(" ");
	}

	if (btnCabRSTrcvd->value()) {
		rst_in = rec->getField(RST_RCVD);
		rst_in = rst_in.substr(0,rst_len);
		qsoline.append(rst_in); qsoline.append(" ");
	}

	if (btnCabSerialIN->value()) {
		exch_in = rec->getField(SRX);
		if (exch_in.length())
			exch_in += ' ';
	}
	if (btnCabXchgIn->value())
		exch_in.append(rec->getField(XCHG1));
	if (exch_in.length() > 14) exch_in = exch_in.substr(0,14);
	if ((len = exch_in.length()) < 14) exch_in.append(14 - len, ' ');
	qsoline.append(exch_in);

	fprintf (fp, "%s\n", qsoline.c_str());
	return;
}

void WriteCabrillo()
{
	if (chkCabBrowser->nchecked() == 0) return;

	cQsoRec *rec;

	string filters = "TEXT\t*.txt";
	string strContest = "";

	const char* p = FSEL::saveas(_("Create cabrillo report"), filters.c_str(),
					 "contest.txt");
	if (!p)
		return;

	for (int i = 0; i < chkCabBrowser->nitems(); i++) {
		if (chkCabBrowser->checked(i + 1)) {
			rec = qsodb.getRec(i);
			rec->putField(EXPORT, "E");
			qsodb.qsoUpdRec (i, rec);
		}
	}

	FILE *cabFile = fopen (p, "w");
	if (!cabFile)
		return;

	strContest = cboContest->value();
	contestnbr = (icontest)cboContest->index();

	fprintf (cabFile,
"START-OF-LOG: 3.0\n\
CREATED-BY: %s %s\n\
\n\
# The callsign used during the contest.\n\
CALLSIGN: %s\n\
\n\
# ASSISTED or NON-ASSISTED\n\
CATEGORY-ASSISTED: \n\
\n\
# Band: ALL, 160M, 80M, 40M, 20M, 15M, 10M, 6M, 2M, 222, 432, 902, 1.2G\n\
CATEGORY-BAND: \n\
\n\
# Mode: SSB, CW, RTTY, MIXED \n\
CATEGORY-MODE: \n\
\n\
# Operator: SINGLE-OP, MULTI-OP, CHECKLOG \n\
CATEGORY-OPERATOR: \n\
\n\
# Power: HIGH, LOW, QRP \n\
CATEGORY-POWER: \n\
\n\
# Station: FIXED, MOBILE, PORTABLE, ROVER, EXPEDITION, HQ, SCHOOL \n\
CATEGORY-STATION: \n\
\n\
# Time: 6-HOURS, 12-HOURS, 24-HOURS \n\
CATEGORY-TIME: \n\
\n\
# Transmitter: ONE, TWO, LIMITED, UNLIMITED, SWL \n\
CATEGORY-TRANSMITTER: \n\
\n\
# Overlay: ROOKIE, TB-WIRES, NOVICE-TECH, OVER-50 \n\
CATEGORY-OVERLAY: \n\
\n\
# Integer number\n\
CLAIMED-SCORE: \n\
\n\
# Name of the radio club with which the score should be aggregated.\n\
CLUB: \n\
\n\
# Contest: AP-SPRINT, ARRL-10, ARRL-160, ARRL-DX-CW, ARRL-DX-SSB, ARRL-SS-CW,\n\
# ARRL-SS-SSB, ARRL-UHF-AUG, ARRL-VHF-JAN, ARRL-VHF-JUN, ARRL-VHF-SEP,\n\
# ARRL-RTTY, BARTG-RTTY, CQ-160-CW, CQ-160-SSB, CQ-WPX-CW, CQ-WPX-RTTY,\n\
# CQ-WPX-SSB, CQ-VHF, CQ-WW-CW, CQ-WW-RTTY, CQ-WW-SSB, DARC-WAEDC-CW,\n\
# DARC-WAEDC-RTTY, DARC-WAEDC-SSB, FCG-FQP, IARU-HF, JIDX-CW, JIDX-SSB,\n\
# NAQP-CW, NAQP-RTTY, NAQP-SSB, NA-SPRINT-CW, NA-SPRINT-SSB, NCCC-CQP,\n\
# NEQP, OCEANIA-DX-CW, OCEANIA-DX-SSB, RDXC, RSGB-IOTA, SAC-CW, SAC-SSB,\n\
# STEW-PERRY, TARA-RTTY \n\
CONTEST: %s\n\
\n\
# Optional email address\n\
EMAIL: \n\
\n\
LOCATION: \n\
\n\
# Operator name\n\
NAME: \n\
\n\
# Maximum 4 address lines.\n\
ADDRESS: \n\
ADDRESS: \n\
ADDRESS: \n\
ADDRESS: \n\
\n\
# A space-delimited list of operator callsign(s). \n\
OPERATORS: \n\
\n\
# Offtime yyyy-mm-dd nnnn yyyy-mm-dd nnnn \n\
# OFFTIME: \n\
\n\
# Soapbox comments.\n\
SOAPBOX: \n\
SOAPBOX: \n\
SOAPBOX: \n\n",
		PACKAGE_NAME, PACKAGE_VERSION,
		"",//progStatus.tag_cll.c_str(),
//	  progStatus.myCall.c_str(),
		strContest.c_str() );

	qsodb.SortByDate();
	for (int i = 0; i < qsodb.nbrRecs(); i++) {
		rec = qsodb.getRec(i);
		if (rec->getField(EXPORT)[0] == 'E') {
			cabrillo_append_qso(cabFile, rec);
			rec->putField(EXPORT,"");
			qsodb.qsoUpdRec(i, rec);
		}
	}
	fprintf(cabFile, "END-OF-LOG:\n");
	fclose (cabFile);
	return;
}

/*
#include <tr1/unordered_map>
typedef tr1::unordered_map<string, unsigned> dxcc_entity_cache_t;
static dxcc_entity_cache_t dxcc_entity_cache;
static bool dxcc_entity_cache_enabled = false;

#include "dxcc.h"

static void dxcc_entity_cache_clear(void)
{
	if (dxcc_entity_cache_enabled)
		dxcc_entity_cache.clear();
}

static void dxcc_entity_cache_add(cQsoRec* r)
{
	if (!dxcc_entity_cache_enabled | !r)
		return;

	const dxcc* e = dxcc_lookup(r->getField(CALL));
	if (e)
		dxcc_entity_cache[e->country]++;
}
static void dxcc_entity_cache_add(cQsoDb& db)
{
	if (!dxcc_entity_cache_enabled)
		return;

	int n = db.nbrRecs();
	for (int i = 0; i < n; i++)
		dxcc_entity_cache_add(db.getRec(i));
	if (!dxcc_entity_cache.empty())
		LOG_INFO("Found %" PRIuSZ " countries in %d QSO records",
			 dxcc_entity_cache.size(), n);
}

static void dxcc_entity_cache_rm(cQsoRec* r)
{
	if (!dxcc_entity_cache_enabled || !r)
		return;

	const dxcc* e = dxcc_lookup(r->getField(CALL));
	if (!e)
		return;
	dxcc_entity_cache_t::iterator i = dxcc_entity_cache.find(e->country);
	if (i != dxcc_entity_cache.end()) {
		if (i->second)
			i->second--;
		else
			dxcc_entity_cache.erase(i);
	}
}

void dxcc_entity_cache_enable(bool v)
{
	if (dxcc_entity_cache_enabled == v)
		return;

	dxcc_entity_cache_clear();
	if ((dxcc_entity_cache_enabled = v))
		dxcc_entity_cache_add(qsodb);
}

bool qsodb_dxcc_entity_find(const char* country)
{
	return dxcc_entity_cache.find(country) != dxcc_entity_cache.end();
}
*/
