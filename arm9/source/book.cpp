#include "book.h"
#include "screens.h"
#include "renderer.h"
#include "controls.h"
#include "encoding_tables.h"
#include <fstream>
#include <sys/dir.h>
#include <sys/unistd.h>

void Book :: read()
{
	renderer::clearScreens(settings::bgCol);
	parse();
	if(0 == total_paragraths()) return;
	loadMarks();
	consoleClear();
	draw_page();
	settings::recent_book = bookFile;
	settings::save();
	otherGrid = false;

	touchPosition t1, t2;
	t1.px = t1.py = t2.px = t2.py = 0;
	
	while(1) {
		renderer::flashClock.hide();
		swiWaitForVBlank();
		scanKeys();
		int down = keysDown();
		if(down & rKey(rRight)) next_page();
		else if(down & rKey(rLeft)) previous_page();
		else if(down & rKey(rUp)) {bookmarkMenu(); draw_page(false, false);}
		else if(down & rKey(rDown)) {
			bool doexit = menu();
			if(doexit) return;
			otherGrid = false;
			draw_page(false, false);
		}
		else if(down & KEY_SELECT) search();
		else if(down & KEY_TOUCH) {
			touchRead(&t1);
		}
		else if(keysHeld() & KEY_TOUCH) {
			touchRead(&t2);
		}
		else if(keysUp() & KEY_TOUCH) {
			if(10 > abs(t1.px - t2.px) + abs(t1.py - t2.py)) {
				if(t2.px == 0) continue;
				u8 X1 = 0, Y1 = 0, X2 = screens::layoutX()/2, Y2 = screens::layoutY();
				toLayoutSpace(X1, Y1);
				toLayoutSpace(X2, Y2);
				if(	!(t2.px >= MIN(X1, X2) && t2.py >= MIN(Y1, Y2) &&
					t2.px <  MAX(X1, X2) && t2.py < MAX(Y1, Y2)) ) 
						 next_page();
					else previous_page();
			}
			else {
				int deltaX = t1.px - t2.px;
				int deltaY = t1.py - t2.py;
				int tehDelta = 0;
				switch (settings::layout) {
					case d0:   tehDelta = deltaX;  break;
					case d90:  tehDelta = deltaY;  break;
					case d180: tehDelta = -deltaX; break;
					case d270: tehDelta = -deltaY; break;
				}
				if(tehDelta > 30) next_page();
				else if (tehDelta < -30) previous_page();
			}
		}
	}
}

void Book :: next_page()
{
	int i = screens::capacity(settings::scrConf != scBoth) + current_page.line_num;
	for (u16 j = current_page.parag_num; j < total_paragraths(); j++) {
		if(j != prev_par_num) fetch_paragrath(j);
		i -= parag.lines.size();
		if (j >= total_paragraths()-1 && i > 0) return; //last page, don't redraw
		if (i <= 0) {
			current_page.parag_num = j;
			current_page.line_num = i + parag.lines.size();
			break;
		}
	}
	draw_page(false, false);
	saveMarks();
}

void Book :: previous_page()
{
	if (0 == current_page.parag_num && 0 == current_page.line_num) return; //first page, don't redraw
	if(current_page.parag_num != prev_par_num) fetch_paragrath (current_page.parag_num);
	int i = screens::capacity(settings::scrConf != scBoth) + parag.lines.size() - current_page.line_num;
	for (u32 j = current_page.parag_num; j >= 0; j--) {
		if(j != prev_par_num) fetch_paragrath(j);
		i -= parag.lines.size();
		if (i <= 0) {
			current_page.parag_num = j;
			current_page.line_num = -i;
			break;
		}
		if (0 == j){
			current_page.parag_num = 0;
			current_page.line_num = 0;
			break;
		}
	}
	draw_page(false, false);
	saveMarks();
}

