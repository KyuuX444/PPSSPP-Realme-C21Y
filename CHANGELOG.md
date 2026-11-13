# Changelog — PPSSPP Realme C21Y (Extreme Performance Edition)

Semua perubahan dibangun di atas basis source code resmi [hrydgard/ppsspp](https://github.com/hrydgard/ppsspp) (basis `v1.20.4+`) pada branch `realme-c21y-optimization` dengan mempertahankan lisensi GPL v2.0+ dan kompatibilitas inti emulator PSP.

## [v1.1] — PSP by Kyuu (`org.ppsspp.kyuu`) & PS3 Original Lightweight UI

### Ditambahkan & Diubah
- **Identitas Aplikasi & Instalasi Side-by-Side (`android/build.gradle.kts`, `android/realmeC21y/res/`)**:
  - Mengubah `applicationId` flavor `realmeC21y` menjadi **`org.ppsspp.kyuu`** (dengan mempertahankan `namespace = "org.ppsspp.ppsspp"` agar binding JNI tetap utuh) sehingga dapat diinstal berdampingan (*side-by-side*) dengan PPSSPP resmi (`org.ppsspp.ppsspp`).
  - Mengganti nama aplikasi menjadi **`PSP by Kyuu`** (`android/realmeC21y/res/values/strings.xml`) beserta ikon launcher, ikon bulat, adaptive foreground/background (`#000000`), dan Android TV banner (`ic_banner.png`).
- **Keystore Rilis Persisten (`RELEASE_STORE_*` Secrets & `.github/workflows/android-build.yml`)**:
  - Penandatanganan APK Release kini memakai keystore PKCS12 persisten (bukan keystore ephemeral per-build), dengan sidik jari sertifikat SHA-256 `73:7D:53:BE:67:C3:6A:84:8F:2C:91:5A:F5:02:85:5E:5C:09:2B:43:87:66:3F:AE:6D:AB:A3:BC:23:CE:19:7F` (`alias: psp_by_kyuu`).
  - Penambahan verifikasi `aapt2 dump badging` (`org.ppsspp.kyuu` / `PSP by Kyuu`), `apksigner verify --verbose --print-certs`, dan pemeriksaan `assets/themes/ps3original.ini` di workflow CI.
- **Custom UI Ringan `PS3 Original` & Atribusi Pembuat Asli**:
  - Menambahkan tema **`assets/themes/ps3original.ini`** (palet gelap *sapphire-obsidian* bergaya PS3 XMB) beserta entri terjemahan `[Themes]` di `assets/lang/en_US.ini` dan `assets/lang/id_ID.ini`.
  - Menetapkan `DefaultThemeName()` ke `"PS3 Original"` dan `DefaultBackgroundAnimation()` ke `OFF` saat `REALME_C21Y_EDITION=1` (`CMakeLists.txt` + `Core/Config.cpp`).
  - Menambahkan rendering latar gradasi statis + garis horizon tipis bergaya PS3 XMB di `UI/Background.cpp` (tanpa animasi gelombang/shader berat).
  - Menampilkan identitas **`PSP by Kyuu`** dan informasi basis **`PPSSPP`** / **`Henrik Rydgård & Contributors`** di `UI/MainScreen.cpp`, `UI/MiscScreens.cpp`, dan `UI/SystemInfoScreen.cpp`.

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
