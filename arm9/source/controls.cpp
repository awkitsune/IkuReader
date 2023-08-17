#include "controls.h"

const int rKeys[4][4] =
{
	{	//d0
		KEY_UP	 | KEY_X,	//up
		KEY_DOWN | KEY_B,	//down
		KEY_LEFT | KEY_Y | KEY_L, //left
		KEY_RIGHT| KEY_A | KEY_R, //right
	},
	{	//d90
		KEY_RIGHT| KEY_A,
		KEY_LEFT | KEY_Y,
		KEY_UP	 | KEY_X | KEY_L,
		KEY_DOWN | KEY_B | KEY_R,
	},
	{	//d180
		KEY_DOWN | KEY_B,
		KEY_UP	 | KEY_X,
		KEY_RIGHT| KEY_A | KEY_R,
		KEY_LEFT | KEY_Y | KEY_L,
	},
	{	//d270
		KEY_LEFT | KEY_Y,
		KEY_RIGHT| KEY_A,
		KEY_DOWN | KEY_B | KEY_R,
		KEY_UP	 | KEY_X | KEY_L,
	}
};