#include "book.h"
#include "pugixml.h"

class epub_book : public Book
{
public:
	epub_book(const string& filename) : Book(filename) {};
	//~epub_book();
private:
	void parse();
	void parag_str (int parag_num);
	vector<pugi::xml_node> par_index;
	//vector<pugi::xml_document*> chapters;
	int parse_doc(const pugi::xml_node& node);
	int extract_par(const pugi::xml_node& node);
	u32 total_paragraths() {return par_index.size();}
	bool push_it;
	pugi::xml_document document;
};