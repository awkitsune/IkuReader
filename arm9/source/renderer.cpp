#include "renderer.h"
#include "utf8.h"
#include "encoding_tables.h"
#include <algorithm>
#include <time.h>
#include <sys/dir.h>
#include <sys/unistd.h>
#include <set>
#include FT_LCD_FILTER_H
#include FT_CACHE_H

namespace renderer
{
u16 *bmp[2];
TFlashClock flashClock;
static const u8 gamma150[32] = {0, 0, 1, 1, 1, 2, 3, 3, 4, 5, 6, 7, 7, 8, 9, 10, 11, 13, 14, 15, 16, 17, 19, 20, 21, 22, 24, 25, 27, 28, 30, 31}; //1.5
static const u8 gamma067[32] = {0, 3, 5, 7, 8, 9, 10, 11, 13, 14, 15, 16, 16, 17, 18, 19, 20, 21, 22, 22, 23, 24, 25, 25, 26, 27, 28, 28, 29, 30, 30, 31};

inline void putPixel150(scr_id scr, u8 x, u8 y, Color c24)
{ 
	toLayoutSpace(x, y);
	u16 p = bmp[scr][(y << 8) + x];
	u8 r = p & 0x1F;
	u8 g = (p>>5) & 0x1F;
	u8 b = (p>>10) & 0x1F;
	using namespace settings;
	Color c5(gamma150[blend(c24.R, fCol.R, r)], gamma150[blend(c24.G, fCol.G, g)], gamma150[blend(c24.B, fCol.B, b)]);
	bmp[scr][(y << 8) + x] = c5;
}


inline void putPixel067(scr_id scr, u8 x, u8 y, Color c24)
{ 
	toLayoutSpace(x, y);
	u16 p = bmp[scr][(y << 8) + x];
	u8 r = p & 0x1F;
	u8 g = (p>>5) & 0x1F;
	u8 b = (p>>10) & 0x1F;
	using namespace settings;
	Color c5(gamma067[blend(c24.R, fCol.R, r)], gamma067[blend(c24.G, fCol.G, g)], gamma067[blend(c24.B, fCol.B, b)]);
	bmp[scr][(y << 8) + x] = c5;
}

template<class T> inline void swapRB(T& r, T& b, scr_id scr)
{
	switch(settings::BGR) {
		case 1: std::swap(r, b); break;
		case 2: if(scr == top_scr) std::swap(r, b); break;
		case 3: if(scr != top_scr) std::swap(r, b); break;
		default: ;
	}
}

void charLcd(scr_id scr, u8 x, u8 y, FTC_SBit bitmap)
{
    u8 *srcLine = bitmap->buffer;
    u8 width = bitmap->width, height = bitmap->height;
    u8 pitch = bitmap->pitch;
	
	if(settings::tech == renderTech::phone) {
		u16 col = settings::fCol;
		for(u8 j = 0; j < height;  ++j, srcLine += pitch)
			for (u8 i = 0; i < width; ++i) if (srcLine[i >> 3] & (0x80 >> (i & 7))) putPixel(scr, x+i, j+y, (u16)col);
		return;
	}
	
	u8 gap_line, gap_pix, s0, s1, s2;	

	if(settings::tech == renderTech::windows || settings::tech == renderTech::linux) {
		width /= 3;
		gap_line = pitch;
		gap_pix = 3;
		s0 = 0; s1 = 1; s2 = 2;
	}
	else { //windows_v || linux_v
		height /= 3;
		gap_line = 3*pitch;
		gap_pix = 1;
		s0 = 0; s1 = pitch; s2 = pitch * 2;
	}
	
	swapRB(s0, s2, scr);
	if (d90 == settings::layout || d180 == settings::layout) std::swap(s0, s2);
	
	void (*putPix)(scr_id, u8, u8, Color) = &putPixel;
	if(settings::gamma != 1) {
		u8 summ = settings::fCol.R + settings::fCol.G + settings::fCol.B;
		if(settings::gamma == 0) summ = 93 - summ;
		putPix = (summ > 46) ? &putPixel067 : &putPixel150;
	}
	for (u8 j = 0; j < height; ++j, srcLine += gap_line)
		for (u8 i = 0, *src = srcLine; i < width; ++i, src += gap_pix)
			if (src[s0] || src[s1] || src[s2]) putPix(scr, x+i, j+y, Color(src[s0], src[s1], src[s2]));
}

void initVideo()
{
	lcdSwap();
	videoSetMode(MODE_5_2D);
	videoSetModeSub(MODE_5_2D);
	vramSetPrimaryBanks(VRAM_A_MAIN_BG_0x06000000, VRAM_B_MAIN_BG_0x06020000, VRAM_C_SUB_BG, VRAM_D_MAIN_BG_0x06040000); 
	consoleInit(NULL, 1, BgType_Text4bpp, BgSize_T_256x256, 8, 9, true, true);
	int bg 	= 	bgInit	 (2, BgType_Bmp16, BgSize_B16_256x256, 10, 0);	//bottom
	int subBg = bgInitSub(2, BgType_Bmp16, BgSize_B16_256x256, 0, 0);	//top
	bmp[bottom_scr] = (u16*)bgGetGfxPtr(bg);
	bmp[top_scr] 	= (u16*)bgGetGfxPtr(subBg);
	dmaFillWords (0, BG_GFX, 256 * 192 * 2);
	keyboardInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x512, 0, 4, true, true);
	clearScreens(0);
}

