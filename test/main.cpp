#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_sdl.h>
#include <imgui/backends/imgui_impl_opengl2.h>
#include "utils/memory.hpp"
#include "utils/module.hpp"

bool g_ShowGUI = false;

static int SDLCALL CustomPollEvent(SDL_Event *event)
{
	int result = SDL_PollEvent(event);
	if (result) {
		if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_INSERT) // Toggle GUI
			g_ShowGUI = !g_ShowGUI;
		
		if (g_ShowGUI)
			ImGui_ImplSDL2_ProcessEvent(event);
	}

	return result;
}

int SDLCALL hkSDL_PollEvent(SDL_Event *event)
{
	int result;
	do {
		result = CustomPollEvent(event);
	} while (g_ShowGUI && result); // Don't poll events for the game when the GUI is shown

	return result;
}

void SDLCALL hkSDL_GL_SwapWindow(SDL_Window *window)
{
	static SDL_GLContext orig_ctx = SDL_GL_GetCurrentContext();
	static SDL_GLContext ctx = nullptr;

	if (!ctx) {
		// Create extra context for custom rendering (GUI + other stuff)
		std::cout << "[*] SDL Window: " << reinterpret_cast<void *>(window) << std::endl;
		std::cout << "[*] Original Context: " << reinterpret_cast<void *>(orig_ctx) << std::endl;
		ctx = SDL_GL_CreateContext(window);
		std::cout << "[*] Context: " << reinterpret_cast<void *>(ctx) << std::endl;

		// Initialize ImGUI
		SDL_GL_MakeCurrent(window, ctx);
		auto imgui_ctx = ImGui::CreateContext();
		ImGui::SetCurrentContext(imgui_ctx);

		ImGui::GetIO().IniFilename = nullptr;
		// OBS: SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE will take over full mouse control

		ImGui_ImplSDL2_InitForOpenGL(window, ctx);
		ImGui_ImplOpenGL2_Init();
		std::cout << "[*] ImGUI Initialized" << std::endl;
	}

	SDL_GL_MakeCurrent(window, ctx);

	ImGui::GetIO().MouseDrawCursor = g_ShowGUI;
	ImGui::GetIO().WantCaptureMouse = g_ShowGUI;
	ImGui::GetIO().WantCaptureKeyboard = g_ShowGUI;

	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();

	if (g_ShowGUI) {
		ImGui::Begin("Test");
		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

	SDL_GL_MakeCurrent(window, orig_ctx);
	return SDL_GL_SwapWindow(window);
}

void __attribute__((constructor)) test_entry()
{
	std::freopen("/tmp/test.log", "w", stdout);
	std::cout << "[*] Injected" << std::endl;

	auto launcher = Module("bin/launcher.so");
	std::cout << "[*] Launcher Name: " << launcher.GetName() << std::endl;
	std::cout << "[*] Launcher Base: " << launcher.GetBase() << std::endl;
	std::cout << "[*] Launcher Size: " << launcher.GetSize() << std::endl;
	std::cout << "[*] Launcher Handle: " << launcher.GetHandle() << std::endl;

	// call <SDL_PollEvent>
	uintptr_t *pSDL_PollEvent = reinterpret_cast<uintptr_t *>(
		&(Memory::FindPattern<char *>("\x8D\xB6\x00\x00\x00\x00\x89\x34\x24\xE8\x00\x00\x00\x00", "xxxxxxxxxx????", launcher.GetBase(), launcher.GetSize()))[10]
	);
	std::cout << "[*] SDL_PollEvent Call Pointer: " << reinterpret_cast<void *>(pSDL_PollEvent) << std::endl;

	// call <SDL_GL_SwapWindow>
	uintptr_t *pSDL_GL_SwapWindow = reinterpret_cast<uintptr_t *>(
		&(reinterpret_cast<char *>(
			Memory::FindPattern("\x89\x55\xCC\x89\04\x24\xE8\x00\x00\x00\x00", "xxxxxxx????", launcher.GetBase(), launcher.GetSize())
		))[6 + 1]
	);
	std::cout << "[*] SDL_GL_SwapWindow Call Pointer: " << reinterpret_cast<void *>(pSDL_GL_SwapWindow) << std::endl;

	// Hook SDL2 functions
	Memory::Protect(reinterpret_cast<void *>(pSDL_PollEvent), sizeof(*pSDL_PollEvent), PROT_EXEC | PROT_READ | PROT_WRITE);
	*pSDL_PollEvent = reinterpret_cast<uintptr_t>(hkSDL_PollEvent) - reinterpret_cast<uintptr_t>(pSDL_PollEvent) - 4;

	Memory::Protect(reinterpret_cast<void *>(pSDL_GL_SwapWindow), sizeof(*pSDL_GL_SwapWindow), PROT_EXEC | PROT_READ | PROT_WRITE);
	*pSDL_GL_SwapWindow = reinterpret_cast<uintptr_t>(hkSDL_GL_SwapWindow) - reinterpret_cast<uintptr_t>(pSDL_GL_SwapWindow) - 4;
	
	std::cout << "[*] Hooks applied" << std::endl;
	std::cout << "====================" << std::endl;
}

void __attribute__((destructor)) test_exit()
{
	std::cout << "[*] Ejected" << std::endl;
}
