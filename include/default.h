#pragma once
#include <nds.h>
#include "ndsx_brightness.h"
#include "ft2build.h"
#include FT_FREETYPE_H
#include <vector>
#include <string>
using std::string;
using std::vector;
using std::pair;

#define buttonFontSize (16)

extern const int giant_buf_size;
extern char giant_buffer[];

void bsod(const char* msg);
void cycleBacklight();
string extention(string name);
string noExt(string name);
string noPath(string name);
u32 loadToGiantBuffer(const char* filename, u32 offset = 0);
enum Layout {d0=0, d90, d180, d270};
enum Encoding {eUtf8, e1251};
enum scr_id {top_scr, bottom_scr};
enum fontStyle {fnormal, fbold, fitalic};
enum scrConfig {scTop, scBottom, scBoth};

namespace renderTech{
enum type{
	linux	 =	(FT_LOAD_RENDER | FT_LOAD_TARGET_LCD | FT_LOAD_FORCE_AUTOHINT),
	linux_v	 =	(FT_LOAD_RENDER | FT_LOAD_TARGET_LCD_V | FT_LOAD_FORCE_AUTOHINT),
	windows	=	(FT_LOAD_RENDER | FT_LOAD_TARGET_LCD),
	windows_v = (FT_LOAD_RENDER | FT_LOAD_TARGET_LCD_V),
	phone  = 	(FT_LOAD_RENDER | FT_LOAD_TARGET_MONO)
};
}

template<class T, class U> inline void clamp(T& x, U min, U max)
{
	if(x > max) x = max;
	else if(x < min) x = min;
}

struct Color
{
	u8 R,G,B;	//each one must be 0..31
	inline Color(u8 r, u8 g, u8 b) : R(r), G(g), B(b) {}
	Color(){}
	inline operator u16() const {return RGB15(R, G, B) | BIT(15);}
	void invert() {R=31-R; G=31-G; B=31-B;}
};

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a > b ? b : a)

bool file_ok(string& file_name);

namespace LWord {enum lword  {

	files,	resume, light,
	codep,	close,	justify,
	invert,	gamma,	colors,
	screens,rotate,	pbar,
	font,	size,	style,
	gap,	sharp, indent,
	top, 	bottom, both,
	language,older, newer,
	set, 	remove,	ok,
	bookmarks,
	
	totalWords};
}
	
void loadTrans(string file);
	
extern const string transPath;
extern string locDict[LWord::totalWords];
#define SAY(i) &locDict[LWord::i]
#define SAY2(i) locDict[LWord::i]