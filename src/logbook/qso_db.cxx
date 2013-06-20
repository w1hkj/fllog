// ---------------------------------------------------------------------
//
//      qso_db.cxx
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
// along with the program; if not, write to the Free Software
// Foundation, Inc.
// 59 Temple Place, Suite 330
// Boston, MA  02111-1307 USA
//
// ---------------------------------------------------------------------

#include <config.h>
#include <fstream>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <iostream>
#include "qso_db.h"
#include "field_def.h"
#include "util.h"

// following needed for localtime_r
#include <pthread.h>
#include "timeops.h"

using namespace std;

// =====================================================================
// support for band / frequency translations
// =====================================================================

enum band_t {
	BAND_160M, BAND_80M, BAND_75M, BAND_60M, BAND_40M, BAND_30M, BAND_20M,
	BAND_17M, BAND_15M, BAND_12M, BAND_10M, BAND_6M, BAND_4M, BAND_2M, BAND_125CM,
	BAND_70CM, BAND_33CM, BAND_23CM, BAND_13CM, BAND_9CM, BAND_6CM, BAND_3CM, BAND_125MM,
	BAND_6MM, BAND_4MM, BAND_2P5MM, BAND_2MM, BAND_1MM, BAND_OTHER, NUM_BANDS
};

band_t band(long long freq_hz);
band_t band(const char* freq_mhz);
const char* band_name(band_t b);
const char* band_name(const char* freq_mhz);
const char* band_freq(band_t b);
const char* band_freq(const char* band_name);

band_t band(long long freq_hz)
{
	switch (freq_hz / 1000000LL) {
		case 0: case 1: return BAND_160M;
		case 3: return BAND_80M;
		case 4: return BAND_75M;
		case 5: return BAND_60M;
		case 7: return BAND_40M;
		case 10: return BAND_30M;
		case 14: return BAND_20M;
		case 18: return BAND_17M;
		case 21: return BAND_15M;
		case 24: return BAND_12M;
		case 28 ... 29: return BAND_10M;
		case 50 ... 54: return BAND_6M;
		case 70 ... 71: return BAND_4M;
		case 144 ... 148: return BAND_2M;
		case 222 ... 225: return BAND_125CM;
		case 420 ... 450: return BAND_70CM;
		case 902 ... 928: return BAND_33CM;
		case 1240 ... 1325: return BAND_23CM;
		case 2300 ... 2450: return BAND_13CM;
		case 3300 ... 3500: return BAND_9CM;
		case 5650 ... 5925: return BAND_6CM;
		case 10000 ... 10500: return BAND_3CM;
		case 24000 ... 24250: return BAND_125MM;
		case 47000 ... 47200: return BAND_6MM;
		case 75500 ... 81000: return BAND_4MM;
		case 119980 ... 120020: return BAND_2P5MM;
		case 142000 ... 149000: return BAND_2MM;
		case 241000 ... 250000: return BAND_1MM;
	}

	return BAND_OTHER;
}

band_t band(const char* freq_mhz)
{
	double d = strtod(freq_mhz, NULL);
	if (d != 0.0)
		return band((long long)(d * 1e6));
	else
		return BAND_OTHER;
}

struct band_freq_t {
	const char* band;
	const char* freq;
};

static struct band_freq_t band_names[NUM_BANDS] = {
	{ "160m", "1.8" },
	{ "80m", "3.4" },
	{ "75m", "4.0" },
	{ "60m", "5.3" },
	{ "40m", "7.0" },
	{ "30m", "10.0" },
	{ "20m", "14.0" },
	{ "17m", "18.0" },
	{ "15m", "21.0" },
	{ "12m", "24.0" },
	{ "10m", "28.0" },
	{ "6m", "50.0" },
	{ "4m", "70.0" },
	{ "2m", "144.0" },
	{ "1.25m", "222.0" },
	{ "70cm", "420.0" },
	{ "33cm", "902.0" },
	{ "23cm", "1240.0" },
	{ "13cm", "2300.0" },
	{ "9cm", "3300.0" },
	{ "6cm", "5650.0" },
	{ "3cm", "10000.0" },
	{ "1.25cm", "24000.0" },
	{ "6mm", "47000.0" },
	{ "4mm", "75500.0" },
	{ "2.5mm", "119980.0" },
	{ "2mm", "142000.0" },
	{ "1mm", "241000.0" },
	{ "other", "" }
};

