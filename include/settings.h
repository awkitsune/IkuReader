#pragma once
#include "default.h"

namespace settings
{
	void load(), save();
	extern s8 font_size, first_indent, line_gap, BGR;
	extern bool justify, pbar;
	extern string font, font_bold, font_italic, recent_book, binname, encname, translname;
	extern Layout layout;
	extern renderTech::type tech;
	extern Color bgCol, fCol;
	extern scrConfig scrConf;
	extern int brightness, gamma;
}

inline scr_id first_scr()
{
	if(settings::scrConf != scBoth) return (scr_id)settings::scrConf;
	else return (settings::layout < d180) ? top_scr : bottom_scr;
}