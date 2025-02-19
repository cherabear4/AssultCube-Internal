#pragma once

#include <Windows.h>
#include <cstdint>
#include <cstdlib>
#include <stdint.h>
#include <iostream>
#include <vector>
#include <thread>
#include <string>
#include <sstream>
#include <detours/detours.h>
#include <mutex>
#include "src/logger/logger.h"
#include "src/memory/memory.h"
#include "src/SDK/SDK.h"
#include "src/IPC/IPC.h"
#include "src/hook/hook.h"

#pragma region IMGUI

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/backend/imgui_impl_opengl3.h"
#include "vendor/imgui/backend/imgui_impl_win32.h"

#pragma region FEATURES

#include "src/features/esp/ESP.h"
#include "src/features/aimbot/Aimbot.h"
#include "src/features/tracers/Tracers.h"
#include "src/features/Recoil/Recoil.h"