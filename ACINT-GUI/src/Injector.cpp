#include "Injector.h"
#include "../includes.h"

bool g_MainUI = false;

DWORD GetProcessID(const std::string processName)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		throw std::runtime_error("Failed to create snapshot");
	}

	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hSnapshot, &pe)) {
		do {
			if (processName == pe.szExeFile) {
				CloseHandle(hSnapshot);
				return pe.th32ProcessID;
			}
		} while (Process32Next(hSnapshot, &pe));
	}

	CloseHandle(hSnapshot);
	throw std::runtime_error("Process not found");
}

bool isModuleLoaded(const std::wstring moduleName, DWORD pid)
{
	bool found = false;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
	if (snapshot != INVALID_HANDLE_VALUE) {
		MODULEENTRY32W moduleEntry;
		moduleEntry.dwSize = sizeof(MODULEENTRY32W);
		if (Module32FirstW(snapshot, &moduleEntry)) {
			do {
				if (moduleName == moduleEntry.szModule) {
					found = true;
					break;
				}
			} while (Module32NextW(snapshot, &moduleEntry));
		}
		CloseHandle(snapshot);
	}
	return found;
}


void injectCheat()
{
    char dllPath[MAX_PATH];
    GetFullPathNameA("cb4_ac.dll", MAX_PATH, dllPath, NULL);

    std::cout << dllPath << std::endl;

    HWND hwnd = FindWindow("SDL_app", "AssaultCube");
    if (!hwnd) {
        std::cout << "AssaultCube not found.\n";
        return;
    }

    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (!pid) {
        std::cout << "Failed to get process ID.\n";
        return;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) {
        std::cout << "Failed to open process. Try running as Admin.\n";
        return;
    }

    void* pAlloc = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pAlloc) {
        std::cout << "Failed to allocate memory in target process.\n";
        CloseHandle(hProcess);
        return;
    }

    SIZE_T bytesWritten;
    BOOL success = WriteProcessMemory(hProcess, pAlloc, dllPath, strlen(dllPath) + 1, &bytesWritten);
    if (!success || bytesWritten != strlen(dllPath) + 1) {
        std::cout << "Failed to write DLL path to memory.\n";
        VirtualFreeEx(hProcess, pAlloc, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    void* loadLibraryAddr = (void*)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
    if (!loadLibraryAddr) {
        std::cout << "Failed to get LoadLibraryA address.\n";
        VirtualFreeEx(hProcess, pAlloc, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddr, pAlloc, 0, NULL);
    if (!hThread) {
        std::cout << "Failed to create remote thread.\n";
        VirtualFreeEx(hProcess, pAlloc, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    WaitForSingleObject(hThread, INFINITE);
    DWORD exitCode = 0;
    GetExitCodeThread(hThread, &exitCode);
    CloseHandle(hThread);

    if (exitCode == NULL) {
        std::cout << "LoadLibraryA failed, DLL was not injected.\n";
    }
    else {
        std::cout << "DLL injected successfully!\n";
    }

    VirtualFreeEx(hProcess, pAlloc, 0, MEM_RELEASE);
    CloseHandle(hProcess);
}

void freeCheat()
{
    HWND hwnd = FindWindow("SDL_app", "AssaultCube");
    if (!hwnd) return;

    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (!pid) return;

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) return;

    HMODULE hModule = GetModuleHandleA("cb4_ac.dll");
    if (!hModule) {
        CloseHandle(hProcess);
        return;
    }

    // Get the address of the UnloadDLL function
    FARPROC pUnloadFunc = GetProcAddress(hModule, "UnloadDLL");
    if (!pUnloadFunc) {
        CloseHandle(hProcess);
        return;
    }

    // Create a remote thread to call UnloadDLL
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pUnloadFunc, NULL, 0, NULL);
    if (hThread) CloseHandle(hThread);

    CloseHandle(hProcess);
}


void RenderInjector()
{
	ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("AC Injector", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
	{
		if(ImGui::Button("Inject AC Cheat"))
		{
			injectCheat();
			g_MainUI = true;
		}
	}
	ImGui::End();
}