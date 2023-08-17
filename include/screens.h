#pragma once
#include "default.h"
#include "settings.h"

struct paragrath;

namespace screens
{
	void print_line (Encoding enc, const paragrath& parag, u16 linenum, u16 total_l, bool onlyTop);
	u32 capacity(bool onlyOne);	//maximum number of lines
	const u16 dimX = 255, dimY = 191;
	extern u8 up_margin, side_margins, bottom_margin;
	extern const u16 *layoutDimX[4], *layoutDimY[4];
	extern scr_id current_scr;
	inline u16 layoutX(){return *layoutDimX[settings::layout];}
	inline u16 layoutY(){return *layoutDimY[settings::layout];}
	inline int line_width() {return layoutX() + 1 - side_margins * 2;}
}


inline void toLayoutSpace(u8& x, u8& y)
{
	using namespace screens;
	switch (settings::layout) {
		case d0:	return;
		case d90:	{u8 c=x; x=dimX-y; y=c; return;}
		case d180:	x=dimX-x; y=dimY-y; return;
		case d270:	{u8 c=x; x=y; y=dimY-c;}
	}
}