#include "editor.hpp"
#include "config/init.hpp"
#include "config/constants.hpp"
#include "ui/ui.hpp"

#include "IconsFontAwesome6.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"

#include <stdexcept>
#include <filesystem>
namespace fs = std::filesystem;

Editor::Editor(SDL_Window * window, SDL_Renderer * renderer) : window(window), renderer(renderer) {
    init::initImGUI(window, renderer);
    initFonts();
    initKeys();
    initAudio();

    setWindowIcon();
    Preferences::Instance().loadFromFile(constants::PREFERENCES_PATH);
    initLastDirPaths();
}

void Editor::initFonts() {
    ImGuiIO & io = ImGui::GetIO(); (void)io;

    ImFontConfig config;
    config.PixelSnapH = true;

    menuFont = io.Fonts->AddFontFromFileTTF(constants::MENU_FONT_PATH.string().c_str(), constants::MENU_FONT_SIZE, &config);

    config.MergeMode = true;
    config.GlyphMinAdvanceX = 13.f;

    static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    io.Fonts->AddFontFromFileTTF(constants::ICON_FONT_PATH.string().c_str(), 13.0f, &config, icon_ranges);
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
    audioSystem.loadSound("keypress", constants::KEYPRESS_SOUND_PATH.string());
}

void Editor::setWindowIcon() {
    // set window icon
    SDL_Surface * icon = IMG_Load(constants::WINDOW_ICON_PATH.string().c_str());
    SDL_SetWindowIcon(window, icon);
    SDL_FreeSurface(icon);
}

void Editor::quit() {
    running = false;

    audioSystem.quitAudioSystem();

    Preferences::Instance().saveToFile(constants::PREFERENCES_PATH);

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

        const Uint8 * keyStates = SDL_GetKeyboardState(nullptr);
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

        showMenuBar(menuFont, renderer, &audioSystem);
        showInitEditWindow(&audioSystem, renderer);
        showOpenChartWindow(renderer, &audioSystem);
        showEditWindows(&audioSystem, keysPressed);

        updateShortcuts();

        Preferences::Instance().showPreferencesWindow(&audioSystem);
    }
}

void Editor::updateShortcuts() {
    if(shortcutsActivated.at(SDLK_n)) {
        startNewEditWindow();
    }

    if(shortcutsActivated.at(SDLK_p)) {
        Preferences::Instance().setShowPreferences(true);
    }

    if(shortcutsActivated.at(SDLK_s)) {
        startSaveCurrentChart();
    }

    if(shortcutsActivated.at(SDLK_o)) {
        startOpenChart();
    }

    if(shortcutsActivated.at(SDLK_z)) {
        setUndo();
    }

    if(shortcutsActivated.at(SDLK_y)) {
        setRedo();
    }
}

void Editor::render() {
    if(running) {
        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, constants::BG_R, constants::BG_G, constants::BG_B, constants::BG_A);
        SDL_RenderClear(renderer);

        ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);
    }
}

bool Editor::isRunning() const {
    return running;
}
