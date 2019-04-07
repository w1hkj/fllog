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
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#include <FL/Fl.H>
#include <FL/filename.H>

#include "fllog.h"
#include "logbook.h"
#include "debug.h"
#include "status.h"

using namespace std;

std::string log_checksum;

void rotate_log(std::string filename)
{
	FILE *fp;
	std::string oldfn, newfn;
	const char *ext[] = {".5", ".4", ".3", ".2", ".1"};

	for (int i = 0; i < 4; i++) {
		newfn.assign(filename).append(ext[i]);
		oldfn.assign(filename).append(ext[i+1]);
		if ((fp = fopen(oldfn.c_str(), "r")) == NULL)
			continue;
		fclose(fp);
		rename(oldfn.c_str(), newfn.c_str());
	}

	newfn.assign(filename).append(ext[4]);
	char buffer[65536];

	FILE *original = fopen(filename.c_str(), "rb");
	if (original) {
		FILE *backup = fopen(newfn.c_str(), "wb");
		if (backup) {
			size_t n;
			while (1) {
				memset(buffer, 0, sizeof(buffer));
				n = fread(buffer, 1, sizeof(buffer), original);
				n = fwrite(buffer, 1, n, backup);
				if (feof(original)) {
					break;
				}
			}
			fflush(backup);
			fclose(backup);
		}
		fclose(original);
	}
}

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

	qsodb.deleteRecs();
	adifFile.readFile (logbook_filename.c_str(), &qsodb);

	rotate_log(logbook_filename);

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