void Book :: draw_page(bool onlyTop, bool cachePar)
{
	if(!onlyTop) {
		if(settings::scrConf == scTop) {
			powerOn(PM_BACKLIGHT_TOP);
			powerOff(PM_BACKLIGHT_BOTTOM);
		}
		else if (settings::scrConf == scBottom) {
			powerOn(PM_BACKLIGHT_BOTTOM);
			powerOff(PM_BACKLIGHT_TOP);
		}
	}
	else powerOn(PM_BACKLIGHT_BOTTOM | PM_BACKLIGHT_TOP);
	
	int line_num = current_page.line_num;
	u32 lines_total = screens::capacity(onlyTop || settings::scrConf != scBoth);
	u32 k = 0;
	for (u16 i = current_page.parag_num; k < lines_total && i < total_paragraths(); i++)
	{
		if(i != prev_par_num || cachePar) fetch_paragrath(i);
		for (u16 j = line_num; j < parag.lines.size() && k < lines_total; j++,k++)
			screens::print_line (encoding, parag, j, k, onlyTop);
		line_num = 0;
	}
	if(k == 0) renderer::clearScreens(settings::bgCol);
	if(k <= screens::capacity(true) && !onlyTop) renderer::clearScreens(settings::bgCol, (scr_id)!first_scr());
	
	if(settings::pbar) {
		u16 fcol = settings::fCol, gray = Blend(64), xw = screens::layoutX() + 1;
		u32 upto = current_page.parag_num * xw / total_paragraths();
		scr_id scr = (scr_id)(settings::scrConf == scBoth ? !first_scr(): first_scr());
		if(onlyTop) scr = top_scr;
		for(u32 i = 0, y = screens::layoutY() - 1u, y2 = screens::layoutY(); i < xw; i++) {
			renderer::putPixel(scr, i, y, (i >= upto ? gray : fcol));
			renderer::putPixel(scr, i, y2, (i >= upto ? gray : settings::bgCol));
		}
		if(upto > 0) renderer::putPixel(scr, upto - 1, screens::layoutY(), fcol);
	}
	if(!onlyTop) {
		if(settings::scrConf != scBoth) renderer::flashClock.show(first_scr());
		else renderer::flashClock.show((scr_id)!first_scr());
	}
}

void Book :: bookmarkMenu()
{
	setMark = button("___Set___", screens::layoutX()/3, screens::layoutY()/2);
	older = button(SAY2(older), 5, screens::layoutY()/3);
	newer = button(SAY2(newer), screens::layoutX() - renderer::strWidth(eUtf8,SAY2(newer),0,0,buttonFontSize) - 10, screens::layoutY()/3);
	progressbar prbar = progressbar();
	draw_page(true, false);
	drawBookmarkMenu();
	while(1) {
		swiWaitForVBlank();
		renderer::printClock(bottom_scr);
		scanKeys();
		int down = keysDown();
		if(down & KEY_TOUCH) {
			float p = prbar.touched();
			if(setMark.touched()) {
				if(bookmarks.find(current_page) == bookmarks.end()) {
					bookmarks.insert(bookmark(current_page.parag_num,0));
					drawBookmarkMenu();
				}
				else {
					bookmarks.erase(current_page);
					drawBookmarkMenu();
				}
			}
			else if (newer.touched() && moreNew()) {
				current_page = *bookmarks.upper_bound(current_page);
				draw_page(true);
				drawBookmarkMenu();
			}
			else if (older.touched() && moreOld()) {
				current_page = *--bookmarks.lower_bound(current_page);
				draw_page(true);
				drawBookmarkMenu();
			}
			else if (p < 1.0f) {
				u32 parag_num = p * total_paragraths();
				if(parag_num == current_page.parag_num) continue;
				current_page.parag_num = parag_num;
				current_page.line_num = 0;
				draw_page(true);
				drawBookmarkMenu();
			}
		}
		else if(down & rKey(rUp)) break;
	}
	saveMarks();
	renderer::clearScreens(settings::bgCol);
}