u16 borderPixelLeft(scr_id scr)
{
	using settings::bgCol;
	u32 r = bgCol.R, g = bgCol.G, b = bgCol.B;
	swapRB(r, b, scr);
	u32 nR = g / 3 + (3*r + 2*g) / 27, 
		nG = (b + g) / 3 + (3*g + 2*r) / 27;
	Color c(nR, nG, b);
	swapRB(c.R, c.B, scr);
	return c;
}

u16 borderPixelRight(scr_id scr)
{
	using settings::bgCol;
	u32 r = bgCol.R, g = bgCol.G, b = bgCol.B;
	swapRB(r, b, scr);
	u32 nB = g / 3 + (3*b + 2*g) / 27,
		nG = (r + g) / 3 + (3*g + 2*b) / 27;
	Color c(r, nG, nB);
	swapRB(c.R, c.B, scr);
	return c;
}

void clearScreens(u16 color, u8 onlyone)
{
		//filter read and blue artifacts on screen edges (reference: http://www.grc.com/cttech.htm)
	u16 leftp[2]  = {borderPixelLeft(top_scr),  borderPixelLeft(bottom_scr)},
		rightp[2] = {borderPixelRight(top_scr), borderPixelRight(bottom_scr)};
	swiWaitForVBlank();
	if(onlyone != top_scr && onlyone != bottom_scr) {
		dmaFillWords (color | (color<<16), bmp[top_scr], 256 * 192 * 2);
		dmaFillWords (color | (color<<16), bmp[bottom_scr], 256 * 192 * 2);
		if(color) for(u32 i = 0, y = 0; i <= screens::dimY; i++) {
			y = i << 8;
			bmp[top_scr][y] = leftp[top_scr];
			bmp[top_scr][y + screens::dimX] = rightp[top_scr];
			bmp[bottom_scr][y] = leftp[bottom_scr];
			bmp[bottom_scr][y + screens::dimX] = rightp[bottom_scr];
		}
	}
	else {
		dmaFillWords (color | (color<<16), bmp[onlyone], 256 * 192 * 2);
		if(color) for(u32 i = 0, y = 0; i <= screens::dimY; i++) {
			y = i << 8;
			bmp[onlyone][y] = leftp[onlyone];
			bmp[onlyone][y + screens::dimX] = rightp[onlyone];
		}
	}
}

