#include "book.h"
#include "hyphen_ru.h"
#include "utf8.h"
#include "screens.h"
#include "renderer.h"

fontStyle accMarks(int start, int end, const vector<marked>& marks, fontStyle defstyle)
{
	for(u32 i = 0; i < marks.size(); i++) {
		if(end < marks[i].start) break;
		else if(start >= marks[i].start && end <= marks[i].end) return marks[i].style;
		
	}
	return defstyle;
}

void Book :: parsePar()
{
	int space_left = screens::line_width();
	if(pnormal == parag.type) space_left -= settings::first_indent;
	parag.lines.push_back(Line());
	const u16 space_width = renderer::strWidth(e1251, " ");

	fontStyle st[] = {fnormal, fbold, fitalic};
	fontStyle style = st[parag.type];
	
	for(u32 start=0, end=0; ; start = end+1){
		fontStyle actual_style = style;
		
		start = parag.str.find_first_not_of(" \t\r\n", start);
		if (start == string::npos) break;
		end = parag.str.find_first_of(" \t\r\n", start);
		if (end == string::npos) end = parag.str.length();
		
		actual_style = accMarks(start, end, parag.marks, style);
		
		u16 width = renderer::strWidth(encoding, parag.str, start, end, settings::font_size, actual_style);
		Word word = {start, end, width, false};

		

		if (word.width > space_left - space_width) {
			if(parag.type != ptitle) while(hyphenate(&word, space_left, actual_style));
			parag.lines.back().width += space_width * (parag.lines.back().words.size() - 1);
			parag.lines.push_back(Line());
			space_left = screens::line_width() - word.width;
		}
		else space_left = space_left - (word.width + space_width);
		parag.lines.back().words.push_back (word);
		parag.lines.back().width += word.width;
	}

}

bool Book :: hyphenate(Word* word_it, int& space_left, fontStyle style)
{
	const u16 space_width = renderer::strWidth(e1251, " ");
	int hyph_width = renderer::strWidth(e1251, "-");
	int pos = 0, first_half_width = 0;
	if(word_it->end - word_it->start < 100) {
		vector<bool> hyphens = hyphen_pos(parag.str, word_it->start, word_it->end, encoding);
		for(u32 i = 0; i < hyphens.size(); i++) {
			if (hyphens[i]) {
				char* break_at;
				if(encoding == eUtf8) {
					break_at = &parag.str[word_it->start];
					utf8::unchecked::advance(break_at, i + 1);
				}
				else break_at = &parag.str[word_it->start + i + 1];
				int width = renderer::strWidth(encoding, parag.str, word_it->start, break_at - &parag.str[0], settings::font_size, style);
				if (width > space_left - space_width - hyph_width) break;
				pos = i + 1;
				first_half_width = width;
				word_it->hyphen = true;
			}
		}
	}

	if(word_it->width >= screens::line_width() && pos == 0)
		first_half_width = renderer::strWidth(encoding, parag.str, word_it->start, word_it->end, settings::font_size, style, &pos, space_left);
	
	if(pos) {
		{
			char* break_at;
			if(encoding == eUtf8) {
				break_at = &parag.str[word_it->start];
				utf8::unchecked::advance(break_at, pos);
			}
			else break_at = &parag.str[word_it->start + pos];
			int new_start = break_at - &parag.str[0];
			int sec_half_width = renderer::strWidth(encoding, parag.str, new_start, word_it->end, settings::font_size, style);
			Word second_half = {new_start, word_it->end, sec_half_width, false};
			if (*(break_at - 1) == '-') word_it->hyphen = false;
			word_it->width = first_half_width + (word_it->hyphen ? hyph_width : 0);
			word_it->end = new_start;
			parag.lines.back().words.push_back(*word_it);
			parag.lines.back().width += word_it->width;
			*word_it = second_half;
		}
		if(word_it->width > screens::line_width()) {
			parag.lines.back().width += space_width * (parag.lines.back().words.size() - 1);
			parag.lines.push_back(Line());
			space_left = screens::line_width() - hyph_width;
			return true;
		}
	}
	return false;
}