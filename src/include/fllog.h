#ifndef _LOG_H
#define _LOG_H

#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <pthread.h>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Enumerations.H>

#ifndef WIN32
#include <unistd.h>
#include <pwd.h>
#endif

#include <FL/fl_ask.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Color_Chooser.H>
#include <FL/fl_draw.H>

#include "config.h"
#include "support.h"

extern Fl_Double_Window *mainwindow;
extern std::string LogHomeDir;

extern std::string defFileName;
extern std::string title;

extern bool LOG_DEBUG;

extern void LOGBOOK_colors_font();
extern void setColorsFonts();

#endif
