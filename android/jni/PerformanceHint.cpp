// Copyright (c) 2012- PPSSPP Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official git repository and contact information can be found at
// https://github.com/hrydgard/ppsspp and http://www.ppsspp.org/.

#include <dlfcn.h>
#include <time.h>
#include <unistd.h>

#include "Common/Log.h"
#include "Core/DeviceOptimization.h"
#include "android/jni/PerformanceHint.h"

struct APerformanceHintManager;

namespace {

// Half a frame. With CPU time as the measure, this leaves the other half as headroom, so the
// scheduler keeps us on a core fast enough to finish well before the frame is due.
constexpr int64_t TARGET_WORK_NS_DEFAULT = 1000000000LL / 60 / 2;
constexpr int64_t TARGET_WORK_NS_THERMAL = 11500000LL;
constexpr int64_t MAX_VALID_WORK_SAMPLE_NS = 200000000LL;

// Looked up at runtime, since we build against API 21 and these are API 33.
struct HintApi {
	APerformanceHintManager *(*getManager)();
	APerformanceHintSession *(*createSession)(APerformanceHintManager *, const int32_t *, size_t, int64_t);
	int (*reportActualWorkDuration)(APerformanceHintSession *, int64_t);
	int (*updateTargetWorkDuration)(APerformanceHintSession *, int64_t);
	void (*closeSession)(APerformanceHintSession *);

	bool Load() {
		void *lib = dlopen("libandroid.so", RTLD_NOW);
		if (!lib) {
			return false;
		}
		getManager = (decltype(getManager))dlsym(lib, "APerformanceHint_getManager");
		createSession = (decltype(createSession))dlsym(lib, "APerformanceHint_createSession");
		reportActualWorkDuration = (decltype(reportActualWorkDuration))dlsym(lib, "APerformanceHint_reportActualWorkDuration");
		updateTargetWorkDuration = (decltype(updateTargetWorkDuration))dlsym(lib, "APerformanceHint_updateTargetWorkDuration");
		closeSession = (decltype(closeSession))dlsym(lib, "APerformanceHint_closeSession");
		return getManager && createSession && reportActualWorkDuration && closeSession;
	}
};

const HintApi *GetApi() {
	static HintApi api{};
	static const bool loaded = api.Load();
	return loaded ? &api : nullptr;
}

// Time spent running on a CPU, so sleeping in the frame limiter or blocking on vsync or the GPU
// doesn't count as work.
int64_t ThreadCpuTimeNs() {
	timespec ts;
	if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts) != 0) {
		return -1;
	}
	return (int64_t)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

}  // namespace

PerformanceHintSession::~PerformanceHintSession() {
	if (session_) {
		GetApi()->closeSession(session_);
	}
}

void PerformanceHintSession::ReportFrame() {
	if (failed_) {
		return;
	}
	const HintApi *api = GetApi();
	const int64_t desiredTargetNs = IsThermalThrottlingSevere() ? TARGET_WORK_NS_THERMAL : TARGET_WORK_NS_DEFAULT;
	if (!session_) {
		APerformanceHintManager *manager = api ? api->getManager() : nullptr;
		const int32_t tid = gettid();
		session_ = manager ? api->createSession(manager, &tid, 1, desiredTargetNs) : nullptr;
		if (!session_) {
			INFO_LOG(Log::System, "Performance hint sessions not available");
			failed_ = true;
			return;
		}
		currentTargetNs_ = desiredTargetNs;
		INFO_LOG(Log::System, "Performance hint session created for thread %d, target %d us", tid, (int)(desiredTargetNs / 1000));
	} else if (desiredTargetNs != currentTargetNs_ && api && api->updateTargetWorkDuration) {
		if (api->updateTargetWorkDuration(session_, desiredTargetNs) == 0) {
			currentTargetNs_ = desiredTargetNs;
		}
	}
	const int64_t now = ThreadCpuTimeNs();
	if (lastCpuTimeNs_ >= 0 && now > lastCpuTimeNs_) {
		const int64_t durationNs = now - lastCpuTimeNs_;
		if (durationNs < MAX_VALID_WORK_SAMPLE_NS) {
			api->reportActualWorkDuration(session_, durationNs);
		}
	}
	lastCpuTimeNs_ = now;
}
