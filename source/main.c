#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>

#define WIDTH 400
#define HEIGHT 240
#define SCREENSIZE WIDTH * HEIGHT * 2
#define BUFSIZE SCREENSIZE * 2
#define WAIT_TIMEOUT 100000000ULL

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

			if((((j>110 && j<130) && (i>198 && i<202)) || ((i>190 && i<210) && (j>118 && j<122))) && !(j == 120 && i == 200)) {
				fb_8[v] = 200;
				fb_8[v+1] = 200;
				fb_8[v+2] = 200;
				continue;
			}

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

	CAMU_StopCapture(PORT_BOTH);

	svcCloseHandle(camReceiveEvent);
	svcCloseHandle(camReceiveEvent2);

	CAMU_Activate(SELECT_NONE);
}

void getColor(void *fb, int row, int col) {
	CAMU_PlayShutterSound(SHUTTER_SOUND_TYPE_NORMAL);
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
	
	CAMU_SetFrameRate(SELECT_OUT1_OUT2, FRAME_RATE_30);

	CAMU_SetNoiseFilter(SELECT_OUT1_OUT2, true);
	CAMU_SetAutoExposure(SELECT_OUT1_OUT2, true);
	CAMU_SetAutoWhiteBalance(SELECT_OUT1_OUT2, true);
	//printf("CAMU_SetEffect: 0x%08X\n", (unsigned int) CAMU_SetEffect(SELECT_OUT1_OUT2, EFFECT_MONO, CONTEXT_A));

	CAMU_SetTrimming(PORT_CAM1, false);
	CAMU_SetTrimming(PORT_CAM2, false);
	//printf("CAMU_SetTrimmingParamsCenter: 0x%08X\n", (unsigned int) CAMU_SetTrimmingParamsCenter(PORT_CAM1, 512, 240, 512, 384));

	u32 bufSize;
	CAMU_GetMaxBytes(&bufSize, WIDTH, HEIGHT);
	CAMU_SetTransferBytes(PORT_BOTH, bufSize, WIDTH, HEIGHT);

	CAMU_Activate(SELECT_OUT1_OUT2);

	Handle camReceiveEvent[4] = {0};
	bool captureInterrupted = false;
	s32 index = 0;

	// events 0 and 1 for interruption
	CAMU_GetBufferErrorInterruptEvent(&camReceiveEvent[0], PORT_CAM1);
	CAMU_GetBufferErrorInterruptEvent(&camReceiveEvent[1], PORT_CAM2);

	CAMU_ClearBuffer(PORT_BOTH);
	CAMU_SynchronizeVsyncTiming(SELECT_OUT1, SELECT_OUT2);

	CAMU_StartCapture(PORT_BOTH);

	printf("Welcome to Colour Cam!\n");

	// Main loop
	while (aptMainLoop())
	{
		if (!captureInterrupted) {
			// Read which buttons are currently pressed or not
			hidScanInput();
			hidKeysDown();
		}

		// events 2 and 3 for capture
		if (camReceiveEvent[2] == 0) {
			CAMU_SetReceiving(&camReceiveEvent[2], buf, PORT_CAM1, SCREENSIZE, (s16)bufSize);
		}
		if (camReceiveEvent[3] == 0) {
			CAMU_SetReceiving(&camReceiveEvent[3], buf + SCREENSIZE, PORT_CAM2, SCREENSIZE, (s16)bufSize);
		}

		if (captureInterrupted) {
			CAMU_StartCapture(PORT_BOTH);
			captureInterrupted = false;
		}

		svcWaitSynchronizationN(&index, camReceiveEvent, 4, false, WAIT_TIMEOUT);
		switch (index) {
		case 0:
			svcCloseHandle(camReceiveEvent[2]);
			camReceiveEvent[2] = 0;

			captureInterrupted = true;
			continue; //skip screen update
			break;
		case 1:
			svcCloseHandle(camReceiveEvent[3]);
			camReceiveEvent[3] = 0;

			captureInterrupted = true;
			continue; //skip screen update
			break;
		case 2:
			svcCloseHandle(camReceiveEvent[2]);
			camReceiveEvent[2] = 0;
			break;
		case 3:
			svcCloseHandle(camReceiveEvent[3]);
			camReceiveEvent[3] = 0;
			break;
		default:
			break;
		}

		writePicToFBuff(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), buf, 0, 0);

		// Your code goes here
		u32 kDown = hidKeysDown();
		if (kDown & KEY_R) {
			buf = flushBuffs(buf);
			gfxFlushBuffers();

			gspWaitForVBlank();
			gfxSwapBuffers();


			getColor(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), 200, 120);
			// break; // break in order to return to hbmenu
		}

		gfxSet3D(false);
		// writePicToFBuff(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), buf, 0, 0);

		// takePicture(buf);

		gfxFlushBuffers();
		gspWaitForVBlank();
		gfxSwapBuffers();
	}

	CAMU_StopCapture(PORT_BOTH);


	free(buf);

	gfxExit();
	return 0;
}