const char* band_name(band_t b)
{
	return band_names[CLAMP(b, 0, NUM_BANDS-1)].band;
}

const char* band_name(const char* freq_mhz)
{
	return band_name(band(freq_mhz));
}

const char* band_freq(band_t b)
{
	return band_names[CLAMP(b, 0, NUM_BANDS-1)].freq;
}

const char* band_freq(const char* band_name)
{
	for (size_t i = 0; i < BAND_OTHER; i++)
		if (!strcmp(band_names[i].band, band_name))
			return band_names[i].freq;

	return "";
}

//======================================================================
// class cQsoRec

static int compby = COMPDATE;
static bool date_off = true;

bool cQsoDb::reverse = false;

cQsoRec::cQsoRec() {
	for (int i=0;i < NUMFIELDS; i++)
		qsofield[i] = "";
}

cQsoRec::~cQsoRec () {
}

void cQsoRec::clearRec () {
	for (int i = 0; i < NUMFIELDS; i++)
		qsofield[i].clear();
}

int cQsoRec::validRec() {
	return 0;
}

void cQsoRec::checkBand() {
	size_t flen = qsofield[FREQ].length(), blen = qsofield[BAND].length();
	if (flen == 0 && blen != 0)
		qsofield[FREQ] =  band_freq(qsofield[BAND].c_str());
	else if (blen == 0 && flen != 0)
		qsofield[BAND] =  band_name(qsofield[FREQ].c_str());
}

void cQsoRec::putField (int n, const char *s){
	if (n < 0 || n >= NUMFIELDS) return;
	qsofield[n] = s;
}

void cQsoRec::putField (int n, const char *s, int len) {
	if (n < 0 || n >= NUMFIELDS) return;
	qsofield[n].clear();
	qsofield[n].append(s, len);
}

void cQsoRec::addtoField (int n, const char *s){
	if (n < 0 || n >= NUMFIELDS) return;
	qsofield[n].append(s);
}

void cQsoRec::checkDateTimes()
{
	size_t len1 = qsofield[TIME_ON].length(), len2 = qsofield[TIME_OFF].length();
	if (len1 == 0 && len2 != 0)
		qsofield[TIME_ON] = qsofield[TIME_OFF];
	else if (len1 != 0 && len2 == 0)
		qsofield[TIME_OFF] = qsofield[TIME_ON];
	len1 = qsofield[QSO_DATE].length();
	len2 = qsofield[QSO_DATE_OFF].length();
	if (len1 == 0 && len2 != 0)
		qsofield[QSO_DATE] = qsofield[QSO_DATE_OFF];
	else if (len1 != 0 && len2 == 0)
		qsofield[QSO_DATE_OFF] = qsofield[QSO_DATE];
}

// Sets the current time, with the right format.
void cQsoRec::setDateTime(bool dtOn)
{
	time_t tmp_time = time(NULL);
	struct tm tmp_tm ;
	if (localtime_r(&tmp_time, &tmp_tm)) {

		char buf_date[64] ;
		snprintf( buf_date, sizeof(buf_date),
			"%04d%02d%02d",
			1900 + tmp_tm.tm_year,
			1 + tmp_tm.tm_mon,
			tmp_tm.tm_mday );

		char buf_time[64] ;
		snprintf( buf_time, sizeof(buf_time),
			"%02d%02d%02d",
			tmp_tm.tm_hour,
			tmp_tm.tm_min,
			tmp_tm.tm_sec );

		if(dtOn) {
			putField(QSO_DATE, buf_date);
			putField(TIME_ON, buf_time);
		} else {
			putField(QSO_DATE_OFF, buf_date);
			putField(TIME_OFF, buf_time);
		}
	}
}

