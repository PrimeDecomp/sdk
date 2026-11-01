#ifndef _SDK_VERSION_H
#define _SDK_VERSION_H

// Identifies the build of the SDK that a game links against. Every version check in the SDK is
// made against one of these values.
//
// The game's build configuration (configure.py) picks the value to use for its version and defines
// SDK_VERSION to it (e.g. -DSDK_VERSION=<number>), and passes the same value to sdk_configure.py.
// This header is the single source of truth for the numbers: sdk_configure.py reads them from here.
//
// A version is named after the SDK build it represents, dated by the "release build" strings that
// the libraries register (OSRegisterVersion). The numbers are ordered by date so that range checks
// work; nothing may depend on the numbers themselves, only on their order. Add a version only when
// a check needs to tell it apart from its neighbours.

// Dolphin SDK (GameCube)
#define SDK_VERSION_DOLPHIN_2002_06 0 // Dolphin OS kernel of Jun 5 2002, before the version strings.
                                      // Metroid Prime Multi-Game Demo Disc v7-9.
#define SDK_VERSION_DOLPHIN_2002_09 1 // Sep 5 2002 release (0x2301).
                                      // Metroid Prime mp-v1.088, mp-v1.093 NTSC-U, mp-v1.097 NTSC-K.
#define SDK_VERSION_DOLPHIN_2002_11 2 // Sep 5 2002 release with the Oct-Nov 2002 OS, DVD, EXI and GX.
                                      // Metroid Prime mp-v1.110 PAL, mp-v1.111 NTSC-J and its demo.
#define SDK_VERSION_DOLPHIN_2003_02 3 // The above with the Feb 7 2003 GX.
                                      // Metroid Prime mp-v1.111 NTSC-U.
#define SDK_VERSION_DOLPHIN_2004_04_OS_APR 4 // Apr 5 2004 release (0x2301), with the Apr 5 2004 OS.
                                              // Metroid Prime 2: Echoes demo discs (D84P01).
#define SDK_VERSION_DOLPHIN_2004_04 5 // Apr 5 2004 release (0x2301), with the May 21 2004 OS.
                                      // Metroid Prime 2: Echoes, all retail GameCube versions.

// Revolution SDK (Wii)
#define SDK_VERSION_RVL_2005_11 6 // Apr 2004 Dolphin libraries with the Nov 2005 pre-release
                                  // Revolution KPAD and WPAD. Corruption prototype.
#define SDK_VERSION_RVL_2008_01 7 // RVL_SDK 0x4199_60831 builds, with the Jan 30 2008 OS.
                                  // New Play Controls and Trilogy, for both Prime and Echoes.

#ifndef SDK_VERSION
#error "SDK_VERSION is not defined: the game's build configuration must define it to one of the SDK_VERSION_* values"
#endif

#endif // _SDK_VERSION_H
