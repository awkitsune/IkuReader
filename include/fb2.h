#include "book.h"
#include "pugixml.h"

class fb2_book : public Book
{
public:
	fb2_book(const string& filename) : Book(filename) {};
private:
	void parse();
	void parag_str (int parag_num);
	vector<pugi::xml_node> par_index;
	int parse_doc(const pugi::xml_node& node);
	int extract_par(const pugi::xml_node& node);
	pugi::xml_document doc;
	u32 total_paragraths() {return par_index.size();}
	bool push_it;
};