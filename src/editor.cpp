#include "editor.hpp"
#include "ui/ui.hpp"

#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"

#include <stdexcept>
#include <filesystem>
namespace fs = std::filesystem;

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

const int MENU_FONT_SIZE = 24;

const std::string PROGRAM_NAME = "Typechart Studio";

const fs::path FONTS_DIR = fs::path("fonts");
const fs::path MENU_FONT_PATH = FONTS_DIR / fs::path("Sen-Regular.ttf");

SDL_Window * initWindow() {
    // Setup window
    SDL_WindowFlags window_flags = (SDL_WindowFlags) (SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window * window = SDL_CreateWindow(PROGRAM_NAME.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, window_flags);

    return window;
}

SDL_Renderer * initRenderer(SDL_Window * window) {
	// setup renderer
	SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        SDL_Log("Error creating SDL_Renderer!");
    }

    return renderer;
}

void initImGUI(SDL_Window * window, SDL_Renderer * renderer) {
	// setup imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// setup platform, rendering backends
	ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer_Init(renderer);
}

Editor::Editor() : window(initWindow()), renderer(initRenderer(window)) {
    if(window == nullptr || renderer == nullptr ) {
        throw std::runtime_error("Failed to initialize SDL window/renderer");
    }

    initImGUI(window, renderer);
    initFonts();
}

void Editor::initFonts() {
    ImGuiIO & io = ImGui::GetIO(); (void)io;

    ImFontConfig config;
    config.PixelSnapH = true;

    menuFont = io.Fonts->AddFontFromFileTTF(MENU_FONT_PATH.c_str(), MENU_FONT_SIZE, &config);
}

void Editor::quit() {
    running = false;

	ImGui_ImplSDLRenderer_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void Editor::loop() {
    handleEvents();
    update();
    render();
}

void Editor::handleEvents() {
    if(running) {
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);
			if(event.type == SDL_QUIT) {
				running = false;
			}
		}
    }
}

void Editor::update() {
    if(running) {
        // new imgui frame
		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

        showMenuBar(menuFont);
    }
}

void Editor::render() {
    if(running) {


		ImGui::Render();
		SDL_SetRenderDrawColor(renderer, (Uint8)(clearColor.x * 255), (Uint8)(clearColor.y * 255), (Uint8)(clearColor.z * 255), (Uint8)(clearColor.w * 255));
		SDL_RenderClear(renderer);

		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
		SDL_RenderPresent(renderer);
    }
}

bool Editor::isRunning() const {
    return running;
}