#include <string>
#include <vector>
#include "default.h"

//std::vector<bool> hyphen_pos(std::string& word, Encoding enc);
vector<bool> hyphen_pos(const string& word_str, u32 strstart, u32 strsend, Encoding enc);