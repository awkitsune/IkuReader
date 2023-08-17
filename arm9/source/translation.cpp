#include "default.h"
#include <fstream>
using namespace std;

const string transPath("/data/ikureader/translations/");

/*namespace LWord {enum lword  {
	
	files,	resume, light,
	codep,	close,	justify,
	invert,	gamma,	colors,
	screens,rotate,	pbar,
	font,	size,	style,
	gap,	sharp, indent
	top, 	bottom, both,
	language,older, newer,
	set, 	remove,	ok,
	bookmarks,
	
	totalWords};}
*/

string locDict[LWord::totalWords] = {
	"Files",	"Resume",	"Backlight",
	"Encoding",	"Close",	"Justify",
	"Invert",	"Thickness","Colors",
	"Screens",	"Rotate",	"Progress",
	"Font",		"Size",		"Style",
	"Line gap",	"Sharpness","Indent",
	"top", 		"bottom",	"both",
	"Language",	"< Older",	"Newer >",
	"Set",		"Remove",	"Ok",
	"Bookmarks"
};


void loadTrans(string file)
{
	std::ifstream is(file.c_str());
	const u8 Bom[] = "\xEF\xBB\xBF";
	char beginning[3];
	is.read(beginning, 3);
	bool hasBom =  !memcmp(&Bom, &beginning, 3);
	if(!hasBom) is.seekg(0);
	for(int i = 0; is && i < LWord::totalWords; i++) {
		getline(is, locDict[i]);
		if(locDict[i][locDict[i].size() - 1] == '\r') locDict[i].erase(locDict[i].size() - 1);
	}
}