// It must match a specific format. Input in Hertz.
void cQsoRec::setFrequency(long long freq)
{
	double freq_dbl = freq / 1000000.0 ;
	char buf_freq[64];
	snprintf( buf_freq, sizeof(buf_freq), "%lf", freq_dbl );
	putField(FREQ, buf_freq );
}

void cQsoRec::trimFields () {
	size_t p;
	string s;
	for (int i = 0; i < NUMFIELDS; i++) {
		s = qsofield[i];
//right trim string
		p = s.length();
		while (p && s[p-1] == ' ') {
			s.erase(p - 1);
			p = s.length();
		}
//left trim string
		p = s.length();
		while (p && s[0] == ' ') {
			s.erase(0,1);
			p = s.length();
		}
//make all upper case if Callsign or Mode  
		if (i == CALL || i == MODE) {
			for (p = 0; p < s.length(); p++)
				s[p] = toupper(s[p]);
		}
		qsofield[i] = s;
	}
}

const char * cQsoRec::getField (int n) const {
	if (n < 0 || n >= NUMFIELDS) return 0;
	return (qsofield[n].c_str());
}

const cQsoRec &cQsoRec::operator=(const cQsoRec &right) {
	if (this != &right) {
		for (int i = 0; i < NUMFIELDS; i++)
			this->qsofield[i] = right.qsofield[i];
	}
	return *this;
}

int compareTimes (const cQsoRec &r1, const cQsoRec &r2) {
	if (date_off)
		return r1.qsofield[TIME_OFF].compare( r2.qsofield[TIME_OFF] );
	return r1.qsofield[TIME_ON].compare( r2.qsofield[TIME_ON] );
}

int compareDates (const cQsoRec &r1, const cQsoRec &r2) {
	if (date_off)
		return r1.qsofield[QSO_DATE_OFF].compare( r2.qsofield[QSO_DATE_OFF] );
	else
		return r1.qsofield[QSO_DATE].compare( r2.qsofield[QSO_DATE] );
}

int compareCalls (const cQsoRec &r1, const cQsoRec &r2) {
	int cmp = 0;
	const char * s1 = r1.qsofield[CALL].c_str();
	const char * s2 = r2.qsofield[CALL].c_str();
	const char * p1 = strpbrk (s1+1, "0123456789");
	const char * p2 = strpbrk (s2+1, "0123456789");

	if (p1 && p2) {
		cmp = (*p1 < *p2) ? -1 : (*p1 > *p2) ? 1 : 0;
		if (cmp == 0) {
			cmp = strncmp (s1, s2, max(p1 - s1, p2 - s2));
			if (cmp == 0)
				cmp = strcmp(p1+1, p2+1);
		}
	} else
		cmp = strcmp(s1, s2);
	return cmp;
}

int compareModes (const cQsoRec &r1, const cQsoRec &r2) {
	return r1.qsofield[MODE].compare( r2.qsofield[MODE] );
}

int compareFreqs (const cQsoRec &r1, const cQsoRec &r2) {
	double f1, f2;
	f1 = atof(r1.qsofield[FREQ].c_str());
	f2 = atof(r2.qsofield[FREQ].c_str());
	return (f1 == f2 ? 0 : f1 < f2 ? -1 : 1);
}

