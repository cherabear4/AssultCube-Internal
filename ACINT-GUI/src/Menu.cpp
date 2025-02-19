#include "../includes.h"

// Compiler directives for OpenGL
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")

// Font pointers
ImFont* headerFont = nullptr;
ImFont* regularFont = nullptr;

// GUI state
bool aimbotEnabled = false;
float aimbotFOV = 90.0f;
float smoothing = 1.0f;
static bool esp = false;
bool dragMode = true;
bool fovCircle;
bool showaOptions;
bool waitingForKey;
float espColor[] = { 1.0f, 0.0f, 0.0f };
float guiColor[] = { 0.0f, 1.0f, 0.65f };
bool tracersEnabled = false;
int aimbotKey = 0;

const char* GetKeyName(int vkCode)
{
	static char keyName[32];
	GetKeyNameTextA(MapVirtualKeyA(vkCode, MAPVK_VK_TO_VSC) << 16, keyName, sizeof(keyName));
	return keyName;
}

void SendCommand(const std::string& command)
{
	const char* pipeName = R"(\\.\pipe\AssaultCubeHackPipe)";
	HANDLE hPipe = CreateFileA(
		pipeName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
	);

	if (hPipe != INVALID_HANDLE_VALUE) {
		DWORD bytesWritten;
		WriteFile(hPipe, command.c_str(), command.size(), &bytesWritten, NULL);
		CloseHandle(hPipe);
	}
}

void SendColorCommand(float r, float g, float b)
{
	std::string command = "ESP_COLOR:" + std::to_string(r) + "," + std::to_string(g) + "," + std::to_string(b);
	SendCommand(command);
}

void SetupImGuiStyle()
{
	ImGuiStyle* style = &ImGui::GetStyle();

	style->WindowPadding = ImVec2(15, 15);
	style->WindowRounding = 5.0f;
	style->FramePadding = ImVec2(5, 5);
	style->FrameRounding = 4.0f;
	style->ItemSpacing = ImVec2(12, 8);
	style->ItemInnerSpacing = ImVec2(8, 6);
	style->IndentSpacing = 25.0f;
	style->ScrollbarSize = 15.0f;
	style->ScrollbarRounding = 9.0f;
	style->GrabMinSize = 5.0f;
	style->GrabRounding = 3.0f;

	style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.05f);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
}

void LoadSystemFonts()
{
	ImGuiIO& io = ImGui::GetIO();
	const char* fontPath = "C:\\Windows\\Fonts\\segoeui.ttf";
	const char* boldFontPath = "C:\\Windows\\Fonts\\seguisb.ttf";

	if (GetFileAttributesA(fontPath) != INVALID_FILE_ATTRIBUTES)
		regularFont = io.Fonts->AddFontFromFileTTF(fontPath, 18.0f);
	if (GetFileAttributesA(boldFontPath) != INVALID_FILE_ATTRIBUTES)
		headerFont = io.Fonts->AddFontFromFileTTF(boldFontPath, 24.0f);

	if (!regularFont) regularFont = io.Fonts->AddFontDefault();
	if (!headerFont) headerFont = io.Fonts->AddFontDefault();
}

