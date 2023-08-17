#include "button.h"
#include "renderer.h"
#include "screens.h"
#include "settings.h"
#include "controls.h"
#include "algorithm"

button :: button(const string& str, int X1, int Y1, int X2, int Y2, u32 fs)
	:txt(str), solid(), x1(X1), y1(Y1), strx(1), stry(),  fsize(fs)
{
	if(-1 == Y1) {
		x1 = 0;
		y1 = X1;
		x2 = screens::layoutX();
		y2 = y1 + fsize;
	}
	else if(-1 == X2) {
		x2 = x1 + renderer::strWidth(eUtf8, str,0,0,fsize) + 5;
		y2 = y1 + fsize;
	}
	else {
		x2 = X2;
		y2 = Y2;
		setText(str);
	}
}

void button :: setText(const string& str)
{
	u16 width = renderer::strWidth(eUtf8, str,0,0,fsize);
	txt = str;
	strx = (x2 - x1 - width) / 2 - 2; 		
	stry = (y2 - y1 - fsize) / 2;
}

bool button :: touched()
{
	touchPosition t;
	touchRead(&t);
	u8 X1 = x1, Y1 = y1, X2 = x2, Y2 = y2;
	toLayoutSpace(X1, Y1);
	toLayoutSpace(X2, Y2);
	if(	!(t.px >= MIN(X1, X2) && t.py >= MIN(Y1, Y2) &&
		t.px <  MAX(X1, X2) && t.py < MAX(Y1, Y2)) ) return false;
	u16 dif;
	if(settings::layout == d0 || settings::layout == d180) dif = abs(X1 - t.px);
	else dif = abs(Y1 - t.py);
	val =  ((2*dif) / (x2 - x1)) ? rRight : rLeft;
	return true;
}

static const int third = screens::dimY / 3, fifth = screens::dimY / 5;

static const string emptystr;

grid :: grid(u32 it) : iter(it)
{
	using namespace renderer;
	using namespace screens;
	const int height = third - 14, sp = (dimX - dimY) / 2;
	int x1 = 5, y1 = (dimY - height) / 2, x2 = sp - 5, y2 = (dimY + height) / 2;
	if(settings::layout == d0 || settings::layout == d180) {
		less = button("", x1, y1, x2, y2);
		more = button("", dimX - x2, y1, dimX - x1, y2);
	}
	else {
		less = button("", y1, x1, y2, x2);
		more = button("", y1, dimX - x2, y2, dimX - x1);
	}

	
	int offx = (layoutX() - dimY)/2 + (screens::dimX % third)/2;
	int offy = (layoutY() - dimY)/2 + (screens::dimX % third)/2;
	for(u32 i = 0; i < 9; i++)
		blanks.push_back(button("", offx + third*(i%3), offy + i/3*third, offx + third*(i%3+1), offy + (i/3+1)*third, 12));
}

grid* grid :: push(const string* str, int skip, bool plusmin)
{
	for(int i = 0; i < skip; i++) {
		cells.push_back(button());
		strPtrs.push_back(0);
		plusMinus.push_back(false);
	}
	cells.push_back(blanks[cells.size() % 9]);
	cells.back().setText(*str);
	strPtrs.push_back(str);
	plusMinus.push_back(plusmin);
	return this;
}

void grid :: print(const string* targ, string mess)
{
	vector<const string*>::iterator found = std::find(strPtrs.begin(), strPtrs.end(), targ);
	if(found == strPtrs.end()) return;
	int i = found - strPtrs.begin();
	renderer::printStr(eUtf8, bottom_scr, cells[i].x1 + 1, cells[i].y2 - 1, mess, 0,0,12);
	//renderer::printStr(eUtf8, top_scr,5,17,"IkuReader 0.064",0,0,12);
}

const string* grid :: update()
{
	if(more.touched() && iter + 9 < cells.size()) {
		iter += 9;
		renderer::clearScreens(settings::bgCol, bottom_scr);
		draw();
	}
	if(less.touched() && iter > 0) {
		iter -= (iter >= 9) ? 9 : iter;
		renderer::clearScreens(settings::bgCol, bottom_scr);
		draw();
	}
	
	for(u32 i = iter; i < cells.size() && i < iter + 9; i++)
		if(cells[i].touched()) {
			val = cells[i].val;
			return strPtrs[i];
		}
	return 0;
}

void grid :: draw()
{ 
	using namespace renderer;
	using namespace screens;
	for(u32 i = iter; i < cells.size() && i < iter + 9; i++) {
		if(!cells[i].txt.empty()) {
			if(plusMinus[i]) {
				int len = renderer::strWidth(eUtf8,"+",0,0,12); 
				printStr(eUtf8, bottom_scr,cells[i].x1+3,cells[i].y1+12,"\xE2\x80\x93",0,0,12); 
				printStr(eUtf8, bottom_scr,cells[i].x2-len-3,cells[i].y1+12,"+",0,0,12); 
				vLine(cells[i].x1 + (cells[i].x2 - cells[i].x1)/2, cells[i].y1, cells[i].y2, Blend(48));
			}
			cells[i].draw();
		}
	}
	if(iter) less.draw();
	if(iter + 9 < cells.size()) more.draw();
}

void button :: draw()
{ 
	using namespace renderer;
	if(solid) fillRect(x1,y1,x2,y2, settings::bgCol);
	rect(x1,y1,x2,y2); 
	printStr(eUtf8, bottom_scr,x1+2+strx,y1+fsize-1+stry,txt,0,0,fsize); 
}

scrollbar :: scrollbar()
{
	using namespace screens;
	x1 = layoutX() - 20;
	y1 = 0;
	x2 = layoutX();
	y2 = layoutY();
}

void scrollbar :: draw(float pos, float size)
{
	renderer::rect(x1,y1,x2,y2); 
	u8 low_border = y1 + pos * (1 - size) * (y2 - y1);
	u8 length = size * (y2 - y1);
	u16 high_border = low_border + length + 1;
	renderer::fillRect(x1,low_border, x2, high_border, Blend(128));
}

progressbar :: progressbar()
{
	using namespace screens;
	x1 = 5;
	y1 = layoutY()*3/4 - 10;
	x2 = layoutX() - 5;
	y2 = layoutY()*3/4 + 10;
}

void progressbar :: draw(float pr)
{
	renderer::rect(x1,y1,x2,y2); 
	renderer::fillRect(x1, y1, x1 + pr * (x2 - x1), y2, Blend(128));
}

void progressbar :: mark(float pr)
{ renderer::vLine(x1 + pr * (x2 - x1), y1, y2); }

float progressbar :: touched()
{
	touchPosition t;
	touchRead(&t);
	u8 X1 = x1, Y1 = y1, X2 = x2, Y2 = y2;
	toLayoutSpace(X1, Y1);
	toLayoutSpace(X2, Y2);
	if(	!(t.px >= MIN(X1, X2) && t.py >= MIN(Y1, Y2) &&
		  t.px <  MAX(X1, X2) && t.py <  MAX(Y1, Y2)) ) return 2.0f;
	u16 dif;
	if(settings::layout == d0 || settings::layout == d180) dif = abs(X1 - t.px);
	else dif = abs(Y1 - t.py);
	return dif / float(x2 - x1);
}