FT_Library		ft_lib;
FT_Face			face;
FT_Face			faceB;
FT_Face			faceI;
FTC_Manager		ftcManager;
FTC_SBitCache	ftcSBitCache;
FTC_SBit		ftcSBit;
FTC_ImageType	ftcImageType;

void setFontSize(u8 f)
{ ftcImageType->height = ftcImageType->width = f; }

void correctTech();

FT_Error ftcFaceRequester(FTC_FaceID faceID, FT_Library lib, FT_Pointer reqData, FT_Face *aface)
{
	string *st = NULL;
	using namespace settings;
	if		(faceID == &font) 			st = &settings::font;
	else if (faceID == &font_bold)		st = &settings::font_bold;
	else if (faceID == &font_italic)	st = &settings::font_italic;
	string file, p("/data/IkuReader/fonts/");
	if(!st->compare(0, p.length(), p)) file = *st;
	else file = p + *st;
	
	FT_Error ft_err = FT_New_Face (ft_lib, file.c_str(), 0, aface); 
	if(ft_err) bsod("Can't load font.");
	FT_Select_Charmap(face, FT_ENCODING_UNICODE);
	return ft_err;
}

void initFonts()
{
	setFontSize(settings::font_size);
	ftcImageType->flags = settings::tech;
	
	string file("/data/IkuReader/fonts/" + settings::font);
	if(!file_ok(file)) changeFont();
	
	if(	
		FT_Init_FreeType(&ft_lib) ||
		FT_Library_SetLcdFilter(ft_lib, FT_LCD_FILTER_DEFAULT) ||
		FTC_Manager_New(ft_lib, 3, 0, 0, ftcFaceRequester, 0, &ftcManager) ||
		FTC_SBitCache_New(ftcManager, &ftcSBitCache)
	) bsod("Freetype error.");
	FTC_Manager_LookupFace(ftcManager, &settings::font, &face);
	FTC_Manager_LookupFace(ftcManager, &settings::font_bold, &faceB);
	FTC_Manager_LookupFace(ftcManager, &settings::font_italic, &faceI);
}

FT_Face* selectStyle(fontStyle style)
{
	FT_Face* f[] = {&face, &faceB, &faceI};
	string* f2[] = {&settings::font, &settings::font_bold, &settings::font_italic};
	ftcImageType->face_id = f2[style];
	return f[style];
}

int printStr(Encoding enc, scr_id scr, u16 x, u16 y, const string& str, u32 start, u32 end, u8 fontSize, fontStyle style)
{
	setFontSize(fontSize);
	correctTech();
	const u8 startx = x;
	if(0 == end) end = str.size();
	if (end > str.size() || start >= end) return 0;
	FT_Face *fc = selectStyle(style);
	bool use_kern = FT_HAS_KERNING((*fc));
	FT_UInt glyph_index, old_gi = 0;
	FT_Vector  delta;
	for(const char* str_it = &str[start]; str_it < &str[end];) {
		u32 cp;
		if (eUtf8 == enc)	cp = utf8::unchecked::next(str_it);
		else cp = cp1251toUtf32[(u8)*str_it++];
		if(cp == L'­') continue; //soft hyphen
		glyph_index = FT_Get_Char_Index(*fc, cp);
		if(use_kern) {
			FT_Get_Kerning(*fc,  old_gi, glyph_index, FT_KERNING_DEFAULT, &delta);
			x += delta.x >> 6;
		}
		FT_Error err = FTC_SBitCache_Lookup (ftcSBitCache, ftcImageType, glyph_index, &ftcSBit, NULL);
		if(err) {old_gi = 0; continue;}
		if(x + ftcSBit->xadvance > screens::layoutX()) break;
		charLcd(scr, x + ftcSBit->left, y - ftcSBit->top, ftcSBit);
		x += ftcSBit->xadvance;
		old_gi = glyph_index;
	}
	return x - startx;
}

