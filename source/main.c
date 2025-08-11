#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>

#define width 400
#define height 240
#define screenSize width * height * 2
#define bufSize screenSize * 2

inline void clearScreen(void) {
	u8 *frame = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL)
	memset(frame, 0, 320 * 240 * 3)
}

void cleanup() {

}

void writePicToFBuff(void *fb, void *img, u16 x, u16 y, u16 width, u16 height) {
	u8 *fb_8 = (u8*) img
	u16 *img_16 = (u16*) img
}

int main(int argc, char* argv[])
{
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);

	printf("Hello, world!\n");

	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		// Your code goes here
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu
	}

	gfxExit();
	return 0;
}
