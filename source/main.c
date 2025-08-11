#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>

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
		for(i = 0; i < WIDTH; i++) {
			drawy = y + HEIGHT - j;
			drawx = x + i;
			u32 v = (drawy + drawx * HEIGHT) * 3;
			u16 data = img_16[j * WIDTH + i];
			uint8_t b = ((data >> 11) & 0x1F) << 3;
			uint8_t g = ((data >> 5) & 0x3F) << 2;
			uint8_t r = (data & 0x1F) << 3;
			fb_8[v] = r;
			fb_8[v+1] = g;
			fb_8[v+2] = b;
		}
	}
}

u8* flushBuffs(u8 *buf) {
	free(buf);
	buf = malloc(BUFSIZE);
	return buf;
}

void takePicture(u8 *buf) {
	u32 bufSize;
	printf("CAMU_GetMaxBytes: 0x%08X\n", (unsigned int) CAMU_GetMaxBytes(&bufSize, WIDTH, HEIGHT));
	printf("CAMU_SetTransferBytes: 0x%08X\n", (unsigned int) CAMU_SetTransferBytes(PORT_BOTH, bufSize, WIDTH, HEIGHT));

	printf("CAMU_Activate: 0x%08X\n", (unsigned int) CAMU_Activate(SELECT_OUT1_OUT2));

	Handle camReceiveEvent = 0;
	Handle camReceiveEvent2 = 0;

	printf("CAMU_ClearBuffer: 0x%08X\n", (unsigned int) CAMU_ClearBuffer(PORT_BOTH));
	printf("CAMU_SynchronizeVsyncTiming: 0x%08X\n", (unsigned int) CAMU_SynchronizeVsyncTiming(SELECT_OUT1, SELECT_OUT2));

	printf("CAMU_StartCapture: 0x%08X\n", (unsigned int) CAMU_StartCapture(PORT_BOTH));

	printf("CAMU_SetReceiving: 0x%08X\n", (unsigned int) CAMU_SetReceiving(&camReceiveEvent, buf, PORT_CAM1, SCREENSIZE, (s16) bufSize));
	printf("CAMU_SetReceiving: 0x%08X\n", (unsigned int) CAMU_SetReceiving(&camReceiveEvent2, buf + SCREENSIZE, PORT_CAM2, SCREENSIZE, (s16) bufSize));
	printf("svcWaitSynchronization: 0x%08X\n", (unsigned int) svcWaitSynchronization(camReceiveEvent, WAIT_TIMEOUT));
	printf("svcWaitSynchronization: 0x%08X\n", (unsigned int) svcWaitSynchronization(camReceiveEvent2, WAIT_TIMEOUT));
	printf("CAMU_PlayShutterSound: 0x%08X\n", (unsigned int) CAMU_PlayShutterSound(SHUTTER_SOUND_TYPE_NORMAL));

	printf("CAMU_StopCapture: 0x%08X\n", (unsigned int) CAMU_StopCapture(PORT_BOTH));

	svcCloseHandle(camReceiveEvent);
	svcCloseHandle(camReceiveEvent2);

	printf("CAMU_Activate: 0x%08X\n", (unsigned int) CAMU_Activate(SELECT_NONE));
}

void getColor(void *fb, int row, int col) {
	u8 *fb_8 = (u8*) fb;
	u32 v = ((HEIGHT - col) + (row*HEIGHT)) * 3;
	uint8_t r = fb_8[v];
	uint8_t g = fb_8[v+1];
	uint8_t b = fb_8[v+2];
	printf("\n---RGB--- %u %u %u\n", r, g, b);
}

int main(int argc, char* argv[])
{
	u8 *buf = malloc(BUFSIZE);

	gfxInitDefault();
	consoleInit(GFX_BOTTOM, NULL);

	camInit();

	CAMU_SetSize(SELECT_OUT1_OUT2, SIZE_CTR_TOP_LCD, CONTEXT_A);
	CAMU_SetOutputFormat(SELECT_OUT1_OUT2, OUTPUT_RGB_565, CONTEXT_A);

	CAMU_SetNoiseFilter(SELECT_OUT1_OUT2, true);
	CAMU_SetAutoExposure(SELECT_OUT1_OUT2, true);
	CAMU_SetAutoWhiteBalance(SELECT_OUT1_OUT2, true);
	//printf("CAMU_SetEffect: 0x%08X\n", (unsigned int) CAMU_SetEffect(SELECT_OUT1_OUT2, EFFECT_MONO, CONTEXT_A));

	CAMU_SetTrimming(PORT_CAM1, false);
	CAMU_SetTrimming(PORT_CAM2, false);
	//printf("CAMU_SetTrimmingParamsCenter: 0x%08X\n", (unsigned int) CAMU_SetTrimmingParamsCenter(PORT_CAM1, 512, 240, 512, 384));

	printf("Hello, world!\n");

	// Main loop
	while (aptMainLoop())
	{
		hidScanInput();

		// Your code goes here
		u32 kDown = hidKeysDown();
		if (kDown & KEY_R) {
			printf("start presed\n");
			buf = flushBuffs(buf);
			gfxFlushBuffers();

			gspWaitForVBlank();
			gfxSwapBuffers();

			takePicture(buf);
			// break; // break in order to return to hbmenu
		}

		gfxSet3D(false);
		writePicToFBuff(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), buf, 0, 0);
		getColor(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), 100, 120);

		gfxFlushBuffers();
		gspWaitForVBlank();
		gfxSwapBuffers();
	}

	free(buf);

	gfxExit();
	return 0;
}
