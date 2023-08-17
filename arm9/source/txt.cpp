#include "txt.h"
#include "utf8.h"

void txt_book :: parse()
{
	file_stream.open (bookFile.c_str(), std::fstream::binary);
	if(!file_stream.good()) bsod("Can't open book");
	const u8 Bom[] = "\xEF\xBB\xBF";
	char beginning[3];
	file_stream.read(beginning, 3);
	bool hasBom =  !memcmp(&Bom, &beginning, 3);
	par_index.push_back(hasBom ? 3 : 0);
	vector<u32> simple_nl(1, hasBom ? 3 : 0);
	
	while(file_stream.good()) {
		file_stream.ignore(1<<30, '\n');
		simple_nl.push_back(file_stream.tellg());
		if(isspace(file_stream.peek()))	par_index.push_back(file_stream.tellg());
	}
	if((par_index.size() && (simple_nl.size() / par_index.size() > 10)) || par_index.size() == 0) {
		newlines = simple;
		std::swap(par_index, simple_nl);
	}
	else newlines = verbose;
}

void txt_book :: parag_str (int parag_num)
{
	parag = paragrath(); //clear
	file_stream.clear();
	file_stream.seekg(par_index[parag_num]);
	for (char ch = file_stream.get(); !file_stream.eof(); file_stream.get(ch)) {
		if('\n' == ch) {
			if(newlines == simple) break;
			else if (isspace(file_stream.peek())) break;
		}
		else parag.str += ch;
	}
	encoding = utf8::is_valid(parag.str.begin(), parag.str.end()) ? eUtf8 : e1251;
	//parsePar();
	//prev_par_num = parag_num;
}