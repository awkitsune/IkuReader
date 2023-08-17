#include "book.h"
#include "screens.h"
#include "renderer.h"

//r,g,b: 0..255; h: 0..360; s, v: 0..255
inline void HSVtoRGB(u32& r, u32& g, u32& b, u32 h, u32 s, u32 v)
{
	u32 f, p, q, t;
	f = ((h % 60) << 8) / 60;
	h /= 60;
	p = (v * (255 - s)) >> 8;
	q = (v * (255 - ((s * f) >> 8))) >> 8;
	t = (v * (255 - ((s * (255 - f)) >> 8))) >> 8;
	switch(h) {
		case 0:	r = v; g = t; b = p; break;
		case 1:	r = q; g = v; b = p; break;
		case 2:	r = p; g = v; b = t; break;
		case 3:	r = p; g = q; b = v; break;
		case 4:	r = t; g = p; b = v; break;
		case 5: r = v; g = p; b = q;
	}
}

//r,g,b: 0..31; h: 0..360; s, v: 0..255
inline void RGBtoHSV(u32 ri, u32 gi, u32 bi, u32& hi, u32& si, u32& vi)
{
	u32 mini = MIN(ri,MIN(gi,bi)), maxi = MAX(ri,MAX(gi,bi));
	u32 deltai = maxi - mini;
	vi = maxi << 3;
	if (maxi > 0) si = deltai << 3;
	else {
		si = 0;
		return;
	}
	float r = ri / 31.0f, g = gi / 31.0f, b = bi / 31.0f;
	float h = 0.0f, delta = (MAX(ri,MAX(gi,bi)) - MIN(ri,MIN(gi,bi))) / 31.0f;
	if (delta > 0.0f) {
		if (maxi == ri)
			 h = (g - b) / delta;
		else if (maxi == gi)
			 h = (2.0f + (b - r) / delta);
		else h = (4.0f + (r - g) / delta);
		h *= 60.0f;
		if(h < 0) h += 360.0f;
		hi = h;
	}
}

inline Color getPixel(u8 x, u8 y)
{ 
	u16 p = renderer::bmp[bottom_scr][(y << 8) + x];
	return Color(p & 0x1F, (p>>5) & 0x1F, (p>>10) & 0x1F);
}

struct pallete
{
	pallete(u32 x, u32 y);
	void draw();
	u16 x, y;
	button b, b2;
	u32 _h, _s, _v;
};

#define RANGE (screens::dimY / 2u - 10u)

pallete :: pallete(u32 X, u32 Y) : x(X), y(Y), b("", x, y, x + RANGE, y + RANGE),
	b2("", x + RANGE + 15u, y, x + RANGE + 35u, y + RANGE), _h(128), _s(128), _v(128) {}

void pallete :: draw()
{
	u32 h = _h, s, v;
	u32 r=0, g=0, b=0;
	for(u32 i = y ; i <= y + RANGE; i++) {
		v = 255 - 255 * (i - y) / RANGE;
		for(u32 j = x; j <= x + RANGE; ++j) {
			s = 255 * (j - x) / RANGE;
			HSVtoRGB(r, g, b, h, s, v);
			renderer::putPixel(bottom_scr, j, i, (u16)Color(r>>3,g>>3,b>>3));
		}
	}
	v = s = 255;
	for(u32 i = y ; i <= y + RANGE; i++) {
		h = 360 * (i - y) / RANGE;
		HSVtoRGB(r, g, b, h, s, v);
		u16 d = Color(r>>3,g>>3,b>>3);
		for(u32 j = x + RANGE + 15u; j <= x + RANGE + 35u; ++j) 
			renderer::putPixel(bottom_scr, j, i, d);
	}
}

void colToP(Color& c, pallete& p)
{
	u32 h=0, s=0, v=0;
	RGBtoHSV(c.R, c.G, c.B, h, s, v);
	p._h = h;
}

void colToP2(Color& c, pallete& p)
{
	u32 r=0, g=0, b=0;
	HSVtoRGB(r, g, b, p._h, p._s, p._v);
	c = Color(r>>3, g>>3, b>>3);
}

