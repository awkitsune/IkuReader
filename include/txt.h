#include "book.h"
#include <fstream>

class txt_book : public Book
{
public:
	txt_book(const string& filename) : Book(filename){};
private:
	void parse();
	void parag_str (int parag_num);
	vector<u32> par_index;
	u32 total_paragraths() {return par_index.size();}
	std::ifstream file_stream;
	enum {simple, verbose} newlines;
};
