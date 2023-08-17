#include "default.h"
#include "button.h"

enum entity {file, folder};
typedef std::pair<entity, string> entry;
typedef std::pair<entity, button> fbutton;

struct file_browser
{
	string run();
private:
	void cd(), upd();
	u16 draw();
	
	vector<entry> flist;
	vector<fbutton> buttons;
	string path;
	scrollbar sbar;
	int pos;
	u16 num;
};