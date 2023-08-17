#include "file_browser.h"
#include "renderer.h"
#include "screens.h"
#include "controls.h"
#include <sys/dir.h>
#include <sys/unistd.h>
#include <algorithm>

bool comp(entry e1, entry e2)
{
	if(e1.first == e2.first) return 0 > strcmpi(e1.second.c_str(), e2.second.c_str());
	else return e1.first > e2.first;
}

void file_browser :: cd()
{
	pos = 0;
	flist.clear();
	char fname[MAXPATHLEN];
	struct stat st;
	DIR_ITER* dir = diropen (path.c_str());
	if (dir == NULL) bsod("Cannot open directory.");
	if(path != "/") flist.push_back(entry(folder, ".."));
	while (0 == dirnext(dir, fname, &st)) {
		string filename(fname);
		if ((st.st_mode & S_IFDIR) && (filename != ".") && (filename != "..")) {
			flist.push_back(entry(folder, filename));
		}
		else {
			string ext(extention(filename));
			if(ext == "txt" || ext == "fb2"  || ext == "epub") flist.push_back(entry(file, filename));
		}
	}
	dirclose(dir);
	sort (flist.begin(), flist.end(), comp);
}

u16 file_browser :: draw()
{
	buttons.clear();
	u16 pen = 0;
	button header(path, 0);
	renderer::clearScreens(settings::bgCol);
	header.draw();
	u16 height = header.height();
	u16 i = pos;
	for( ; i < flist.size() && pen <= screens::layoutY() - 2*height; i++) {
		pen += height;
		buttons.push_back(fbutton(flist[i].first, button(flist[i].second, pen)));
		buttons.back().second.draw();
	}
	return i - pos;
}

void file_browser :: upd()
{
	num = draw();
	if (num < flist.size() && flist.size()) sbar.draw(float(pos) / (flist.size() - num), float(buttons.size())/flist.size());
}

string file_browser :: run()
{
	powerOff(PM_BACKLIGHT_TOP);
	
	string start_path = settings::recent_book.substr(0, settings::recent_book.find_last_of('/')) + '/';
	DIR_ITER* dir = diropen(start_path.c_str());
	if(dir == NULL || settings::recent_book.empty()) {
		start_path = "/books/";
		DIR_ITER* dir = diropen(start_path.c_str());
		if(dir == NULL)  start_path = "/";
		else dirclose(dir);
	}
	else dirclose(dir);
	path = start_path;

	cd();
	upd();
	
	while(1){
		swiWaitForVBlank();
		scanKeys();
		int down = keysDown();
		if(!down) continue;
		if(down & rKey(rUp)){
			if (0 == pos) continue;
			pos -= num;
			clamp(pos, 0, int(flist.size() - num));
			upd();
		}
		else if(down & rKey(rDown)){
			if ((u16)pos >= flist.size() - num) continue;
			pos += num;
			clamp(pos, 0, int(flist.size() - num));
			upd();
		}
		else if(down & KEY_TOUCH)
			for(u16 i=0; i < buttons.size(); i++) {
				if(buttons[i].second.touched()) {
					if(folder == buttons[i].first) {
						if(".." != buttons[i].second.getText()) path += buttons[i].second.getText() + '/';
						else path.erase(path.find_last_of('/', path.size()-2) + 1);
						cd();
						upd();
					}
					else {
						powerOn(PM_BACKLIGHT_TOP);
						return path + buttons[i].second.getText();
					}
				}
			}
	}
}


string extention(string name)
{
	string ext (name.substr(name.find_last_of('.') + 1));
	transform (ext.begin(), ext.end(), ext.begin(), tolower);
	return ext;
}

string noExt(string name)
{
	uint found = name.find_last_of('/');
	string n;
	if (found == string::npos) 
		 n = name.substr(0, name.find_last_of('.'));
	else n = name.substr(found + 1, name.find_last_of('.') - found - 1);
	transform (n.begin(), n.end(), n.begin(), tolower);
	return n;
}

string noPath(string name)
{
	uint found = name.find_last_of('/');
	string n;
	if (found == string::npos) 
		 n = name;
	else n = name.substr(found + 1);
	return n;
}