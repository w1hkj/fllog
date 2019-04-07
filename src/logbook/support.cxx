// ---------------------------------------------------------------------
//
//      support.cxx, a part of fllog
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

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Enumerations.H>

#include "support.h"
#include "logsupport.h"
#include "debug.h"
#include "gettext.h"
#include "status.h"
#include "logbook.h"
#include "threads.h"

using namespace std;

static pthread_mutex_t exec_mutex = PTHREAD_MUTEX_INITIALIZER;

void upcase(string &s)
{
	for (size_t n = 0; n < s.length(); n++) s[n] = toupper(s[n]);
}

void cbExit()
{
	progStatus.saveLastState();

	qsodb.SortByDate();
	adifFile.writeLog (logbook_filename.c_str(), &qsodb, true);
	restore_sort();

	exit(0);
}

int main_handler(int event)
{
	if (event != FL_SHORTCUT)
		return 0;
	Fl_Widget* w = Fl::focus();

	if (w == mainwindow || w->window() == mainwindow) {
		int key = Fl::event_key();
		if (key == FL_Escape) 
			return 1;
	}
	return 0;
}

//======================================================================
// xmlrpc support
//======================================================================
#include "XmlRpc.h"

using namespace XmlRpc;

// The server
XmlRpcServer log_server;
cAdifIO xml_adif;

// Request record if it exists else return "NO_RECORD"
// Returns ADIF record
class log_get_record : public XmlRpcServerMethod
{
public:
  log_get_record(XmlRpcServer* log_server) : XmlRpcServerMethod("log.get_record", log_server) {}

  void execute(XmlRpcValue& params, XmlRpcValue& result)
  {
	std::string callsign = std::string(params[0]);
	std::string resultString = fetch_record(callsign.c_str());

	if (resultString.empty()) {
		result = "NO RECORD";
	} else {
		result = resultString;
		std::string st = "Found record for: ";
		st.append(callsign);
		LOG_INFO("%s", st.c_str());
	}

  }

  std::string help() { return std::string("log.get_record CALL"); }

} log_get_record(&log_server);    // This constructor registers the method with the server

// Arguments: CALLSIGN MODE TIME_SPAN FREQ
static int duprecnbr;
void goto_dup_rec(void *)
{
	guard_lock elock(&exec_mutex);

	EditRecord(duprecnbr);

	inpSearchString->value((qsodb.getRec(duprecnbr))->getField(CALL));

	int brow = 0;
	int bcol = 6;
	char szrec[10];
	snprintf(szrec, sizeof(szrec), "%d", duprecnbr);
	wBrowser->search(brow, bcol, false, szrec);
	wBrowser->GotoRow(brow);
}

class log_check_dup : public XmlRpcServerMethod
{
public:
	log_check_dup(XmlRpcServer* log_server) : XmlRpcServerMethod("log.check_dup", log_server) {}

	void execute(XmlRpcValue& params, XmlRpcValue& result)
	{
		if (params.size() != 6) {
			result = "Wrong # parameters";
			LOG_INFO("%s", "Wrong # parameters");
			return;
		}
		std::string callsign = std::string(params[0]);
		std::string mode = std::string(params[1]);
		std::string spn = std::string(params[2]);
		std::string freq = std::string(params[3]);
		std::string state = std::string(params[4]);
		std::string xchg_in = std::string(params[5]);
		int ispn = atoi(spn.c_str());
		int ifreq = atoi(freq.c_str());
		bool bspn = (ispn > 0);
		bool bfreq = (ifreq > 0);
		bool bmode = (mode != "0");
		bool bstate = (state != "0");
		bool bxchg = (xchg_in != "0");
		std::string st;
		int res = 0;
		{
			guard_lock elock(&exec_mutex);
			res = qsodb.duplicate(
				callsign.c_str(),
				(const char *)szDate(6), (const char *)szTime(0), (unsigned int)ispn, bspn,
				freq.c_str(), bfreq,
				state.c_str(), bstate,
				mode.c_str(), bmode,
				xchg_in.c_str(), bxchg, duprecnbr);
		}
		switch (res) {
			case 1:
				result = "true"; 
				st = "Duplicate: ";
				break;
			case 2:
				result = "possible";
				st = "Possible dup: ";
				break;
			case 0:
			default:  result = "false";
		}

		if (res > 0) {
			st.append(callsign);
			LOG_INFO("%s", st.c_str());
			Fl::awake(goto_dup_rec);
		}

	}

