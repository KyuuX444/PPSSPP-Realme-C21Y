// Copyright (c) 2012- PPSSPP Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see https://www.gnu.org/licenses/

// Official git repository and contact information can be found at
// https://github.com/hrydgard/ppsspp and https://www.ppsspp.org/.

#include "ppsspp_config.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <string_view>

#include "Common/CPUDetect.h"
#include "Common/Data/Text/I18n.h"
#include "Common/Log.h"
#include "Common/Math/CrossSIMD.h"
#include "Common/StringUtils.h"
#include "Common/System/OSD.h"
#include "Common/System/System.h"
#include "Core/Config.h"
#include "Core/DeviceOptimization.h"

namespace {

std::atomic<int> g_thermalStatus{-1};

bool ContainsCaseInsensitive(std::string_view haystack, std::string_view needle) {
	if (needle.empty() || haystack.size() < needle.size()) {
		return false;
	}
	auto it = std::search(
		haystack.begin(), haystack.end(),
		needle.begin(), needle.end(),
		[](unsigned char a, unsigned char b) {
			return std::tolower(a) == std::tolower(b);
		});
	return it != haystack.end();
}

}  // namespace

int GetDeviceTotalRamMB() {
	int64_t ramMB = System_GetPropertyInt(SYSPROP_TOTAL_RAM_MB);
	return ramMB > 0 ? static_cast<int>(ramMB) : 0;
}

DeviceHardwareProfile DetectDeviceHardwareProfile() {
	DeviceHardwareProfile profile;
	profile.modelName = System_GetProperty(SYSPROP_NAME);
	std::string board = System_GetProperty(SYSPROP_BOARDNAME);
	std::string soc = System_GetProperty(SYSPROP_HARDWARE_SOC);
	if (!soc.empty() && !board.empty()) {
		profile.hardwareSoc = soc + " (" + board + ")";
	} else {
		profile.hardwareSoc = !soc.empty() ? soc : board;
	}

	profile.totalRamMB = GetDeviceTotalRamMB();
	profile.cpuCores = cpu_info.num_cores;

#if PPSSPP_PLATFORM(ANDROID) && PPSSPP_ARCH(ARM64)
	const std::string combinedSoc = board + " " + soc;
	profile.isUnisocT610 =
		ContainsCaseInsensitive(combinedSoc, "ums512") ||
		ContainsCaseInsensitive(combinedSoc, "t610");

	const bool isRealmeModel =
		ContainsCaseInsensitive(profile.modelName, "rmx3261") ||
		ContainsCaseInsensitive(profile.modelName, "rmx3262") ||
		ContainsCaseInsensitive(profile.modelName, "rmx3263") ||
		(ContainsCaseInsensitive(profile.modelName, "realme") && ContainsCaseInsensitive(profile.modelName, "c21y"));

	profile.isRealmeC21Y = isRealmeModel || (ContainsCaseInsensitive(profile.modelName, "realme") && profile.isUnisocT610);
	profile.isLowRamVariant = System_GetPropertyBool(SYSPROP_LOW_MEMORY_DEVICE) ||
		(profile.totalRamMB > 0 && profile.totalRamMB <= 3400);

	if (profile.isRealmeC21Y) {
		const char *ramLabel = profile.isLowRamVariant ? "3 GB RAM" : (profile.totalRamMB > 0 ? "4 GB RAM" : "Unknown RAM");
		profile.profileLabel = StringFromFormat("Realme C21Y (Unisoc T610, %s)", ramLabel);
	} else if (profile.isUnisocT610) {
		profile.profileLabel = StringFromFormat("Unisoc T610 ARM64 (%d MB RAM)", profile.totalRamMB);
	} else {
		profile.profileLabel = profile.totalRamMB > 0
			? StringFromFormat("Android Generic (%d MB RAM)", profile.totalRamMB)
			: "Android Generic";
	}
#else
	profile.profileLabel = "Standard";
#endif

	return profile;
}

