# Changelog — PPSSPP Realme C21Y (Extreme Performance Edition)

Semua perubahan dibangun di atas basis source code resmi [hrydgard/ppsspp](https://github.com/hrydgard/ppsspp) (basis `v1.20.4+`) pada branch `realme-c21y-optimization` dengan mempertahankan lisensi GPL v2.0+ dan kompatibilitas inti emulator PSP.

## [v1.0] — Realme C21Y Extreme Performance Edition

### Ditambahkan
- **`docs/REALME_C21Y_ANALYSIS.md`**: Laporan analisis arsitektur CPU/JIT, GPU, audio, RAM, kode Android, dan aset sebelum modifikasi kode.
- **`Core/DeviceOptimization.h`, `Core/DeviceOptimization.cpp`**:
  - Deteksi otomatis perangkat Realme C21Y (`RMX3261/3262/3263`), chipset Unisoc T610 (`ums512`/`t610`), dan kapasitas RAM varian 3 GB vs 4 GB.
  - Tiga profil performa pengguna: `ULTRA_PERFORMANCE`, `BALANCED`, dan `COMPATIBILITY` (`ApplyPerformanceProfile`).
  - Pemantauan status termal (`UpdateThermalStatus`, `IsThermalSustainedPerfForced`, `IsThermalThrottlingSevere`).
- **Integrasi Android Publik (`PowerSaveModeReceiver.java`, `PpssppActivity.java`, `app-android.cpp`)**:
  - Registrasi `PowerManager.OnThermalStatusChangedListener` (API 29+) dan `onTrimMemory(int)`.
  - Pengiriman informasi RAM & SoC ke native sebelum inisialisasi inti emulator.
- **UI Pengaturan & Info Sistem**:
  - Pilihan **Performance Mode** di `UI/Settings/GraphicsPage.cpp`.
  - Tampilan **Total RAM**, **Hardware Profile**, dan **Thermal Status** di `UI/SystemInfoScreen.cpp`.
- **Flavor Build & Keamanan Rilis**:
  - Flavor Gradle `realmeC21y` (`arm64-v8a` only) beserta `android/proguard-rules.pro` dan overlay `android/realmeC21y/AndroidManifest.xml`.
  - Workflow CI/CD otomatis `.github/workflows/android-build.yml` dan dokumentasi `README.md`.

### Dioptimalkan
- **CPU & ARM64 NEON SIMD (`Common/Data/Convert/ColorConv.cpp`, `unittest/UnitTest.cpp`)**:
  - Akselerasi ARM64 NEON (`vqtbl1q_u8`, `vld2q_u8`/`vst4q_u8`) untuk konversi pixel `BGRA8888 -> RGBA8888`, `RGB565 -> RGBA8888`, `RGBA5551 -> RGBA8888`, dan `RGBA4444 -> RGBA8888`, dilengkapi pengujian unit.
- **Compiler & Linker (`CMakeLists.txt`)**:
  - Opsi `REALME_C21Y_TUNE` (`-O3 -mtune=cortex-a75 -fomit-frame-pointer -ffunction-sections -fdata-sections`, `--gc-sections`, `--icf=safe`) dan `USE_THIN_LTO` (`-flto=thin`).
- **ADPF & Logging Release (`android/jni/PerformanceHint.*`, `Common/Log.h`, `Core/Config.cpp`)**:
  - Dukungan dinamis `APerformanceHint_updateTargetWorkDuration` dengan batas termal `8.33 ms` ↔ `11.5 ms` serta penyaringan lonjakan frame resume `>= 200 ms`.
  - Pembatasan log verbose pada build Release Android (`LogLevel::LWARNING`, `MAX_LOGLEVEL INFO_LEVEL`, `bEnableLogging=false`).
- **GPU Mali-G52 & Manajemen Memori Adaptif (`Common/GPU/Vulkan/*`, `GPU/Vulkan/DrawEngineVulkan.cpp`, `GPU/Common/TextureCacheCommon.*`, `GPU/GPUCommon*`, `UI/NativeApp.cpp`)**:
  - Pembatasan `preferredLargeHeapBlockSize` VMA ke 16 MB (3 GB RAM) / 32 MB (4 GB RAM).
  - Penyesuaian blok `VulkanPushPool` (`pushBuffer`, `pushVertex`, `pushIndex`) pada varian RAM rendah.
  - Ambang batas eviksi cache tekstur adaptif (48 MB/16 MB pada 3 GB atau termal berat, 80 MB/24 MB pada 4 GB) dan eviksi cepat pada `TRIM_MEMORY`.
  - Fallback backend Vulkan ↔ OpenGL ES yang aman khusus perangkat Realme C21Y di Android 11+.

### Dihapus / Dipangkas pada Varian `realmeC21y`
- ABI non-ARM64 (`armeabi-v7a`, `x86_64`).
- Dependensi build opsional: Discord RPC (`USE_DISCORD=OFF`), libadrenotools (`USE_ADRENOTOOLS=OFF`), target headless & unittest (`HEADLESS=OFF`, `UNITTEST=OFF`).
- Bytecode/resource Java tak terpakai via R8 (`isMinifyEnabled = true`, `isShrinkResources = true`).
- Aset paket `compatvr.ini`, `redump.csv`, `lang/README.md`, serta terjemahan selain `en_US.ini` dan `id_ID.ini`.
- Izin/fitur manifest tak relevan pada ponsel Realme C21Y (`TRANSMIT_IR`, `archos.permission.FULLSCREEN.FULL`, lokasi/GPS, `type.pc`, `leanback`, `<profileable>`).
