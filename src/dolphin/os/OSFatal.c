#include <SDKVersion.h>
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
#include "dolphin/base/PPCArch.h"
#else
#include "dolphin/PPCArch.h"
#endif
#include "dolphin/ai.h"
#include "dolphin/os.h"
#include "dolphin/hw_regs.h"
#include "dolphin/os/OSArena.h"
#include "dolphin/os/OSBootInfo.h"
#include "dolphin/vi.h"
#include "string.h"
#pragma fp_contract off
extern void __OSStopAudioSystem();
#if SDK_VERSION <= SDK_VERSION_RVL_2005_11
extern BOOL __OSCallResetFunctions(u32);
#else
extern BOOL __OSCallShutdownFunctions(BOOL final, u32 event);
#endif
extern void OSDefaultExceptionHandler(__OSException exception, OSContext* context);
extern void __OSSetExceptionHandler(__OSException exception, void* handler);

typedef struct OSFatalParam {
  GXColor fg;
  GXColor bg;
  const char* msg;
} OSFatalParam;

#if SDK_VERSION <= SDK_VERSION_RVL_2005_11
static OSFatalParam FatalParam;
static OSContext FatalContext;
#else
static OSContext FatalContext;
static OSFatalParam FatalParam;
#endif
static void Halt();

static void ScreenClear(void* xfb, u16 xfbW, u16 xfbH, GXColor yuv) {
  int i;
  int j;
  u8* ptr;

  ptr = xfb;
  for (i = 0; i < xfbH; i++) {
    for (j = 0; j < xfbW; j += 2) {
      *ptr++ = yuv.r;
      *ptr++ = yuv.g;
      *ptr++ = yuv.r;
      *ptr++ = yuv.b;
    }
  }
}

static void ScreenReport(void* xfb, u16 xfbW, u16 xfbH, GXColor yuv, s32 x, s32 y, s32 leading,
                         const char* string) {
  u8* ptr;
  s32 width;
  u32 i;
  u32 j;
  u32 image[72];
  u32 k;
  u32 l;
  u8 Y;
  u32 pixel;
  s32 col;

loop_1:
  if (xfbH - 24 >= y) {
    ptr = (u8*)xfb + ((x + (y * xfbW)) * 2);
    col = x;

    while ((s8)*string != 0) {
      if ((s8)*string == '\n') {
        string++;
        y += leading;
        goto loop_1;
      }

      if (xfbW - 48 < col) {
        y += leading;
        goto loop_1;
      }

      for (i = 0; i < 24; i++) {
        j = (i & 7) + ((i >> 3) * 24);
        image[j + 0] = 0;
        image[j + 8] = 0;
        image[j + 16] = 0;
      }

      string = OSGetFontTexel((char*)string, image, 0, 6, &width);

      for (i = 0; i < 24; i++) {
        j = (i & 7) + ((i >> 3) * 24);

        for (k = 0; k < 24; k++) {
          l = j + (k & 0xFFFFFFF8);

          Y = (image[l] >> ((7 - (k & 7)) * 4)) & 0xF;
          if (Y != 0) {
            Y = (((yuv.r * (Y * 0xEF)) / 255) / 15) + 0x10;
            pixel = k + (i * xfbW);
            ptr[pixel * 2] = Y;

            if ((col + k) & 1) {
              ptr[(pixel * 2) - 1] = yuv.g;
              ptr[(pixel * 2) + 1] = yuv.b;
            } else {
              ptr[(pixel * 2) - 1] = yuv.b;
              ptr[(pixel * 2) + 1] = yuv.g;
            }
          }
        }
      }

      ptr += width * 2;
      col += width;
    }
  }
}

static void ConfigureVideo(u16 xfbW, u16 xfbH) {
  GXRenderModeObj mode;
  mode.fbWidth = xfbW;
  mode.efbHeight = 480;
  mode.xfbHeight = xfbH;
  mode.viXOrigin = 40;
  mode.viWidth = 640;
  mode.viHeight = xfbH;

  switch (VIGetTvFormat()) {
  case 2:
  case 0:
    if (__VIRegs[54] & 1) {
      mode.viTVmode = 2;
      mode.viYOrigin = 0;
      mode.xFBmode = 0;
    } else {
      mode.viTVmode = 0;
      mode.viYOrigin = 0;
      mode.xFBmode = 1;
    }
    break;
  case 5:
    mode.viTVmode = 20;
    mode.viYOrigin = 0;
    mode.xFBmode = 1;
    break;
  case 1:
    mode.viTVmode = 4;
    mode.viYOrigin = 47;
    mode.xFBmode = 1;
    break;
  }

  VIConfigure(&mode);
  VIConfigurePan(0, 0, 640, 480);
}

static GXColor RGB2YUV(GXColor rgb) {
  f32 Y;
  f32 Cb;
  f32 Cr;
  GXColor yuv;

  Y = 0.5f + (16.0f + ((0.098f * (f32)rgb.b) + ((0.257f * (f32)rgb.r) + (0.504f * (f32)rgb.g))));
  Cb = 0.5f + (128.0f + ((0.439f * (f32)rgb.b) + ((-0.148f * (f32)rgb.r) - (0.291f * (f32)rgb.g))));
  Cr = 0.5f + (128.0f + (((0.439f * (f32)rgb.r) - (0.368f * (f32)rgb.g)) - (0.071f * (f32)rgb.b)));

  yuv.r = (Y > 235.0f) ? 235.0f : (Y < 16.0f) ? 16.0f : Y;
  yuv.g = (Cb > 240.0f) ? 240.0f : (Cb < 16.0f) ? 16.0f : Cb;
  yuv.b = (Cr > 240.0f) ? 240.0f : (Cr < 16.0f) ? 16.0f : Cr;
  yuv.a = 0;

  return yuv;
}

