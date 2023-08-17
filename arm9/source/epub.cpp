#include "epub.h"
#include <algorithm>
#include "renderer.h"
#include "screens.h"
#include "zlib.h"
#include "unzip.h"
#include <stdio.h>
#include <fstream>
#include <map>

u32 loadFromZip(unzFile& zip, const char* file, char *&buf)
{
	unz_file_info info;
	if(unzLocateFile(zip, file, 0) == UNZ_OK) {
		if (unzOpenCurrentFile(zip) == UNZ_OK) {
			unzGetCurrentFileInfo(zip, &info, NULL, 0, NULL, 0, NULL, 0);
			buf = new (std::nothrow) char[info.uncompressed_size];
			if(buf == NULL) bsod("Out of memory");
			unzReadCurrentFile(zip, buf, info.uncompressed_size);
			unzCloseCurrentFile(zip);
		} else bsod("Can't open epub. (3)");
	} else bsod("Can't open epub. (2)");
	return info.uncompressed_size;
}

void epub_book :: parse()
{
	vector<u32> offsets;
	{
		encoding = eUtf8;
		pugi::xml_document doc;
		const char err[] = "Can't load epub. (1)";
		char *buf = NULL;
		unzFile hArchiveFile = unzOpen(bookFile.c_str());
		if (hArchiveFile == NULL) bsod("Can't open epub.");
			
		loadFromZip(hArchiveFile, "META-INF/container.xml", buf);
		pugi::xml_parse_result result = doc.load(buf);
		delete[] buf;
		
		if(result.status != pugi::status_ok) bsod(err);
		string cont_path = doc.child("container").child("rootfiles").child("rootfile").attribute("full-path").value();

		string chapter_path;
		u32 found = cont_path.find_last_of('/');
		if(found != string::npos) chapter_path = cont_path.substr(0, found+1);

		doc.reset();
		loadFromZip(hArchiveFile, cont_path.c_str(), buf);
		result = doc.load(buf);
		delete[] buf;
		if(result.status != pugi::status_ok) bsod(err);
		
		pugi::xml_node item = doc.child("package").child("manifest").first_child();
		std::map<string, string> chapters_unordered;
		for(;item; item = item.next_sibling())
			if(!strcmp("application/xhtml+xml", item.attribute("media-type").value()))
				chapters_unordered[item.attribute("id").value()] = item.attribute("href").value();
		
		vector<string> chapter_files;
		item = doc.child("package").child("spine").first_child();
		for(;item; item = item.next_sibling())
			chapter_files.push_back(chapters_unordered[item.attribute("idref").value()]);
		
		doc.reset();
		
		offsets.push_back(0);
		renderer::clearScreens(0);
		for(u32 i = 0; i < chapter_files.size(); i++) {
			consoleClear();
			iprintf("unpacking %d/%d\n", i+1, chapter_files.size());
			u32 size = loadFromZip(hArchiveFile, (chapter_path + chapter_files[i]).c_str(), buf);
			if(offsets.back() + size > giant_buf_size - 1u) bsod("book buffer exhausted");
			memcpy(giant_buffer + offsets[i], buf, size);
			offsets.push_back(offsets.back() + size);
			delete[] buf;
		}
		unzClose(hArchiveFile);
	}	
	consoleClear();
	iprintf("parsing...\n");
	pugi::xml_parse_result result = document.load_buffer_inplace(giant_buffer, offsets.back());
	if(result.status != pugi::status_ok) {
		if(result.status == pugi::status_out_of_memory) bsod("Out of memory.");
		//else bsod("Parser error (bad format).");
	}
	parse_doc(document);
	consoleClear();
}

static const string nl_tags(" br div dt h1 h2 h3 h4 h5 h6 hr li p pre ol td ul body ");
static const string br_tags = " br ";

void epub_book :: parag_str (int parag_num)
{
	parag = paragrath(); //clear
	if (par_index [parag_num]) extract_par (par_index [parag_num]);
	//parsePar();
	//prev_par_num = parag_num;
}

int epub_book :: parse_doc(const pugi::xml_node& node)
{
	bool newl, ret, br;
	{
		string tag = node.name();
		newl = !tag.empty() && string::npos != nl_tags.find(' '+tag+' ');
		br = !tag.empty() && string::npos != br_tags.find(' '+tag+' ');
		ret = !tag.empty() && string::npos != string(" title script style binary image ").find(' '+tag+' ');
	}
	if (newl) {
		if(br) par_index.push_back(node.next_sibling());
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

int epub_book :: extract_par(const pugi::xml_node& node)
{
	bool newl, nospace, ret, isTitle, bold, italic;
	{
		string tag = node.name();
		string parentTag = node.parent().name();
		newl = !tag.empty() && string::npos != nl_tags.find(' '+tag+' ');
		isTitle = !parentTag.empty() && string::npos != string(" title h1 h2 h3 h4 h5 h6 ").find(' '+parentTag+' ');
		isTitle |= !tag.empty() && string::npos != string(" title h1 h2 h3 h4 h5 h6 ").find(' '+tag+' ');
		
		bold = (parentTag == "b") || (parentTag == "strong");
		italic = ("i" == parentTag)  || ("em" == parentTag);
		nospace = (bold || italic) || (!parentTag.empty() && string::npos != string(" b tt big small ").find(' '+parentTag+' '));
		ret = !tag.empty() && string::npos != string(" script style binary image ").find(' '+tag+' ');
	}
	if (ret) return 0;
	if(isTitle) parag.type = ptitle;
	
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