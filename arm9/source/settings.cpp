#include "settings.h"
#include "renderer.h"
#include "screens.h"
#include <fstream>

namespace settings
{
s8 font_size, first_indent, line_gap, BGR;
bool justify, pbar;
string binname;
string font, font_bold, font_italic, recent_book, encname, translname;
renderTech::type tech;
Layout layout;
Color bgCol, fCol;
scrConfig scrConf;
int brightness, gamma;

void reset()
{
	font_size = 15; first_indent = 15; line_gap = 0;
	BGR = 1;	// 0..3: 1 for DS Lite, 2 for Phat and DSi, 0 for emulator
	justify = true;
	font = "LiberationSerif.ttf";
	font_bold = "LiberationSerifb.ttf";
	font_italic = "LiberationSerifi.ttf";
	tech = renderTech::windows;
	encname = "CP1251_Russian.txt";
	translname = "English.txt";
	recent_book = "";
	layout = d0;
	fCol  = Color(0,0,0);
	bgCol = Color(31,31,31);
	scrConf = scBoth;
	brightness = 3;
	pbar = true;
	gamma = 1;
}

struct settStruct {
	s8 Font_size, First_indent, Line_gap, bGR;
	bool Justify;
	Layout llayout;
	renderTech::type Tech;
	Color BgCol, FCol;
	scrConfig ScrConf;
	settStruct() : Font_size(font_size), First_indent(first_indent), Line_gap(line_gap), bGR(BGR),
					Justify(justify), llayout(layout),Tech(tech),BgCol(bgCol),FCol(fCol), ScrConf(scrConf) {}
	void apply() {
		font_size = Font_size; first_indent = First_indent;
		line_gap = Line_gap; BGR = bGR, justify = Justify;
		layout = llayout; tech = Tech;
		bgCol = BgCol; fCol = FCol;
		scrConf = ScrConf;
	}
};

#define SETTFILE (("/data/ikureader/settings_" + binname + ".b").c_str())

void load()
{
	reset();
	std::ifstream is(SETTFILE, std::ios::binary);
	is.seekg(0 , std::ios::beg);
	
	if(!is.good() || !is.is_open()) {
		is.close();
		save();
		return;
	}
	
	settStruct st;
	string end;
	
	is.read((char*)&st, sizeof(settStruct));
	getline(is, font);
	getline(is, font_bold);
	getline(is, font_italic);
	getline(is, recent_book);
	getline(is, end);
	is >> brightness >> pbar >> gamma >> encname >> translname;
	is.close();
	
	bool techCorrect = true;
	switch(st.Tech) {
		using namespace renderTech;
		case linux:   case linux_v:		
		case windows: case windows_v:
		case phone:	break;
		default: techCorrect = false;
	}
	
	if(end.compare("end") || !techCorrect) {  //bad settings file
		iprintf("Warning: Can't read settings. Try using different firmware/loader.\n");
		iprintf("Press A to load defaults.\n");
		while(!(keysDown() & KEY_A)) {scanKeys();}
		reset();
		save();
	}
	else st.apply(), fifoSendValue32(BACKLIGHT_FIFO, brightness);
}

void save()
{
	std::ofstream os;
	os.open(SETTFILE, std::ios::binary);
	os.seekp(0 , std::ios::beg);
	if(!os.good() || !os.is_open()) bsod("Can't save settigns.");
	settStruct st;
	os.write((char*)&st, sizeof(settStruct));
	os<<font		<<'\n'<<
		font_bold	<<'\n'<<
		font_italic	<<'\n'<<
		recent_book	<<'\n'<<
		"end"		<<'\n'<<
		brightness <<'\n'<<
		pbar <<'\n'<<
		gamma <<'\n'<<
		encname <<'\n'<<
		translname;
}

} // namespace settings