// ---------------------------------------------------------------------
//
//      adif_io.cxx
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

#include <FL/Fl.H>
#include <FL/filename.H>
#include <FL/fl_ask.H>

#include <string>
#include <cstring>
#include <cstdlib>
#include <string>

#include "adif_io.h"
#include "config.h"
#include "lgbook.h"
#include "icons.h"
#include "gettext.h"
#include "debug.h"
#include "util.h"
#include "date.h"
#include "status.h"

#ifdef __WOE32__
static const char *szEOL = "\r\n";
#else
static const char *szEOL = "\n";
#endif
static const char *szEOR = "<EOR>";

// These ADIF fields define the QSO database

const char *fieldnames[] = {
"ADDRESS",
"AGE",
"ARRL_SECT",
"BAND",
"CALL",
"CNTY",
"COMMENT",
"CONT",
"CONTEST_ID",
"COUNTRY",
"STATE",
"CQZ",
"DXCC",
"EXPORT",
"FREQ",
"GRIDSQUARE",
"IOTA",
"ITUZ",
"MODE",
"MYXCHG",
"NAME",
"NOTES",
"OPERATOR",
"PFX",
"PROP_MODE",
"QSLRDATE",
"QSLSDATE",
"QSL_RCVD",
"QSL_SENT",
"EQSL_QSLRDATE",
"EQSL_QSLSDATE",
"EQSL_RCVD",
"EQSL_SENT",
"LOTW_QSLRDATE",
"LOTW_QSLSDATE",
"LOTW_QSL_RCVD",
"LOTW_QSL_SENT",
"QSL_MSG",
"QSL_VIA",
"QSO_DATE",
"QSO_DATE_OFF",
"QTH",
"RST_RCVD",
"RST_SENT",
"RX_PWR",
"SAT_MODE",
"SAT_NAME",
"SRX",
"STX",
"TEN_TEN",
"TIME_OFF",
"TIME_ON",
"TX_PWR",
"VE_PROV",
"XCHG1"
};

// 16 chars per  position in string div 16 gives index to field name
// this string and the fieldname[] must have a 1:1 correspondence.

static const char fastlookup[] = "\
ADDRESS:        \
AGE:            \
ARRL_SECT:      \
BAND:           \
CALL:           \
CNTY:           \
COMMENT:        \
CONT:           \
CONTEST_ID:     \
COUNTRY:        \
STATE:          \
CQZ:            \
DXCC:           \
EXPORT:         \
FREQ:           \
GRIDSQUARE:     \
IOTA:           \
ITUZ:           \
MODE:           \
MYXCHG:         \
NAME:           \
NOTES:          \
OPERATOR:       \
PFX:            \
PROP_MODE:      \
QSLRDATE:       \
QSLSDATE:       \
QSL_RCVD:       \
QSL_SENT:       \
EQSL_QSLRDATE:  \
EQSL_QSLSDATE:  \
EQSL_RCVD:      \
EQSL_SENT:      \
LOTW_QSLRDATE:  \
LOTW_QSLSDATE:  \
LOTW_RCVD:      \
LOTW_SENT:      \
QSL_MSG:        \
QSL_VIA:        \
QSO_DATE:       \
QSO_DATE_OFF:   \
QTH:            \
RST_RCVD:       \
RST_SENT:       \
RX_PWR:         \
SAT_MODE:       \
SAT_NAME:       \
SRX:            \
STX:            \
TEN_TEN:        \
TIME_OFF:       \
TIME_ON:        \
TX_PWR:         \
VE_PROV:        \
XCHG1:          ";

