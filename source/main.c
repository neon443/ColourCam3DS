#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>

#define WIDTH 400
#define HEIGHT 240
#define SCREENSIZE WIDTH * HEIGHT * 2
#define BUFSIZE SCREENSIZE * 2

inline void clearScreen(void) {
	u8 *frame = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
	memset(frame, 0, 320 * 240 * 3);
}

void cleanup() {

}

void writePicToFBuff(void *fb, void *img, u16 x, u16 y) {
	// u8 *fb_8 = (u8*) img;
	// u16 *img_16 = (u16*) img;
}

void flushBuffs(u8 *buf) {
	free(buf);
	buf = malloc(BUFSIZE);
}

int main(int argc, char* argv[])
{
	u8 *buf = malloc(BUFSIZE);

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
		if (kDown & KEY_START) {
			flushBuffs(buf);
			gfxFlushBuffers();
			gspWaitForVBlank();
			gfxSwapBuffers();
			// clearScreen();
			// break; // break in order to return to hbmenu
		}
	}

	gfxExit();
	return 0;
}