bool IsRealmeC21YDevice() {
#if PPSSPP_PLATFORM(ANDROID) && PPSSPP_ARCH(ARM64)
	return DetectDeviceHardwareProfile().isRealmeC21Y;
#else
	return false;
#endif
}

bool IsUnisocT610Device() {
#if PPSSPP_PLATFORM(ANDROID) && PPSSPP_ARCH(ARM64)
	return DetectDeviceHardwareProfile().isUnisocT610;
#else
	return false;
#endif
}

bool IsLowRamDeviceVariant() {
#if PPSSPP_PLATFORM(ANDROID)
	if (System_GetPropertyBool(SYSPROP_LOW_MEMORY_DEVICE)) {
		return true;
	}
	int ramMB = GetDeviceTotalRamMB();
	return ramMB > 0 && ramMB <= 3400;
#else
	return false;
#endif
}

int GetAdaptiveTextureCachePressureBytes() {
#if PPSSPP_PLATFORM(ANDROID)
	int ramMB = GetDeviceTotalRamMB();
	if (IsLowRamDeviceVariant() || IsThermalThrottlingSevere()) {
		return 48 * 1024 * 1024;
	}
	if (ramMB > 0 && ramMB <= 4500) {
		return 80 * 1024 * 1024;
	}
#endif
	return 128 * 1024 * 1024;
}

int GetAdaptiveTextureCacheSecondaryPressureBytes() {
#if PPSSPP_PLATFORM(ANDROID)
	int ramMB = GetDeviceTotalRamMB();
	if (IsLowRamDeviceVariant() || IsThermalThrottlingSevere()) {
		return 16 * 1024 * 1024;
	}
	if (ramMB > 0 && ramMB <= 4500) {
		return 24 * 1024 * 1024;
	}
#endif
	return 32 * 1024 * 1024;
}

int DefaultPerformanceProfileForDevice() {
	return (int)PerformanceProfile::BALANCED;
}

