#include "ESP.h"

typedef SDK::Vec3 vec3;
typedef SDK::Vec2 vec2;



void DrawESP(ImDrawList* drawList)
{
	uintptr_t Base = (uintptr_t)GetModuleHandle(NULL);
	uintptr_t LocalPlayer = *(uintptr_t*)(Base + SDK::Offsets::LocalPlayer);
	uintptr_t EntityList = *(uintptr_t*)(Base + SDK::Offsets::EntityList);
	std::vector<SDK::Player> players;

	for (int i = 1; i < *(int*)(Base + SDK::Offsets::PlayerCount); i++)
	{
		SDK::Player entity;
		uintptr_t Entity = *(uintptr_t*)(EntityList + (i * 0x4));

		if (*(int*)(LocalPlayer + SDK::Offsets::Team) == *(int*)(Entity + SDK::Offsets::Team))
			continue;

		if (*(int*)(Entity + SDK::Offsets::Health) <= 0)
			continue;

		entity.headpos = *(vec3*)(Entity + SDK::Offsets::X);
		entity.footpos = { entity.headpos.x, entity.headpos.y, *(float*)(Entity + SDK::Offsets::Zfoot) };

		players.push_back(entity);
	}
	for (const auto& player : players)
	{
		vec2 HeadScreen;
		vec2 FootScreen;
		if (WorldToScreen(player.headpos, HeadScreen) && WorldToScreen(player.footpos, FootScreen))
		{
			HeadScreen.y = HeadScreen.y - 20;
			float flHeight = abs(FootScreen.y - HeadScreen.y);
			float flWidth = flHeight / 2.0F;
			// draw
			drawList->AddText(ImVec2(FootScreen.x, FootScreen.y + 5), ImColor(SDK::Globals::g_ESPColor.x, SDK::Globals::g_ESPColor.y, SDK::Globals::g_ESPColor.z), "PLAYER");
		}
	}
}