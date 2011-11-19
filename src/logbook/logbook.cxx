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

void start(void *)
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

	dlgLogbook->copy_label("Logbook Server");

	restore_sort();
	loadBrowser();

	qsodb.isdirty(0);
	activateButtons();
}

void start_logbook ()
{
	Fl::add_timeout(0.2, start);
}

void close_logbook()
{
	saveLogbook();
}