void OSFatal(GXColor fg, GXColor bg, const char* msg) {
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  u32 count = 0;
  OSBootInfo* bootInfo = (OSBootInfo*)OSPhysicalToCached(0);
#else
  OSBootInfo* bootInfo;
  u32 count;
  OSTime t;

  bootInfo = (OSBootInfo*)OSPhysicalToCached(0);
#endif
  OSDisableInterrupts();
  OSDisableScheduler();
  OSClearContext(&FatalContext);
  OSSetCurrentContext(&FatalContext);
  __OSStopAudioSystem();
#if SDK_VERSION <= SDK_VERSION_RVL_2005_11
  AISetStreamVolLeft(0);
  AISetStreamVolRight(0);
#endif
  VIInit();
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2003_04
  __OSUnmaskInterrupts(0x80);
#endif
  VISetBlack(TRUE);
  VIFlush();
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2003_04
  VISetPreRetraceCallback(NULL);
  VISetPostRetraceCallback(NULL);
#endif
  OSEnableInterrupts();

  count = VIGetRetraceCount();
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  while ((s32)(VIGetRetraceCount() - count) < 1)
    ;
#else
  do {
  } while ((s32)(VIGetRetraceCount() - count) < 1);
#endif

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  while (!__OSCallResetFunctions(0))
    ;

  OSDisableInterrupts();
  __OSCallResetFunctions(1);
#else
  t = OSGetTime();
#if SDK_VERSION >= SDK_VERSION_DOLPHIN_2003_04 && SDK_VERSION <= SDK_VERSION_RVL_2005_11
  while (!__OSCallResetFunctions(FALSE) && OSGetTime() - t < OSMillisecondsToTicks(1000)) {
  }

  OSDisableInterrupts();
  __OSCallResetFunctions(TRUE);
#else
  while (!__OSCallShutdownFunctions(FALSE, 0) && OSGetTime() - t < OSMillisecondsToTicks(1000)) {
  }

  OSDisableInterrupts();
  __OSCallShutdownFunctions(TRUE, 0);
#endif
  EXISetExiCallback(0, NULL);
  EXISetExiCallback(2, NULL);

  while (!EXILock(0, 1, NULL)) {
    EXISync(0);
    EXIDeselect(0);
    EXIUnlock(0);
  }
  EXIUnlock(0);

  do {
  } while ((__EXIRegs[3] & 1) == 1);

#endif
  __OSSetExceptionHandler(8, OSDefaultExceptionHandler);
  GXAbortFrame();
#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  OSSetArenaLo((void*)0x81200000);
  OSSetArenaHi(bootInfo->FSTLocation);
#elif SDK_VERSION <= SDK_VERSION_RVL_2005_11
  OSSetArenaLo((void*)0x81400000);
  OSSetArenaHi(bootInfo->FSTLocation);

#else
  OSSetArenaLo((void*)0x81400000);
  if (bootInfo->FSTLocation == NULL) {
    OSSetArenaHi(*(void**)OSPhysicalToCached(0x3110));
  } else {
    OSSetArenaHi(bootInfo->FSTLocation);
  }

#endif
  FatalParam.fg = fg;
  FatalParam.bg = bg;
  FatalParam.msg = msg;
  OSSwitchFiber((u32)Halt, (u32)OSGetArenaHi());
}

static void Halt() {
  u32 count;
  OSFontHeader* fontData;
  void* xfb;
  u32 len;
  OSFatalParam* fp;

  OSEnableInterrupts();
  fp = &FatalParam;
  len = strlen(fp->msg) + 1;
  fp->msg = memmove(OSAllocFromArenaLo(len, 32), fp->msg, len);

#if SDK_VERSION <= SDK_VERSION_DOLPHIN_2003_02
  if (OSGetFontEncode() == OS_FONT_ENCODE_SJIS) {
    fontData = OSAllocFromArenaLo(0x90ee4, 32);
  } else {
    fontData = OSAllocFromArenaLo(0x10120, 32);
  }
#else
  fontData = OSAllocFromArenaLo(0xA1004, 32);
#endif
  OSLoadFont(fontData, OSGetArenaLo());

  xfb = OSAllocFromArenaLo(0x96000, 32);
  ScreenClear(xfb, 640, 480, RGB2YUV(fp->bg));
  VISetNextFrameBuffer(xfb);
  ConfigureVideo(640, 480);
  VIFlush();

  count = VIGetRetraceCount();
  do {
  } while ((s32)(VIGetRetraceCount() - count) < 2);

  ScreenReport(xfb, 640, 480, RGB2YUV(fp->fg), 48, 100, fontData->leading, fp->msg);
  DCFlushRange(xfb, 0x96000);
  VISetBlack(FALSE);
  VIFlush();

  count = VIGetRetraceCount();
  do {
  } while ((s32)(VIGetRetraceCount() - count) < 1);

  OSDisableInterrupts();
  OSReport("%s\n", fp->msg);
  PPCHalt();
}

#pragma fp_contract reset