int compareqsos (const void *p1, const void *p2) {
	cQsoRec *r1, *r2;
	if (cQsoDb::reverse) {
		r2 = (cQsoRec *)p1;
		r1 = (cQsoRec *)p2;
	} else {
		r1 = (cQsoRec *)p1;
		r2 = (cQsoRec *)p2;
	}

	int cmp;
	switch (compby) {
		case COMPCALL :
			if ((cmp = compareCalls(*r1, *r2)) != 0) return cmp;
			if ((cmp = compareDates(*r1, *r2)) != 0) return cmp;
			if ((cmp = compareTimes(*r1, *r2)) != 0) return cmp;
			if ((cmp = compareModes(*r1, *r2)) != 0) return cmp;
			return compareFreqs(*r1, *r2);
		case COMPMODE :
			if ((cmp = compareModes (*r1, *r2)) != 0) return cmp;
			if ((cmp = compareDates(*r1, *r2)) != 0) return cmp;
			if ((cmp = compareTimes(*r1, *r2)) != 0) return cmp;
			if ((cmp = compareCalls(*r1, *r2)) != 0) return cmp;
			return compareFreqs(*r1, *r2);
		case COMPFREQ :
			if ((cmp = compareFreqs (*r1, *r2)) != 0) return cmp;
			if ((cmp = compareDates(*r1, *r2)) != 0) return cmp;
			if ((cmp = compareTimes(*r1, *r2)) != 0) return cmp;
			if ((cmp = compareCalls(*r1, *r2)) != 0) return cmp;
			return compareModes(*r1, *r2);
		case COMPDATE :
		default :
			if ((cmp = compareDates (*r1, *r2)) != 0) return cmp;
			if ((cmp = compareTimes(*r1, *r2)) != 0) return cmp;
			if ((cmp = compareCalls(*r1, *r2)) != 0) return cmp;
			if ((cmp = compareModes (*r1, *r2)) != 0) return cmp;
			return compareFreqs (*r1, *r2);
	}
}

bool cQsoRec::operator==(const cQsoRec &right) const {
	if (compareDates (*this, right) != 0) return false;
	if (compareTimes (*this, right) != 0) return false;
	if (compareCalls (*this, right) != 0) return false;
	if (compareFreqs (*this, right) != 0) return false;
	return true;
}

bool cQsoRec::operator<(const cQsoRec &right) const {
	if (compareDates (*this, right) > -1) return false;
	if (compareTimes (*this, right) > -1) return false;
	if (compareCalls (*this, right) > -1) return false;
	if (compareFreqs (*this, right) > -1) return false;
	return true;
}

static char delim_in = '\t';
static char delim_out = '\t';
static bool isVer3 = false;

ostream &operator<< (ostream &output, const cQsoRec &rec) {
	for (int i = 0; i < EXPORT; i++)
		output << rec.qsofield[i].c_str() << delim_out;
	return output;
}

istream &operator>> (istream &input, cQsoRec &rec ) {
	static char buf[1024]; // Must be big enough for a field.
	for (int i = 0; i < NUMFIELDS; i++) {
		input.getline( buf, sizeof(buf), delim_in );
		rec.qsofield[i] = buf ;
	}
	return input;
}

//======================================================================
// class cQsoDb

#define MAXRECS 32768
#define INCRRECS 8192

cQsoDb::cQsoDb() {
  nbrrecs = 0;
  maxrecs = MAXRECS;
  qsorec = new cQsoRec[maxrecs];
  compby = COMPDATE;
  dirty = 0;
}

cQsoDb::cQsoDb(cQsoDb *db) {
  nbrrecs = 0;
  maxrecs = db->nbrRecs();
  qsorec = new cQsoRec[maxrecs];
  for (int i = 0; i < maxrecs; i++)
	qsorec[i] = db->qsorec[i];
  compby = COMPDATE;
  dirty = 0;
}

cQsoDb::~cQsoDb() {
  delete [] qsorec;
} 

void cQsoDb::deleteRecs() {
  delete [] qsorec;
  nbrrecs = 0;
  maxrecs = MAXRECS;
  qsorec = new cQsoRec[maxrecs];
  dirty = 0;
}

void cQsoDb::clearDatabase() {
  deleteRecs();
}

int cQsoDb::qsoFindRec(cQsoRec *rec) {
  for (int i = 0; i < nbrrecs; i++)
    if (qsorec[i] == *rec)
      return i;
  return -1;
}