FIELD fields[] = {
//  TYPE, NAME, WIDGET
	{ADDRESS,        0,  NULL},                // contacted stations mailing address
	{AGE,            0,  NULL},                // contacted operators age in years
	{ARRL_SECT,      0,  NULL},                // contacted stations ARRL section
	{BAND,           0,  &btnSelectBand},      // QSO band
	{CALL,           0,  &btnSelectCall},      // contacted stations CALLSIGN
	{CNTY,           0,  NULL},                // secondary political subdivision, ie: county
	{COMMENT,        0,  NULL},                // comment field for QSO
	{CONT,           0,  &btnSelectCONT},      // contacted stations continent
	{CONTEST_ID,     0,  NULL},                // QSO contest identifier
	{COUNTRY,        0,  &btnSelectCountry},   // contacted stations DXCC entity name
	{STATE,          0,  &btnSelectState},     // contacted stations STATE
	{CQZ,            0,  &btnSelectCQZ},       // contacted stations CQ Zone
	{DXCC,           0,  &btnSelectDXCC},      // contacted stations Country Code
	{EXPORT,         0,  NULL},                // used to indicate record is to be exported
	{FREQ,           0,  &btnSelectFreq},      // QSO frequency in Mhz
	{GRIDSQUARE,     0,  &btnSelectLOC},       // contacted stations Maidenhead Grid Square
	{IOTA,           0,  &btnSelectIOTA},      // Islands on the air
	{ITUZ,           0,  &btnSelectITUZ},      // ITU zone
	{MODE,           0,  &btnSelectMode},      // QSO mode
	{MYXCHG,         0,  &btnSelectMyXchg},    // contest exchange sent
	{NAME,           0,  &btnSelectName},      // contacted operators NAME
	{NOTES,          0,  &btnSelectNotes},     // QSO notes
	{OPERATOR,       0,  NULL},                // Callsign of person logging the QSO
	{PFX,            0,  NULL},                // WPA prefix
	{PROP_MODE,      0,  NULL},                // propogation mode
	{QSLRDATE,       0,  &btnSelectQSLrcvd},   // QSL received date
	{QSLSDATE,       0,  &btnSelectQSLsent},   // QSL sent date
	{QSL_RCVD,       0,  NULL},                // QSL received status
	{QSL_SENT,       0,  NULL},                // QSL sent status
	{EQSL_QSLRDATE,  0,  NULL},                // EQSL received date
	{EQSL_QSLSDATE,  0,  NULL},                // EQSL sent date
	{EQSL_RCVD,      0,  NULL},                // EQSL received status
	{EQSL_SENT,      0,  NULL},                // EQSL sent status
	{LOTW_QSLRDATE,  0,  NULL},                // LOTW received date
	{LOTW_QSLSDATE,  0,  NULL},                // LOTW sent date
	{LOTW_RCVD,      0,  NULL},                // LOTW received status
	{LOTW_SENT,      0,  NULL},                // LOTW sent status
	{QSL_MSG,        0,  NULL},                // personal message to appear on qsl card
	{QSL_VIA,        0,  NULL},
	{QSO_DATE,       0,  &btnSelectQSOdateOn}, // QSO data
	{QSO_DATE_OFF,   0,  &btnSelectQSOdateOff},// QSO data OFF, according to ADIF 2.2.6
	{QTH,            0,  &btnSelectQth},       // contacted stations city
	{RST_RCVD,       0,  &btnSelectRSTrcvd},   // received signal report
	{RST_SENT,       0,  &btnSelectRSTsent},   // sent signal report
	{RX_PWR,         0,  NULL},                // power of other station in watts
	{SAT_MODE,       0,  NULL},                // satellite mode
	{SAT_NAME,       0,  NULL},                // satellite name
	{SRX,            0,  &btnSelectSerialIN},  // received serial number for a contest QSO
	{STX,            0,  &btnSelectSerialOUT}, // QSO transmitted serial number
	{TEN_TEN,        0,  NULL},                // ten ten # of other station
	{TIME_OFF,       0,  &btnSelectTimeOFF},   // HHMM or HHMMSS in UTC
	{TIME_ON,        0,  &btnSelectTimeON},    // HHMM or HHMMSS in UTC
	{TX_PWR,         0,  &btnSelectTX_pwr},    // power transmitted by this station
	{VE_PROV,        0,  &btnSelectProvince},  // 2 letter abbreviation for Canadian Province
	{XCHG1,          0,  &btnSelectXchgIn}     // contest exchange #1 / free1 in xlog
};

void initfields()
{
	for (int i = 0; i < NUMFIELDS; i++) {
		fields[i].name = new string(fieldnames[i]);
	}
}

static int findfield( char *p )
{
	if (strncasecmp (p, "EOR>", 4) == 0)
		return -1;

	char *p1, *p2;
	char *pos;
	char *fl = (char *)fastlookup;
	int n;
	p1 = strchr(p, ':');
	p2 = strchr(p, '>');
	if (p1 && p2) {
		if (p1 < p2) {
			*p1 = 0;
			pos = strstr(fl, p);
			*p1 = ':';
			if (pos) {
				n = (pos - fastlookup) / 16;
				if (n > 0 && n < NUMFIELDS)
					return fields[n].type;
			}
		}
	}
	return -2;		//search key not found
}

