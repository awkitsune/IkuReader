#include "book.h"
#include "renderer.h"
#include "button.h"
#include <algorithm>
#include <locale>

namespace {
	// find substring (case insensitive)
template<typename charT>
struct my_equal {
    my_equal( const std::locale& loc ) : loc_(loc) {}
    bool operator()(charT ch1, charT ch2) {
        return std::toupper(ch1, loc_) == std::toupper(ch2, loc_);
    }
private:
    const std::locale& loc_;
};
template<typename T>
int ci_find_substr( const T& str1, const T& str2, const std::locale& loc = std::locale() )
{
    typename T::const_iterator it = std::search( str1.begin(), str1.end(), 
        str2.begin(), str2.end(), my_equal<typename T::value_type>(loc) );
    if ( it != str1.end() ) return it - str1.begin();
    else return -1; // not found
}

const string CIstr ("Case insensitive: ");

inline void print_serarch() {consoleClear(); iprintf("Search for: ");}

void switchCI(bool& CI, button& b)
{
	CI = !CI;
	b.setText(CIstr + (CI ? "yes" : "no"));
	b.draw();
}
bool unifind(bool CI, const string& str1, const string& str2)
{
	return CI ? (ci_find_substr(str1, str2) != -1)
		:(str1.find(str2) != string::npos);
}
void percent(u32 i, u32 total_parag, string& str)
{
	if(i % 20 != 0) return;
	print_serarch();
	iprintf("%s\n%d%%", str.c_str(), i * 100 / total_parag);
}
}

void Book :: search()
{
	bool doCI = false;
	const Layout old_layout = settings::layout;
	settings::layout = d0;
	current_page.line_num = 0;
	draw_page(true);
	renderer::clearScreens(0, bottom_scr);
	print_serarch();
	keyboardShow();
	string searchstr;
	button prev("Previous", 5, 30), next("Next", 5, 30 + 10 + buttonFontSize)
		, CI((CIstr + "yes").c_str(), 5, 30 + 20 + 2 * buttonFontSize);
	CI.solid = prev.solid = next.solid = true;
	switchCI(doCI, CI);
	prev.draw();
	next.draw();
	CI.draw();
	const u32 total_parag = total_paragraths();
	string& paragStr = parag.str;
	while(1) {
		swiWaitForVBlank();
		scanKeys();
		if(keysDown() & KEY_SELECT) break;
		else if(keysDown() & KEY_TOUCH) {
			if(next.touched() && searchstr.length()) {
				u32 i = 0;
				for(i = current_page.parag_num + 1; i < total_parag; i++) {
					percent(i, total_parag, searchstr);
					parag_str(i);
					if(unifind(doCI, paragStr, searchstr)) {
						current_page.parag_num = i;
						current_page.line_num = 0;
						draw_page(true);
						break;
					}
				}
				print_serarch();
				iprintf("%s\n", searchstr.c_str());
				if (i >= total_parag) {
					print_serarch();
					iprintf("%s\nnot found\n", searchstr.c_str());
				}
			}
			else if(prev.touched() && searchstr.length() && current_page.parag_num) {
				u32 i = 0;
				for(i = current_page.parag_num - 1; i; i--) {
					percent(i, total_parag, searchstr);
					parag_str(i);
					if(unifind(doCI, paragStr, searchstr)) {
						current_page.parag_num = i;
						current_page.line_num = 0;
						draw_page(true);
						break;
					}
				}
				print_serarch();
				iprintf("%s\n", searchstr.c_str());
				if (i == 0) {
					print_serarch();
					iprintf("%s\nnot found\n", searchstr.c_str());
				}
			}
			else if(CI.touched()) switchCI(doCI, CI);
		}
        int key = keyboardUpdate();
        if(key > 0) {
			print_serarch();
			if(key == 8) {	 //backspace
				if(searchstr.length()) searchstr.erase(searchstr.length() - 1);
			}
			else searchstr += key;
			iprintf("%s\n", searchstr.c_str());
		}
    }
	keyboardHide();
	consoleClear();
	settings::layout = old_layout;
	draw_page();
}