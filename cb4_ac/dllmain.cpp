#include "includes.h"

typedef BOOL(APIENTRY* glSwapBuffers_t)(HDC hdc);
glSwapBuffers_t oSwapBuffers = nullptr;
static bool imguiInitialized = false;
HANDLE g_hProcess = nullptr;

BOOL __stdcall APIENTRY hkSwapBuffers(HDC hdc)
{
	if (!imguiInitialized)
	{
		ImGui::CreateContext();
		ImGui_ImplWin32_Init(WindowFromDC(hdc));
		ImGui_ImplOpenGL3_Init("#version 130");
		imguiInitialized = true;
	}

	// Start the ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Draw directly to the background (no ImGui window needed)
	ImDrawList* drawList = ImGui::GetBackgroundDrawList();

	if (SDK::Globals::g_FOV)
	{
		drawList->AddCircle
		(
			ImVec2(1920 / 2, 1080 / 2),
			SDK::Globals::g_FOVSize,
			IM_COL32(255, 255, 255, 255),
			50,
			1.0f
		);
	}

	if (SDK::Globals::g_aimbotEnabled && GetAsyncKeyState(SDK::Globals::g_aimbotKey))
	{
		DoAimbot();
	}

	if (SDK::Globals::g_Tracers)
	{
		DrawTracers(drawList);
	}

	// Render ESP if enabled
	if (SDK::Globals::g_ESP) DrawESP(drawList);

	// Render ImGui
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Call original function
	return oSwapBuffers(hdc);
}

void internalThread()
{
	Logger logger;
	logger.log("INIT");
	//CreateHooks();
	Hook SwapBuffers("wglSwapBuffers", "opengl32.dll", (BYTE*)hkSwapBuffers, (BYTE*)&oSwapBuffers, 5);
	SwapBuffers.Enable();
	while (true)
	{
		Sleep(1000);
	}
}

DWORD WINAPI mainThread(LPVOID lpparam)
{
	AllocConsole();
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
	std::thread intThread(internalThread);
	intThread.detach();
	return 0;
}

DWORD WINAPI UnloadThread(LPVOID lpParam)
{
	HMODULE hModule = (HMODULE)lpParam;
	FreeConsole();
	FreeLibraryAndExitThread(hModule, 0);
	return 0;
}

extern "C" __declspec(dllexport) void UnloadDLL()
{
	HMODULE hModule = NULL;
	if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)&UnloadDLL, &hModule))
	{
		HANDLE hThread = CreateThread(NULL, 0, UnloadThread, hModule, 0, NULL);
		if (hThread)
		{
			CloseHandle(hThread);
		}
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		StartIPCServer();
		CreateThread(nullptr, 0, mainThread, hModule, 0, nullptr);
	}
	return TRUE;
}
