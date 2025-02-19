#pragma once
#include "../../includes.h"
#include "../IPC/IPC.h"

namespace SDK
{
	namespace Offsets
	{
		inline uintptr_t LocalPlayer = 0x18AC00;
		inline uintptr_t EntityList = 0x18AC04;
		inline uintptr_t PlayerCount = 0x18AC0C;
		inline uintptr_t viewMatrix = 0x17DFD0;

		inline uintptr_t X = 0x4;
		inline uintptr_t Y = 0x8;
		inline uintptr_t ZHead = 0xC;
		inline uintptr_t Zfoot = 0x30;

		inline uintptr_t Health = 0xEC;
		inline uintptr_t Name = 0x205;
		inline uintptr_t Team = 0x30C;
	};

	struct Vec2
	{
		float x;
		float y;
	};

	struct Vec3
	{
		float x;
		float y;
		float z;
	};

	struct Vector4
	{
		float x;
		float y;
		float z;
		float w;
	};

	struct glMatrix
	{
		float matrix[16];
	};

	namespace Globals
	{
		inline bool g_aimbotEnabled = false;
		inline float g_FOVSize = 90.0f;
		inline bool g_FOV = false;
		inline SDK::Vec3 g_ESPColor = SDK::Vec3(1.0f, 0.0f, 0.0f);
		inline bool g_ESP = false;
		inline bool g_Health = false;
		inline int g_aimbotKey = 0;
		inline float g_aimbotSmoothing = 5.0f;
		inline float g_RemainderX = 0.0f;
		inline float g_RemainderY = 0.0f;
		inline bool g_Tracers = false;
	}

	namespace Math
	{
		template <typename T>
		constexpr T clamp(T value, T min, T max) {
			return (value < min) ? min : (value > max) ? max : value;
		}
		inline float DistanceSquared(const SDK::Vec2& a, const SDK::Vec2& b)
		{
			float dx = a.x - b.x;
			float dy = a.y - b.y;
			return dx * dx + dy * dy;
		}
		inline bool IsTargetWithinScreenFOV(const SDK::Vec2& targetScreenPos, const SDK::Vec2& screenCenter, float fovRadius) {
			return DistanceSquared(targetScreenPos, screenCenter) <= (fovRadius * fovRadius);
		}
	}

	inline bool WorldToScreen(Vec3 pos, Vec2& screenPos)
	{
		RECT rctAC;
		HWND hwndAC = FindWindow("SDL_app", "AssaultCube");
		GetClientRect(hwndAC, &rctAC);
		int windowWidth = rctAC.right;
		int windowHeight = rctAC.bottom;

		glMatrix* matrix = (glMatrix*)((uintptr_t)GetModuleHandle(NULL) + SDK::Offsets::viewMatrix);

		//Matrix-vector Product, multiplying world(eye) coordinates by projection matrix = clipCoords
		Vector4 clipCoords;
		clipCoords.x = pos.x * matrix->matrix[0] + pos.y * matrix->matrix[4] + pos.z * matrix->matrix[8] + matrix->matrix[12];
		clipCoords.y = pos.x * matrix->matrix[1] + pos.y * matrix->matrix[5] + pos.z * matrix->matrix[9] + matrix->matrix[13];
		clipCoords.z = pos.x * matrix->matrix[2] + pos.y * matrix->matrix[6] + pos.z * matrix->matrix[10] + matrix->matrix[14];
		clipCoords.w = pos.x * matrix->matrix[3] + pos.y * matrix->matrix[7] + pos.z * matrix->matrix[11] + matrix->matrix[15];

		if (clipCoords.w < 0.1f)
			return false;

		//Perspective division, dividing by clip.W = Normalized Device Coordinates
		Vec3 NDC;
		NDC.x = clipCoords.x / clipCoords.w;
		NDC.y = clipCoords.y / clipCoords.w;
		NDC.z = clipCoords.z / clipCoords.w;

		//Transform to window coordinates
		screenPos.x = (windowWidth / 2 * NDC.x) + (NDC.x + windowWidth / 2);
		screenPos.y = -(windowHeight / 2 * NDC.y) + (NDC.y + windowHeight / 2);
		return true;
	}

	inline double GetDistance(Vec2 enemy)
	{
		RECT rctAC;
		HWND hwndAC = FindWindow("SDL_app", "AssaultCube");
		GetClientRect(hwndAC, &rctAC);
		int windowWidth = rctAC.right;
		int windowHeight = rctAC.bottom;

		return sqrt(pow((enemy.x - windowWidth / 2), 2) + pow((enemy.y - windowHeight / 2), 2));
	}

	inline void AimAtPos(float x, float y)
	{
		RECT rctAC;
		HWND hwndAC = FindWindow("SDL_app", "AssaultCube");
		GetClientRect(hwndAC, &rctAC);
		int windowWidth = rctAC.right;
		int windowHeight = rctAC.bottom;

		float ScreenCenterX = (windowWidth / 2);
		float ScreenCenterY = (windowHeight / 2);
		float TargetX = 0;
		float TargetY = 0;

		if (x == 69696 && y == -69696)
			return;

		// Calculate raw target offsets
		if (x != 0) {
			TargetX = x - ScreenCenterX;
			TargetX = Math::clamp(TargetX, -ScreenCenterX, ScreenCenterX);
		}

		if (y != 0) {
			TargetY = y - ScreenCenterY;
			TargetY = Math::clamp(TargetY, -ScreenCenterY, ScreenCenterY);
		}

		// Apply smoothing with remainder preservation
		TargetX += SDK::Globals::g_RemainderX;
		TargetY += SDK::Globals::g_RemainderY;

		float SmoothX = TargetX / SDK::Globals::g_aimbotSmoothing;
		float SmoothY = TargetY / SDK::Globals::g_aimbotSmoothing;

		// Store remainder for next frame
		SDK::Globals::g_RemainderX = TargetX - SmoothX;
		SDK::Globals::g_RemainderY = TargetY - SmoothY;

		// Convert to integer for mouse_event
		int MoveX = static_cast<int>(std::round(SmoothX));
		int MoveY = static_cast<int>(std::round(SmoothY));

		// Send mouse input
		if (MoveX != 0 || MoveY != 0) {
			mouse_event(MOUSEEVENTF_MOVE, MoveX, MoveY, NULL, NULL);
		}
	}
	struct Player
	{
		int health;
		SDK::Vec3 headpos;
		SDK::Vec3 footpos;
	};
}