void colToP_sv(Color& c, pallete& p)
{
	u32 h=0, s=0, v=0;
	RGBtoHSV(c.R, c.G, c.B, h, s, v);
	p._s = s; p._v = v;
} 

void Book :: colorPicker()
{
	using namespace screens;
	int third = screens::dimY / 3;
	int offx = (layoutX() - dimY)/2 + (screens::dimX % third)/2;
	int offy = (layoutY() - dimY)/2 + (screens::dimX % third)/2;
	button ok(SAY2(ok), offx + RANGE + 50, offy + RANGE);

	pallete fon(5 + offx, 5 + offy);
	pallete bg(5 + offx, screens::dimY / 2 + 5 + offy);
	colToP(settings::bgCol, bg);
	colToP_sv(settings::bgCol, bg);
	colToP(settings::fCol, fon);
	colToP_sv(settings::fCol, fon);
	renderer::clearScreens(settings::bgCol, bottom_scr);
	fon.draw(); bg.draw(); ok.draw();
	
	while(1) {
		swiWaitForVBlank();
		scanKeys();
		if(!(keysDown() & KEY_TOUCH)) continue;
		if(ok.touched()) return;
		if(fon.b.touched()) {	//s,v
			touchPosition t1;
			touchRead(&t1);
			settings::fCol = getPixel(t1.px, t1.py);
			colToP_sv(settings::fCol, fon);
			draw_page(true, false);
			renderer::clearScreens(settings::bgCol, bottom_scr);
			fon.draw(); bg.draw(); ok.draw();
		}
		else if(fon.b2.touched()) { //h
			touchPosition t1;
			touchRead(&t1);
			settings::fCol = getPixel(t1.px, t1.py);
			colToP(settings::fCol, fon);
			colToP2(settings::fCol, fon);
			draw_page(true, false);
			renderer::clearScreens(settings::bgCol, bottom_scr);
			fon.draw(); bg.draw(); ok.draw();
		}
		else if(bg.b.touched()) { //s,v
			touchPosition t1;
			touchRead(&t1);
			settings::bgCol = getPixel(t1.px, t1.py);
			colToP_sv(settings::bgCol, bg);
			draw_page(true, false);
			renderer::clearScreens(settings::bgCol, bottom_scr);
			fon.draw(); bg.draw(); ok.draw();
		}
		else if(bg.b2.touched()) { //h
			touchPosition t1;
			touchRead(&t1);
			settings::bgCol = getPixel(t1.px, t1.py);
			colToP(settings::bgCol, bg);
			colToP2(settings::bgCol, bg);
			draw_page(true, false);
			renderer::clearScreens(settings::bgCol, bottom_scr);
			fon.draw(); bg.draw(); ok.draw();
		}
	}
}

void Book :: sharpness()
{
	u32 height = buttonFontSize * 6;
	string titles[] = {"DS Lite", "Original DS & DSi", "Emulator", "UFO", SAY2(ok)};
	u32 width = renderer::strWidth(eUtf8, titles[1],0 ,0, buttonFontSize);
	button b[5];
	int offx = screens::layoutX() / 2 - width / 2;
	int offy = screens::layoutY() / 2 - height / 2;
	renderer::clearScreens(settings::bgCol, bottom_scr);
	for(int i = 0; i < 5; i++) {
		b[i] = button(titles[1], offx, offy + (i + (i > 3)) * buttonFontSize);
		b[i].txt = titles[i];
		b[i].draw();
	}
	while(1) {
		swiWaitForVBlank();
		scanKeys();
		if(!(keysDown() & KEY_TOUCH)) continue;
		if(b[5].touched()) return;
		for(int i = 0; i < 5; i++) {
			if(b[i].touched()) {
				switch(i) {
					case 0: settings:: BGR = 1; break;
					case 1: settings:: BGR = 2; break;
					case 2: settings:: BGR = 0; break;
					case 3: settings:: BGR = 3; break;
					case 4: return;
				}
				draw_page(true, false);
				renderer::clearScreens(settings::bgCol, bottom_scr);
				for(int i = 0; i < 5; i++) b[i].draw();
				continue;
			}
		}
	}
	drawMenu(false);
}