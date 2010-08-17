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
