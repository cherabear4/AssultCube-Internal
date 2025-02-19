#include "IPC.h"

void initSDK()
{

}

void IPCServerThread()
{
	const char* pipeName = R"(\\.\pipe\AssaultCubeHackPipe)";
	while (true)
	{
		HANDLE hPipe = CreateNamedPipeA(
			pipeName,
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			1024,  // Output buffer size
			1024,  // Input buffer size
			0,     // Default timeout
			NULL
		);

		if (hPipe == INVALID_HANDLE_VALUE) break;

		BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
		if (connected) {
			char buffer[256];
			DWORD bytesRead;

			// Read command from client
			while (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead != 0)
			{
				buffer[bytesRead] = '\0';
				std::string command(buffer);

				if (command.find("FOV_CIRCLE_ON:") != std::string::npos)
				{
					size_t colonPos = command.find(":");
					if (colonPos != std::string::npos)
					{
						std::string valueStr = command.substr(colonPos + 1);
						try {
							int fovValue = std::stoi(valueStr);
							SDK::Globals::g_FOV = true;
							SDK::Globals::g_FOVSize = fovValue;
							printf("FOV Circle enabled with value: %d\n", fovValue);
						}
						catch (const std::exception& e) {
							printf("Invalid FOV value: %s\n", valueStr.c_str());
						}
					}
				}

				else if (command.find("AIMBOT_KEY:") != std::string::npos)
				{
					size_t colonPos = command.find(":");
					if (colonPos != std::string::npos)
					{
						std::string valueStr = command.substr(colonPos + 1);
						try {
							int keyVal = std::stoi(valueStr);
							SDK::Globals::g_aimbotKey = keyVal;
							printf("Aimbot Key: %d\n", keyVal);
						}
						catch (const std::exception& e) {
							printf("Invalid key value: %s\n", valueStr.c_str());
						}
					}
				}
				else if (command.find("AIMBOT_SMOOTHING:") != std::string::npos)
				{
					size_t colonPos = command.find(":");
					if (colonPos != std::string::npos)
					{
						std::string valueStr = command.substr(colonPos + 1);
						try {
							int keyVal = std::stoi(valueStr);
							SDK::Globals::g_aimbotSmoothing = keyVal;
							printf("Aimbot smoothing: %d\n", keyVal);
						}
						catch (const std::exception& e) {
							printf("Invalid smoothing value: %s\n", valueStr.c_str());
						}
					}
				}
				else if (command.find("ESP_COLOR:") != std::string::npos)
				{
					size_t colonPos = command.find(":");
					if (colonPos != std::string::npos)
					{
						std::string values = command.substr(colonPos + 1);
						std::vector<float> colorComponents;
						std::stringstream ss(values);
						std::string component;

						try {
							while (std::getline(ss, component, ',')) {
								colorComponents.push_back(std::stof(component));
							}

							if (colorComponents.size() >= 3) {
								SDK::Globals::g_ESPColor.x = colorComponents[0]; // Red
								SDK::Globals::g_ESPColor.y = colorComponents[1]; // Green
								SDK::Globals::g_ESPColor.z = colorComponents[2]; // Blue
								printf("ESP Color Updated: %.2f, %.2f, %.2f\n",
									SDK::Globals::g_ESPColor.x,
									SDK::Globals::g_ESPColor.y,
									SDK::Globals::g_ESPColor.z);
							}
							else {
								printf("Not enough color components received: %s\n", values.c_str());
							}
						}
						catch (const std::exception& e) {
							printf("Invalid color format: %s\n", values.c_str());
						}
					}
				}
				else if (command == "FOV_CIRCLE_OFF")
				{
					SDK::Globals::g_FOV = false;
					printf("FOV Circle disabled\n");
				}
				else if (command == "HEALTH_ON")
				{
					SDK::Globals::g_Health = true;
					printf("Health hack enabled\n");
				}
				else if (command == "HEALTH_OFF")
				{
					SDK::Globals::g_Health = false;
					printf("Health hack disabled\n");
				}
				else if (command == "ESP_ON")
				{
					SDK::Globals::g_ESP = true;
					printf("ESP enabled\n");
				}
				else if (command == "ESP_OFF")
				{
					SDK::Globals::g_ESP = false;
					printf("ESP disabled\n");
				}
				else if (command == "AIMBOT_ON")
				{
					SDK::Globals::g_aimbotEnabled = true;
				}
				else if (command == "AIMBOT_OFF")
				{
					SDK::Globals::g_aimbotEnabled = false;
				}
				else if (command == "TRACERS_ON")
				{
					SDK::Globals::g_Tracers = true;
				}
				else if (command == "TRACERS_OFF")
				{
					SDK::Globals::g_Tracers = false;
				}
				else
				{
					printf("Unknown command: %s\n", command.c_str());
				}
			}
		}

		DisconnectNamedPipe(hPipe);
		CloseHandle(hPipe);
	}
}

// Start IPC server when DLL loads
void StartIPCServer()
{
	std::thread(IPCServerThread).detach();
}