cAdifIO::cAdifIO ()
{
	initfields();
}

void cAdifIO::fillfield (int fieldnum, char *buff)
{
char *p = buff;
int fldsize;
	while (*p != ':' && *p != '>') p++;
	if (*p == '>') return; // bad ADIF specifier ---> no ':' after field name
// found first ':'
	p++;
	fldsize = 0;
	char *p2 = strchr(buff,'>');
	if (!p2) return;
	while (p != p2) {
		if (*p >= '0' && *p <= '9') {
			fldsize = fldsize * 10 + *p - '0';
		}
		p++;
	}
	if ((fieldnum == TIME_ON || fieldnum == TIME_OFF) && fldsize < 6) {
		string tmp = "";
		tmp.assign(p2+1, fldsize);
		while (tmp.length() < 6) tmp += '0';
		adifqso->putField(fieldnum, tmp.c_str(), 6);
	} else
		adifqso->putField (fieldnum, p2+1, fldsize);
}


// ---------------------------------------------------------------------
// add_record
//
// used by xmlrpc server to add a record to the database
//   buffer contains an ADIF record with <eor> as terminator
//   db points to the current database
//
//----------------------------------------------------------------------
void cAdifIO::add_record(const char *buffer, cQsoDb &db)
{
	int found;
	char * p = strchr((char *)buffer,'<'); // find first ADIF specifier

	adifqso = 0;
	while (p) {
		found = findfield(p + 1);
		if (found > -1) {
			if (!adifqso) adifqso = db.newrec(); // need new record in db
			fillfield (found, p + 1);
		} else if (found == -1) // <eor> reached;
			adifqso = 0;
		p = strchr(p + 1,'<');
	}
	db.SortByDate();

}

void cAdifIO::readFile (const char *fname, cQsoDb *db) 
{
	long filesize = 0;
	char *buff;
	int found;
	int retval;

// open the adif file
	FILE *adiFile = fopen (fname, "r");

	if (adiFile == NULL)
		return;
// determine its size for buffer creation
	fseek (adiFile, 0, SEEK_END);
	filesize = ftell (adiFile);

	if (filesize == 0) {
		return;
	}

	buff = new char[filesize + 1];

// read the entire file into the buffer

	fseek (adiFile, 0, SEEK_SET);
	retval = fread (buff, filesize, 1, adiFile);
	fclose (adiFile);

// relaxed file integrity test to all importing from non conforming log programs
	if ((strcasestr(buff, "<ADIF_VER:") != 0) &&
		(strcasestr(buff, "<CALL:") == 0)) {
		delete [] buff;
		return;
	}
	if (strcasestr(buff, "<CALL:") == 0) {
		delete [] buff;
		return;
	}

	static char msg[100];
	int len = 0;

	char *p1 = buff, *p2;
	if (*p1 != '<') { // yes, skip over header to start of records
		p1 = strchr(buff, '<');
		while (strncasecmp (p1+1,"EOH>", 4) != 0) {
			p1 = strchr(p1+1, '<'); // find next <> field
		}
		if (!p1) {
			delete [] buff;
			return;	 // must not be an ADIF compliant file
		}
		p1 += 1;
	}

	p2 = strchr(p1,'<'); // find first ADIF specifier

	txtLogFile->value("Parsing records:");
	dlgLogbook->redraw();
	Fl::flush();

	adifqso = 0;
	while (p2) {
		found = findfield(p2+1);
		if (found > -1) {
			if (!adifqso) adifqso = db->newrec(); // need new record in db
			fillfield (found, p2+1);
		} else if (found == -1) { // <eor> reached;
			adifqso = 0;
			if (((++len % 100) == 0)) {
				snprintf(msg, sizeof(msg), "Parsing records:%8d", len);
				txtLogFile->value(msg);
				txtLogFile->redraw();
				Fl::flush();
			}
		}
		p1 = p2 + 1;
		p2 = strchr(p1,'<');
	}

	db->SortByDate();
	delete [] buff;
}

static const char *adifmt = "<%s:%d>";

// write ALL or SELECTED records to the designated file

