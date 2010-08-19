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

pthread_t logbook_thread;
pthread_mutex_t logbook_mutex = PTHREAD_MUTEX_INITIALIZER;
bool logbook_exit = false;

void do_load_browser(void *)
{
	loadBrowser();
}

static void *logbook_loop(void *args)
{
	int cnt = 5;
	for (;;) {
	/* see if we are being canceled */
		if (logbook_exit)
			break;
		if (!cnt--) {
			cnt = 5;
			pthread_mutex_lock (&logbook_mutex);
			if (adifFile.log_changed(logbook_filename.c_str())) {
				qsodb.deleteRecs();
				adifFile.readFile (logbook_filename.c_str(), &qsodb);
				Fl::awake(do_load_browser);
				qsodb.isdirty(0);
			}
			pthread_mutex_unlock (&logbook_mutex);
		}
		MilliSleep(100);
	}
// exit the arq thread
	return NULL;
}

void start_logbook ()
{
//	create_logbook_dialogs();

	if (progStatus.logbookfilename.empty()) {
		logbook_filename = LogHomeDir;
		logbook_filename.append("logbook." ADIF_SUFFIX);
		progStatus.logbookfilename = logbook_filename;
	} else
		logbook_filename = progStatus.logbookfilename;

	adifFile.readFile (logbook_filename.c_str(), &qsodb);
	if (qsodb.nbrRecs() == 0)
		adifFile.writeFile(logbook_filename.c_str(), &qsodb);

	string label = "Logbook - ";
	label.append(fl_filename_name(logbook_filename.c_str()));
	dlgLogbook->copy_label(label.c_str());

	loadBrowser();
	qsodb.isdirty(0);
	activateButtons();

//	if (pthread_create(&logbook_thread, NULL, logbook_loop, NULL) < 0)
//		LOG_ERROR("%s", "pthread_create failed");
}

void close_logbook()
{
	saveLogbook();
// tell the logbook thread to kill it self
//	logbook_exit = true;
// and then wait for it to die
//	pthread_join(logbook_thread, NULL);
//	logbook_exit = false;
}

