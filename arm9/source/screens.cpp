#include "screens.h"
#include "renderer.h"
#include "book.h"
#include "settings.h"

extern fontStyle accMarks(int start, int end, const vector<marked>& marks, fontStyle defstyle);

namespace screens
{

u8 up_margin=0, side_margins=3, bottom_margin=0; 
u16 penX, penY;
scr_id current_scr;
const u16* layoutDimX[] = {&dimX, &dimY,&dimX, &dimY};
const u16* layoutDimY[] = {&dimY, &dimX,&dimY, &dimX};

u32 capacity(bool onlyOne)
{
	return (onlyOne ? 1 : 2) * ((layoutY() - up_margin - bottom_margin - settings::font_size/4)
			/ (settings::font_size + settings::line_gap));
}

void print_line(Encoding enc, const paragrath& parag, u16 linenum, u16 total_l, bool onlyTop)
{
	if(total_l == 0) {
		scr_id scr = onlyTop ? top_scr : first_scr();
		renderer::clearScreens(settings::bgCol, scr);
		current_scr = scr;
		penY = up_margin;
	}
	else if(total_l == capacity(false)/2) {
		current_scr = (scr_id)!current_scr;
		renderer::clearScreens(settings::bgCol, current_scr);
		penY = up_margin;
	}
	if(penY != up_margin) penY += settings::line_gap;
	penY += settings::font_size;
	penX = side_margins;
	u8 space_width = renderer::strWidth(e1251, " ");
	if (0 == linenum && pnormal == parag.type) penX += settings::first_indent;
	int remained = line_width() - parag.lines[linenum].width;
	
	if(0 == linenum) remained -= settings::first_indent;
	u8 space_justify = 0;
	if (settings::justify && pnormal == parag.type && remained > 0 && linenum != parag.lines.size()-1 
			&& parag.lines[linenum].words.size() > 1)
		space_justify = remained / (parag.lines[linenum].words.size()-1);
	
	if(parag.type == ptitle) penX = (line_width() - parag.lines[linenum].width) / 2;
	
	fontStyle st[] = {fnormal, fbold, fitalic};
	fontStyle style = st[parag.type];
	for(vector<Word>::const_iterator it = parag.lines[linenum].words.begin(); it != parag.lines[linenum].words.end(); ++it) {
		fontStyle actual_style = style;
		actual_style = accMarks(it->start, it->end, parag.marks, style);
		
		penX += renderer::printStr(enc, current_scr, penX, penY, parag.str, it->start, it->end, settings::font_size, actual_style)
			+ space_width;
		if (it->hyphen) renderer::printStr(enc, current_scr, penX - space_width, penY, "-");
		penX += space_justify;
	}
}

} //namespace screens