int cAdifIO::writeFile (const char *fname, cQsoDb *db)
{
	string ADIFHEADER;
	ADIFHEADER = "File: %s";
	ADIFHEADER.append(szEOL);
	ADIFHEADER.append("<ADIF_VER:%d>%s");
	ADIFHEADER.append(szEOL);
	ADIFHEADER.append("<PROGRAMID:%d>%s");
	ADIFHEADER.append(szEOL);
	ADIFHEADER.append("<PROGRAMVERSION:%d>%s");
	ADIFHEADER.append(szEOL);
	ADIFHEADER.append("<EOH>");
	ADIFHEADER.append(szEOL);
// open the adif file
	cQsoRec *rec;
	string sFld;
	adiFile = fopen (fname, "w");
	if (!adiFile)
		return 1;
	fprintf (adiFile, ADIFHEADER.c_str(),
			 fl_filename_name(fname),
			 strlen(ADIF_VERS), ADIF_VERS,
			 strlen(PACKAGE_NAME), PACKAGE_NAME,
			 strlen(PACKAGE_VERSION), PACKAGE_VERSION);
	for (int i = 0; i < db->nbrRecs(); i++) {
		rec = db->getRec(i);
		if (rec->getField(EXPORT)[0] == 'E') {
			for (int j = 0; j < NUMFIELDS; j++) {
				if (fields[j].btn != NULL)
					if ((*fields[j].btn)->value()) {
					sFld = rec->getField(fields[j].type);
						if (!sFld.empty())
							fprintf(adiFile, adifmt,
								fields[j].name->c_str(),
								sFld.length());
							fprintf(adiFile, "%s", sFld.c_str());
				}
			}
			rec->putField(EXPORT,"");
			db->qsoUpdRec(i, rec);
			fprintf(adiFile, "%s", szEOR);
			fprintf(adiFile, "%s", szEOL);
		}
	}
	fclose (adiFile);
	return 0;
}

// write ALL records to the common log

int cAdifIO::writeLog (const char *fname, cQsoDb *db) {

	string ADIFHEADER;
	ADIFHEADER = "File: %s";
	ADIFHEADER.append(szEOL);
	ADIFHEADER.append("<ADIF_VER:%d>%s");
	ADIFHEADER.append(szEOL);
	ADIFHEADER.append("<PROGRAMID:%d>%s");
	ADIFHEADER.append(szEOL);
	ADIFHEADER.append("<PROGRAMVERSION:%d>%s");
	ADIFHEADER.append(szEOL);
	ADIFHEADER.append("<EOH>");
	ADIFHEADER.append(szEOL);
// open the adif file
	string sFld;
	cQsoRec *rec;

	adiFile = fopen (fname, "w");
	if (!adiFile) {
		LOG_ERROR("Cannot write to %s", fname);
		return 1;
	}

	string records;
	string record;
	char recfield[200];

	records.clear();
	for (int i = 0; i < db->nbrRecs(); i++) {
		rec = db->getRec(i);
		record.clear();
		for (int j = 0; j < NUMFIELDS; j++) {
			sFld = rec->getField(j);
			if (!sFld.empty()) {
				snprintf(recfield, sizeof(recfield), adifmt,
					fields[j].name->c_str(), sFld.length());
				record.append(recfield).append(sFld);
			}
		}
		record.append(szEOR);
		record.append(szEOL);
		records.append(record);
		db->qsoUpdRec(i, rec);
	}

	fprintf (adiFile, ADIFHEADER.c_str(),
		 fl_filename_name(fname),
		 strlen(ADIF_VERS), ADIF_VERS,
		 strlen(PACKAGE_NAME), PACKAGE_NAME,
		 strlen(PACKAGE_VERSION), PACKAGE_VERSION
		);
	fprintf (adiFile, "%s", records.c_str());

	fclose (adiFile);

	return 0;
}

// not used in current version - deprecated
void cAdifIO::do_checksum(cQsoDb &db)
{
	Ccrc16 checksum;
	string sFld;
	cQsoRec *rec;
	string records;
	string record;
	char recfield[200];

	records.clear();
	for (int i = 0; i < db.nbrRecs(); i++) {
		rec = db.getRec(i);
		record.clear();
		for (int j = 0; j < NUMFIELDS; j++) {
			sFld = rec->getField(j);
			if (!sFld.empty()) {
				snprintf(recfield, sizeof(recfield), adifmt,
					fields[j].name->c_str(), sFld.length());
				record.append(recfield).append(sFld);
			}
		}
		record.append(szEOR);
		record.append(szEOL);
		records.append(record);
	}
	log_checksum = checksum.scrc16(records);
}

