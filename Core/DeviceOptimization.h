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

#pragma once

#include <string>
#include "Core/ConfigValues.h"

struct DeviceHardwareProfile {
	bool isRealmeC21Y = false;
	bool isUnisocT610 = false;
	bool isLowRamVariant = false;
	int totalRamMB = 0;
	int cpuCores = 0;
	std::string modelName;
	std::string hardwareSoc;
	std::string profileLabel;
};

DeviceHardwareProfile DetectDeviceHardwareProfile();
bool IsRealmeC21YDevice();
bool IsUnisocT610Device();
bool IsLowRamDeviceVariant();
int GetDeviceTotalRamMB();

int GetAdaptiveTextureCachePressureBytes();
int GetAdaptiveTextureCacheSecondaryPressureBytes();

int DefaultPerformanceProfileForDevice();
void ApplyPerformanceProfile(PerformanceProfile profile, bool notifyChange);

void UpdateThermalStatus(int status);
int GetCurrentThermalStatus();
const char *GetThermalStatusName(int status);
bool IsThermalSustainedPerfForced();
bool IsThermalThrottlingSevere();
