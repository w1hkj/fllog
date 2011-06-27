#include <config.h>

#include <cstring>

#include <FL/Fl.H>
#include <FL/filename.H>

#include "fllog.h"
#include "logbook.h"
#include "debug.h"
#include "status.h"

using namespace std;

std::string log_checksum;

void do_load_browser(void *)
{
	loadBrowser();
}

void start_logbook ()
{
	if (progStatus.logbookfilename.empty()) {
		logbook_filename = LogHomeDir;
		logbook_filename.append("logbook." ADIF_SUFFIX);
		progStatus.logbookfilename = logbook_filename;
	} else
		logbook_filename = progStatus.logbookfilename;

	adifFile.readFile (logbook_filename.c_str(), &qsodb);
	if (qsodb.nbrRecs() == 0)
		adifFile.writeFile(logbook_filename.c_str(), &qsodb);

	txtLogFile->value(progStatus.logbookfilename.c_str());
	txtLogFile->redraw();

	string label = "Logbook Server - ";
	label.append(fl_filename_name(logbook_filename.c_str()));
	dlgLogbook->copy_label(label.c_str());

	restore_sort();
	loadBrowser();

	qsodb.isdirty(0);
	activateButtons();
}

void close_logbook()
{
	saveLogbook();
}

