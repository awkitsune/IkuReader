#pragma once
#include "default.h"

struct button
{
	string txt;
	int val;
	bool touched();
	void draw();
	button(const string&, int X1, int Y1=-1, int X2=-1, int Y2=-1, u32 = buttonFontSize);
	u16 height(){return y2-y1;}
	u16 width(){return x2-x1;}
	string getText() {return txt;}
	void setText(const string&);
	button() : solid() {}
	bool solid;
private:
	friend struct grid;
	u16 x1, y1, x2, y2, strx, stry;
	u32 fsize;
};

struct grid
{
	const string* update();
	void draw();
	grid* push(const string*, int skip = 0, bool plusmin = false);
	grid(u32 it = 0);
	void print(const string* targ, string mess);
	int val;
	u32 iter;
private:
	grid();
	vector<button> cells;
	vector<button> blanks;
	vector<bool> plusMinus;
	button more, less;
	vector<const string*> strPtrs;
};

struct scrollbar
{
	scrollbar();
	void draw(float pos, float size);
private:
	u16 x1, y1, x2, y2;
};

struct progressbar
{
	progressbar();
	void draw(float pr);
	void mark(float pr);
	float touched();
private:
	u16 x1, y1, x2, y2;
};