// generated by Fast Light User Interface Designer (fluid) version 1.0300

#include "gettext.h"
#include "colorsfonts.h"
#include <config.h>
#include "fllog.h"
#include "status.h"
#include <FL/Fl_Color_Chooser.H>
#include <FL/fl_show_colormap.H>

static void choose_color(Fl_Color & c) {
  unsigned char r, g, b;
  Fl::get_color(c, r, g, b);
  if (fl_color_chooser("Font color", r, g, b))
      c = fl_rgb_color(r, g, b);
}

static void cbLOGBOOKFontBrowser(Fl_Widget*, void*) {
  Fl_Font font = font_browser->fontNumber();
      int size = font_browser->fontSize();
      Fl_Color color = font_browser->fontColor();
  
      progStatus.LOGBOOKtextfont = font;
      progStatus.LOGBOOKtextsize = size;
      progStatus.LOGBOOKtextcolor = color;
  
      LOGBOOKdisplay->textsize(size);
      LOGBOOKdisplay->textcolor(color);
      LOGBOOKdisplay->textfont(font);
      LOGBOOKdisplay->redraw();
      
      LOGBOOK_colors_font();
  
      font_browser->hide();
}

Fl_Double_Window *dlgColorFont=(Fl_Double_Window *)0;

Fl_Button *btnClrFntClose=(Fl_Button *)0;

static void cb_btnClrFntClose(Fl_Button* o, void*) {
  o->window()->hide();
}

Fl_Output *LOGBOOKdisplay=(Fl_Output *)0;

Fl_Button *btnLOGBOOK_color=(Fl_Button *)0;

static void cb_btnLOGBOOK_color(Fl_Button*, void*) {
  uchar r, g, b;
    Fl::get_color(progStatus.LOGBOOKcolor, r, g, b);

    if (!fl_color_chooser("Background", r, g, b))
        return;

    progStatus.LOGBOOKcolor = fl_rgb_color(r, g, b);
    
    LOGBOOKdisplay->color(progStatus.LOGBOOKcolor);
    LOGBOOKdisplay->redraw();

    LOGBOOK_colors_font();
}

Fl_Button *btn_LOGBOOK_font=(Fl_Button *)0;

static void cb_btn_LOGBOOK_font(Fl_Button*, void*) {
  font_browser->fontNumber(progStatus.LOGBOOKtextfont);
font_browser->fontSize(progStatus.LOGBOOKtextsize);
font_browser->fontColor(progStatus.LOGBOOKtextcolor);
font_browser->fontFilter(Font_Browser::ALL_TYPES);
font_browser->callback(cbLOGBOOKFontBrowser);
font_browser->show();
}

Fl_Button *btnLOGBOOKdefault_colors_font=(Fl_Button *)0;

static void cb_btnLOGBOOKdefault_colors_font(Fl_Button*, void*) {
  progStatus.LOGBOOKcolor = FL_BACKGROUND2_COLOR;
progStatus.LOGBOOKtextfont = (Fl_Font)0;
progStatus.LOGBOOKtextsize = 12;
progStatus.LOGBOOKtextcolor = FL_BLACK;

LOGBOOKdisplay->color(progStatus.LOGBOOKcolor);
LOGBOOKdisplay->textsize(progStatus.LOGBOOKtextsize);
LOGBOOKdisplay->textcolor(progStatus.LOGBOOKtextcolor);
LOGBOOKdisplay->textfont(progStatus.LOGBOOKtextfont);
LOGBOOKdisplay->redraw();
    
LOGBOOK_colors_font();
}

Fl_Double_Window* make_colorsfonts() {
  font_browser = new Font_Browser;
  { dlgColorFont = new Fl_Double_Window(375, 120, _("Colors and Fonts"));
    { btnClrFntClose = new Fl_Button(292, 84, 75, 25, _("Close"));
      btnClrFntClose->callback((Fl_Callback*)cb_btnClrFntClose);
    } // Fl_Button* btnClrFntClose
    { Fl_Group* o = new Fl_Group(1, 7, 370, 65, _("Logbook Dialog"));
      o->box(FL_ENGRAVED_FRAME);
      o->align(Fl_Align(FL_ALIGN_TOP_LEFT|FL_ALIGN_INSIDE));
      { Fl_Output* o = LOGBOOKdisplay = new Fl_Output(17, 31, 98, 25);
        o->textfont(progStatus.LOGBOOKtextfont);o->textsize(progStatus.LOGBOOKtextsize);o->textcolor(progStatus.LOGBOOKtextcolor);
        o->color(progStatus.LOGBOOKcolor);
        o->value("W3NR/4");
        o->redraw();
      } // Fl_Output* LOGBOOKdisplay
      { btnLOGBOOK_color = new Fl_Button(125, 31, 80, 25, _("Bg Color"));
        btnLOGBOOK_color->callback((Fl_Callback*)cb_btnLOGBOOK_color);
      } // Fl_Button* btnLOGBOOK_color
      { btn_LOGBOOK_font = new Fl_Button(215, 31, 55, 25, _("Font"));
        btn_LOGBOOK_font->callback((Fl_Callback*)cb_btn_LOGBOOK_font);
      } // Fl_Button* btn_LOGBOOK_font
      { btnLOGBOOKdefault_colors_font = new Fl_Button(281, 31, 80, 25, _("Default"));
        btnLOGBOOKdefault_colors_font->callback((Fl_Callback*)cb_btnLOGBOOKdefault_colors_font);
      } // Fl_Button* btnLOGBOOKdefault_colors_font
      o->end();
    } // Fl_Group* o
    dlgColorFont->xclass(PACKAGE_TARNAME);
    dlgColorFont->end();
  } // Fl_Double_Window* dlgColorFont
  return dlgColorFont;
}
