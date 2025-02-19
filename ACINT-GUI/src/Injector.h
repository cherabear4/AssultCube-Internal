#include "../includes.h"

extern bool g_MainUI;

DWORD GetProcessID(const std::string processName);

bool isModuleLoaded(const std::wstring moduleName, DWORD pid);

void RenderInjector();

void injectCheat();

void freeCheat();