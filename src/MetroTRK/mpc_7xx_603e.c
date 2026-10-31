#include "ppc/Generic/mpc_7xx_603e.h"
#include "ppc/Generic/targimpl.h"

extern u8 gTRKRestoreFlags[9 + 3 /* padding */];

// clang-format off
asm void TRKSaveExtended1Block() {
    nofralloc
    lis r2, gTRKCPUState@h
    ori r2, r2, gTRKCPUState@l
    mfsr r16, 0
    mfsr r17, 1
    mfsr r18, 2
    mfsr r19, 3
    mfsr r20, 4
    mfsr r21, 5
    mfsr r22, 6
    mfsr r23, 7
    mfsr r24, 8
    mfsr r25, 9
    mfsr r26, 10
    mfsr r27, 11
    mfsr r28, 12
    mfsr r29, 13
    mfsr r30, 14
    mfsr r31, 15
    stmw r16, 0x1a8(r2)
    mftb r27, 268
    mftb r28, 269
    mfspr r29, HID0
    mfspr r30, HID1
    mfsrr1 r31
    stmw r27, 0x1e8(r2)
    mfspr r15, PVR
    mfibatu r16, 0
    mfibatl r17, 0
    mfibatu r18, 1
    mfibatl r19, 1
    mfibatu r20, 2
    mfibatl r21, 2
    mfibatu r22, 3
    mfibatl r23, 3
    mfdbatu r24, 0
    mfdbatl r25, 0
    mfdbatu r26, 1
    mfdbatl r27, 1
    mfdbatu r28, 2
    mfdbatl r29, 2
    mfdbatu r30, 3
    mfdbatl r31, 3
    stmw r15, 0x1fc(r2)
    mfspr r24, 560
    mfspr r25, 561
    mfspr r26, 562
    mfspr r27, 563
    mfspr r28, 564
    mfspr r29, 565
    mfspr r30, 566
    mfspr r31, 567
    stmw r24, 0x240(r2)
    mfsdr1 r22
    mfdar r23
    mfdsisr r24
    mfsprg r25, 0
    mfsprg r26, 1
    mfsprg r27, 2
    mfsprg r28, 3
    mfdec r29
    mfspr r30, IABR
    mfear r31
    stmw r22, 0x25c(r2)
    mfspr r24, DABR
    mfspr r25, PMC1
    mfspr r26, PMC2
    mfspr r27, PMC3
    mfspr r28, PMC4
    mfspr r29, SIA
    mfspr r30, MMCR0
    mfspr r31, MMCR1
    stmw r24, 0x284(r2)
    mfspr r29, 567
    mfspr r30, 568
    mfspr r31, 569
    stmw r29, 0x2a4(r2)
    mfspr r30, ICTC
    mfspr r31, L2CR
    stmw r30, 0x2b0(r2)
    mfsrr0 r16
    stw r16, 0x2b8(r2)
    mfspr r17, 570
    stw r17, 0x2bc(r2)
    mfspr r25, UMMCR0
    mfspr r26, UPMC1
    mfspr r27, UPMC2
    mfspr r28, USIA
    mfspr r29, UMMCR1
    mfspr r30, UPMC3
    mfspr r31, UPMC4
    stmw r25, 0x2c0(r2)
    mfspr r25, 571
    mfspr r26, 572
    mfspr r27, 573
    mfspr r28, 574
    mfspr r29, 575
    mfspr r30, 920
    mfspr r31, 1011
    stmw r25, 0x2dc(r2)
    mfspr r20, GQR0
    mfspr r21, GQR1
    mfspr r22, GQR2
    mfspr r23, GQR3
    mfspr r24, GQR4
    mfspr r25, GQR5
    mfspr r26, GQR6
    mfspr r27, GQR7
    mfspr r28, 920
    mfspr r29, WPAR
    mfspr r30, DMA_U
    mfspr r31, DMA_L
    stmw r20, 0x2fc(r2)
    blr
}

asm void TRKRestoreExtended1Block() {
    nofralloc
    lis r2, gTRKCPUState@h
    ori r2, r2, gTRKCPUState@l
    lis r5, gTRKRestoreFlags@h
    ori r5, r5, gTRKRestoreFlags@l
    lbz r3, 0(r5)
    lbz r6, 0x1(r5)
    li r0, 0x0
    stb r0, 0(r5)
    stb r0, 0x1(r5)
    cmpwi r3, 0x0
    beq L_3270
    lwz r24, 0x1e8(r2)
    lwz r25, 0x1ec(r2)
    mttbl r24
    mttbu r25
L_3270:
    lmw r20, 0x2fc(r2)
    mtspr GQR0, r20
    mtspr GQR1, r21
    mtspr GQR2, r22
    mtspr GQR3, r23
    mtspr GQR4, r24
    mtspr GQR5, r25
    mtspr GQR6, r26
    mtspr GQR7, r27
    mtspr 920, r28
    mtspr DMA_U, r30
    mtspr DMA_L, r31
    b L_32A4
L_32A4:
    lmw r19, 0x284(r2)
    mtspr DABR, r19
    mtspr PMC1, r20
    mtspr PMC2, r21
    mtspr PMC3, r22
    mtspr PMC4, r23
    mtspr SIA, r24
    mtspr MMCR0, r25
    mtspr MMCR1, r26
    mtspr ICTC, r30
    mtspr L2CR, r31
    b L_32D4
L_32D4:
    lmw r16, 0x1a8(r2)
    mtsr 0, r16
    mtsr 1, r17
    mtsr 2, r18
    mtsr 3, r19
    mtsr 4, r20
    mtsr 5, r21
    mtsr 6, r22
    mtsr 7, r23
    mtsr 8, r24
    mtsr 9, r25
    mtsr 10, r26
    mtsr 11, r27
    mtsr 12, r28
    mtsr 13, r29
    mtsr 14, r30
    mtsr 15, r31
    lmw r12, 0x1f0(r2)
    mtspr HID0, r12
    mtspr HID1, r13
    mtsrr1 r14
    mtspr PVR, r15
    mtibatu 0, r16
    mtibatl 0, r17
    mtibatu 1, r18
    mtibatl 1, r19
    mtibatu 2, r20
    mtibatl 2, r21
    mtibatu 3, r22
    mtibatl 3, r23
    mtdbatu 0, r24
    mtdbatl 0, r25
    mtdbatu 1, r26
    mtdbatl 1, r27
    mtdbatu 2, r28
    mtdbatl 2, r29
    mtdbatu 3, r30
    mtdbatl 3, r31
    lmw r22, 0x25c(r2)
    mtsdr1 r22
    mtdar r23
    mtdsisr r24
    mtsprg 0, r25
    mtsprg 1, r26
    mtsprg 2, r27
    mtsprg 3, r28
    mtspr IABR, r30
    mtear r31
    blr
}
// clang-format on