void Book :: drawBookmarkMenu()
{
	prbar = progressbar();
	if(bookmarks.find(current_page) == bookmarks.end()) setMark.setText(SAY2(set));
	else setMark.setText(SAY2(remove));
	renderer::clearScreens(settings::bgCol, bottom_scr);
	renderer::printStr(eUtf8, bottom_scr, 10, settings::font_size + 5, SAY2(bookmarks),0,0,buttonFontSize);
	setMark.draw();
	if(moreNew()) newer.draw();
	if(moreOld()) older.draw();
	prbar.draw (float(current_page.parag_num) / total_paragraths());
	for (std::set<bookmark>::iterator it = bookmarks.begin(); it != bookmarks.end(); ++it)
		prbar.mark(float(it->parag_num) / total_paragraths());
	renderer::printClock(bottom_scr, true);
	powerOn(PM_BACKLIGHT_BOTTOM | PM_BACKLIGHT_TOP);
}

bool Book :: menu()
{
	drawMenu(false);
	while(1) {
		swiWaitForVBlank();
		scanKeys();
		int down = keysDown();
		if(down & rKey(rDown)) break;
		
		if(!(down & KEY_TOUCH)) continue;
		
		const string* t = menuGrid.update();
		if(SAY(rotate) == t) {
			int i = settings::layout;
			++i;
			if(i > d270) i = d0;
			current_page.line_num = 0;
			settings::layout = Layout(i);
			drawMenu();
		}
		else if(SAY(close) == t) return true;
		else if(SAY(light) == t) cycleBacklight();
		else if(SAY(invert) == t) {
			std::swap(settings::fCol, settings::bgCol);
			drawMenu(false);
		}
		else if(SAY(screens) == t) {
			int i = settings::scrConf;
			i++;
			if(i > scBoth) i = scTop;
			settings::scrConf  = scrConfig(i);
			drawMenu(false);
			string o;
			switch(settings::scrConf) {
				case scTop: o = *SAY(top); break;
				case scBottom: o = *SAY(bottom); break;
				case scBoth: o = *SAY(both); break;
			}
			menuGrid.print(SAY(screens), o);
		}
		else if(SAY(codep) == t) {
			code_page();
			current_page.line_num = 0;
			drawMenu();
		}
		else if(SAY(font) == t) {
			renderer::changeFont();
			current_page.line_num = 0;
			drawMenu();
			menuGrid.print(SAY(font), settings::font);
		}
		else if(SAY(colors) == t) {
			colorPicker();
			drawMenu(false);
		}
		else if(SAY(style) == t) {
			using settings::tech;
			using namespace renderTech;
			switch(tech) {
				case linux:   case linux_v:		tech = windows; break;
				case windows: case windows_v:	tech = phone; 	break;
				case phone:		tech = linux;	break; default: ;
			}
			current_page.line_num = 0;
			drawMenu();
		}
		else if(SAY(sharp) == t) {
			sharpness();
			drawMenu(false);
		}
		else if(SAY(pbar) == t) {
			settings::pbar = !settings::pbar;
			drawMenu(false);
		}
		else if(SAY(justify) == t) {
			settings::justify = !settings::justify;
			drawMenu(false);
		}
		else if(SAY(size) == t) {
			using settings::font_size;
			int old = font_size;
			font_size += (rLeft == menuGrid.val) ? -1: 1;
			clamp(font_size, 8, 24);
			if(old == font_size) continue;
			current_page.line_num = 0;
			drawMenu();
			char buf[5];
			sprintf(buf, "%d", settings::font_size);
			menuGrid.print(SAY(size), string(buf));
		}
		else if(SAY(gamma) == t) {
			using settings::gamma;
			int old = gamma;
			gamma += (rLeft == menuGrid.val) ? -1: 1;
			clamp(gamma, 0, 2);
			if(old == gamma) continue;
			drawMenu(false);
		}
		else if(SAY(gap) == t) {
			using settings::line_gap;
			int old = line_gap;
			line_gap += (rLeft == menuGrid.val) ? -1: 1;
			clamp(line_gap, 0, 15);
			if(old == line_gap) continue;
			drawMenu(false);
		}
		else if(SAY(indent) == t) {
			using settings::first_indent;
			int old = first_indent;
			first_indent += (rLeft == menuGrid.val) ? -1: 1;
			clamp(first_indent, 0, 50);
			if(old == first_indent) continue;
			drawMenu();
		}
		else if(SAY(language) == t) {
			translation();
			drawMenu();
		}
	}
	settings::save();
	renderer::clearScreens(settings::bgCol);
	return false;
}

