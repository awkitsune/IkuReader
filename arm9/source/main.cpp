#include <fat.h>
#include "renderer.h"
#include "txt.h"
#include "fb2.h"
#include "epub.h"
#include "settings.h"
#include "button.h"
#include "file_browser.h"
#include "extlink.h"
#include <sys/dir.h>
#include <stdio.h>
#include "encoding_tables.h"


const int giant_buf_size = 1800000;	// ~1.8 MB
char giant_buffer[giant_buf_size];

bool file_ok(string& file_name)
{ return std::ifstream(file_name.c_str()).good(); }

static grid menu(0);

void drawMenu()
{
	menu = grid(0);
	menu.push(SAY(resume), 1);
	menu.push(SAY(files), 2);
	menu.push(SAY(light), 3);
	renderer::clearScreens(settings::bgCol);
	menu.draw();
	renderer::printStr(eUtf8, top_scr,5,17,"IkuReader 0.065",0,0,12);
	powerOn(PM_BACKLIGHT_BOTTOM | PM_BACKLIGHT_TOP);
}

void openBook(string file)
{
	string ext = extention(file);
	if ("txt" == ext) txt_book(file).read();
	else if ("fb2" == ext) fb2_book(file).read();
	else if ("epub" == ext) epub_book(file).read();
	drawMenu();
}

int main(int argc, char *argv[])
{
	powerOff(PM_SOUND_AMP | PM_SOUND_MUTE);	//save battery life
	renderer::initVideo();

	string binname = "iku", argfile;
	if(argc) {
		binname = argv[0];
		if(binname.length() > 4 && !binname.compare(binname.length() - 4, 4, ".nds")) binname.erase(binname.length() - 4);
		u32 found = binname.find_last_of('/');
		if(found != string::npos) binname.erase(0, found + 1);
		if(argc >= 2) argfile = argv[1];
	}
	settings::binname = binname;

	iprintf("loading file system... ");
	if (!fatInitDefault()) bsod("error\n\ntried DLDI patch?");
	consoleClear();
	DIR_ITER* dir = diropen("/data/ikureader/");
	if(!dir) bsod("\nFolder data/ikureader not found.\nCopy it to the root of your\nflash card from the installationpackage.");
	dirclose(dir);
	settings::load();
	iprintf("loading fonts... ");
	renderer::initFonts();
	string moonsh = GetFileBody_From_MoonShell2_ExtLink();
	consoleClear();
	string enc = encPath + settings::encname;
	if(file_ok(enc)) loadEnc(enc.c_str());
	string trans = transPath + settings::translname;
	if(file_ok(trans)) loadTrans(trans);
	consoleClear();
	
	if(!moonsh.empty()) openBook(moonsh);
	else if (file_ok(argfile)) openBook(argfile);
	if(!file_ok(settings::recent_book)) openBook(file_browser().run());
	
	drawMenu();
	while(1) {
		swiWaitForVBlank();
		scanKeys();
		if(!(keysDown() & KEY_TOUCH)) continue;
		const string* t = menu.update();
		if(SAY(files) == t) openBook(file_browser().run());
		else if(SAY(light) == t) cycleBacklight();
		else if(SAY(resume) == t) openBook(settings::recent_book);
	}
}

void bsod(const char* msg)
{
	renderer::clearScreens(0);
	iprintf(msg);
	while(1) swiWaitForVBlank();
}

void cycleBacklight()
{
	int& b = settings::brightness;
	b++;
	b %= 4;
	fifoSendValue32(BACKLIGHT_FIFO, b);
}