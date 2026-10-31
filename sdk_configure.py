from typing import List, Dict, Any, Optional

from tools.project import (
    Object,
)


VERSIONS = [
    "GM8EAB_00",  # Multi-Game Demo Disc v7-9
    "GM8E01_00",  # mp-v1.088 NTSC-U
    "GM8E01_01",  # mp-v1.093 NTSC-U
    "GM8E01_48",  # mp-v1.097 NTSC-K
    "GM8E01_02",  # mp-v1.111 NTSC-U
    "GM8P01_00",  # mp-v1.110 PAL
    "GM8J01_DD",  # mp-v1.111 NTSC-J DEMO
    "GM8J01_00",  # mp-v1.111 NTSC-J
    "G2ME01_00",  # mp2-v1.028 NTSC-U
    "G2MP01_00",  # mp2-v1.035 PAL
    "G2MJ01_00",  # mp2-v1.036 NTSC-J
    "R3IJ01_00",  # mp-v3.570 New Play Controls
    "R32J01_00",  # mp2-v3.561 New Play Controls
    "R3ME01_00",  # mp-v3.593 Trilogy NTSC
    "R3MP01_00",  # mp-v3.629 Trilogy PAL
]


def modules_for(
    version: str,
    cflags_base: list[str],
    sdk_dir: str = "extern/sdk",
    non_matching: bool = False,
) -> list[dict]:
    src_dir = f"{sdk_dir}/src"
    version_num = VERSIONS.index(version)
    is_wii = version_num >= VERSIONS.index("R3IJ01_00")

    # Dolphin flags
    cflags_dolphin = [
        *cflags_base,
        "-fp_contract off",
    ]

    # The Wii SDK keeps its inline assembly blocks in source order.
    cflags_rvl_sdk = [
        *cflags_dolphin,
        '-pragma "optimizewithasm off"',
    ]

    # Metrowerks library flags
    cflags_runtime = [
        *cflags_base,
        "-use_lmw_stmw on",
        "-str reuse,pool,readonly",
        "-gccinc",
        "-common off",
        # "-inline auto",
    ]
    if is_wii:
        cflags_runtime.append("-fp_contract off")

    mw_version = "GC/2.7"
    if is_wii:
        mw_version = "Wii/1.0RC1"
        cflags_runtime.append("-inline auto")
    else:
        cflags_runtime.append("-inline deferred,auto")

    if is_wii:
        cflags_runtime.append("-func_align 4")

    # The Wii SDK libraries name their compiler in their version strings.
    RVL_SDK_MW_VERSIONS = {
        "ai": "GC/3.0a5.2",
        "ax": "GC/3.0a5.2",
        "kpad": "GC/3.0a5.2",
        "arc": "GC/3.0a3",
        "base": "GC/3.0a5.2",
        "dsp": "GC/3.0a5.2",
        "dvd": "GC/3.0a5.2",
        "exi": "GC/3.0a5.2",
        "ipc": "GC/3.0a5.2",
        "wpad": "GC/3.0a3",
        "fs": "GC/3.0a3",
        "euart": "GC/3.0a3",
        "nand": "GC/3.0a3",
        "wud": "GC/3.0a3",
        "usb": "GC/3.0a3",
        "tpl": "GC/3.0a3",
        "esp": "GC/3.0a3",
        "mem": "GC/3.0a3",
        "sc": "GC/3.0a3",
        "bte": "GC/3.0a3",
        "gx": "GC/3.0a3.4",
        "mtx": "GC/3.0a5.2",
        "os": "GC/3.0a5.2",
        "pad": "GC/3.0a5.2",
        "NdevExi2AD": "GC/3.0a5",
        "si": "GC/3.0a5.2",
        "thp": "GC/3.0a5.2",
        "vi": "GC/3.0a5.2",
        "nwc24": "GC/3.0a5.2",
        "vf": "GC/3.0a3",
        "wenc": "GC/3.0a5.2",
        "rso": "GC/3.0a5.2",
        "odh": "GC/3.0a5.2",
        "net": "GC/3.0a5.2",
    }

    # The Bluetooth stack keeps its own headers next to the public ones.
    BTE_CFLAGS = [
        f"-i {sdk_dir}/include/revolution/bte/internal",
        "-inline auto,level=3",
    ]

    # Helper function for Dolphin libraries
    def DolphinLib(
        lib_name: str, objects: List[Object], gc_only: bool = False
    ) -> Dict[str, Any]:
        if gc_only and is_wii:
            # The Wii versions use the separate Revolution library instead.
            objects = []
        mw_version = "GC/1.2.5n"
        cflags = cflags_dolphin
        if lib_name in RVL_SDK_MW_VERSIONS and is_wii:
            mw_version = RVL_SDK_MW_VERSIONS[lib_name]
            cflags = cflags_rvl_sdk
        return {
            "lib": lib_name,
            "src_dir": src_dir,
            "mw_version": mw_version,
            "cflags": cflags,
            "progress_category": "sdk",
            "host": False,
            "objects": objects,
        }

    # Helper function for Revolution (Wii) SDK libraries
    def RevolutionLib(
        lib_name: str, objects: List[Object], extra_cflags: Optional[List[str]] = None
    ) -> Dict[str, Any]:
        return {
            "lib": f"rvl_{lib_name}",
            "src_dir": src_dir,
            "mw_version": RVL_SDK_MW_VERSIONS[lib_name],
            "cflags": [*cflags_rvl_sdk, "-ipa file", *(extra_cflags or [])],
            "progress_category": "sdk",
            "host": False,
            "objects": objects if is_wii else [],
        }

    NonMatching = False

    # Object is only matching for specific versions
    def MatchingFor(*versions):
        return version in versions

    def EquivalentFor(*versions):
        return MatchingFor(*versions) and non_matching

    GC_VERSIONS = ["G2ME01", "G2MP01", "G2MJ01"]

    return [
        {
            "lib": "MetroTRK",
            "src_dir": src_dir,
            "cflags": cflags_runtime,
            "progress_category": "sdk",
            "host": False,
            "objects": [
                Object(MatchingFor("G2ME01", "G2MP01", "G2MJ01"), "MetroTRK/mslsupp.c"),
            ],
        },
        {
            "lib": "TRK",
            "src_dir": src_dir,
            "mw_version": "Wii/1.0RC1",
            "progress_category": "sdk",
            "cflags": [
                *cflags_rvl_sdk,
                "-use_lmw_stmw on",
                "-str reuse,pool",
                "-func_align 4",
                f"-i {sdk_dir}/include/MetroTRK",
                "-DMETRO_TRK",
                "-D__REGISTER=register",
            ],
            "progress_category": "sdk",
            "host": False,
            "objects": [
                Object(MatchingFor("R3ME01"), "MetroTRK/CircleBuffer.c"),
                Object(MatchingFor("R3ME01"), "MetroTRK/MWCriticalSection_gc.c"),
                Object(MatchingFor("R3ME01"), "MetroTRK/gdev_main.c"),
                Object(MatchingFor("R3ME01"), "MetroTRK/dispatch.c"),
                Object(MatchingFor("R3ME01"), "MetroTRK/exception.s"),
                Object(NonMatching, "MetroTRK/dolphin_trk.c"),
                Object(MatchingFor("R3ME01"), "MetroTRK/dolphin_trk_glue.c"),
                Object(MatchingFor("R3ME01"), "MetroTRK/flush_cache.c"),
                Object(MatchingFor("R3ME01"), "MetroTRK/main_TRK.c"),
                Object(MatchingFor("R3ME01"), "MetroTRK/mainloop.c"),
                Object(NonMatching, "MetroTRK/mem_TRK.c"),
                Object(MatchingFor("R3ME01"), "MetroTRK/mpc_7xx_603e.c"),
                Object(MatchingFor("R3ME01"), "MetroTRK/msg.c"),
                Object(MatchingFor("R3ME01"), "MetroTRK/msgbuf.c"),
                Object(MatchingFor("R3ME01"), "MetroTRK/msghndlr.c"),
                Object(MatchingFor("R3ME01"), "MetroTRK/Export/mslsupp.c"),
                Object(MatchingFor("R3ME01"), "MetroTRK/notify.c"),
                Object(MatchingFor("R3ME01"), "MetroTRK/nubevent.c"),
                Object(MatchingFor("R3ME01"), "MetroTRK/nubinit.c"),
                Object(MatchingFor("R3ME01"), "MetroTRK/serpoll.c"),
                Object(MatchingFor("R3ME01"), "MetroTRK/string_TRK.c"),
                Object(MatchingFor("R3ME01"), "MetroTRK/support.c"),
                Object(MatchingFor("R3ME01"), "MetroTRK/Os/targcont.c"),
                Object(MatchingFor("R3ME01"), "MetroTRK/Os/target_options.c"),
                Object(NonMatching, "MetroTRK/targimpl.c"),
                Object(MatchingFor("R3ME01"), "MetroTRK/targsupp.c"),
            ],
        },
        {
            "lib": "Runtime.PPCEABI.H",
            "src_dir": src_dir,
            "mw_version": mw_version,
            "cflags": cflags_runtime,
            "progress_category": "sdk",
            "objects": [
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/__va_arg.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/global_destructor_chain.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS),
                    "Runtime/CPlusLibPPC.cpp",
                ),
                Object(
                    MatchingFor("G2ME01", "G2MP01"),
                    "Runtime/New.cp",
                    extra_cflags=["-RTTI on", "-Cpp_exceptions on"],
                ),
                Object(
                    MatchingFor("G2ME01", "G2MP01"),
                    "Runtime/NMWException.cp",
                    extra_cflags=["-RTTI on", "-Cpp_exceptions on"],
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/ptmf.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS),
                    "Runtime/runtime.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/__init_cpp_exceptions.cpp",
                ),
                Object(
                    EquivalentFor("G2ME01"),
                    "Runtime/Gecko_ExceptionPPC.cp",
                    extra_cflags=["-RTTI on", "-Cpp_exceptions on"],
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/__mem.c",
                ),
            ],
        },
        {
            "lib": "MSL_C.PPCEABI.bare.H",
            "src_dir": src_dir,
            "mw_version": mw_version,
            "cflags": [
                *cflags_runtime,
                "-DMSL_OLD_FP_CLASSIFY",
                "-DMSL_NO_INLINE_SQRT",
            ],
            "progress_category": "sdk",
            "host": False,
            "objects": [
                Object(
                    MatchingFor(*GC_VERSIONS),
                    "Runtime/abort_exit.c",
                ),
                Object(MatchingFor(*GC_VERSIONS), "Runtime/alloc.c"),
                Object(
                    MatchingFor("G2ME01", "G2MP01", "R3ME01"),
                    "Runtime/errno.c",
                ),
                Object(MatchingFor(*GC_VERSIONS), "Runtime/ansi_files.c"),
                Object(MatchingFor("G2ME01", "G2MP01"), "Runtime/ansi_fp.c"),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/arith.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/buffer_io.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS),
                    "Runtime/critical_regions.gamecube.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/ctype.c",
                ),
                Object(MatchingFor("G2ME01", "G2MP01"), "Runtime/locale.c"),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/direct_io.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/file_io.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS),
                    "Runtime/FILE_POS.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/mbstring.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/mem.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS),
                    "Runtime/mem_funcs.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/misc_io.c",
                ),
                Object(MatchingFor("G2ME01", "G2MP01"), "Runtime/printf.c"),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/qsort.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS),
                    "Runtime/rand.c",
                ),
                Object(
                    MatchingFor("G2ME01", "G2MP01", "R3ME01"),
                    "Runtime/float.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/sscanf.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS),
                    "Runtime/signal.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/string.c",
                ),
                Object(MatchingFor("G2ME01", "G2MP01"), "Runtime/strtold.c"),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/wchar_io.c",
                ),
                Object(MatchingFor("R3ME01"), "Runtime/wstring.c"),
                Object(MatchingFor("R3ME01"), "Runtime/wctype.c"),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/uart_console_io.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS),
                    "Runtime/e_acos.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS),
                    "Runtime/e_asin.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/e_atan2.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/e_fmod.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/e_log.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/e_log10.c",
                ),
                Object(MatchingFor(*GC_VERSIONS), "Runtime/e_pow.c"),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/e_rem_pio2.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/k_cos.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/k_rem_pio2.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/k_sin.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/k_tan.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/s_atan.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/s_copysign.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/s_cos.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/s_floor.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/s_frexp.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/s_ldexp.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/s_modf.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/s_sin.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/s_tan.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/w_acos.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/w_asin.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/w_atan2.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/w_fmod.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/w_log10.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/w_pow.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/e_sqrt.c",
                ),
                Object(MatchingFor(*GC_VERSIONS), "Runtime/math_ppc.c"),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "Runtime/w_sqrt.c",
                ),
            ],
        },
        DolphinLib(
            "dsp",
            [
                Object(MatchingFor("G2ME01", "G2MP01", "R3ME01"), "dolphin/dsp/dsp.c"),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "dolphin/dsp/dsp_debug.c",
                ),
                Object(
                    MatchingFor("G2ME01", "G2MP01", "R3ME01"), "dolphin/dsp/dsp_task.c"
                ),
            ],
        ),
        DolphinLib(
            "dtk",
            [
                Object(MatchingFor(*GC_VERSIONS), "dolphin/dtk.c"),
            ],
        ),
        DolphinLib(
            "dvd",
            [
                Object(MatchingFor("G2ME01", "G2MP01"), "dolphin/dvd/dvdlow.c"),
                Object(MatchingFor(*GC_VERSIONS), "dolphin/dvd/dvdfs.c"),
                Object(MatchingFor(*GC_VERSIONS), "dolphin/dvd/dvd.c"),
                Object(MatchingFor(*GC_VERSIONS), "dolphin/dvd/dvdqueue.c"),
                Object(MatchingFor(*GC_VERSIONS), "dolphin/dvd/dvderror.c"),
                Object(
                    MatchingFor(*GC_VERSIONS),
                    "dolphin/dvd/dvdidutils.c",
                ),
                Object(MatchingFor(*GC_VERSIONS), "dolphin/dvd/dvdfatal.c"),
                Object(MatchingFor(*GC_VERSIONS), "dolphin/dvd/fstload.c"),
            ],
            gc_only=True,
        ),
        RevolutionLib(
            "dvd",
            [
                Object(MatchingFor("R3ME01"), "revolution/dvd/dvdfs.c"),
                Object(EquivalentFor("G2ME01"), "revolution/dvd/dvd.c"),
                Object(MatchingFor("R3ME01"), "revolution/dvd/dvdqueue.c"),
                Object(MatchingFor("R3ME01"), "revolution/dvd/dvderror.c"),
                Object(MatchingFor("R3ME01"), "revolution/dvd/dvdidutils.c"),
                Object(MatchingFor("R3ME01"), "revolution/dvd/dvdFatal.c"),
                Object(EquivalentFor("G2ME01"), "revolution/dvd/dvdDeviceError.c"),
                Object(MatchingFor("R3ME01"), "revolution/dvd/dvd_broadway.c"),
            ],
        ),
        RevolutionLib(
            "NdevExi2AD",
            [
                Object(MatchingFor("R3ME01"), "revolution/NdevExi2AD/DebuggerDriver.c"),
                Object(MatchingFor("R3ME01"), "revolution/NdevExi2AD/exi2.c"),
            ],
        ),
        RevolutionLib(
            "os",
            [
                Object(MatchingFor("R3ME01"), "revolution/os/OSUtf.c"),
                Object(MatchingFor("R3ME01"), "revolution/os/OSIpc.c"),
                Object(MatchingFor("R3ME01"), "revolution/os/OSStateTM.c"),
                Object(MatchingFor("R3ME01"), "revolution/os/OSPlayRecord.c"),
                Object(MatchingFor("R3ME01"), "revolution/os/OSStateFlags.c"),
                Object(MatchingFor("R3ME01"), "revolution/os/OSNet.c"),
                Object(MatchingFor("R3ME01"), "revolution/os/OSNandbootInfo.c"),
                Object(MatchingFor("R3ME01"), "revolution/os/OSPlayTime.c"),
                Object(MatchingFor("R3ME01"), "revolution/os/OSLaunch.c"),
                Object(MatchingFor("R3ME01"), "revolution/os/OSFont.c"),
                Object(MatchingFor("R3ME01"), "revolution/os/OSExec.c"),
                Object(MatchingFor("R3ME01"), "revolution/os/OSReset.c"),
                Object(MatchingFor("R3ME01"), "revolution/os/OSLink.c"),
            ],
        ),
        RevolutionLib(
            "kpad",
            [
                Object(MatchingFor("R3ME01"), "revolution/kpad/KPAD.c"),
            ],
        ),
        RevolutionLib(
            "wpad",
            [
                Object(
                    MatchingFor("R3ME01"),
                    "revolution/wpad/WPAD.c",
                    extra_cflags=["-fp soft"],
                ),
                Object(MatchingFor("R3ME01"), "revolution/wpad/WPADHIDParser.c"),
                Object(MatchingFor("R3ME01"), "revolution/wpad/WPADMem.c"),
                Object(MatchingFor("R3ME01"), "revolution/wpad/WPADEncrypt.c"),
                Object(MatchingFor("R3ME01"), "revolution/wpad/debug_msg.c"),
            ],
        ),
        RevolutionLib(
            "fs",
            [
                Object(MatchingFor("R3ME01"), "revolution/fs/fs.c"),
            ],
        ),
        RevolutionLib(
            "euart",
            [
                Object(MatchingFor("R3ME01"), "revolution/euart/euart.c"),
            ],
        ),
        RevolutionLib(
            "ipc",
            [
                Object(MatchingFor("R3ME01"), "revolution/ipc/ipcMain.c"),
                Object(MatchingFor("R3ME01"), "revolution/ipc/ipcclt.c"),
                Object(MatchingFor("R3ME01"), "revolution/ipc/memory.c"),
                Object(MatchingFor("R3ME01"), "revolution/ipc/ipcProfile.c"),
            ],
        ),
        RevolutionLib(
            "nand",
            [
                Object(MatchingFor("R3ME01"), "revolution/nand/nand.c"),
                Object(MatchingFor("R3ME01"), "revolution/nand/NANDOpenClose.c"),
                Object(MatchingFor("R3ME01"), "revolution/nand/NANDCore.c"),
                Object(MatchingFor("R3ME01"), "revolution/nand/NANDSecret.c"),
                Object(MatchingFor("R3ME01"), "revolution/nand/NANDCheck.c"),
                Object(MatchingFor("R3ME01"), "revolution/nand/NANDLogging.c"),
            ],
        ),
        RevolutionLib(
            "bte",
            [
                Object(MatchingFor("R3ME01"), "revolution/bte/gki_buffer.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/gki_time.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/gki_ppc.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/hcisu_h2.c"),
                Object(
                    MatchingFor("R3ME01"),
                    "revolution/bte/uusb_ppc.c",
                    mw_version="GC/3.0a5.2",
                ),
                Object(MatchingFor("R3ME01"), "revolution/bte/bta_dm_cfg.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/bta_hh_cfg.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/bta_sys_cfg.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/bte_hcisu.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/bte_init.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/bte_logmsg.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/bte_main.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/btu_task1.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/bd.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/bta_sys_conn.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/bta_sys_main.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/ptim.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/utl.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/bta_dm_act.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/bta_dm_api.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/bta_dm_main.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/bta_dm_pm.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/bta_hh_act.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/bta_hh_api.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/bta_hh_main.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/bta_hh_utils.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/btm_acl.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/btm_dev.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/btm_devctl.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/btm_discovery.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/btm_inq.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/btm_main.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/btm_pm.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/btm_sco.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/btm_sec.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/btu_hcif.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/btu_init.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/wbt_ext.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/gap_api.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/gap_conn.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/gap_utils.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/hcicmds.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/hidd_api.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/hidd_conn.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/hidd_mgmt.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/hidd_pm.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/hidh_api.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/hidh_conn.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/l2c_api.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/l2c_csm.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/l2c_link.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/l2c_main.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/l2c_utils.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/port_api.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/port_rfc.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/port_utils.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/rfc_l2cap_if.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/rfc_mx_fsm.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/rfc_port_fsm.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/rfc_port_if.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/rfc_ts_frames.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/rfc_utils.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/sdp_api.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/sdp_db.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/sdp_discovery.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/sdp_main.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/sdp_server.c"),
                Object(MatchingFor("R3ME01"), "revolution/bte/sdp_utils.c"),
            ],
            extra_cflags=BTE_CFLAGS,
        ),
        RevolutionLib(
            "wud",
            [
                Object(
                    MatchingFor("R3ME01"),
                    "revolution/wud/WUD.c",
                    extra_cflags=["-fp soft"],
                ),
                Object(MatchingFor("R3ME01"), "revolution/wud/WUDHidHost.c"),
                Object(MatchingFor("R3ME01"), "revolution/wud/debug_msg.c"),
            ],
        ),
        RevolutionLib(
            "usb",
            [
                Object(MatchingFor("R3ME01"), "revolution/usb/usb.c"),
            ],
        ),
        RevolutionLib(
            "mem",
            [
                Object(MatchingFor("R3ME01"), "revolution/mem/mem_heapCommon.c"),
                Object(MatchingFor("R3ME01"), "revolution/mem/mem_expHeap.c"),
                Object(MatchingFor("R3ME01"), "revolution/mem/mem_list.c"),
            ],
        ),
        RevolutionLib(
            "sc",
            [
                Object(MatchingFor("R3ME01"), "revolution/sc/scsystem.c"),
                Object(MatchingFor("R3ME01"), "revolution/sc/scapi.c"),
                Object(MatchingFor("R3ME01"), "revolution/sc/scapi_prdinfo.c"),
            ],
        ),
        RevolutionLib(
            "ai",
            [
                Object(MatchingFor("R3ME01"), "revolution/ai/ai.c"),
            ],
        ),
        RevolutionLib(
            "ax",
            [
                Object(MatchingFor("R3ME01"), "revolution/ax/AX.c"),
                Object(MatchingFor("R3ME01"), "revolution/ax/AXAlloc.c"),
                Object(MatchingFor("R3ME01"), "revolution/ax/AXAux.c"),
                Object(MatchingFor("R3ME01"), "revolution/ax/AXCL.c"),
                Object(MatchingFor("R3ME01"), "revolution/ax/AXOut.c"),
                Object(MatchingFor("R3ME01"), "revolution/ax/AXSPB.c"),
                Object(MatchingFor("R3ME01"), "revolution/ax/AXVPB.c"),
                Object(MatchingFor("R3ME01"), "revolution/ax/AXComp.c"),
                Object(MatchingFor("R3ME01"), "revolution/ax/DSPCode.c"),
                Object(MatchingFor("R3ME01"), "revolution/ax/AXProf.c"),
            ],
        ),
        RevolutionLib(
            "si",
            [
                Object(MatchingFor("R3ME01"), "revolution/si/SIBios.c"),
                Object(MatchingFor("R3ME01"), "revolution/si/SISamplingRate.c"),
            ],
        ),
        RevolutionLib(
            "arc",
            [
                Object(MatchingFor("R3ME01"), "revolution/arc/arc.c"),
            ],
        ),
        RevolutionLib(
            "tpl",
            [
                Object(MatchingFor("R3ME01"), "revolution/tpl/TPL.c"),
            ],
        ),
        RevolutionLib(
            "nwc24",
            [
                Object(MatchingFor("R3ME01"), "revolution/nwc24/NWC24StdAPI.c"),
                Object(MatchingFor("R3ME01"), "revolution/nwc24/NWC24FileAPI.c"),
                Object(MatchingFor("R3ME01"), "revolution/nwc24/NWC24Config.c"),
                Object(MatchingFor("R3ME01"), "revolution/nwc24/NWC24Utils.c"),
                Object(MatchingFor("R3ME01"), "revolution/nwc24/NWC24Manage.c"),
                Object(MatchingFor("R3ME01"), "revolution/nwc24/NWC24MsgObj.c"),
                Object(EquivalentFor("G2ME01"), "revolution/nwc24/NWC24MBoxCtrl.c"),
                Object(MatchingFor("R3ME01"), "revolution/nwc24/NWC24Mime.c"),
                Object(MatchingFor("R3ME01"), "revolution/nwc24/NWC24Parser.c"),
                Object(EquivalentFor("G2ME01"), "revolution/nwc24/NWC24MsgCommit.c"),
                Object(MatchingFor("R3ME01"), "revolution/nwc24/NWC24Schedule.c"),
                Object(MatchingFor("R3ME01"), "revolution/nwc24/NWC24DateParser.c"),
                Object(MatchingFor("R3ME01"), "revolution/nwc24/NWC24FriendList.c"),
                Object(MatchingFor("R3ME01"), "revolution/nwc24/NWC24SecretFList.c"),
                Object(MatchingFor("R3ME01"), "revolution/nwc24/NWC24Time.c"),
                Object(MatchingFor("R3ME01"), "revolution/nwc24/NWC24Ipc.c"),
                Object(MatchingFor("R3ME01"), "revolution/nwc24/NWC24Download.c"),
                Object(MatchingFor("R3ME01"), "revolution/nwc24/NWC24System.c"),
            ],
            extra_cflags=["-inline auto,level=3"],
        ),
        RevolutionLib(
            "vf",
            [
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_clib.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_code.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_service.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_str.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_w_clib.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_driver.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pdm_bpb.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pdm_disk.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pdm_partition.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pdm_mbr.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pdm_dskmng.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_cache.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_cluster.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_dir.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_entry.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_entry_iterator.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_fat.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_fat12.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_fat16.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_fat32.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_fatfs.c"),
                Object(
                    EquivalentFor("G2ME01"),
                    "revolution/vf/pf_file.c",
                    extra_cflags=["-fp soft"],
                ),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_path.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_sector.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_volume.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_cp932.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_api_util.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_attach.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_detach.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_errnum.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_fclose.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_finfo.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_fopen.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_fread.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_fseek.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_fwrite.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_getdev.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_init_prfile2.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_remove.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_unmount.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_filelock.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/pf_system.c"),
                Object(
                    MatchingFor("R3ME01"),
                    "revolution/vf/d_vf.c",
                    extra_cflags=["-fp soft"],
                ),
                Object(
                    MatchingFor("R3ME01"),
                    "revolution/vf/d_vf_sys.c",
                    extra_cflags=["-fp soft"],
                ),
                Object(MatchingFor("R3ME01"), "revolution/vf/d_hash.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/d_time.c"),
                Object(MatchingFor("R3ME01"), "revolution/vf/d_common.c"),
                Object(
                    MatchingFor("R3ME01"),
                    "revolution/vf/nand_drv.c",
                    extra_cflags=["-fp soft"],
                ),
                Object(MatchingFor("R3ME01"), "revolution/vf/sd_drv.c"),
            ],
            extra_cflags=["-inline auto,level=3"],
        ),
        RevolutionLib(
            "wenc",
            [
                Object(MatchingFor("R3ME01"), "revolution/wenc/wenc.c"),
            ],
        ),
        RevolutionLib(
            "rso",
            [
                Object(MatchingFor("R3ME01"), "revolution/rso/RSOLink.c"),
            ],
        ),
        RevolutionLib(
            "odh",
            [
                Object(EquivalentFor("G2ME01"), "revolution/odh/odh.cpp"),
            ],
        ),
        RevolutionLib(
            "net",
            [
                Object(MatchingFor("R3ME01"), "revolution/net/nettime.c"),
                Object(MatchingFor("R3ME01"), "revolution/net/NETVersion.c"),
            ],
        ),
        RevolutionLib(
            "esp",
            [
                Object(MatchingFor("R3ME01"), "revolution/esp/esp.c"),
            ],
        ),
        DolphinLib(
            "exi",
            [
                Object(
                    MatchingFor("G2ME01", "G2MP01", "R3ME01"),
                    "dolphin/exi/EXIBios.c",
                    extra_cflags=["-ipa file"] if is_wii else [],
                ),
                Object(MatchingFor(*GC_VERSIONS), "dolphin/exi/EXIUart.c"),
                Object(MatchingFor("R3ME01"), "revolution/exi/EXIUart.c"),
                Object(MatchingFor("R3ME01"), "revolution/exi/EXICommon.c"),
            ],
        ),
        DolphinLib(
            "gx",
            [
                Object(
                    MatchingFor("G2ME01", "G2MP01", "R3ME01"),
                    "dolphin/gx/GXInit.c",
                    extra_cflags=["-ipa file"] if is_wii else [],
                ),
                Object(
                    MatchingFor(*GC_VERSIONS),
                    "dolphin/gx/GXFifo.c",
                    extra_cflags=["-ipa file"] if is_wii else [],
                ),
                Object(MatchingFor("G2ME01", "G2MP01"), "dolphin/gx/GXAttr.c"),
                Object(MatchingFor(*GC_VERSIONS), "dolphin/gx/GXMisc.c"),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "dolphin/gx/GXGeometry.c",
                    extra_cflags=["-ipa file"] if is_wii else [],
                ),
                Object(MatchingFor("G2ME01", "G2MP01"), "dolphin/gx/GXFrameBuf.c"),
                Object(
                    MatchingFor("G2ME01", "G2MP01", "R3ME01"), "dolphin/gx/GXLight.c"
                ),
                Object(MatchingFor("G2ME01", "G2MP01"), "dolphin/gx/GXTexture.c"),
                Object(MatchingFor("G2ME01", "G2MP01"), "dolphin/gx/GXBump.c"),
                Object(MatchingFor("G2ME01", "G2MP01", "R3ME01"), "dolphin/gx/GXTev.c"),
                Object(MatchingFor(*GC_VERSIONS), "dolphin/gx/GXPixel.c"),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "dolphin/gx/GXDisplayList.c",
                ),
                Object(
                    MatchingFor("G2ME01", "G2MP01", "R3ME01"),
                    "dolphin/gx/GXTransform.c",
                    extra_cflags=["-ipa file"] if is_wii else [],
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "dolphin/gx/GXPerf.c",
                ),
            ],
        ),
        DolphinLib(
            "mtx",
            [
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "dolphin/mtx/mtx.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "dolphin/mtx/mtxvec.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "dolphin/mtx/mtx44.c",
                ),
            ],
        ),
        DolphinLib(
            "pad",
            [
                Object(MatchingFor("G2ME01", "G2MP01"), "dolphin/pad/PadClamp.c"),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "dolphin/pad/Pad.c",
                    extra_cflags=["-char unsigned"],
                ),
            ],
        ),
        DolphinLib(
            "vi",
            [
                Object(
                    MatchingFor("G2ME01", "G2MP01", "R3ME01"),
                    "dolphin/vi/vi.c",
                    extra_cflags=["-ipa file"] if is_wii else [],
                ),
                Object(
                    MatchingFor("R3ME01"),
                    "dolphin/vi/i2c.c",
                    extra_cflags=["-ipa file"] if is_wii else [],
                ),
                Object(MatchingFor("R3ME01"), "dolphin/vi/vi3in1.c"),
            ],
        ),
        DolphinLib(
            "ai",
            [
                Object(MatchingFor(*GC_VERSIONS), "dolphin/ai.c"),
            ],
        ),
        DolphinLib(
            "si",
            [
                Object(MatchingFor("G2ME01", "G2MP01"), "dolphin/si/SIBios.c"),
                Object(MatchingFor("G2ME01", "G2MP01"), "dolphin/si/SISamplingRate.c"),
            ],
        ),
        DolphinLib(
            "thp",
            [
                Object(MatchingFor(*GC_VERSIONS), "dolphin/thp/THPDec.c"),
                Object(MatchingFor(*GC_VERSIONS), "dolphin/thp/THPAudio.c"),
            ],
            gc_only=True,
        ),
        RevolutionLib(
            "thp",
            [
                Object(MatchingFor("R3ME01"), "revolution/thp/THPDec.c"),
                Object(MatchingFor("R3ME01"), "revolution/thp/THPAudio.c"),
            ],
        ),
        DolphinLib(
            "ar",
            [
                Object(MatchingFor("G2ME01", "G2MP01"), "dolphin/ar/ar.c"),
                Object(MatchingFor("G2ME01", "G2MP01"), "dolphin/ar/arq.c"),
            ],
        ),
        DolphinLib(
            "card",
            [
                Object(MatchingFor("G2ME01", "G2MP01"), "dolphin/card/CARDBios.c"),
                Object(MatchingFor("G2ME01", "G2MP01"), "dolphin/card/CARDUnlock.c"),
                Object(MatchingFor(*GC_VERSIONS), "dolphin/card/CARDRdwr.c"),
                Object(
                    MatchingFor(*GC_VERSIONS),
                    "dolphin/card/CARDBlock.c",
                ),
                Object(MatchingFor(*GC_VERSIONS), "dolphin/card/CARDDir.c"),
                Object(
                    MatchingFor(*GC_VERSIONS),
                    "dolphin/card/CARDCheck.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS),
                    "dolphin/card/CARDMount.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS),
                    "dolphin/card/CARDFormat.c",
                ),
                Object(MatchingFor(*GC_VERSIONS), "dolphin/card/CARDOpen.c"),
                Object(
                    MatchingFor(*GC_VERSIONS),
                    "dolphin/card/CARDCreate.c",
                ),
                Object(MatchingFor(*GC_VERSIONS), "dolphin/card/CARDRead.c"),
                Object(
                    MatchingFor(*GC_VERSIONS),
                    "dolphin/card/CARDWrite.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS),
                    "dolphin/card/CARDDelete.c",
                ),
                Object(MatchingFor(*GC_VERSIONS), "dolphin/card/CARDStat.c"),
                Object(MatchingFor("G2ME01", "G2MP01"), "dolphin/card/CARDNet.c"),
                Object(MatchingFor("G2ME01", "G2MP01"), "dolphin/card/CARDRename.c"),
                Object(MatchingFor("G2ME01", "G2MP01"), "dolphin/card/CARDStatEx.c"),
                Object(MatchingFor("G2ME01", "G2MP01"), "dolphin/card/CARDRaw.c"),
                Object(MatchingFor("G2ME01", "G2MP01"), "dolphin/card/CARDErase.c"),
                Object(MatchingFor("G2ME01", "G2MP01"), "dolphin/card/CARDProgram.c"),
            ],
        ),
        DolphinLib(
            "base",
            [
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "dolphin/PPCArch.c",
                ),
                Object(MatchingFor(*GC_VERSIONS, "R3ME01"), "dolphin/db.c"),
            ],
        ),
        DolphinLib(
            "os",
            [
                Object(
                    MatchingFor("G2ME01", "G2MP01", "R3ME01"),
                    "dolphin/os/OS.c",
                    extra_cflags=["-ipa file"] if is_wii else [],
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "dolphin/os/OSAlarm.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "dolphin/os/OSArena.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "dolphin/os/OSAudioSystem.c",
                ),
                Object(
                    MatchingFor("G2ME01", "G2MP01", "R3ME01"), "dolphin/os/OSCache.c"
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "dolphin/os/OSContext.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "dolphin/os/OSError.c",
                ),
                Object(MatchingFor("G2ME01", "G2MP01"), "dolphin/os/OSExec.c"),
                Object(
                    MatchingFor("G2ME01", "G2MP01", "R3ME01"), "dolphin/os/OSFatal.c"
                ),
                Object(
                    MatchingFor("G2ME01", "G2MP01"),
                    "dolphin/os/OSFont.c",
                    extra_cflags=["-char unsigned"],
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "dolphin/os/OSInterrupt.c",
                    extra_cflags=["-ipa file"] if is_wii else [],
                ),
                Object(MatchingFor("G2ME01", "G2MP01"), "dolphin/os/OSLink.c"),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "dolphin/os/OSMemory.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "dolphin/os/OSMutex.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "dolphin/os/OSReboot.c",
                ),
                Object(MatchingFor(*GC_VERSIONS), "dolphin/os/OSReset.c"),
                Object(MatchingFor(*GC_VERSIONS), "dolphin/os/OSResetSW.c"),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "dolphin/os/OSRtc.c",
                    extra_cflags=["-ipa file"] if is_wii else [],
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "dolphin/os/OSSync.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "dolphin/os/OSThread.c",
                    extra_cflags=["-ipa file"] if is_wii else [],
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "dolphin/os/OSTime.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "dolphin/os/__start.c",
                ),
                Object(
                    MatchingFor(*GC_VERSIONS, "R3ME01"),
                    "dolphin/os/__ppc_eabi_init.cpp",
                ),
            ],
        ),
    ]