void ApplyPerformanceProfile(PerformanceProfile profile, bool notifyChange) {
	const bool canJit = System_GetPropertyBool(SYSPROP_CAN_JIT);
	const bool lowRam = IsLowRamDeviceVariant();

	switch (profile) {
	case PerformanceProfile::ULTRA_PERFORMANCE:
		g_Config.iInternalResolution = 1;
		g_Config.bAutoFrameSkip = true;
		g_Config.iFrameSkip = 1;
		g_Config.iTexFiltering = TEX_FILTER_FORCE_NEAREST;
		g_Config.bSmart2DTexFiltering = false;
		g_Config.iAnisotropyLevel = 0;
		g_Config.vPostShaderNames.clear();
		g_Config.bVSync = false;
		g_Config.bLowLatencyPresent = true;
		g_Config.bSkipBufferEffects = false;
		g_Config.bHardwareTransform = true;
		g_Config.bFastMemory = canJit;
		g_Config.bHardwareTessellation = false;
		g_Config.iBloomHack = 1;
		g_Config.iDepthRasterMode = (int)DepthRasterMode::OFF;
		g_Config.iInflightFrames = 2;
		g_Config.iBackgroundAnimation = (int)BackgroundAnimation::OFF;
		g_Config.bTransparentBackground = false;
		g_Config.iAudioPlaybackMode = (int)AudioSyncMode::GRANULAR;
		break;

	case PerformanceProfile::COMPATIBILITY:
		g_Config.iInternalResolution = 1;
		g_Config.bAutoFrameSkip = false;
		g_Config.iFrameSkip = 0;
		g_Config.iTexFiltering = TEX_FILTER_AUTO;
		g_Config.bSmart2DTexFiltering = false;
		g_Config.iAnisotropyLevel = 0;
		g_Config.vPostShaderNames.clear();
		g_Config.bVSync = true;
		g_Config.bLowLatencyPresent = false;
		g_Config.bSkipBufferEffects = false;
		g_Config.bHardwareTransform = true;
		g_Config.bFastMemory = false;
		g_Config.bHardwareTessellation = false;
		g_Config.iBloomHack = 0;
		g_Config.iDepthRasterMode = (int)DepthRasterMode::DEFAULT;
		g_Config.iSkipGPUReadbackMode = (int)SkipGPUReadbackMode::NO_SKIP;
		g_Config.iInflightFrames = 2;
		g_Config.iAudioPlaybackMode = (int)AudioSyncMode::GRANULAR;
		break;

	case PerformanceProfile::BALANCED:
	default:
		g_Config.iInternalResolution = 1;
		g_Config.bAutoFrameSkip = true;
		g_Config.iFrameSkip = 1;
		g_Config.iTexFiltering = TEX_FILTER_AUTO;
		g_Config.bSmart2DTexFiltering = false;
		g_Config.iAnisotropyLevel = 0;
		g_Config.vPostShaderNames.clear();
		g_Config.bVSync = true;
		g_Config.bLowLatencyPresent = true;
		g_Config.bSkipBufferEffects = false;
		g_Config.bHardwareTransform = true;
		g_Config.bFastMemory = canJit;
		g_Config.bHardwareTessellation = false;
		g_Config.iBloomHack = 0;
#if PPSSPP_ARCH(ARM64_NEON)
		g_Config.iDepthRasterMode = (int)DepthRasterMode::LOW_QUALITY;
#else
		g_Config.iDepthRasterMode = (int)DepthRasterMode::DEFAULT;
#endif
		g_Config.iInflightFrames = 2;
		if (lowRam) {
			g_Config.iBackgroundAnimation = (int)BackgroundAnimation::OFF;
			g_Config.bTransparentBackground = false;
		}
		g_Config.iAudioPlaybackMode = (int)AudioSyncMode::GRANULAR;
		break;
	}

#if PPSSPP_PLATFORM(ANDROID)
	if (System_GetPropertyInt(SYSPROP_SYSTEMVERSION) >= 27 &&
		System_GetPropertyInt(SYSPROP_DEVICE_TYPE) != DEVICE_TYPE_TV) {
		g_Config.iAndroidAudioApi = (int)AndroidAudioApi::AAUDIO;
		g_Config.bAndroidAudioMMap = true;
	}
#endif

	if (notifyChange) {
		System_PostUIMessage(UIMessage::GPU_RENDER_RESIZED);
		System_PostUIMessage(UIMessage::GPU_CONFIG_CHANGED);
		System_Notify(SystemNotification::AUDIO_MODE_CHANGED);
		System_Notify(SystemNotification::SUSTAINED_PERF_CHANGE);
	}
}

void UpdateThermalStatus(int status) {
	int prev = g_thermalStatus.exchange(status);
	if (prev == status) {
		return;
	}

	const bool prevSustained = prev >= 2;
	const bool nowSustained = status >= 2;
	if (prevSustained != nowSustained) {
		System_Notify(SystemNotification::SUSTAINED_PERF_CHANGE);
	}

	if (status >= 3 && prev < 3) {
		WARN_LOG(Log::System, "Android thermal status severe (%s): enabling sustained performance throttling", GetThermalStatusName(status));
		g_OSD.Show(OSDType::MESSAGE_WARNING, "Device warm: sustained thermal throttling active", 3.0f, "device_thermal");
	}
}

int GetCurrentThermalStatus() {
	return g_thermalStatus.load();
}

const char *GetThermalStatusName(int status) {
	switch (status) {
	case 0: return "None";
	case 1: return "Light";
	case 2: return "Moderate";
	case 3: return "Severe";
	case 4: return "Critical";
	case 5: return "Emergency";
	case 6: return "Shutdown";
	default: return "Unavailable";
	}
}

bool IsThermalSustainedPerfForced() {
	return g_thermalStatus.load() >= 2;
}

bool IsThermalThrottlingSevere() {
	return g_thermalStatus.load() >= 3;
}
