#ifndef SUPPORT_H
#define SUPPORT_H

#include <fstream>
#include <vector>
#include <string>

#include <math.h>
#ifndef WIN32
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#endif

#include "status.h"

#include <FL/fl_show_colormap.H>
#include <FL/fl_ask.H>

extern void cb_events();
extern void cbExit();

extern int  main_handler(int);

extern void close_logbook();

extern void about();
extern void on_line_help();

extern void start_server(int);
extern void exit_server();

#endif
