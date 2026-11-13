# Pre-Optimization Source Code Analysis — PPSSPP Realme C21Y Edition

This document records the concrete architectural findings and performance/memory/size bottlenecks identified in the upstream PPSSPP (`v1.20.4+`) codebase prior to applying device-specific optimizations for **Realme C21Y** (Unisoc Tiger T610: 2x Cortex-A75 + 6x Cortex-A55 ARM64, ARM Mali-G52 GPU, 3 GB & 4 GB RAM variants, 1600x720 display, Android 10–12).

---

## 1. Compiler Toolchain, Build & Binary Size (`CMakeLists.txt`, `ext/CMakeLists.txt`, `android/build.gradle.kts`)

1. **Missing ARM64-specific compiler/linker optimizations on Android**:
   - Upstream `CMakeLists.txt` does not tune Release builds specifically for Unisoc T610's DynamIQ big.LITTLE pair (`-mtune=cortex-a75.cortex-a55`), nor does it consistently enforce `-O3`, ThinLTO (`-flto=thin`), section GC (`-ffunction-sections -fdata-sections -Wl,--gc-sections`), or safe ICF (`-Wl,--icf=safe`).
   - Note: `-march=armv8-a` must remain the instruction baseline (avoiding forced `dotprod`) so binaries execute safely across all Unisoc T610 silicon steppings without `SIGILL`.
2. **Unnecessary native libraries compiled & linked on Mali-G52 Android**:
   - `CMakeLists.txt` defaults `option(USE_DISCORD "Build with Discord support" ON)` and defines `USE_DISCORD=1` even on Android, causing `ext/CMakeLists.txt` (`ext/discord-rpc-build`) to be compiled unnecessarily even though Discord RPC is inactive on Android.
   - `ext/CMakeLists.txt` unconditionally builds `ext/libadrenotools` for all `ANDROID && ARM64` builds and `Common/CMakeLists.txt` links `adrenotools` directly into `Common` (`Common/GPU/Vulkan/VulkanLoader.cpp`). Realme C21Y uses an ARM Mali-G52 GPU, not Qualcomm Adreno, so custom Adreno driver loading is dead code (~linkernsbypass/elf_soname_patcher overhead) for this target.
   - OpenXR (`OpenXR-SDK`) VR integration adds binary/runtime weight unrelated to Realme C21Y.
3. **Android Gradle packaging & R8/ABI bloat (`android/build.gradle.kts`)**:
   - Upstream `normal` flavor compiles 3 ABIs (`armeabi-v7a`, `arm64-v8a`, `x86_64`), tripling native `.so` size vs an `arm64-v8a`-only Realme C21Y build.
   - `buildTypes.release` sets `isMinifyEnabled = false` (no R8 code shrinking/resource shrinking).
   - Release signing relies strictly on `RELEASE_STORE_FILE` without CI fallback keystore provisioning.

---

## 2. CPU, ARM64 JIT & NEON SIMD (`Core/MIPS/ARM64/`, `Common/Data/Convert/ColorConv.cpp`, `android/jni/PerformanceHint.cpp`)

1. **ARM64 JIT (`Arm64IRJit`)**:
   - Already implements IR-to-ARM64 compilation, VFPU NEON dot-product helpers (`Arm64VfpuDot.h`), and vector compilation (`Arm64IRCompVec.cpp`), plus partial `TryCompileLoadStorePair` (`Arm64IRCompLoadStore.cpp`).
2. **Unvectorized pixel format conversions on ARM64 NEON (`Common/Data/Convert/ColorConv.cpp`)**:
   - `ConvertBGRA8888ToRGBA8888`, `ConvertRGB565ToRGBA8888`, `ConvertRGBA5551ToRGBA8888`, and `ConvertRGBA4444ToRGBA8888` all contain `SSE2` SIMD paths for x86 but fall back to **scalar C loops** on ARM64 NEON, impacting software texture decoding and framebuffer upload throughput on Cortex-A75/A55.
