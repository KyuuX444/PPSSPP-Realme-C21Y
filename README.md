# PSP by Kyuu — PPSSPP Edisi Realme C21Y (Extreme Performance & PS3 Original UI)

[![Build Status](https://github.com/KyuuX444/PPSSPP-Realme-C21Y/actions/workflows/android-build.yml/badge.svg?branch=realme-c21y-optimization)](https://github.com/KyuuX444/PPSSPP-Realme-C21Y/actions/workflows/android-build.yml)
[![Release](https://img.shields.io/github/v/release/KyuuX444/PPSSPP-Realme-C21Y?label=Rilis%20APK)](https://github.com/KyuuX444/PPSSPP-Realme-C21Y/releases/latest)
[![Platform](https://img.shields.io/badge/Android-ARM64%20(arm64--v8a)-3DDC84)](#spesifikasi-perangkat-target)
[![License](https://img.shields.io/badge/License-GPL%20v2.0%2B-blue.svg)](LICENSE.TXT)

**PSP by Kyuu** (`PPSSPP Realme C21Y — Extreme Performance Edition`) adalah fork dan modifikasi khusus dari emulator PlayStation Portable (PSP) sumber terbuka **[PPSSPP](https://github.com/hrydgard/ppsspp)** karya **Henrik Rydgård** beserta para kontributornya, yang dirancang dan dikompilasi khusus untuk ponsel **Realme C21Y** (*chipset Unisoc Tiger T610, GPU Mali-G52, ARM64, varian RAM 3 GB & 4 GB*).

> [!NOTE]
> Proyek ini mempertahankan penuh lisensi **GNU General Public License v2.0 atau lebih baru** ([LICENSE.TXT](LICENSE.TXT)), hak cipta serta atribusi pengembang asli PPSSPP (**Henrik Rydgård & Contributors**), dan seluruh komponen inti emulasi PSP (CPU JIT, GPU rendering, audio, controller, *save data*, *save state*, Memory Stick, dan kompatibilitas game).

---

## Tentang Emulator PSP Ini

PPSSPP adalah emulator PSP (*PlayStation Portable*) berbasis **HLE (*High-Level Emulation*)** sehingga Anda dapat menjalankan game PSP secara langsung **tanpa membutuhkan file BIOS eksternal**. Pada edisi **PSP by Kyuu** untuk Realme C21Y ini, konfigurasi bawaan, tema UI ringan bergaya PS3 Original, alokasi memori, pipeline rendering, dan biner APK disesuaikan agar langsung siap pakai, ringan, hemat RAM, dan stabil di perangkat bertenaga Unisoc T610.

### Fitur Inti & Identitas Baru (`v1.1`)
- **Instalasi Side-by-Side (`org.ppsspp.kyuu`)**: memakai `applicationId` terpisah dan **Release keystore persisten** (bukan keystore sementara/ephemeral), sehingga dapat dipasang **berdampingan dengan PPSSPP resmi (`org.ppsspp.ppsspp`)** tanpa harus mencopot versi resmi terlebih dahulu.
- **Custom UI Ringan `PS3 Original`**: tema bawaan `PS3 Original` (`assets/themes/ps3original.ini`) bernuansa gelap *sapphire-obsidian* bergaya PS3 XMB dengan latar gradasi statis + garis horizon tipis (bebas animasi gelombang berat) dan ikon/nama aplikasi **PSP by Kyuu**.
- **Emulasi CPU & JIT ARM64** (`Arm64IRJit`) lengkap dengan percepatan *Vector FPU* (VFPU).
- **Dukungan Format Game PSP** (sesuai `Core/Loaders.cpp`): `.iso`, `.cso`, `.chd`, `EBOOT.PBP` (`.pbp`), `.elf`/`.prx`, serta dukungan berkas terkompresi `.zip`, `.7z`, dan `.rar`.
- **Save Data & Save State**: kompatibel penuh dengan direktori standar Memory Stick (`PSP/SAVEDATA` & `PSP/PPSSPP_STATE`).
- **Kontrol Lengkap**: *on-screen touch controls* yang dapat diatur tata letaknya serta dukungan gamepad eksternal USB/Bluetooth.
- **Dukungan Bahasa UI**: **Bahasa Indonesia (`id_ID`)** dan **Bahasa Inggris (`en_US`)**.

---

## Unduh & Verifikasi APK

APK dikompilasi otomatis menggunakan Android NDK `29.0.14206865` dan diverifikasi tanda tangan digital, metadata paket (`aapt2`), serta isi arsitekturnya oleh GitHub Actions.

| Informasi Rilis | Detail |
| :--- | :--- |
| **Versi Terbaru** | [**Halaman Rilis (`v1.1` / Latest)**](https://github.com/KyuuX444/PPSSPP-Realme-C21Y/releases/latest) |
| **Berkas APK Terbaru** | [`PSP-by-Kyuu-Realme-C21Y-ARM64.apk`](https://github.com/KyuuX444/PPSSPP-Realme-C21Y/releases/latest/download/PSP-by-Kyuu-Realme-C21Y-ARM64.apk) ([`.sha256`](https://github.com/KyuuX444/PPSSPP-Realme-C21Y/releases/latest/download/PSP-by-Kyuu-Realme-C21Y-ARM64.apk.sha256)) |
| **Nama Aplikasi / Package ID** | **`PSP by Kyuu`** / **`org.ppsspp.kyuu`** |
| **Arsitektur** | `arm64-v8a` (khusus 64-bit ARM64 Android) |
| **SHA-256 Sertifikat Keystore Rilis** | `73:7D:53:BE:67:C3:6A:84:8F:2C:91:5A:F5:02:85:5E:5C:09:2B:43:87:66:3F:AE:6D:AB:A3:BC:23:CE:19:7F` (`alias: psp_by_kyuu`) |
| **Rilis Awal (`v1.0`, Arsip)** | [`PPSSPP-Realme-C21Y-ARM64.apk`](https://github.com/KyuuX444/PPSSPP-Realme-C21Y/releases/tag/v1.0) (`20.77 MB`, SHA256 `499fa2a24701042a89a7a8b822c6e0e06257a508852440a33e63e1681f7e6fbe`) |

---

## Cara Pasang & Pakai di Realme C21Y

### 1. Instalasi APK
1. Unduh [`PSP-by-Kyuu-Realme-C21Y-ARM64.apk`](https://github.com/KyuuX444/PPSSPP-Realme-C21Y/releases/latest/download/PSP-by-Kyuu-Realme-C21Y-ARM64.apk) dari halaman **Releases** (`v1.1` atau terbaru).
2. Karena **PSP by Kyuu** menggunakan `applicationId` terpisah (`org.ppsspp.kyuu`), aplikasi ini **dapat diinstal langsung berdampingan** dengan PPSSPP resmi (`org.ppsspp.ppsspp`) maupun PPSSPP Gold tanpa perlu mencopot versi lama. *(Jika sebelumnya Anda memasang rilis lama `v1.0` `org.ppsspp.ppsspp` dari repo ini, aplikasi `v1.1` tetap terpasang terpisah dan tidak bentrok tanda tangan).*
3. Aktifkan izin *Install from Unknown Sources* (*Pasang aplikasi yang tidak dikenal*) di Android.
4. Pasang APK dan buka **PSP by Kyuu**. Berikan izin akses folder agar emulator dapat membuat atau memilih folder Memory Stick (`PSP`).

### 2. Memainkan Game PSP
1. Siapkan file game PSP legal milik Anda berformat `.iso`, `.cso`, `.chd`, atau folder `EBOOT.PBP`.
2. Letakkan di folder yang mudah diakses (misalnya `Internal Storage/PSP/GAME/` atau `Downloads/`), lalu buka dari tab **Games → Browse**.
3. Pengaturan optimal Realme C21Y langsung aktif secara otomatis sejak pertama kali dijalankan.
4. Untuk mengganti mode performa kapan saja, masuk ke **Settings → Graphics → Performance Mode**.

### 3. Rekomendasi Pemakaian Profil & Backend Grafis
- **Balanced (Bawaan)**: direkomendasikan untuk pemakaian harian — menjaga keseimbangan kelancaran 30/60 FPS, akurasi grafis, dan suhu perangkat.
- **Ultra Performance**: gunakan untuk game berat (*God of War*, *Midnight Club*, *Tekken 6*) terutama pada varian **RAM 3 GB** atau saat baterai/suhu tinggi.
- **Compatibility**: gunakan apabila game mengalami glitch grafis, tekstur hilang, atau membutuhkan sinkronisasi readback CPU-GPU ketat.
- **Backend Grafis (Vulkan vs OpenGL ES)**: pada sebagian game atau versi firmware Mali-G52 tertentu, **OpenGL ES** dapat lebih stabil dibanding **Vulkan** (atau sebaliknya). Edisi ini mengaktifkan kembali mekanisme fallback aman `FailedGraphicsBackends.txt` pada Android 11+ khusus Realme C21Y agar perangkat tidak terjebak crash saat berpindah backend.

---

## Spesifikasi Perangkat Target

Optimasi agresif dibatasi secara otomatis hanya untuk perangkat berikut (melalui deteksi `Core/DeviceOptimization.cpp` agar tidak memaksakan profil yang belum diuji ke perangkat lain):

- **Model**: Realme C21Y (`RMX3261`, `RMX3262`, `RMX3263`)
- **SoC / Chipset**: Unisoc Tiger T610 (`ums512` / `t610`)
- **CPU**: Octa-core ARM64 (2× Cortex-A75 + 6× Cortex-A55)
- **GPU**: ARM Mali-G52
- **Arsitektur Biner**: `arm64-v8a`
- **RAM**: Varian **3 GB** (`<= 3400 MB`, mode proteksi memori ketat) dan **4 GB** (`<= 4500 MB`), dideteksi via Android `ActivityManager.MemoryInfo` dengan fallback `/proc/meminfo`
- **Resolusi Layar**: 1600 × 720, Android 10–12

---

## Tiga Profil Performa (`Performance Mode`)

Pengguna dapat memilih tiga profil langsung dari **Settings → Graphics → Performance Mode**:

| Parameter | `Balanced` *(Default)* | `Ultra Performance` | `Compatibility` |
| :--- | :--- | :--- | :--- |
| **Rendering Resolution** | `1x PSP` | `1x PSP` | `1x PSP` |
| **Frameskip** | Auto (`1`) | Auto (`1`) | Off (`0`) |
| **Texture Filtering** | Auto | Nearest (Performa) | Auto |
| **Anisotropic Filtering** | Off (`0`) | Off (`0`) | Off (`0`) |
| **Post-Processing Shader** | Off | Off | Off |
| **VSync / Present** | On + Low Latency | Off + Low Latency | On (FIFO Standar) |
| **Buffered Rendering** | Enabled | Enabled | Enabled |
| **Hardware Transform** | Enabled | Enabled | Enabled |
| **Fast Memory** | Enabled (jika JIT siap) | Enabled (jika JIT siap) | Disabled (Aman) |
| **Depth Raster Mode** | `Low Quality` (ARM64 NEON) | `OFF` | `Default` |
| **Bloom Hack** | Off (`0`) | `1` | Off (`0`) |
| **GPU Readback Mode** | Default profil | Default profil | `NO_SKIP` (Akurat) |
| **UI Background Animation** | Off pada varian 3 GB | Off | Tidak diubah |
| **Audio Sync Mode** | `Granular` (`AAudio` MMap) | `Granular` (`AAudio` MMap) | `Granular` (`AAudio` MMap) |

---

## Ringkasan Optimasi Teknis di Source Code

Dokumen analisis lengkap pra-optimasi tersedia di [`docs/REALME_C21Y_ANALYSIS.md`](docs/REALME_C21Y_ANALYSIS.md) dan catatan riwayat perubahan di [`CHANGELOG.md`](CHANGELOG.md).

### 1. CPU, ARM64 NEON SIMD & Compiler
- **NEON SIMD Pixel Conversion** (`Common/Data/Convert/ColorConv.cpp`): implementasi kernel ARM64 NEON menggunakan `vqtbl1q_u8` (`ConvertBGRA8888ToRGBA8888`, aman untuk operasi *in-place*) serta `vld2q_u8`/`vst4q_u8` (`ConvertRGB565ToRGBA8888`, `ConvertRGBA5551ToRGBA8888`, `ConvertRGBA4444ToRGBA8888`), dilengkapi unit test pada `unittest/UnitTest.cpp`.
- **Tuning Compiler & Linker T610** (`CMakeLists.txt`): opsi `REALME_C21Y_TUNE` mengaktifkan `-O3 -mtune=cortex-a75 -fomit-frame-pointer -ffunction-sections -fdata-sections`, link `--gc-sections --icf=safe -O1`, serta `USE_THIN_LTO` (`-flto=thin`) pada build `Release`/`RelWithDebInfo` dengan tetap mempertahankan baseline `-march=armv8-a` yang aman tanpa instruksi ilegal.
- **Pemangkasan Logging Release** (`Common/Log.h`, `Core/Config.cpp`): menaikkan batas log Release Android ke `LogLevel::LWARNING` (`MAX_LOGLEVEL INFO_LEVEL`) dan default `bEnableLogging = false`.
- **ADPF & Pemantauan Suhu** (`android/jni/PerformanceHint.*`, `PowerSaveModeReceiver.java`): dukungan API publik `PowerManager.OnThermalStatusChangedListener` (API 29+) dan `APerformanceHint_updateTargetWorkDuration` dengan batas termal `8.33 ms` ↔ `11.5 ms` serta penyaringan lonjakan waktu frame saat resume.

### 2. GPU Mali-G52 & Manajemen RAM 3 GB / 4 GB
- **Batas Blok VMA & PushPool UMA** (`Common/GPU/Vulkan/VulkanContext.cpp`, `thin3d_vulkan.cpp`, `GPU/Vulkan/DrawEngineVulkan.cpp`): membatasi `preferredLargeHeapBlockSize` menjadi **16 MB** (RAM ≤ 3400 MB) atau **32 MB** (RAM ≤ 4500 MB) dibanding default 256 MB, dan mengecilkan ukuran blok awal `VulkanPushPool` (`2 MB` untuk `pushBuffer`/`pushVertex_`, `256 KB` untuk `pushIndex_`) sambil tetap mendukung ekspansi dinamis.
- **Cache Tekstur Adaptif & `onTrimMemory`** (`GPU/Common/TextureCacheCommon.*`, `GPU/GPUCommon*`, `UI/NativeApp.cpp`): menurunkan ambang batas eviksi tekstur ke **48 MB / 16 MB** (3 GB RAM atau termal berat) dan **80 MB / 24 MB** (4 GB RAM) dengan *kill age* lebih ketat (`120`/`60` frame), serta memicu pembersihan aman cache latar & tekstur ketika Android mengirim `onTrimMemory`.
- **Fallback Backend Vulkan ↔ OpenGL ES**: mengaktifkan kembali pengecekan `FailedGraphicsBackends.txt` pada Android 11+ khusus Realme C21Y.

### 3. Custom UI Ringan `PS3 Original` & Build `realmeC21y` (`PSP by Kyuu`)
- **Identitas & Resource Overlay** (`android/build.gradle.kts`, `android/realmeC21y/res/`): mengubah `applicationId` flavor `realmeC21y` ke `org.ppsspp.kyuu` agar bisa berdampingan dengan PPSSPP resmi, mengganti `app_name` ke `PSP by Kyuu`, serta menyertakan ikon launcher/adaptif/TV banner kustom.
- **Tema `PS3 Original` & Latar Statis Ringan** (`assets/themes/ps3original.ini`, `Core/Config.cpp`, `UI/Background.cpp`): menerapkan palet gelap *sapphire-obsidian* dengan radius sudut ringkas, `DefaultBackgroundAnimation()` -> `OFF`, dan gradasi XMB + garis horizon 4-segitiga tanpa shader gelombang berat.
- **Identitas & Atribusi Pembuat Asli** (`UI/MainScreen.cpp`, `UI/MiscScreens.cpp`, `UI/SystemInfoScreen.cpp`): menampilkan nama `PSP by Kyuu` sambil mempertahankan logo, lisensi **GPL v2.0+**, dan kredit asli **Henrik Rydgård & Contributors** secara utuh.
- **Single ABI `arm64-v8a`**: tidak menyertakan native library `armeabi-v7a`, `x86`, atau `x86_64`.
- **Penonaktifan Modul Non-Esensial**: `-DUSE_DISCORD=OFF`, `-DUSE_ADRENOTOOLS=OFF` (GPU Mali-G52 tidak memakai driver kustom Qualcomm Adreno), `-DHEADLESS=OFF`, `-DUNITTEST=OFF`.
- **R8 Shrinking & Pemangkasan Aset**: `isMinifyEnabled = true`, `isShrinkResources = true` dengan `android/proguard-rules.pro`, serta membatasi aset `realmeC21y` tanpa menyentuh file sumber upstream (`compatvr.ini`, `redump.csv`, dan bahasa selain `en_US.ini` & `id_ID.ini` dikecualikan dari paket).
- **Manifest Overlay** (`android/realmeC21y/AndroidManifest.xml`): menghapus deklarasi/izin yang tidak dibutuhkan pada Realme C21Y (`TRANSMIT_IR`, `ACCESS_FINE_LOCATION`, `ACCESS_COARSE_LOCATION`, `archos.permission.FULLSCREEN.FULL`, `hardware.type.pc`, `software.leanback`, `<profileable>`).

---

## Build Otomatis & Kompilasi Lokal

### GitHub Actions CI/CD
Workflow [`.github/workflows/android-build.yml`](.github/workflows/android-build.yml) otomatis menjalankan build **`PSP-by-Kyuu-Realme-C21Y-ARM64.apk`** pada setiap push ke branch `realme-c21y-optimization`, `master`, tag `v*`, pull request, atau eksekusi manual (`workflow_dispatch` untuk `Release`/`Debug`).

### Kompilasi Lokal dengan Gradle
Pastikan Android SDK, NDK `29.0.14206865`, CMake `3.22.1+`, JDK 17, dan submodule (`git submodule update --init --recursive`) sudah tersedia:

```bash
# Build Debug ARM64 (realmeC21y flavor)
./gradlew :android:assembleRealmeC21yDebug

# Build Release ARM64 (memerlukan keystore penandatanganan)
./gradlew :android:assembleRealmeC21yRelease \
  -PRELEASE_STORE_FILE=/path/ke/release.keystore \
  -PRELEASE_STORE_PASSWORD=*** \
  -PRELEASE_KEY_ALIAS=*** \
  -PRELEASE_KEY_PASSWORD=***
```

---

## Catatan Verifikasi & Kredit

- **Yang sudah diverifikasi**: kompilasi penuh APK `realmeC21yRelease` ARM64 lewat GitHub Actions, kelengkapan isi APK (`libppsspp_jni.so` ARM64-only, hanya bahasa `en_US` & `id_ID`, `assets/themes/ps3original.ini`, minifikasi R8 `classes.dex`), metadata `aapt2` (`org.ppsspp.kyuu` / `PSP by Kyuu`), SHA256 checksum, dan verifikasi sertifikat `apksigner`.
- **Keterbatasan pengujian**: benchmark FPS nyata pada game berat, suhu perangkat, serta ketahanan baterai **belum dijalankan langsung pada unit fisik Realme C21Y** karena tidak ada perangkat Android fisik yang terhubung di lingkungan build ini.
- **Kredit Proyek Asli**: seluruh fondasi emulator berasal dari [PPSSPP](https://www.ppsspp.org/) ([hrydgard/ppsspp](https://github.com/hrydgard/ppsspp)) karya **Henrik Rydgård** dan para kontributor di bawah lisensi **GNU General Public License v2.0+** ([`LICENSE.TXT`](LICENSE.TXT)).
