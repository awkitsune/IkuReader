#include "hyphen_ru.h"
#include "utf8.h"
#include "encoding_tables.h"
#include <vector>
#include <fstream>
#include <set>
using namespace std;

static u16 vos[]={L'у',L'е',L'ы',L'а',L'о',L'э',L'я',L'и',L'ю',L'ё'}, spes[]={L'ь',L'ъ',L'й'};
static set<u16> vow(vos, vos + sizeof(vos)/sizeof(u16)), spec(spes, spes + sizeof(spes)/sizeof(u16));

bool isLetter(u32 ch)
{
	return ((ch >= L'а') && ch <= (L'я')) || ch == L'ё';
}

u16 toLower(u16 ch)
{
	if((ch >= L'А') && ch <= (L'Я')) ch += 32;
	else if (ch == L'Я') ch = L'я';
	return ch;
}

enum charT {vo = '0', co, spe, not_letter};

struct rule {u8 pos; string pattern;};
static rule rules[] = {{0, "2"}, {0,"00"}, {1,"0110"}, {1,"1010"}, {1,"01110"}, {2,"011110"}};

vector<bool> hyphen_pos(const string& word_str, u32 strstart, u32 strsend, Encoding enc)
{
	int i = 0, start = -1, end = -1;
	vector<bool> out;
	out.reserve(100);
	string mask;
	mask.reserve(100);
	for(const char* str_it = &word_str[strstart]; str_it < &word_str[strsend]; i++) {
		u32 cp;
		if (eUtf8 == enc) cp = utf8::unchecked::next(str_it);
		else cp = cp1251toUtf32[(u8)*str_it++];
		out.push_back(false);
		if(cp == L'-' || cp ==  L'­')  out.back() = true;
		cp = toLower(cp);
		if (isLetter(cp)) {
			if(start == -1) start = i;
			else end = i;
			if (spec.find(cp) != spec.end()) mask += spe;
			else if (vow.find(cp) != vow.end()) mask += vo;
			else mask += co;
		} 
		else mask += not_letter;
	}
	if (end - start < 3) return out;
	for(int k = start; k < end && k < 30; k++) {
		for(int i = 0; (u32)i < sizeof(rules)/sizeof(rule); i++) {
			u32 j = 0;
			for(; j < rules[i].pattern.length(); j++)
				if(rules[i].pattern[j] != mask[k + j]) break;
			if(j == rules[i].pattern.length()) {
				if(i == 0) {	
					if(k < start + 2) continue; // "съ-есть"
					if(mask[k + j] == not_letter || mask[k + j + 1] == not_letter) continue; //"крутить--вертеть"
				}
				else if(i == 1) {	
					if(mask[k + j] == not_letter) continue; //каки-е-то
				}
				out[k + rules[i].pos] = true;
			}
		}
	}
	int pos[] = {start, end, end - 1};
	for(u32 i = 0; i < sizeof(pos)/sizeof(pos[0]); i++) out[pos[i]] = false;
	return out;
}