#pragma once
#include "default.h"
#include "button.h"
#include <set>
#include "base64.h"

struct Word	{int start, end, width; bool hyphen;};

struct marked {int start, end; fontStyle style;};

struct Line
{
	u16 width;
	vector<Word> words;
	Line() : width(0) {}
};

enum parType{pnormal, ptitle, pstanza};

struct paragrath
{
	parType type;
	string str;
	vector<Line> lines;
	vector<marked> marks;
	paragrath() : type(pnormal) {}
};

struct bookmark
{
	u32 parag_num, line_num;
	bookmark(u32 p, u32 l) : parag_num(p), line_num(l) {}
	bookmark(){}
	bool operator<(const bookmark& b) const
	{ return parag_num < b.parag_num; }
};

class Book
{
public:
	void read();
	Book(const string& filename) : bookFile(filename), menuGrid(0) { 
		current_page.parag_num = 0;
		current_page.line_num = 0;
		prev_par_num = 1 << 30;
		string name = noPath(bookFile);
		encname = "/data/IkuReader/bookmarks/" + base64_encode(reinterpret_cast<const unsigned char*>(name.c_str()), name.length()) +".bm";
	}
protected:
	string bookFile, encname;
	bookmark current_page;
	std::set<bookmark> bookmarks;
	virtual void parse() = 0;
	virtual void parag_str (int parag_num) = 0;
	void fetch_paragrath (int parag_num) {
		parag_str (parag_num);
		parsePar();
		prev_par_num = parag_num;
	}
	u32 prev_par_num;
	void parsePar();
	bool hyphenate(Word* word_it, int& space_left, fontStyle style);
	
	virtual u32 total_paragraths() = 0;
	paragrath parag;
	Encoding encoding;
private:
	void next_page(), previous_page(), draw_page(bool onlyTop = false, bool cachePar = true);
	
	void bookmarkMenu();
	void drawBookmarkMenu();
	button setMark, older, newer;
	progressbar prbar;
	bool moreNew() {return !bookmarks.empty() && current_page < *bookmarks.rbegin();}
	bool moreOld() {return !bookmarks.empty() && *bookmarks.begin()  < current_page;}
	
	bool menu();
	void drawMenu(bool recache = true);
	grid menuGrid;
	bool otherGrid;
	void colorPicker(), sharpness(), search();
	
	void loadMarks(), saveMarks(), code_page(), translation();
};