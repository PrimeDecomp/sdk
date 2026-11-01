#ifndef _SDK_VERSION_H
#define _SDK_VERSION_H

// Identifies the build of the SDK that a game links against. Every version check in the SDK is
// made against one of these values.
//
// The game's build configuration (configure.py) picks the value to use for its version and defines
// SDK_VERSION to it (e.g. -DSDK_VERSION=<number>), and passes the same value to sdk_configure.py.
// This header is the single source of truth for the numbers: sdk_configure.py reads them from here.
//
// The numbers are ordered so that range checks work:
//   - SDK_VERSION < SDK_VERSION_G2ME01_00 is the SDK build used by Metroid Prime's GameCube
//     versions (from the multi-game demo up to the 1.111 releases);
//   - SDK_VERSION >= SDK_VERSION_R32J01_00 is the Wii SDK.
//
// A version is named after the first game version known to use that SDK build. Game versions that
// share a build share a value. Add new versions at the position that keeps the ordering above;
// nothing may depend on the numbers themselves, only on their order.

// Metroid Prime, GameCube
#define SDK_VERSION_GM8EAB_00 0 // Multi-Game Demo Disc v7-9
#define SDK_VERSION_GM8E01_00 1 // mp-v1.088 NTSC-U, also mp-v1.093 NTSC-U and mp-v1.097 NTSC-K
#define SDK_VERSION_GM8E01_02 2 // mp-v1.111 NTSC-U
#define SDK_VERSION_GM8P01_00 3 // mp-v1.110 PAL
#define SDK_VERSION_GM8J01_DD 4 // mp-v1.111 NTSC-J demo
#define SDK_VERSION_GM8J01_00 5 // mp-v1.111 NTSC-J

// Metroid Prime 2: Echoes, GameCube
#define SDK_VERSION_G2ME01_00 6 // mp2-v1.028 NTSC-U
#define SDK_VERSION_G2MP01_00 7 // mp2-v1.035 PAL
#define SDK_VERSION_G2MJ01_00 8 // mp2-v1.036 NTSC-J

// Wii (New Play Controls and Trilogy)
#define SDK_VERSION_R32J01_00 9  // mp2-v3.561 New Play Controls
#define SDK_VERSION_R3ME01_00 10 // mp-v3.570 New Play Controls, mp-v3.593 Trilogy NTSC and
                                 // mp-v3.629 Trilogy PAL, for both Prime and Echoes

#ifndef SDK_VERSION
#error "SDK_VERSION is not defined: the game's build configuration must define it to one of the SDK_VERSION_* values"
#endif

#endif // _SDK_VERSION_H
