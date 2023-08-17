#pragma once
#include "settings.h"

enum rDirec{rUp, rDown, rLeft, rRight};

extern const int rKeys[4][4];

inline int rKey(rDirec d)
{ return rKeys[settings::layout][d]; }