	std::string help() { 
		return std::string("log.check_dup CALL, MODE(0), TIME_SPAN(0), FREQ_HZ(0), STATE(0), XCHG_IN(0)"); 
	}

} log_check_dup(&log_server);

static std::string adif_add_record;
static void add_record(void *)
{
	guard_lock elock(&exec_mutex);

	xml_adif.add_record(adif_add_record.c_str(), qsodb);
	loadBrowser(false);
}

class log_add_record : public XmlRpcServerMethod
{
public:
	log_add_record(XmlRpcServer* log_server) : XmlRpcServerMethod("log.add_record", log_server) {}

	void execute(XmlRpcValue& params, XmlRpcValue& result)
	{
		{
			guard_lock elock(&exec_mutex);
			adif_add_record = std::string(params[0]);
		}
		Fl::awake(add_record);
	}
	std::string help() { return std::string("log.add_record ADIF RECORD"); }

} log_add_record(&log_server);

static std::string adif_update_record;
static void update_record(void *)
{
	guard_lock elock(&exec_mutex);
	xml_adif.update_record(adif_update_record.c_str(), qsodb);
}

class log_update_record : public XmlRpcServerMethod
{
public:
	log_update_record(XmlRpcServer* log_server) : XmlRpcServerMethod("log.update_record", log_server) {}

	void execute(XmlRpcValue& params, XmlRpcValue& result)
	{
		{
			guard_lock elock(&exec_mutex);
			adif_update_record = std::string(params[0]);
		}
		Fl::awake(update_record);
	}
	std::string help() { return std::string("log.update_record ADIF RECORD"); }

} log_update_record(&log_server);

struct MLIST {
	string name; string signature; string help;
} mlist[] = {
	{ "log.add_record",    "s:s", "adds new ADIF-RECORD" },
	{ "log.get_record",    "s:s", "returns ADIF-RECORD for CALL" },
	{ "log.update_record", "s:s", "updates current record with specified ADIF-RECORD" },
	{ "log.check_dup",     "s:s", "return true/false/possible for ADIF record" },
	{ "log.list_methods",  "s:s", "return this list" }
};

class log_list_methods : public XmlRpcServerMethod {
public:
	log_list_methods(XmlRpcServer *log_server) : XmlRpcServerMethod("log.list_methods", log_server) {}

	void execute(XmlRpcValue& params, XmlRpcValue& result) {

		vector<XmlRpcValue> methods;
		for (size_t n = 0; n < sizeof(mlist) / sizeof(*mlist); ++n) {
			XmlRpcValue::ValueStruct item;
			item["name"]      = mlist[n].name;
			item["signature"] = mlist[n].signature;
			item["help"]      = mlist[n].help;
			methods.push_back(item);
		}

		result = methods;
	}
	std::string help() { return std::string("get fllog methods"); }
} log_list_methods(&log_server);


pthread_t *xml_thread = 0;

void * xml_thread_loop(void *d)
{
	for(;;) {
		log_server.work(-1.0);
	}
	return NULL;
}

void start_server(int port)
{
	XmlRpc::setVerbosity(0);

// Create the server socket on the specified port
	log_server.bindAndListen(port);

// Enable introspection
	log_server.enableIntrospection(true);

	xml_thread = new pthread_t;
	if (pthread_create(xml_thread, NULL, xml_thread_loop, NULL)) {
		perror("pthread_create");
		exit(EXIT_FAILURE);
	}
}

void exit_server()
{
	log_server.exit();
}

