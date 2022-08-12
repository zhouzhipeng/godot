import os
import sys
from methods import detect_darwin_sdk_path


def is_active():
    return True


def get_name():
    return "iOS"


def can_build():
    if sys.platform == "darwin" or ("OSXCROSS_IOS" in os.environ):
        return True

    return False


def get_opts():
    from SCons.Variables import BoolVariable

    return [
        (
            "IOS_TOOLCHAIN_PATH",
            "Path to iOS toolchain",
            "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain",
        ),
        ("IOS_SDK_PATH", "Path to the iOS SDK", ""),
        BoolVariable("ios_simulator", "Build for iOS Simulator", False),
        BoolVariable("ios_exceptions", "Enable exceptions", False),
        ("ios_triple", "Triple for ios toolchain", ""),
    ]


def get_flags():
    return [
        ("tools", False),
        ("use_volk", False),
    ]


def configure(env):
    ## Build type

    if env["target"].startswith("release"):
        env.Append(CPPDEFINES=["NDEBUG", ("NS_BLOCK_ASSERTIONS", 1)])
        if env["optimize"] == "speed":  # optimize for speed (default)
            # `-O2` is more friendly to debuggers than `-O3`, leading to better crash backtraces
            # when using `target=release_debug`.
            opt = "-O3" if env["target"] == "release" else "-O2"
            env.Append(CCFLAGS=[opt, "-ftree-vectorize", "-fomit-frame-pointer"])
            env.Append(LINKFLAGS=[opt])
        elif env["optimize"] == "size":  # optimize for size
            env.Append(CCFLAGS=["-Os", "-ftree-vectorize"])
            env.Append(LINKFLAGS=["-Os"])

    elif env["target"] == "debug":
        env.Append(CCFLAGS=["-gdwarf-2", "-O0"])
        env.Append(CPPDEFINES=["_DEBUG", ("DEBUG", 1)])

    if env["use_lto"]:
        env.Append(CCFLAGS=["-flto"])
        env.Append(LINKFLAGS=["-flto"])

    ## Architecture
    env["bits"] = "64"
    if env["arch"] != "x86_64":
        env["arch"] = "arm64"

    ## Compiler configuration

    # Save this in environment for use by other modules
    if "OSXCROSS_IOS" in os.environ:
        env["osxcross"] = True

    env["ENV"]["PATH"] = env["IOS_TOOLCHAIN_PATH"] + "/Developer/usr/bin/:" + env["ENV"]["PATH"]

    compiler_path = "$IOS_TOOLCHAIN_PATH/usr/bin/${ios_triple}"
    s_compiler_path = "$IOS_TOOLCHAIN_PATH/Developer/usr/bin/"

    ccache_path = os.environ.get("CCACHE")
    if ccache_path is None:
        env["CC"] = compiler_path + "clang"
        env["CXX"] = compiler_path + "clang++"
        env["S_compiler"] = s_compiler_path + "gcc"
    else:
        # there aren't any ccache wrappers available for iOS,
        # to enable caching we need to prepend the path to the ccache binary
        env["CC"] = ccache_path + " " + compiler_path + "clang"
        env["CXX"] = ccache_path + " " + compiler_path + "clang++"
        env["S_compiler"] = ccache_path + " " + s_compiler_path + "gcc"
    env["AR"] = compiler_path + "ar"
    env["RANLIB"] = compiler_path + "ranlib"

    ## Compile flags

    if env["ios_simulator"]:
        detect_darwin_sdk_path("iossimulator", env)
        env.Append(ASFLAGS=["-mios-simulator-version-min=13.0"])
        env.Append(CCFLAGS=["-mios-simulator-version-min=13.0"])
        env.extra_suffix = ".simulator" + env.extra_suffix
    else:
        detect_darwin_sdk_path("ios", env)
        env.Append(ASFLAGS=["-miphoneos-version-min=11.0"])
        env.Append(CCFLAGS=["-miphoneos-version-min=11.0"])

    if env["arch"] == "x86_64":
        env["ENV"]["MACOSX_DEPLOYMENT_TARGET"] = "10.9"
        env.Append(
            CCFLAGS=(
                "-fobjc-arc -arch x86_64"
                " -fobjc-abi-version=2 -fobjc-legacy-dispatch -fmessage-length=0 -fpascal-strings -fblocks"
                " -fasm-blocks -isysroot $IOS_SDK_PATH"
            ).split()
        )
        env.Append(ASFLAGS=["-arch", "x86_64"])
    elif env["arch"] == "arm64":
        env.Append(
            CCFLAGS=(
                "-fobjc-arc -arch arm64 -fmessage-length=0 -fno-strict-aliasing"
                " -fdiagnostics-print-source-range-info -fdiagnostics-show-category=id -fdiagnostics-parseable-fixits"
                " -fpascal-strings -fblocks -fvisibility=hidden -MMD -MT dependencies"
                " -isysroot $IOS_SDK_PATH".split()
            )
        )
        env.Append(ASFLAGS=["-arch", "arm64"])
        env.Append(CPPDEFINES=["NEED_LONG_INT"])

    # Disable exceptions on non-tools (template) builds
    if not env["tools"]:
        if env["ios_exceptions"]:
            env.Append(CCFLAGS=["-fexceptions"])
        else:
            env.Append(CCFLAGS=["-fno-exceptions"])

    # Temp fix for ABS/MAX/MIN macros in iOS SDK blocking compilation
    env.Append(CCFLAGS=["-Wno-ambiguous-macro"])

    env.Prepend(
        CPPPATH=[
            "$IOS_SDK_PATH/usr/include",
            "$IOS_SDK_PATH/System/Library/Frameworks/AudioUnit.framework/Headers",
        ]
    )

    env.Prepend(CPPPATH=["#platform/ios"])
    env.Append(CPPDEFINES=["IOS_ENABLED", "UNIX_ENABLED", "COREAUDIO_ENABLED"])

    if env["vulkan"]:
        env.Append(CPPDEFINES=["VULKAN_ENABLED"])
