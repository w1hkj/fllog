#include "dialogs.h"
#include "util.h"
#include "debug.h"
#include "serial.h"
#include "support.h"
#include "wkey_dialogs.h"
#include "font_browser.h"
#include "status.h"

#include <string>

#include <iostream>

using namespace std;

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef __WIN32__
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <termios.h>
#endif

#ifdef __APPLE__
#  include <glob.h>
#endif

Fl_Double_Window *dlgSetup = (Fl_Double_Window *)0;

//======================================================================
// test comm ports
//======================================================================

void clear_combos()
{
	selectCommPort->clear();
	selectCommPort->add("NONE");
}

void add_combos(char *port)
{
	selectCommPort->add(port);
}

//======================================================================
// WIN32 init_port_combos
//======================================================================

#ifdef __WIN32__
static bool open_serial(const char* dev)
{
	bool ret = false;
	HANDLE fd = CreateFile(dev, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	if (fd != INVALID_HANDLE_VALUE) {
		CloseHandle(fd);
		ret = true;
	}
	return ret;
}

#  define TTY_MAX 255
void init_port_combos()
{
	int retval;
	clear_combos();

	char ttyname[21];
	const char tty_fmt[] = "//./COM%u";

	for (unsigned j = 0; j < TTY_MAX; j++) {
		snprintf(ttyname, sizeof(ttyname), tty_fmt, j);
		if (!open_serial(ttyname))
			continue;
		snprintf(ttyname, sizeof(ttyname), "COM%u", j);
		LOG_INFO("Found serial port %s", ttyname);
		add_combos(ttyname);
	}
}
#endif //__WIN32__

//======================================================================
// Linux init_port_combos
//======================================================================

#ifdef __linux__
#ifndef PATH_MAX
#  define PATH_MAX 1024
#endif
#  define TTY_MAX 8

void init_port_combos()
{
	int retval;


	struct stat st;
	char ttyname[PATH_MAX + 1];
	bool ret = false;

	DIR* sys = NULL;
	char cwd[PATH_MAX] = { '.', '\0' };

	clear_combos();

	if (getcwd(cwd, sizeof(cwd)) == NULL || chdir("/sys/class/tty") == -1 ||
	    (sys = opendir(".")) == NULL)
		goto out;

	ssize_t len;
	struct dirent* dp;
	while ((dp = readdir(sys))) {
#  ifdef _DIRENT_HAVE_D_TYPE
		if (dp->d_type != DT_LNK)
			continue;
#  endif
		if ((len = readlink(dp->d_name, ttyname, sizeof(ttyname)-1)) == -1)
			continue;
		ttyname[len] = '\0';
		if (!strstr(ttyname, "/devices/virtual/")) {
			snprintf(ttyname, sizeof(ttyname), "/dev/%s", dp->d_name);
			if (stat(ttyname, &st) == -1 || !S_ISCHR(st.st_mode))
				continue;
			LOG_INFO("Found serial port %s", ttyname);
			add_combos(ttyname);
		}
	}
	ret = true;

out:
	if (sys)
		closedir(sys);
	retval = chdir(cwd);
	if (ret) // do we need to fall back to the probe code below?
		return;

	const char* tty_fmt[] = {
		"/dev/ttyS%u",
		"/dev/ttyUSB%u",
		"/dev/usb/ttyUSB%u"
	};

	for (size_t i = 0; i < sizeof(tty_fmt)/sizeof(*tty_fmt); i++) {
		for (unsigned j = 0; j < TTY_MAX; j++) {
			snprintf(ttyname, sizeof(ttyname), tty_fmt[i], j);
			if ( !(stat(ttyname, &st) == 0 && S_ISCHR(st.st_mode)) )
				continue;

			LOG_INFO("Found serial port %s", ttyname);
			add_combos(ttyname);
		}
	}

}
#endif // __linux__

//======================================================================
// APPLE init_port_combos
//======================================================================

#ifdef __APPLE__
#ifndef PATH_MAX
#  define PATH_MAX 1024
#endif

void init_port_combos()
{
	int retval;

	clear_combos();

	struct stat st;

	const char* tty_fmt[] = {
		"/dev/cu.*",
		"/dev/tty.*"
	};

	glob_t gbuf;

	for (size_t i = 0; i < sizeof(tty_fmt)/sizeof(*tty_fmt); i++) {
		glob(tty_fmt[i], 0, NULL, &gbuf);
		for (size_t j = 0; j < gbuf.gl_pathc; j++) {
			if ( !(stat(gbuf.gl_pathv[j], &st) == 0 && S_ISCHR(st.st_mode)) ||
			     strstr(gbuf.gl_pathv[j], "modem") )
				continue;
			LOG_INFO("Found serial port %s", gbuf.gl_pathv[j]);
			add_combos(gbuf.gl_pathv[j]);
		}
		globfree(&gbuf);
	}
}
#endif //__APPLE__

//======================================================================
// FreeBSD init_port_combos
//======================================================================

#ifdef __FreeBSD__
#ifndef PATH_MAX
#  define PATH_MAX 1024
#endif
#  define TTY_MAX 8

void init_port_combos()
{
	int retval;
	struct stat st;
	char ttyname[PATH_MAX + 1];
	const char* tty_fmt[] = {
		"/dev/ttyd%u"
	};

	clear_combos();

	for (size_t i = 0; i < sizeof(tty_fmt)/sizeof(*tty_fmt); i++) {
		for (unsigned j = 0; j < TTY_MAX; j++) {
			snprintf(ttyname, sizeof(ttyname), tty_fmt[i], j);
			if ( !(stat(ttyname, &st) == 0 && S_ISCHR(st.st_mode)) )
				continue;
			LOG_INFO("Found serial port %s", ttyname);
			add_combos(ttyname);
		}
	}
}
#endif //__FreeBSD__

void cbCancelSetupDialog()
{
	dlgSetup->hide();
}

void cbOkSetupDialog()
{
	pthread_mutex_lock(&mutex_serial);
		WKEY_serial.ClosePort();
		bypass_serial_thread_loop = true;
	pthread_mutex_unlock(&mutex_serial);

	progStatus.serial_port_name = selectCommPort->value();

	if (!start_wkey_serial()) {
		if (progStatus.serial_port_name.compare("NONE") == 0) {
			debug::show();
			LOG_WARN("No comm port ... test mode");
		} else {
			progStatus.serial_port_name = "NONE";
			selectCommPort->value(progStatus.serial_port_name.c_str());
		}
		return;
	}
	dlgSetup->hide();
	Fl::flush();
	bypass_serial_thread_loop = false;
	open_wkeyer();
}

void config_comm_port()
{
LOG_INFO("%s", progStatus.serial_port_name.c_str());
	selectCommPort->value(progStatus.serial_port_name.c_str());
	dlgSetup->show();
}

void create_comm_dialog()
{
	if (!dlgSetup) {
		dlgSetup = SetupDialog();
		init_port_combos();
	}
}

void cb_events()
{
	debug::show();
}

// parameter configuration dialog

Fl_Double_Window *parameter_dialog = (Fl_Double_Window *)0;

void config_parameters()
{
	if (!parameter_dialog) {
		parameter_dialog = make_parameters_dialog();
		choice_keyer_mode->add("Iambic B");
		choice_keyer_mode->add("Iambic A");
		choice_keyer_mode->add("Ultimatic");
		choice_keyer_mode->add("Bug Mode");
		choice_output_pins->add("Key 1");
		choice_output_pins->add("Key 2");
		choice_output_pins->add("Key 1 & 2");
		choice_sidetone->add("4000");
		choice_sidetone->add("2000");
		choice_sidetone->add("1333");
		choice_sidetone->add("1000");
		choice_sidetone->add("800");
		choice_sidetone->add("666");
		choice_sidetone->add("571");
		choice_sidetone->add("500");
		choice_sidetone->add("444");
		choice_sidetone->add("400");
		choice_hang->add("Wait 1.0");
		choice_hang->add("Wait 1.33");
		choice_hang->add("Wait 1.66");
		choice_hang->add("Wait 2.0");
		cntr_tail->minimum(0); cntr_tail->maximum(250); cntr_tail->step(10);
		cntr_leadin->minimum(0); cntr_leadin->maximum(250); cntr_leadin->step(10);
		cntr_weight->minimum(10); cntr_weight->maximum(90); cntr_weight->step(1);
		cntr_sample->minimum(10); cntr_sample->maximum(90); cntr_sample->step(1);
		cntr_first_ext->minimum(0); cntr_first_ext->maximum(250); cntr_first_ext->step(1);
		cntr_comp->minimum(0); cntr_comp->maximum(250); cntr_comp->step(1);
		cntr_ratio->minimum(2.0); cntr_ratio->maximum(4.0); cntr_ratio->step(0.1);
		cntr_cmd_wpm->minimum(10); cntr_cmd_wpm->maximum(30); cntr_cmd_wpm->step(1);
		cntr_farnsworth->minimum(10); cntr_farnsworth->maximum(99); cntr_farnsworth->step(1);
		cntr_rng_wpm->minimum(10); cntr_rng_wpm->maximum(40); cntr_rng_wpm->step(1);
		cntr_min_wpm->minimum(5); cntr_min_wpm->maximum(89); cntr_min_wpm->step(1);
	}

	cntr_tail->value(progStatus.tail_time);
	cntr_leadin->value(progStatus.lead_in_time);
	cntr_weight->value(progStatus.weight);
	cntr_sample->value(progStatus.paddle_setpoint);
	cntr_first_ext->value(progStatus.first_extension);
	cntr_comp->value(progStatus.key_compensation);
	cntr_ratio->value(progStatus.dit_dah_ratio);
	cntr_cmd_wpm->value(progStatus.cmd_wpm);
	cntr_farnsworth->value(progStatus.farnsworth_wpm);
	cntr_rng_wpm->value(progStatus.rng_wpm);
	cntr_min_wpm->value(progStatus.min_wpm);

	choice_keyer_mode->index((progStatus.mode_register & 0x30) >> 4);
printf("pins %02X\n", progStatus.pin_configuration & 0x06);
	choice_output_pins->index(((progStatus.pin_configuration & 0x06) >> 2) - 1);
	choice_hang->index((progStatus.pin_configuration & 0x30) >> 4);
	choice_sidetone->index((progStatus.sidetone & 0x0F) - 1);

	btn_sidetone_on->value((progStatus.sidetone & 0x80) ? true : false);
	btn_tone_on->value(progStatus.pin_configuration & 0x02 ? true : false);
	btn_ptt_on->value(progStatus.pin_configuration & 0x01);
	btn_cut_zeronine->value(progStatus.cut_zeronine);
	btn_paddledog->value(progStatus.mode_register & 0x80);
	btn_ct_space->value(progStatus.mode_register & 0x01);
	btn_auto_space->value(progStatus.mode_register & 0x02);
	btn_swap->value(progStatus.mode_register & 0x08);
	btn_paddle_echo->value(progStatus.mode_register & 0x40);
	btn_serial_echo->value(progStatus.mode_register & 0x04);

	parameter_dialog->show();
}

void done_parameters()
{
	parameter_dialog->hide();
}

void change_choice_keyer_mode()
{
	int modebits = choice_keyer_mode->index() << 4;
	progStatus.mode_register = (progStatus.mode_register & 0xCF) | modebits;
	LOG_WARN("mode reg: %02X", progStatus.mode_register);
	load_defaults();
}

void change_choice_output_pins()
{
	int pinbits = (choice_output_pins->index() + 1) << 2;
	progStatus.pin_configuration = (progStatus.pin_configuration & 0xF3) | pinbits;
printf("pinbits %02X, new pins %02X\n", pinbits, progStatus.pin_configuration);
	load_defaults();
}

void change_choice_sidetone()
{
	progStatus.sidetone = choice_sidetone->index() + 1;
	progStatus.sidetone |= (btn_sidetone_on->value() ? 0x80 : 0x00);
	load_defaults();
}

void change_btn_tone_on()
{
	progStatus.pin_configuration = (progStatus.pin_configuration & 0xFD) | btn_tone_on->value() ? 2 : 0;
	load_defaults();
}

void change_btn_ptt_on()
{
	progStatus.pin_configuration = (progStatus.pin_configuration & 0xFE) | btn_ptt_on->value() ? 1 : 0;
	load_defaults();
}


void change_choice_hang()
{
	int hangbits = choice_hang->index() << 4;
	progStatus.pin_configuration = (progStatus.pin_configuration & 0xCF) | hangbits;
	load_defaults();
}

void change_cntr_tail()
{
	progStatus.tail_time = cntr_tail->value();
	load_defaults();
}

void change_cntr_leadin()
{
	progStatus.lead_in_time = cntr_leadin->value();
	load_defaults();
}

void change_cntr_weight()
{
	progStatus.weight = cntr_weight->value();
	load_defaults();
}

void change_cntr_sample()
{
	progStatus.paddle_setpoint = cntr_sample->value();
	load_defaults();
}

void change_cntr_first_ext()
{
	progStatus.first_extension = cntr_first_ext->value();
	load_defaults();
}

void change_cntr_comp()
{
	progStatus.key_compensation = cntr_comp->value();
	load_defaults();
}

void change_cntr_ratio()
{
	progStatus.dit_dah_ratio = cntr_ratio->value();
	load_defaults();
}

void change_cntr_cmd_wpm()
{
	progStatus.cmd_wpm = cntr_cmd_wpm->value();
}

void change_cntr_farnsworth()
{
	progStatus.farnsworth_wpm = cntr_farnsworth->value();
	load_defaults();
}

void change_cntr_rng_wpm()
{
	progStatus.rng_wpm = cntr_rng_wpm->value();
	cntr_wpm->minimum(progStatus.min_wpm);
	cntr_wpm->maximum(progStatus.rng_wpm + progStatus.min_wpm);
	if (progStatus.speed_wpm > progStatus.min_wpm + progStatus.rng_wpm) {
		progStatus.speed_wpm = progStatus.min_wpm + progStatus.rng_wpm;
		cntr_wpm->value(progStatus.speed_wpm);
	}
	load_defaults();
}

void change_cntr_min_wpm()
{
	progStatus.min_wpm = cntr_min_wpm->value();
	cntr_wpm->minimum(progStatus.min_wpm);
	cntr_wpm->maximum(progStatus.rng_wpm + progStatus.min_wpm);
	if (progStatus.speed_wpm < progStatus.min_wpm) {
		progStatus.speed_wpm = progStatus.min_wpm;
		cntr_wpm->value(progStatus.speed_wpm);
	}
	load_defaults();
}

void change_btn_sidetone_on()
{
	change_choice_sidetone();
}

void change_btn_cut_zeronine()
{
	progStatus.cut_zeronine = btn_cut_zeronine->value();
}

void change_btn_paddledog()
{
	progStatus.mode_register &=0x7F;
	progStatus.mode_register |= btn_paddledog->value() ? 0x80 : 0x00;
	LOG_WARN("mode reg: %2X", progStatus.mode_register);
	load_defaults();
}

void change_btn_ct_space()
{
	progStatus.mode_register &= 0xFE;
	progStatus.mode_register |= btn_ct_space->value();
	LOG_WARN("mode reg: %2X", progStatus.mode_register);
	load_defaults();
}

void change_btn_auto_space()
{
	progStatus.mode_register &=0xFD;
	progStatus.mode_register |= btn_auto_space->value() ? 0x02 : 0x00;
	LOG_WARN("mode reg: %2X", progStatus.mode_register);
	load_defaults();
}

void change_btn_swap()
{
	progStatus.mode_register &=0xF7;
	progStatus.mode_register |= btn_swap->value() ? 0x08 : 0x00;
	LOG_WARN("mode reg: %2X", progStatus.mode_register);
	load_defaults();
}

void change_btn_paddle_echo()
{
	progStatus.mode_register &=0xBF;
	progStatus.mode_register |= btn_paddle_echo->value() ? 0x40 : 0x00;
	LOG_WARN("mode reg: %2X", progStatus.mode_register);
	load_defaults();
}

void change_btn_serial_echo()
{
	progStatus.mode_register &=0xFB;
	progStatus.mode_register |= btn_serial_echo->value() ? 0x04 : 0x00;
	LOG_WARN("mode reg: %2X", progStatus.mode_register);
	load_defaults();
}

// message configuration dialog
Fl_Double_Window *dialog_messages = (Fl_Double_Window *)0;

void config_messages()
{
	if (!dialog_messages)
		dialog_messages = make_message_editor();
	label_1->value(progStatus.label_1.c_str());
	edit_msg1->value(progStatus.edit_msg1.c_str());
	label_2->value(progStatus.label_2.c_str());
	edit_msg2->value(progStatus.edit_msg2.c_str());
	label_3->value(progStatus.label_3.c_str());
	edit_msg3->value(progStatus.edit_msg3.c_str());
	label_4->value(progStatus.label_4.c_str());
	edit_msg4->value(progStatus.edit_msg4.c_str());
	label_5->value(progStatus.label_5.c_str());
	edit_msg5->value(progStatus.edit_msg5.c_str());
	label_6->value(progStatus.label_6.c_str());
	edit_msg6->value(progStatus.edit_msg6.c_str());
	label_7->value(progStatus.label_7.c_str());
	edit_msg7->value(progStatus.edit_msg7.c_str());
	label_8->value(progStatus.label_8.c_str());
	edit_msg8->value(progStatus.edit_msg8.c_str());
	label_9->value(progStatus.label_9.c_str());
	edit_msg9->value(progStatus.edit_msg9.c_str());
	label_10->value(progStatus.label_10.c_str());
	edit_msg10->value(progStatus.edit_msg10.c_str());
	char snbr[10];
	snprintf(snbr, sizeof(snbr), "%d", progStatus.serial_nbr);
	txt_serial_nbr->value(snbr);
	dialog_messages->show();
}

void update_msg_labels()
{
	btn_msg1->label(progStatus.label_1.c_str()); btn_msg1->redraw();
	btn_msg2->label(progStatus.label_2.c_str()); btn_msg2->redraw();
	btn_msg3->label(progStatus.label_3.c_str()); btn_msg3->redraw();
	btn_msg4->label(progStatus.label_4.c_str()); btn_msg4->redraw();
	btn_msg5->label(progStatus.label_5.c_str()); btn_msg5->redraw();
	btn_msg6->label(progStatus.label_6.c_str()); btn_msg6->redraw();
	btn_msg7->label(progStatus.label_7.c_str()); btn_msg7->redraw();
	btn_msg8->label(progStatus.label_8.c_str()); btn_msg8->redraw();
	btn_msg9->label(progStatus.label_9.c_str()); btn_msg9->redraw();
	btn_msg10->label(progStatus.label_10.c_str()); btn_msg10->redraw();
}

void apply_edit()
{
	progStatus.label_1 = label_1->value();
	progStatus.edit_msg1 = edit_msg1->value();

	progStatus.label_2 = label_2->value();
	progStatus.edit_msg2 = edit_msg2->value();

	progStatus.label_3 = label_3->value();
	progStatus.edit_msg3 = edit_msg3->value();

	progStatus.label_4 = label_4->value();
	progStatus.edit_msg4 = edit_msg4->value();

	progStatus.label_5 = label_5->value();
	progStatus.edit_msg5 = edit_msg5->value();

	progStatus.label_6 = label_6->value();
	progStatus.edit_msg6 = edit_msg6->value();

	progStatus.label_7 = label_7->value();
	progStatus.edit_msg7 = edit_msg7->value();

	progStatus.label_8 = label_8->value();
	progStatus.edit_msg8  = edit_msg8->value();

	progStatus.label_9 = label_9->value();
	progStatus.edit_msg9 = edit_msg9->value();

	progStatus.label_10 = label_10->value();
	progStatus.edit_msg10 = edit_msg10->value();

	update_msg_labels();
}

void done_edit()
{
	apply_edit();
	dialog_messages->hide();
}

void cancel_edit()
{
	dialog_messages->hide();
}

// operator setup dialog
Fl_Double_Window * op_dialog = (Fl_Double_Window *)0;

void change_txt_cll()
{
	progStatus.tag_cll = txt_cll->value();
}

void change_txt_qth()
{
	progStatus.tag_qth = txt_qth->value();
}

void change_txt_loc()
{
	progStatus.tag_loc = txt_loc->value();
}

void change_txt_opr()
{
	progStatus.tag_opr = txt_opr->value();
}

void cb_done_op_dialog()
{
	op_dialog->hide();
}

void open_operator_dialog()
{
	if (!op_dialog)
		op_dialog = make_operator_dialog();
	txt_cll->value(progStatus.tag_cll.c_str());
	txt_qth->value(progStatus.tag_qth.c_str());
	txt_loc->value(progStatus.tag_loc.c_str());
	txt_opr->value(progStatus.tag_opr.c_str());
	op_dialog->show();
}

Fl_Double_Window *contest_dialog = (Fl_Double_Window *)0;

void cb_contest()
{
	if (!contest_dialog)
		contest_dialog = make_contest_dialog();
	if (!contest_dialog->visible())
		contest_dialog->show();
}

void close_contest()
{
	contest_dialog->hide();
}