int strWidth (Encoding enc, const string& str, u32 start, u32 end, u8 fontSize, fontStyle style, int* breakat, int spaceleft)
{
	if(breakat != NULL) *breakat = 0;
	setFontSize(fontSize);
	correctTech();
	int width = 0;
	if(0 == end) end = str.length();
	if (end > str.size() || start >= end) return 0;
	FT_Face *fc = selectStyle(style);
	bool use_kern = FT_HAS_KERNING((*fc));
	FT_UInt glyph_index, old_gi = 0;
	FT_Vector  delta;
	int i = 0;
	int prev_width = 0;
	for(const char* str_it = &str[start], *prev = NULL; str_it < &str[end];) {
		if(breakat != NULL) {
			if(prev != NULL) *breakat = i - 1;
			if(width >= spaceleft) {
				width = prev_width;
				break;
			}
		}
		prev_width = width;
		prev = str_it;
		i++;

		u32 cp;
		if (eUtf8 == enc)	cp = utf8::unchecked::next(str_it);
		else cp = cp1251toUtf32[(u8)*str_it++];
		if(cp == L'­') continue; //soft hyphen
		glyph_index = FT_Get_Char_Index(*fc, cp);
		if(use_kern) {
			FT_Get_Kerning(*fc,  old_gi, glyph_index, FT_KERNING_DEFAULT, &delta);
			width += delta.x >> 6;
		}
		FT_Error err = FTC_SBitCache_Lookup (ftcSBitCache, ftcImageType, glyph_index, &ftcSBit, NULL);
		if(err) {old_gi = 0; continue;}
		width += ftcSBit->xadvance;
		if(width > screens::layoutX()) return 9999;
		old_gi = glyph_index;
	}
	return width;
}

void drawLine(u8 x1, u8 y1, u8 x2, u8 y2, u16 color, scr_id scr)
{
	toLayoutSpace(x1, y1);
	toLayoutSpace(x2, y2);
	
	int deltaX = abs(x2 - x1);
	int deltaY = abs(y2 - y1);
	int signX = x1 < x2 ? 1 : -1;
	int signY = y1 < y2 ? 1 : -1;
	int error = deltaX - deltaY;
	while(true) {
		bmp[scr][(y1 << 8) + x1] = color;
		if(x1 == x2 && y1 == y2)
				break;
		int error2 = error * 2;
		if(error2 > -deltaY) {
				error -= deltaY;
				x1 += signX;
		}
		if(error2 < deltaX) {
				error += deltaX;
				y1 += signY;
		}
	}
}

void rect(u16 x1, u16 y1, u16 x2, u16 y2, scr_id scr)
{
	const u16 gray = Blend(128);
	drawLine(x1, y1 + 2, x1, y2 - 2, gray, scr);
	drawLine(x2, y1 + 2, x2, y2 - 2, gray, scr);
	drawLine(x1 + 2, y1, x2 - 2, y1, gray, scr);
	drawLine(x1 + 2, y2, x2 - 2, y2, gray, scr);
	putPixel(scr, x1 + 1, y1 + 1, gray);
	putPixel(scr, x2 - 1, y1 + 1, gray);
	putPixel(scr, x1 + 1, y2 - 1, gray);
	putPixel(scr, x2 - 1, y2 - 1, gray);
}

void fillRect(u16 x1, u16 y1, u16 x2, u16 y2, u16 col, scr_id scr)
{
	drawLine(x1 + 2, y1, x2 - 2, y1, col, scr);
	drawLine(x1 + 1, y1 + 1, x2 - 1, y1 + 1, col, scr);
	for (u32 i = y1 + 2u; i <= y2 - 2u; i++) drawLine(x1, i, x2, i, col, scr);
	drawLine(x1 + 1, y2 - 1, x2 - 1, y2 - 1, col, scr);
	drawLine(x1 + 2, y1, x2 - 2, y1, col, scr);
}