void RenderMenu(GLFWwindow* window)
{
	ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("AC External", nullptr,
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
	{

		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		// Get the window position and size
		ImVec2 windowPos = ImGui::GetWindowPos();
		ImVec2 windowSize = ImGui::GetWindowSize();

		// Draw a colored rectangle at the top
		float headerHeight = 30.0f; // Height of the colored header
		draw_list->AddRectFilled(
			windowPos,
			ImVec2(windowPos.x + windowSize.x, windowPos.y + headerHeight),
			ImColor(guiColor[0], guiColor[1], guiColor[2])
		);

		// Add the title text manually
		draw_list->AddText(
			ImVec2(windowPos.x + 10, windowPos.y + 5),
			ImColor(1.0f, 1.0f, 1.0f, 1.0f),
			"AC External"
		);

		// Window dragging implementation
		static bool dragging = false;
		static ImVec2 dragOffset;

		if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
		{
			if (dragging && dragMode)
			{
				// Get current mouse position in screen coordinates
				double mouseX, mouseY;
				glfwGetCursorPos(window, &mouseX, &mouseY);
				int windowX, windowY;
				glfwGetWindowPos(window, &windowX, &windowY);
				ImVec2 screenMousePos = ImVec2(windowX + mouseX, windowY + mouseY);

				// Update window position
				glfwSetWindowPos(window,
					static_cast<int>(screenMousePos.x - dragOffset.x),
					static_cast<int>(screenMousePos.y - dragOffset.y)
				);
			}
			else if (ImGui::IsWindowHovered())
			{
				// Get initial drag offset
				ImVec2 windowPos = ImGui::GetWindowPos();
				ImVec2 mousePos = ImGui::GetMousePos();
				dragOffset = ImVec2(mousePos.x - windowPos.x, mousePos.y - windowPos.y);
				dragging = true;
			}
		}
		else
		{
			dragging = false;
		}

		enum tab { TAB_CLIENT, TAB_VISUAL, TAB_MISC };
		static int currentTab = TAB_CLIENT; // Initialize to the first tab

		// Tab buttons
		if (ImGui::Button("Client")) { currentTab = TAB_CLIENT; }
		ImGui::SameLine(); // Place the next button on the same line
		if (ImGui::Button("Visuals")) { currentTab = TAB_VISUAL; }
		ImGui::SameLine(); // Place the next button on the same line
		if (ImGui::Button("Misc")) { currentTab = TAB_MISC; }

		// Tab content
		ImGui::BeginChild("TabContent", ImVec2(0, 0), true); // Create a child region for the tab content
		{
			switch (currentTab)
			{
			case TAB_CLIENT:
				if (ImGui::Checkbox("Aimbot", &aimbotEnabled))
				{
					SendCommand(aimbotEnabled ? "AIMBOT_ON" : "AIMBOT_OFF");
				}

				ImGui::SameLine();

				// Use a collapsible header or a separate window for "Show options"
				if (ImGui::Button("Show options"))
				{
					showaOptions = !showaOptions; // Toggle the visibility of the options
				}

				// Render the options if "Show options" is active
				if (showaOptions)
				{
					ImGui::Indent(); // Indent the options for better visual hierarchy
					if (ImGui::Button("Draw FOV"))
					{
						fovCircle = !fovCircle;
						std::string fov = std::to_string(aimbotFOV);
						std::string full = "FOV_CIRCLE_ON:" + fov;
						SendCommand(fovCircle ? full : "FOV_CIRCLE_OFF");
					}
					char buttonLabel[64];
					std::snprintf(buttonLabel, sizeof(buttonLabel), "Current Keybind: %s", aimbotKey ? GetKeyName(aimbotKey) : "None");

					if (ImGui::Button(buttonLabel))
					{
						waitingForKey = true;
					}

					if (waitingForKey)
					{
						ImGui::Text("Waiting for key...");

						for (int vk = 0x01; vk <= 0xFE; vk++)
						{
							if (GetAsyncKeyState(vk) & 0x8000)
							{
								aimbotKey = vk;
								std::string keyString = std::to_string(vk);
								std::string fullKeyString = "AIMBOT_KEY:" + keyString;
								SendCommand(fullKeyString);
								waitingForKey = false;
								break;
							}
						}
					}
				}
				if (showaOptions)
				{
					if (ImGui::SliderFloat("FOV", &aimbotFOV, 1.0f, 180.0f, "%.1f"))
					{
						if (fovCircle)
						{
							std::string fov = std::to_string(aimbotFOV);
							std::string full = "FOV_CIRCLE_ON:" + fov;
							SendCommand(full);
						}
					}
				}
				if (showaOptions)
				{
					if (ImGui::SliderFloat("Smoothing", &smoothing, 1.0f, 20.0f, "%.1f"))
					{
						if (fovCircle)
						{
							std::string smoothingStr = std::to_string(smoothing);
							std::string full = "AIMBOT_SMOOTHING:" + smoothingStr;
							SendCommand(full);
						}
					}
				}
				break;

			case TAB_VISUAL:
				if (ImGui::Checkbox("ESP", &esp))
				{
					SendCommand(esp ? "ESP_ON" : "ESP_OFF"); // Send ESP command
				}
				if (ImGui::ColorEdit3("Color", espColor))
				{
					SendColorCommand(espColor[0], espColor[1], espColor[2]);
				}
				if (ImGui::Checkbox("Tracers", &tracersEnabled))
				{
					SendCommand(tracersEnabled ? "TRACERS_ON" : "TRACERS_OFF");
				}
				break;

			case TAB_MISC:
				if (ImGui::Button("Unload"))
				{
					freeCheat();
					exit(0);
				}
				if (ImGui::ColorEdit3("Color", guiColor))
				{
					
				}
				break;
			}
		}
		ImGui::EndChild();

		ImGui::Separator();
		ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Active");
		ImGui::Checkbox("Drag mode", &dragMode);
	}
	ImGui::End();
}

int main()
{
	DWORD acPid = GetProcessID("ac_client.exe");
	if (acPid == 0)
	{
		std::cout << "Cannot find Assult Cube\n";
	}
	ShowWindow(GetConsoleWindow(), 0);
	if (!glfwInit()) {
		MessageBoxA(NULL, "Failed to initialize GLFW", "Error", MB_ICONERROR);
		return 1;
	}

	// Window configuration
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "AC Toolkit", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		MessageBoxA(NULL, "Failed to create window", "Error", MB_ICONERROR);
		return 1;
	}

	// Center window
	const GLFWvidmode* vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowPos(window,
		(vidmode->width - 800) / 2,
		(vidmode->height - 600) / 2
	);

	glfwMakeContextCurrent(window);

	// Initialize ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	
	LoadSystemFonts();
	SetupImGuiStyle();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");

	// Enable transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (isModuleLoaded(L"cb4_ac.dll", acPid) && g_MainUI)
		{
			RenderMenu(window);
		}
		else
		{
			RenderInjector();
		}

		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}