/*---------------------------------------------------------------------------------

	default ARM7 core

		Copyright (C) 2005 - 2010
		Michael Noland (joat)
		Jason Rogers (dovoto)
		Dave Murphy (WinterMute)

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you
		must not claim that you wrote the original software. If you use
		this software in a product, an acknowledgment in the product
		documentation would be appreciated but is not required.

	2.	Altered source versions must be plainly marked as such, and
		must not be misrepresented as being the original software.

	3.	This notice may not be removed or altered from any source
		distribution.

---------------------------------------------------------------------------------*/
#include <nds.h>
//#include <dswifi7.h>
//#include <maxmod7.h>

#define BACKLIGHT_FIFO	FIFO_USER_08
#define PM_DSLITE_REG	(4)
#define PM_IS_LITE		BIT(6)
#define PM_BACKLIGHTS	(PM_BACKLIGHT_BOTTOM | PM_BACKLIGHT_TOP)

//---------------------------------------------------------------------------------
void VblankHandler(void) {
//---------------------------------------------------------------------------------
	//Wifi_Update();
}


//---------------------------------------------------------------------------------
void VcountHandler() {
//---------------------------------------------------------------------------------
	inputGetAndSend();
}

volatile bool exitflag = false;

//---------------------------------------------------------------------------------
void powerButtonCB() {
//---------------------------------------------------------------------------------
	exitflag = true;
}

// FifoValue32HandlerFunc type function
void brightness_fifo(u32 msg, void* data) { //incoming fifo message
	msg%=4;
	if((bool)(readPowerManagement(PM_DSLITE_REG) & PM_IS_LITE))
		writePowerManagement(PM_DSLITE_REG, msg);
}

//---------------------------------------------------------------------------------
int main() {
//---------------------------------------------------------------------------------
	readUserSettings();

	irqInit();
	// Start the RTC tracking IRQ
	initClockIRQ();
	fifoInit();
	touchInit();

	//mmInstall(FIFO_MAXMOD);

	SetYtrigger(80);

	//installWifiFIFO();
	//installSoundFIFO();

	installSystemFIFO();

	fifoSetValue32Handler(BACKLIGHT_FIFO,&brightness_fifo,0);
	
	irqSet(IRQ_VCOUNT, VcountHandler);
	irqSet(IRQ_VBLANK, VblankHandler);

	irqEnable( IRQ_VBLANK | IRQ_VCOUNT | IRQ_NETWORK);
	
	setPowerButtonCB(powerButtonCB);   

	// Keep the ARM7 mostly idle
	while (!exitflag) {
		if ( 0 == (REG_KEYINPUT & (KEY_SELECT | KEY_START | KEY_L))) { //removed KEY_R because my dsi has inconsistent pressing one
			exitflag = true;
		}
		swiWaitForVBlank();
	}
	return 0;
}