void vLine(u16 x, u16 y1, u16 y2, u16 col)
{ for (u32 i = y1 ; i <= y2; i++) putPixel(bottom_scr, x, i, col); }

void correctTech()
{
	using namespace settings;
	using namespace renderTech;
	if(windows == tech || windows_v == tech)
		tech = (layout == d0 || layout == d180) ? windows : windows_v;
	else if(linux == tech || linux_v == tech)
		tech = (layout == d0 || layout == d180) ? linux : linux_v;
	ftcImageType->flags = tech;
}

void printClock(scr_id scr, bool forced)
{
	static int olds = -69;
	time_t unixTime = time(NULL);
	struct tm* timeStruct = gmtime((const time_t *)&unixTime);
	int s = timeStruct->tm_sec;
	if(s == olds && !forced) return;
	olds = s;
	char buf[14]; 
	sprintf(buf, "%02d:%02d:%02d", timeStruct->tm_hour, timeStruct->tm_min, s);
	u32 y1 = screens::layoutY() - buttonFontSize*3/2;
	u32 y2 = screens::layoutY();
	u32 length = strWidth(eUtf8, buf, 0, 0, buttonFontSize);
	fillRect(5, y1, length + 7, y2, settings::bgCol, scr);
	renderer::printStr(eUtf8, scr, 5, screens::layoutY()-buttonFontSize/2, buf, 0, 0, buttonFontSize);
}

void changeFont()
{
	char fname[MAXPATHLEN];
	struct stat st;
	DIR_ITER* dir = diropen ("/data/ikureader/fonts/");
	if (dir == NULL) bsod("Cannot open directory.");
	std::set<string> files;
	while (0 == dirnext(dir, fname, &st))
		if (!(st.st_mode & S_IFDIR)) if(extention(fname) == "ttf") files.insert(noExt(fname));
	dirclose(dir);
	std::set<string> fonts;
	
	for (std::set<string>::iterator it = files.begin(); it != files.end(); ++it)
		if(files.find(*it + 'b') != files.end() && files.find(*it + 'i') != files.end())
			fonts.insert(*it);
	
	if(fonts.empty()) bsod("No fonts found.");
	
	std::set<string>::iterator current = ++fonts.find(noExt(settings::font));
	if(current == fonts.end()) current = fonts.begin();
	using namespace settings;
	font = *current + ".ttf";
	font_bold = *current + "b.ttf";
	font_italic = *current + "i.ttf";
	FTC_Manager_Done(ftcManager);
	FT_Done_FreeType(ft_lib);
	initFonts();
}

void TFlashClock :: show(scr_id scr)
{
	_scr = scr;
	frames = 0;
	dmaCopy (bmp[_scr], bmp[bottom_scr] + 256*256, 256 * 192 * 2);
	time_t unixTime = time(0);
	struct tm* timeStruct = gmtime((const time_t *)&unixTime);
	char buf[14];
	sprintf(buf, "%02d:%02d:%02d", timeStruct->tm_hour, timeStruct->tm_min, timeStruct->tm_sec);
	u32 y1 = screens::layoutY() - buttonFontSize*3/2;
	u32 y2 = y1 + buttonFontSize + 1;
	u32 length = strWidth(eUtf8, buf, 0, 0, buttonFontSize);
	fillRect(5, y1, length + 7, y2, settings::bgCol, scr);
	rect(5, y1, length + 7, y2, scr);
	printStr(eUtf8, scr, 6, screens::layoutY()-buttonFontSize/2, buf, 0, 0, buttonFontSize);
}

void TFlashClock :: hide()
{
	if(++frames > 20 && _scr != 66) {
		dmaCopy (bmp[bottom_scr] + 256*256, bmp[_scr], 256 * 192 * 2);
		_scr = 66;
	}
}

} // namespace renderer
