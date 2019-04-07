#ifndef LOG_SUPPORT_H
#define LOG_SUPPORT_H

#include <string>

#include <FL/Fl_Widget.H>
#include <FL/Fl_Menu_.H>

#include "qso_db.h"
#include "adif_io.h"

#include "lgbook.h"

#ifdef __WOE32__
	#define ADIF_SUFFIX "adi"
#else
	#define ADIF_SUFFIX "adif"
#endif

enum savetype {ADIF, CSV, TEXT, LO};

extern cQsoDb        qsodb;
extern cAdifIO       adifFile;
extern std::string logbook_filename;
extern std::string sDate_on;

extern void loadBrowser(bool keep_pos = false);

extern void Export_log();
extern void cb_SortByCall();
extern void cb_SortByDate();
extern void cb_SortByMode();
extern void cb_SortByFreq();
extern void cb_browser(Fl_Widget *, void *);

extern void cb_mnuNewLogbook();
extern void cb_mnuOpenLogbook();
extern void cb_mnuSaveLogbook();
extern void cb_mnuMergeADIF_log();
extern void cb_mnuExportADIF_log();
extern void cb_mnuExportCSV_log();
extern void cb_mnuExportTEXT_log();
extern void cb_mnuEvents();
extern void cb_Export_Cabrillo();

extern void saveLogbook();
extern void OpenLogbook();

extern void activateButtons();
extern void saveRecord ();
extern void clearRecord ();
extern void updateRecord ();
extern void deleteRecord ();
extern void AddRecord ();
extern void EditRecord(int);
extern void SearchLastQSO (const char *);
extern cQsoRec* SearchLog(const char *callsign);
extern void DupCheck();
extern void cb_search(Fl_Widget* w, void*);
extern int log_search_handler(int);
extern void restore_sort();
extern void EditRecord(int);
extern std::string fetch_record(const char *);

extern char *szTime(int typ);
extern char *szDate(int fmt);

extern void cb_doExport();

extern void WriteCabrillo();

extern void dxcc_entity_cache_enable(bool v);
bool qsodb_dxcc_entity_find(const char* country);

extern void cb_export_date_select();

#endif