void Book :: drawMenu(bool recache)
{
	u32 it = menuGrid.iter;
	menuGrid = grid(it);
	menuGrid.push(SAY(close), 1)
	->push(SAY(invert))
	->push(SAY(justify))	
	->push(SAY(rotate))
	->push(SAY(gamma), 0 , true)
	->push(SAY(pbar))
	->push(SAY(screens))
	->push(SAY(light))
	->push(SAY(size), 0 , true)
	->push(SAY(font))
	->push(SAY(style))
	->push(SAY(gap), 0 , true)
	->push(SAY(indent), 0 , true)	
	->push(SAY(colors))
	->push(SAY(sharp))
	->push(SAY(codep))
	->push(SAY(language));
	draw_page(true, recache);
	renderer::clearScreens(settings::bgCol, bottom_scr);
	menuGrid.draw();
	powerOn(PM_BACKLIGHT_BOTTOM | PM_BACKLIGHT_TOP);
}

void Book :: saveMarks()
{
	std::ofstream os(("/data/IkuReader/bookmarks/"+noPath(bookFile)+".bm").c_str());
	if(!os.good()) {
		os.close();
		os.clear();
		os.open(encname.c_str());
	}
	if(!os.good()) bsod("Can't save bookmarks.");
	os<<current_page.parag_num<<'\n';
	for (std::set<bookmark>::iterator it = bookmarks.begin(); it != bookmarks.end(); ++it)
		os<<it->parag_num<<'\n';
}

void Book :: loadMarks()
{
	std::ifstream is(("/data/IkuReader/bookmarks/"+noPath(bookFile)+".bm").c_str());
	if(!is.good()) {
		is.close();
		is.clear();
		is.open(encname.c_str());
	}
	if(!is.good()) return;
	const u32 total = total_paragraths();
	u32 temp;
	is >> temp;
	if(temp < total) current_page.parag_num = temp;
	while(is >> temp) if(temp < total) bookmarks.insert(bookmark(temp,0));
}

string fileReq(const string& path)
{
	char fname[MAXPATHLEN];
	struct stat st;
	DIR_ITER* dir = diropen(path.c_str());
	if(!dir) bsod(("cannot open "  + path).c_str());
	vector<button> buttons;
	int peny = 0;
	renderer::clearScreens(settings::bgCol);
	while (0 == dirnext(dir, fname, &st)) {
		string filename(fname);
		if (!(st.st_mode & S_IFDIR)) {
			if(extention(filename) == "txt")
				buttons.push_back(button(filename.erase(filename.find_last_of('.')), peny));
			buttons.back().draw();
			peny += buttonFontSize;
		}
	}
	dirclose(dir);
	while(1) {
		swiWaitForVBlank();
		scanKeys();
		if(!(keysDown() & KEY_TOUCH)) continue;
		else for(u32 i = 0; i < buttons.size(); i++)
			if(buttons[i].touched()) {
				return buttons[i].getText() + ".txt";
			}
	}
}

void Book :: code_page()
{
	string f = fileReq(encPath);
	settings::encname = f;
	loadEnc((encPath + f).c_str());
}

void Book :: translation()
{
	string f = fileReq(transPath);
	settings::translname = f;
	loadTrans(transPath + f);
}