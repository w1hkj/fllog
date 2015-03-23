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
#include "debug.h"
#include "gettext.h"
#include "status.h"
#include "logbook.h"

using namespace std;

void upcase(string &s)
{
	for (size_t n = 0; n < s.length(); n++) s[n] = toupper(s[n]);
}

void cbExit()
{

	progStatus.saveLastState();

	close_logbook();

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
XmlRpcServer s;
cAdifIO xml_adif;

// Request record if it exists else return "NO_RECORD"
// Returns ADIF record
class log_get_record : public XmlRpcServerMethod
{
public:
  log_get_record(XmlRpcServer* s) : XmlRpcServerMethod("log.get_record", s) {}

  void execute(XmlRpcValue& params, XmlRpcValue& result)
  {
	std::string callsign = std::string(params[0]);
    std::string resultString = fetch_record(callsign.c_str());
	result = resultString;
  }

  std::string help() { return std::string("log.get_record CALL"); }

} log_get_record(&s);    // This constructor registers the method with the server

// Arguments: CALLSIGN MODE TIME_SPAN FREQ
class log_check_dup : public XmlRpcServerMethod
{
public:
  log_check_dup(XmlRpcServer* s) : XmlRpcServerMethod("log.check_dup", s) {}

  void execute(XmlRpcValue& params, XmlRpcValue& result)
  {
	if (params.size() != 6) {
		result = "Wrong # parameters";
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
	bool res = qsodb.duplicate(
			callsign.c_str(),
			(const char *)szDate(6), (const char *)szTime(0), (unsigned int)ispn, bspn,
			freq.c_str(), bfreq,
			state.c_str(), bstate,
			mode.c_str(), bmode,
			xchg_in.c_str(), bxchg);
	result = (res ? "true" : "false");
	}

	std::string help() { 
		return std::string("log.check_dup CALL, MODE(0), TIME_SPAN(0), FREQ_HZ(0), STATE(0), XCHG_IN(0)"); 
	}

} log_check_dup(&s);

void updateBrowser(void *)
{
	loadBrowser(false);
}

// One argument is passed, result is "Hello, " + arg.
class log_add_record : public XmlRpcServerMethod
{
public:
  log_add_record(XmlRpcServer* s) : XmlRpcServerMethod("log.add_record", s) {}

  void execute(XmlRpcValue& params, XmlRpcValue& result)
  {
    std::string adif_record = std::string(params[0]);
	xml_adif.add_record(adif_record.c_str(), qsodb);
	Fl::awake(updateBrowser);
  }
	std::string help() { return std::string("log.add_record ADIF RECORD"); }

} log_add_record(&s);

pthread_t *xml_thread = 0;

void * xml_thread_loop(void *d)
{
	for(;;) {
		s.work(-1.0);
	}
	return NULL;
}

void start_server(int port)
{
	XmlRpc::setVerbosity(0);

// Create the server socket on the specified port
	s.bindAndListen(port);

// Enable introspection
	s.enableIntrospection(true);

	xml_thread = new pthread_t;
	if (pthread_create(xml_thread, NULL, xml_thread_loop, NULL)) {
		perror("pthread_create");
		exit(EXIT_FAILURE);
	}
}

void exit_server()
{
	s.exit();
}


