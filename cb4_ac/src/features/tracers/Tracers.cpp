#include "Tracers.h"

void DrawTracers(ImDrawList* drawList)
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

		entity.footpos = *(SDK::Vec3*)(Entity + SDK::Offsets::X);

		players.push_back(entity);
	}
	for (const auto& player : players)
	{
		SDK::Vec2 FootScreen;
		if (WorldToScreen(player.footpos, FootScreen))
		{
			SDK::Vec2 screenCenter = { 1920 / 2.0f, 1080 / 2.0f };
			drawList->AddLine(ImVec2(screenCenter.x, screenCenter.y), ImVec2(FootScreen.x, FootScreen.y), ImColor(1.0f, 1.0f, 1.0f), 1);
		}
	}
}