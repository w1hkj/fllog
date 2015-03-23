// ---------------------------------------------------------------------
//
//      logbook.cxx
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