3. **Android Dynamic Performance Framework (`android/jni/PerformanceHint.cpp`)**:
   - ADPF hint session scheduling (`reportActualWorkDuration` / target work duration) should align smoothly with target PSP frame timing and thermal state rather than fixed assumptions.
4. **Release Logging Overhead (`Common/Log.h`, `Core/Config.cpp`)**:
   - `Common/Log.h` defines `#define MAX_LOGLEVEL DEBUG_LEVEL` and initializes `LogChannel::level = LogLevel::LDEBUG` even when `!defined(_DEBUG)` (Release).
   - `Core/Config.cpp` defaults `bEnableLogging = true` (`"Enable Logging"`), causing hot paths to check log levels, retain debug strings/file metadata, and write `__android_log_print` output during gameplay.

---

## 3. GPU — ARM Mali-G52 Vulkan & OpenGL ES (`Common/GPU/`, `GPU/`, `UI/NativeApp.cpp`)

1. **Default resolution & anisotropy too heavy for Mali-G52 MP2 at 1600x720**:
   - `Core/Config.cpp` (`DefaultInternalResolution()`) selects `2x PSP` (`longestDisplaySide >= 1000` on Android, triggered by 1600x720) instead of `1x PSP`.
   - `AnisotropyLevel` defaults to `4` (`16x` anisotropic filtering), `bAutoFrameSkip` defaults to `false`, and `bSustainedPerformanceMode` defaults to `false`.
2. **Android 11+ Vulkan backend crash fallback blocked (`UI/NativeApp.cpp`)**:
   - `CheckFailedGPUBackends()` returns early on `SYSPROP_SYSTEMVERSION >= 30` unless `g_Config.sCustomDriver` is non-empty. If stock Mali Vulkan fails on a Realme C21Y firmware update, automatic fallback to OpenGL ES via `FailedGraphicsBackends.txt` is bypassed.
3. **Excessive contiguous Vulkan memory reservations for 3 GB / 4 GB UMA (`Common/GPU/Vulkan/VulkanContext.cpp`, `GPU/Vulkan/DrawEngineVulkan.cpp`)**:
   - `VmaAllocatorCreateInfo` leaves `preferredLargeHeapBlockSize = 0` (VMA defaults to 256 MB large blocks), excessive for a 3 GB unified-memory phone.
   - `DrawEngineVulkan` allocates `4 MB` (`pushVertex_`) + `512 KB` (`pushIndex_`) per-pool plus `VulkanPushPool` for UBOs without UMA-conscious scaling on low-RAM Android devices.
4. **Mali-G52 classification (`VulkanContext.cpp`)**:
   - `VulkanContext.cpp` (`VULKAN_VENDOR_ARM`, `maliG < 72`) classifies `Mali-G52` as `PerfClass::SLOW`, affecting feature heuristics; pairing Vulkan/GLES auto-selection with proper framebuffer/invalidation and adaptive texture budgets is essential.

---

## 4. RAM & Cache Management (3 GB vs 4 GB Variants)

1. **Missing total RAM detection (`android/jni/app-android.cpp`, `Common/System/System.h`)**:
   - `SYSPROP_ENOUGH_RAM_FOR_FULL_ISO` is unimplemented (`false`) in `android/jni/app-android.cpp`, and there is no runtime distinction between Realme C21Y's **3 GB** and **4 GB** RAM variants.
2. **Fixed texture cache eviction thresholds (`GPU/Common/TextureCacheCommon.cpp`)**:
   - `TEXCACHE_MIN_PRESSURE = 128 * 1024 * 1024`, `TEXCACHE_SECOND_MIN_PRESSURE = 32 * 1024 * 1024`, and `TEXTURE_KILL_AGE = 200` are static regardless of available system RAM or Android memory pressure (`onTrimMemory()`). On a 3 GB Realme C21Y (typically ~1.1–1.5 GB free RAM under Android), 128 MB+ texture retention plus decoded buffers risks Android Low Memory Killer (LMK) termination.

