#include "editor.hpp"
#include "ui/ui.hpp"
#include "ui/windowsizes.hpp"

#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"

#include <stdexcept>
#include <filesystem>
namespace fs = std::filesystem;

#include <SDL2/SDL_image.h>

const int MENU_FONT_SIZE = 24;

const std::string PROGRAM_NAME = "Typechart Studio";

const fs::path FONTS_DIR = fs::path("fonts");
const fs::path MENU_FONT_PATH = FONTS_DIR / fs::path("Sen-Regular.ttf");

const fs::path IMAGES_DIR = fs::path("images");
const fs::path WINDOW_ICON_PATH = IMAGES_DIR / fs::path("windowIcon.png");

SDL_Window * initWindow() {
    // Setup window
    SDL_WindowFlags window_flags = (SDL_WindowFlags) (SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window * window = SDL_CreateWindow(PROGRAM_NAME.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, window_flags);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

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

    ImGuiIO & io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
}

Editor::Editor() : window(initWindow()), renderer(initRenderer(window)) {
    if(window == nullptr || renderer == nullptr ) {
        throw std::runtime_error("Failed to initialize SDL window/renderer");
    }

    initImGUI(window, renderer);
    initFonts();
    initKeys();
    initAudio();
    setWindowIcon();
}

void Editor::initFonts() {
    ImGuiIO & io = ImGui::GetIO(); (void)io;

    ImFontConfig config;
    config.PixelSnapH = true;

    menuFont = io.Fonts->AddFontFromFileTTF(MENU_FONT_PATH.c_str(), MENU_FONT_SIZE, &config);
}

void Editor::initKeys() {
    int numKeys;
    SDL_GetKeyboardState(&numKeys);
    for(int i = 0; i < numKeys; i++) {
        keysPressed.push_back(false);
        keysHeld.push_back(false);
    }
}

void Editor::initAudio() {
    audioSystem.initAudioSystem(window);
    audioSystem.loadSound("keypress", (fs::path("sounds") / fs::path("keypress.wav")).string());
}

void Editor::setWindowIcon() {
    // set window icon
    SDL_Surface * icon = IMG_Load(WINDOW_ICON_PATH.c_str());
    SDL_SetWindowIcon(window, icon);
    SDL_FreeSurface(icon);
}

void Editor::quit() {
    running = false;

    audioSystem.quitAudioSystem();

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


        const Uint8 * keyStates = SDL_GetKeyboardState(NULL);
        for(unsigned int i = 0; i < keysPressed.size(); i++) {
            if(keyStates[i] && !keysHeld[i]) {
                // if key was pressed this frame but not the last frame, mark as pressed + held
                keysPressed[i] = true;
                keysHeld[i] = true;
            } else if(keyStates[i] && keysHeld[i]) {
                // if pressed frame and last frame, mark as not pressed, but maintain held as true
                keysPressed[i] = false;
            } else {
                // otherwise, keyStates[i] is false, so mark as neither held nor pressed
                keysPressed[i] = false;
                keysHeld[i] = false;
            }
        }

        handleShortcutEvents();
    }
}

void Editor::handleShortcutEvents() {
    for(auto & [shortcutKey, activated] : shortcutsActivated) {
        SDL_Scancode currShortcutScancode = SDL_GetScancodeFromKey(shortcutKey);

        activated = ((keysHeld[SDL_SCANCODE_LCTRL] || keysHeld[SDL_SCANCODE_RCTRL]) && keysPressed[currShortcutScancode]);
    }
}

void Editor::update() {
    if(running) {
        audioSystem.update(window);

        // new imgui frame
		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

        showMenuBar(menuFont);
        showInitEditWindow(&audioSystem);
        showEditWindows();

        updateShortcuts();

        Preferences::showPreferencesWindow();
    }
}

void Editor::updateShortcuts() {
    if(shortcutsActivated.at(SDLK_n)) {
        startNewEditWindow();
    }

    if(shortcutsActivated.at(SDLK_p)) {
        Preferences::setShowPreferences();
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