void cQsoDb::qsoNewRec (cQsoRec *nurec) {
  if (nbrrecs == maxrecs) {
    maxrecs += INCRRECS;
    cQsoRec *atemp = new cQsoRec[maxrecs];
    for (int i = 0; i < nbrrecs; i++)
        atemp[i] = qsorec[i];
    delete [] qsorec;
    qsorec = atemp;
  }
  qsorec[nbrrecs] = *nurec;
  qsorec[nbrrecs].checkBand();
  nbrrecs++;
  dirty = 1;
}

cQsoRec* cQsoDb::newrec() {
  if (nbrrecs == maxrecs) {
    maxrecs += INCRRECS;
    cQsoRec *atemp = new cQsoRec[maxrecs];
    for (int i = 0; i < nbrrecs; i++)
        atemp[i] = qsorec[i];
    delete [] qsorec;
    qsorec = atemp;
  }
  nbrrecs++;
  dirty = 1;
  return &qsorec[nbrrecs - 1];
}

void cQsoDb::qsoDelRec (int rnbr) {
  if (rnbr < 0 || rnbr > (nbrrecs - 1)) 
    return;
  for (int i = rnbr; i < nbrrecs - 1; i++)
    qsorec[i] = qsorec[i+1];
  nbrrecs--;
  qsorec[nbrrecs].clearRec();
}
  
void cQsoDb::qsoUpdRec (int rnbr, cQsoRec *updrec) {
  if (rnbr < 0 || rnbr > (nbrrecs - 1))
    return;
  qsorec[rnbr] = *updrec;
  qsorec[rnbr].checkBand();
  return;
}

void cQsoDb::SortByDate (bool how) {
  date_off = how;
  compby = COMPDATE;
  qsort (qsorec, nbrrecs, sizeof (cQsoRec), compareqsos);
}

void cQsoDb::SortByCall () {
  compby = COMPCALL;
  qsort (qsorec, nbrrecs, sizeof (cQsoRec), compareqsos);
}

void cQsoDb::SortByMode () {
  compby = COMPMODE;
  qsort (qsorec, nbrrecs, sizeof (cQsoRec), compareqsos);
}

void cQsoDb::SortByFreq () {
	compby = COMPFREQ;
	qsort (qsorec, nbrrecs, sizeof (cQsoRec), compareqsos);
}

bool cQsoDb::qsoIsValidFile(const char *fname) {
  char buff[256];
  ifstream inQsoFile (fname, ios::in);
  if (!inQsoFile)
    return false;
  inQsoFile.getline (buff, 256);
  if (strstr (buff, "_LOGBODUP DB") == 0) {
    inQsoFile.close();
    return false;
  }
  inQsoFile.close();
  return true;
}

int cQsoDb::qsoReadFile (const char *fname) {
char buff[256];
  ifstream inQsoFile (fname, ios::in);
  if (!inQsoFile)
    return 1;
  inQsoFile.getline (buff, 256);
  if (strstr (buff, "_LOGBODUP DB") == 0) {
    inQsoFile.close();
    return 2;
  }
  if (strstr (buff, "_LOGBODUP DBX") == 0) // new file format
    delim_in = '\n';
  if (strstr (buff, "3.0") != 0)
	isVer3 = true;    
  
  cQsoRec inprec;
  while (inQsoFile >> inprec) {
    qsoNewRec (&inprec);
    inprec.clearRec();
  }
  inQsoFile.close();
  SortByDate();
  return 0;
}

int cQsoDb::qsoWriteFile (const char *fname) {
  ofstream outQsoFile (fname, ios::out);
  if (!outQsoFile) {
  	printf("write failure: %s\n", fname);
    return 1;
  }
  outQsoFile << "_LOGBODUP DBX 3.0" << '\n';
  for (int i = 0; i < nbrrecs; i++)
    outQsoFile << qsorec[i];
  outQsoFile.close();
  return 0;
}

const int cQsoDb::jdays[2][13] = {
  { 0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 },
  { 0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 }
};

