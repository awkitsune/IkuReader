#include "fb2.h"
#include <fstream>
#include <algorithm>
#include "renderer.h"

const char giant_buf_err[] = "Exhausted book buffer.";

u32 bufferFb2(const char* file)
{
	std::ifstream is(file, std::ios::binary);
	int i = 0, j = 0;
	const char start[] = "<binary";
	for(is.get(giant_buffer[i]) ; is.good(); is.get(giant_buffer[i])) {
		if(giant_buffer[i] == start[j]) {
			j++;
			if (j == 7) {
				is.ignore(1<<30, '>');
				is.ignore(1<<30, '>');
				j = 0; i -= 6;
				continue;
			}
		}
		else j = 0;
		++i;
		if(i > giant_buf_size) bsod(giant_buf_err);
	}	
	return i;
}

u32 fb2_loadToGiantBuffer(const char* filename)
{
	FILE* File = fopen(filename, "rb"); 
	if(File == NULL) bsod("Can't load file.");
	fseek(File, 0 , SEEK_END);
	int size = ftell (File);
	rewind(File);
	if(size > giant_buf_size) {
		fclose(File);
		return bufferFb2(filename);
	}
	int read = fread(giant_buffer, 1, size, File);
	if(read < size) bsod("Error reading file.");
	fclose(File);
	return size;
}

void fb2_book :: parse()
{
	u32 size = fb2_loadToGiantBuffer(bookFile.c_str());
	pugi::xml_parse_result result = doc.load_buffer_inplace(giant_buffer, size, pugi::parse_default | pugi::parse_declaration);
	if(result.status != pugi::status_ok) {
		if(result.status == pugi::status_out_of_memory) bsod("Out of memory.");
		else bsod("Parser error (bad format).");
	}
	push_it = true;
	string enc = doc.first_child().attribute("encoding").value();
	transform (enc.begin(), enc.end(), enc.begin(), tolower);
	if("utf-8" == enc) encoding = eUtf8;
	else encoding = e1251;
	parse_doc(doc);
}

void fb2_book :: parag_str (int parag_num)
{
	parag = paragrath(); //clear
	if (par_index [parag_num]) extract_par (par_index [parag_num] );
}

static const string nl_tags(" p v br empty-line section author book-title date stanza poem text-author cite subtitle code ");
static const string br_tags = " br empty-line ";

int fb2_book :: parse_doc(const pugi::xml_node& node)
{
	bool newl, ret, br;
	{
		string tag = node.name();
		newl = !tag.empty() && string::npos != nl_tags.find(' '+tag+' ');
		br = !tag.empty() && string::npos != br_tags.find(' '+tag+' ');
		ret = !tag.empty() && string::npos != string(" document-info publish-info genre lang id binary image ").find(' '+tag+' ');
	}
	if (newl) {
		if(br) {
			par_index.push_back(pugi::xml_node());
			par_index.push_back(node.next_sibling());
		}
		else if (push_it) par_index.push_back (node);
		push_it = false;
	}
	if (ret) return 0;
	for (pugi::xml_node elem = node.first_child(); elem; elem = elem.next_sibling()) {
		int r = parse_doc(elem);
		if (r == -1) return -1;
	}
	if (newl && !br) {
		if (push_it) par_index.push_back (pugi::xml_node()); //null node, means empty string
		push_it = true;
	} 
	return 0;
}

int fb2_book :: extract_par(const pugi::xml_node& node)
{
	bool newl, nospace, ret, isTitle, isStanza, bold, italic;
	{
		string tag = node.name();
		string parentTag = node.parent().name();
		newl = !tag.empty() && string::npos != nl_tags.find(' '+tag+' ');
		isTitle = !parentTag.empty() && string::npos != string(" title author subtitle ").find(' '+parentTag+' ');
		isTitle |= !tag.empty() && string::npos != string(" title book-title date subtitle ").find(' '+tag+' ');
		isStanza = !parentTag.empty() && string::npos != string(" v poem cite epigraph ").find(' '+parentTag+' ');
		isStanza |= !tag.empty() && string::npos != string(" v poem cite epigraph ").find(' '+tag+' ');
		bold = ("strong" == parentTag);
		italic = ("emphasis" == parentTag);
		nospace = (bold || italic) || (!parentTag.empty() && string::npos != string(" a striketrough sub sup ").find(' '+parentTag+' '));
		ret = !tag.empty() && string::npos != string(" document-info publish-info genre lang id binary image ").find(' '+tag+' ');
	}
	if(ret) return 0;
	if(isTitle) parag.type = ptitle;
	if(isStanza) parag.type = pstanza;
	
	if(bold) {
		marked mark = {parag.str.length(), parag.str.length(), fbold};
		parag.marks.push_back(mark);
	}
	else if(italic) {
		marked mark = {parag.str.length(), parag.str.length(), fitalic};
		parag.marks.push_back(mark);
	}

	
	if (node.type() == pugi::node_pcdata) parag.str += node.value();
	
	
	for(pugi::xml_node elem = node.first_child(); elem; elem = elem.next_sibling()) {
		int r = extract_par(elem);
		if (r == -1) return -1;
	}
	
	if((italic || bold) && parag.marks.size()) {
		parag.marks.back().end = parag.str.length();
	}
	
	if (newl) return -1;
	else if (node.type() == pugi::node_pcdata && !nospace && node.parent().next_sibling()) parag.str += ' ';
	return 0;
}