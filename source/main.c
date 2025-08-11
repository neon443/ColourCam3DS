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

void getColor(void *fb, int row, int col) {
	u8 *fb_8 = (u8*) fb;
	u32 v = ((HEIGHT - col) + (row*HEIGHT)) * 3;
	uint8_t r = fb_8[v];
	uint8_t g = fb_8[v+1];
	uint8_t b = fb_8[v+2];
	printf("\n---RGB2---%u %u %u\n", r, g, b);
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
			printf("start presed\n");
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
