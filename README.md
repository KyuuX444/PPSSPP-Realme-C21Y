# PPSSPP — Edisi Realme C21Y (Extreme Performance Edition)

Fork dan modifikasi khusus dari emulator PSP sumber terbuka **[PPSSPP](https://github.com/hrydgard/ppsspp)** karya **Henrik Rydgård** beserta para kontributornya, dioptimalkan secara khusus untuk perangkat **Realme C21Y**.

> [!NOTE]
> Proyek ini mempertahankan penuh lisensi **GNU General Public License v2.0 atau lebih baru** (`LICENSE.TXT`), hak cipta pengembang asli PPSSPP, serta kompatibilitas inti CPU/JIT, GPU, audio, *save data*, *save state*, dan Memory Stick.

---

## Target Perangkat Keras

- **Perangkat**: Realme C21Y (`RMX3261`, `RMX3262`, `RMX3263`)
- **SoC / Chipset**: Unisoc Tiger T610 (`ums512` / `t610`)
- **CPU**: Octa-core ARM64 (2× Cortex-A75 + 6× Cortex-A55)
- **GPU**: ARM Mali-G52
- **Arsitektur**: `arm64-v8a`
- **RAM**: Varian **3 GB** dan **4 GB** (deteksi otomatis saat runtime)
- **Layar**: 1600 × 720, Android 10–12

Pengaturan optimasi agresif dibatasi pada perangkat Realme C21Y / Unisoc T610 yang terdeteksi atau varian RAM rendah Android agar tidak memaksakan konfigurasi yang belum diuji pada perangkat lain.

---

## Ringkasan Optimasi yang Diterapkan

### 1. Deteksi Hardware Otomatis, Profil Performa & Pemantauan Termal
- Deteksi model Realme C21Y (`RMX3261/3262/3263`), chipset `ums512`/`t610`, serta kapasitas RAM total melalui Android `ActivityManager.MemoryInfo` (dengan fallback `/proc/meminfo` & `/proc/cpuinfo` di native).
- Penyesuaian otomatis batas RAM antara varian **3 GB** (`<= 3400 MB`) dan **4 GB** (`3.5–4.5 GB`), termasuk pembatasan `SYSPROP_ENOUGH_RAM_FOR_FULL_ISO`.
- Tiga profil performa yang dapat diubah pengguna pada menu **Settings → Graphics → Performance Mode**:
  - **Ultra Performance**: resolusi `1x PSP`, Auto Frameskip (1), Texture Filtering Nearest, tanpa VSync, tanpa animasi latar, Bloom Hack `1`, Depth Raster `OFF`, Audio `Granular`.
  - **Balanced** *(default Realme C21Y)*: resolusi `1x PSP`, Auto Frameskip (1), Texture Filtering Auto, VSync aktif dengan presentasi latensi rendah (`bLowLatencyPresent`), Depth Raster `Low Quality` di ARM64 NEON, Audio `Granular`.
  - **Compatibility**: Auto Frameskip nonaktif, Fast Memory nonaktif, Depth Raster & GPU Readback standar untuk game yang membutuhkan akurasi tinggi.
- Integrasi API Android publik `PowerManager.OnThermalStatusChangedListener` (API 29+) dan Android `PerformanceHint` (ADPF): menurunkan target work duration saat *throttling* berat (`11.5 ms`) serta memberi sinyal pemangkasan cache tekstur lebih agresif.

### 2. Optimasi CPU & ARM64 NEON SIMD
- Implementasi kernel **ARM64 NEON SIMD** pada `Common/Data/Convert/ColorConv.cpp` (`ConvertBGRA8888ToRGBA8888` menggunakan `vqtbl1q_u8` termasuk aman in-place, serta `ConvertRGB565ToRGBA8888`, `ConvertRGBA5551ToRGBA8888`, dan `ConvertRGBA4444ToRGBA8888` menggunakan `vld2q_u8`/`vst4q_u8`), disertai cakupan unit test di `unittest/UnitTest.cpp`.
- Opsi CMake `REALME_C21Y_TUNE` (`-O3 -mtune=cortex-a75.cortex-a55 -fomit-frame-pointer -ffunction-sections -fdata-sections`), linker `--gc-sections --icf=safe -O1`, serta `USE_THIN_LTO` (`-flto=thin`) pada build Release dengan ABI baseline aman `-march=armv8-a`.
- Pengurangan overhead logging pada build Release Android (`LogLevel::LWARNING`, `MAX_LOGLEVEL INFO_LEVEL`, `bEnableLogging=false` default).

### 3. Optimasi GPU Mali-G52 & Manajemen RAM Adaptif
- Pembatasan `preferredLargeHeapBlockSize` pada Vulkan Memory Allocator (VMA): **16 MB** untuk varian RAM 3 GB (`<= 3400 MB`) dan **32 MB** untuk varian 4 GB (`<= 4500 MB`) guna mencegah satu alokasi mengunci blok memori 256 MB.
- Pengecilan blok awal `VulkanPushPool` (`pushBuffer`, `pushVertex`, `pushIndex`) dari 4 MB menjadi 2 MB (dan index 512 KB menjadi 256 KB) pada varian RAM rendah sambil tetap mendukung ekspansi dinamis bila dibutuhkan.
- Ambang batas eviksi `TextureCacheCommon` adaptif: **48 MB / 16 MB** (3 GB atau termal berat) dan **80 MB / 24 MB** (4 GB) dibanding bawaan 128 MB / 32 MB, serta *kill age* lebih ketat (`120`/`60` frame) pada RAM 3 GB / termal berat.
- Handler `Activity.onTrimMemory()` yang diteruskan secara aman melalui `UIMessage::TRIM_MEMORY` (`clearCacheNextFrame_` atomik pada `TextureCacheCommon` dan pembebasan latar `GameInfoCache::FlushBGs()` / `Clear()`).
- Mengaktifkan kembali mekanisme fallback aman `FailedGraphicsBackends.txt` (Vulkan ↔ OpenGL ES) pada Android 11+ khusus untuk Realme C21Y agar tidak memaksakan satu backend jika driver bermasalah.

### 4. Pemangkasan Fitur & Ukuran APK (`realmeC21y` Build Flavor)
- Kompilasi ABI tunggal **`arm64-v8a`** (menghapus binary `armeabi-v7a` dan `x86_64`).
- Menonaktifkan dependensi non-esensial pada flavor `realmeC21y`: `-DUSE_DISCORD=OFF`, `-DUSE_ADRENOTOOLS=OFF` (loader khusus GPU Adreno tidak diperlukan pada Mali-G52), `-DHEADLESS=OFF`, `-DUNITTEST=OFF`.
- R8 *code shrinking* (`isMinifyEnabled = true`) & *resource shrinking* (`isShrinkResources = true`) dengan `android/proguard-rules.pro`.
- Pemangkasan aset paket pada varian `realmeC21y` tanpa mengubah aset sumber upstream: mengecualikan `compatvr.ini`, `redump.csv`, dan terjemahan selain `en_US.ini` & `id_ID.ini`.
- Overlay manifest `android/realmeC21y/AndroidManifest.xml` menghapus izin/fitur yang tidak relevan di Realme C21Y (`TRANSMIT_IR`, `archos.permission.FULLSCREEN.FULL`, `ACCESS_FINE_LOCATION`, `ACCESS_COARSE_LOCATION`, `hardware.type.pc`, `software.leanback`, `<profileable>`).

---

## Cara Build & Unduh APK

### Melalui GitHub Actions (Otomatis)
Workflow [`.github/workflows/android-build.yml`](.github/workflows/android-build.yml) otomatis membangun **`PPSSPP-Realme-C21Y-ARM64.apk`** pada setiap push ke branch `realme-c21y-optimization`, tag `v*`, atau pemicu manual (`workflow_dispatch` untuk Release/Debug), lalu mengunggah artifact dan merilis ke GitHub Release untuk tag `v*`.

### Build Lokal dengan Gradle
Pastikan Android SDK, NDK `29.0.14206865`, JDK 17, dan submodule (`git submodule update --init --recursive`) sudah tersedia:

```bash
# Build Debug ARM64
./gradlew :android:assembleRealmeC21yDebug

# Build Release ARM64 (memerlukan keystore rilis)
./gradlew :android:assembleRealmeC21yRelease \
  -PRELEASE_STORE_FILE=/path/ke/release.keystore \
  -PRELEASE_STORE_PASSWORD=*** \
  -PRELEASE_KEY_ALIAS=*** \
  -PRELEASE_KEY_PASSWORD=***
```

### Pemasangan di Realme C21Y
1. Unduh **`PPSSPP-Realme-C21Y-ARM64.apk`** dari halaman **GitHub Releases** atau **Actions Artifact**.
2. Aktifkan *Install from Unknown Sources* di pengaturan Android Realme C21Y.
3. Pasang APK dan jalankan; aplikasi akan mendeteksi Realme C21Y serta varian RAM 3 GB/4 GB secara otomatis.
4. Anda dapat mengganti mode di **Settings → Graphics → Performance Mode** sesuai kebutuhan game.

---

## Catatan Pengujian & Lisensi
- Analisis kode sumber terdokumentasi di [`docs/REALME_C21Y_ANALYSIS.md`](docs/REALME_C21Y_ANALYSIS.md) dan riwayat perubahan di [`CHANGELOG.md`](CHANGELOG.md).
- Kompilasi native ARM64, komposisi paket APK, dan validasi tanda tangan (`apksigner verify`) diverifikasi di lingkungan CI GitHub Actions. **Benchmark FPS nyata, suhu perangkat, serta uji kompatibilitas game secara langsung pada unit fisik Realme C21Y belum dijalankan** karena tidak ada perangkat fisik yang terhubung di lingkungan build.
- Atribusi asli PPSSPP oleh Henrik Rydgård dan para kontributor tetap dipertahankan sepenuhnya di bawah lisensi **GPL v2.0+** (`LICENSE.TXT`).
