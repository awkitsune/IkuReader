#include "settings.h"
#include "screens.h"

inline u8 blend(u8 alpha, u8 color, u8 backg)
{ return (alpha * (color - backg) >> 8) + backg; }

inline Color SubpixelBlend(const Color a)
{
	using namespace settings;
	return Color(blend(a.R, fCol.R, bgCol.R), blend(a.G, fCol.G, bgCol.G), blend(a.B, fCol.B, bgCol.B));
}

inline Color Blend(u8 a)
{ return SubpixelBlend(Color(a,a,a)); }

namespace renderer
{
	void initVideo();
	void initFonts();
	void clearScreens(u16 color, u8 onlyone = 42);
	void rect(u16 x1, u16 y1, u16 x2, u16 y2, scr_id = bottom_scr);
	void fillRect(u16 x1, u16 y1, u16 x2, u16 y2, u16 col, scr_id = bottom_scr);
	void vLine(u16 x, u16 y1, u16 y2, u16 col = settings::fCol);
	int strWidth(Encoding, const string&, u32 start=0, u32 end=0, u8 = settings::font_size, fontStyle = fnormal, int* breakat = NULL, int spaceleft=99);
	int printStr(Encoding, scr_id, u16 x, u16 y, const string&, u32 start=0, u32 end=0, u8 = settings::font_size, fontStyle = fnormal);
	void printClock(scr_id, bool forced = false);
	void changeFont();
	extern u16 *bmp[2];
	inline void putPixel(scr_id scr, u8 x, u8 y, u16 color)
	{ 
		toLayoutSpace(x, y);
		bmp[scr][(y << 8) + x] = color;
	}
	inline void putPixel(scr_id scr, u8 x, u8 y, Color c24)
	{ 
		toLayoutSpace(x, y);
		u16 p = bmp[scr][(y << 8) + x];
		u8 r = p & 0x1F;
		u8 g = (p>>5) & 0x1F;
		u8 b = (p>>10) & 0x1F;
		using namespace settings;
		Color c5(blend(c24.R, fCol.R, r), blend(c24.G, fCol.G, g), blend(c24.B, fCol.B, b));
		bmp[scr][(y << 8) + x] = c5;
	}
	class TFlashClock {
		int frames, _scr;
	public:
		TFlashClock() :frames(0), _scr(66) {}
		void show(scr_id scr), hide();
	} extern flashClock;
}