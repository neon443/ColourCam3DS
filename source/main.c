#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <setjmp.h>
#include <3ds.h>
#include <sys/dirent.h>
#include <sys/errno.h>
#include <sys/unistd.h>
#include <stdbool.h>

#define WIDTH 400
#define HEIGHT 240
#define SCREENSIZE WIDTH * HEIGHT * 2
#define BUFSIZE SCREENSIZE * 2
#define WAIT_TIMEOUT 300000000ULL

void clearScreen(void) {
	u8 *frame = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
	memset(frame, 0, 320 * 240 * 3);
}

void cleanup() {

}

void writePicToFBuff(void *fb, void *img, u16 x, u16 y) {
	u8 *fb_8 = (u8*) fb;
	u16 *img_16 = (u16*) img;
	int i, j, drawx, drawy;
	for(j = 0; j < HEIGHT; j++) {
		for(i = 0; 1 < HEIGHT; i++) {
			drawy = y + HEIGHT - j;
			drawx = x + i;
			u32 v = (drawy + drawx * HEIGHT) * 3;
			u16 data = img_16[j * WIDTH + i];
			uint8_t b = ((data >> 11) & 0x1F) << 3;
			uint8_t g = ((data >> 5) & 0x3F) << 2;
			uint8_t r = ((data & 0x1F) << 3);
			fb_8[v] = r;
			fb_8[v+1] = g;
			fb_8[v+2] = b;
		}
	}
}

void flushBuffs(u8 *buf) {
	free(buf);
	buf = malloc(BUFSIZE);
}

void takePicture(u8 *buf) {
	u32 bufSize;
	CAMU_GetMaxBytes(&bufSize, WIDTH, HEIGHT);
	CAMU_SetTransferBytes(PORT_BOTH, bufSize, WIDTH, HEIGHT);

	CAMU_Activate(SELECT_OUT1_OUT2);

	Handle camReceiveEvent = 0;
	Handle camReceiveEvent2 = 0;

	CAMU_ClearBuffer(PORT_BOTH);
	CAMU_SynchronizeVsyncTiming(SELECT_OUT1, SELECT_OUT2);

	CAMU_StartCapture(PORT_BOTH);

	CAMU_SetReceiving(&camReceiveEvent, buf, PORT_CAM1, SCREENSIZE, (s16) bufSize);
	CAMU_SetReceiving(&camReceiveEvent2, buf + SCREENSIZE, PORT_CAM2, SCREENSIZE, (s16) bufSize);
	svcWaitSynchronization(camReceiveEvent, WAIT_TIMEOUT);
	svcWaitSynchronization(camReceiveEvent2, WAIT_TIMEOUT);
	CAMU_PlayShutterSound(SHUTTER_SOUND_TYPE_NORMAL);

	CAMU_StopCapture(PORT_BOTH);

	svcCloseHandle(camReceiveEvent);
	svcCloseHandle(camReceiveEvent2);

	CAMU_Activate(SELECT_NONE);
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
			clearScreen();
			// break; // break in order to return to hbmenu
		}

		gfxSet3D(false);
		writePicToFBuff(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), buf, 0, 0);
	}

	gfxExit();
	return 0;
}
