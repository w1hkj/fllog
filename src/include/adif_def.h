#ifndef ADIF_DEF
#define ADIF_DEF

#include <string>
#include <cstring>

#include <FL/Fl_Check_Button.H>

#include "field_def.h"

using namespace std;

struct FIELD {
  int  type;
  string *name;
//  int  len;
//  int  size;
  Fl_Check_Button **btn;
};

extern FIELD fields[];
//extern int numfields;

#endif