bool cQsoDb::isleapyear( int y )
{
  if( y % 400 == 0 || ( y % 100 != 0 && y % 4 == 0 ) )
    return true;
  return false;
}

int cQsoDb::dayofyear (int year, int mon, int mday)
{
  return mday + jdays[isleapyear (year) ? 1 : 0][mon];
}

unsigned int cQsoDb::epoch_dt (const char *szdate, const char *sztime)
{
  unsigned int  doe;
  int  era, cent, quad, rest;
  int year, mon, mday;
  int mins;
  
  year = ((szdate[0]*10 + szdate[1])*10 + szdate[2])*10 + szdate[3];
  mon  = szdate[4]*10 + szdate[5];
  mday = szdate[6]*10 + szdate[7];
  
  mins = ((sztime[0]*10 + sztime[1])*60 + sztime[2]*10 + sztime[3])*60 +
         + sztime[4]*10 + sztime[5];
  
  /* break down the year into 400, 100, 4, and 1 year multiples */
  rest = year - 1;
  quad = rest / 4;        rest %= 4;
  cent = quad / 25;       quad %= 25;
  era = cent / 4;         cent %= 4;
  
  /* set up doe */
  doe = dayofyear (year, mon, mday);
  doe += era * (400 * 365 + 97);
  doe += cent * (100 * 365 + 24);
  doe += quad * (4 * 365 + 1);
  doe += rest * 365;
  
  return doe*60*24 + mins;
}

bool cQsoDb::duplicate(
		const char *callsign, 
		const char *szdate, const char *sztime, unsigned int interval, bool chkdatetime,
		const char *freq, bool chkfreq,
		const char *state, bool chkstate,
		const char *mode, bool chkmode,
		const char *xchg1, bool chkxchg1 )
{
	int f1, f2 = 0;
	f1 = (int)(atof(freq)/1000.0);
	bool b_freqDUP = true, b_stateDUP = true, b_modeDUP = true,
		 b_xchg1DUP = true,
		 b_dtimeDUP = true;
	unsigned int datetime = epoch_dt(szdate, sztime);
	unsigned int qsodatetime;
	
	for (int i = 0; i < nbrrecs; i++) {
		if (strcasecmp(qsorec[i].getField(CALL), callsign) == 0) {
// found callsign duplicate
			b_freqDUP = b_stateDUP = b_modeDUP = 
				   	   b_xchg1DUP = b_dtimeDUP = false;
			if (chkfreq) {
				f2 = (int)atof(qsorec[i].getField(FREQ));
				b_freqDUP = (f1 == f2);
			}
			if (chkstate)
				b_stateDUP = (qsorec[i].getField(STATE)[0] == 0 && state[0] == 0) ||
							 (strcasestr(qsorec[i].getField(STATE), state) != 0);
			if (chkmode)
				b_modeDUP  = (qsorec[i].getField(MODE)[0] == 0 && mode[0] == 0) ||
							 (strcasestr(qsorec[i].getField(MODE), mode) != 0);
			if (chkxchg1)
				b_xchg1DUP = (qsorec[i].getField(XCHG1)[0] == 0 && xchg1[0] == 0) ||
							 (strcasestr(qsorec[i].getField(XCHG1), xchg1) != 0);

			if (chkdatetime) {
				qsodatetime = epoch_dt (
								qsorec[i].getField(QSO_DATE),
								qsorec[i].getField(TIME_OFF));
				if ((datetime - qsodatetime) < interval) b_dtimeDUP = true;
			}
 			if ( (!chkfreq     || (chkfreq     && b_freqDUP)) &&
			     (!chkstate    || (chkstate    && b_stateDUP)) &&
			     (!chkmode     || (chkmode     && b_modeDUP)) &&
			     (!chkxchg1    || (chkxchg1    && b_xchg1DUP)) &&
			     (!chkdatetime || (chkdatetime && b_dtimeDUP))) {
			     return true;
			 }
		}
	}
	return false;
}