---

## 5. Audio (`android/jni/OboeContext.cpp`, `Core/HW/StereoResampler.cpp`)

- Oboe (`AAudio` / OpenSL ES fallback) handles Android low-latency audio output, and `StereoResampler.cpp` already uses NEON `ClampBufferToS16`.
- We should ensure Realme C21Y defaults enable low-latency audio buffering without crackles under high CPU load and integrate cleanly with `PerformanceProfile` presets.

---

## 6. Android Manifest, Lifecycle & Asset Bloat (`android/AndroidManifest.xml`, `assets/`, `Core/Config.cpp`)

1. **Manifest & Permissions**:
   - Requests unnecessary permissions/hardware declarations for Realme C21Y gaming: `android.permission.TRANSMIT_IR`, `android.permission.ACCESS_FINE_LOCATION` / `ACCESS_COARSE_LOCATION`, `archos.permission.FULLSCREEN.FULL`, desktop/TV features (`android.hardware.type.pc`, `android.software.leanback`), `android.hardware.camera`, and includes `<profileable>` overhead.
2. **Packaged Asset Bloat (~17 MB uncompressed in `assets/`)**:
   - `assets/lang/` (`4.8 MB` across ~50 language `.ini` files; keeping `en_US.ini` `77 KB` + `id_ID.ini` `89 KB` saves ~4.6 MB uncompressed while preserving full English and Indonesian UI).
   - `assets/redump.csv` (`554 KB`, info overlay DB only; `GameDB::LoadIfNeeded` safely returns if absent).
   - `assets/compatvr.ini` (`25 KB`, VR compatibility list unused when OpenXR/VR is disabled).
3. **Default UI Overhead**:
   - `iBackgroundAnimation` defaults to `BackgroundAnimation::WAVE` in `Core/Config.cpp`, rendering continuous background shaders/waves in menus.

---

## 7. Planned Implementation Phases

1. **Hardware Detection, Realme C21Y Defaults, 3 Profiles & Thermal Monitor**:
   - Public Android SDK RAM (`ActivityManager.MemoryInfo`), device/SoC (`Build.MODEL`, `Build.HARDWARE`, `Build.SOC_MODEL` on API 31+ / `/proc/cpuinfo`), GPU renderer detection, and `PowerManager` thermal status monitoring (API 29+) with smooth sustained-performance throttling.
   - Add `ULTRA PERFORMANCE`, `BALANCED`, and `COMPATIBILITY` profiles exposed in UI Settings and `Core/Config`.
2. **CPU, ARM64 NEON SIMD & Release Logging Optimizations**:
   - Implement NEON vector kernels in `Common/Data/Convert/ColorConv.cpp` (`BGRA8888`, `RGB565`, `RGBA5551`, `RGBA4444` -> `RGBA8888`) and verify correctness.
   - Lower Release `MAX_LOGLEVEL` and default `bEnableLogging = false`; tune `PerformanceHint` + ARM64 Release CMake flags (`-O3`, `-mtune=cortex-a75.cortex-a55`, ThinLTO, section GC).
3. **Mali-G52 GPU & Adaptive 3 GB / 4 GB RAM Management**:
   - Fix Android 11+ stock Vulkan fallback check, scale VMA block size & Vulkan push pools on UMA Android, add RAM-aware `TextureCacheCommon` budgets (`3 GB` strict vs `4 GB` balanced), and wire Android `onTrimMemory()` to purge idle caches cleanly.
4. **Lightweight Build, Bloat Removal & GitHub Actions CI/CD**:
   - Optional CMake switches (`-DUSE_DISCORD=OFF`, `-DUSE_ADRENOTOOLS=OFF`, `-DENABLE_VR=OFF`), `arm64-v8a`-focused Realme C21Y Gradle configuration with R8 minification/shrinking, safe manifest/asset cleanup, Indonesian `README.md`, `CHANGELOG.md`, and automated Release APK workflow (`.github/workflows/android-build.yml`).
