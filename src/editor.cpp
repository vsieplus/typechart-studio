#include "editor.hpp"

#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"

#include <stdexcept>

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const std::string PROGRAM_NAME = "Typechart Studio";

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
	ImGuiIO & io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard gui Controls

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
        lastTime = currTime;
        currTime = SDL_GetPerformanceCounter();
        float delta = (currTime - lastTime) * 1000.f / SDL_GetPerformanceFrequency();


        // new imgui frame
		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

        showBase();
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

void Editor::showBase() {
    static float f = 0.f;
    static int counter = 0;

    ImGui::Begin("hello world");

    ImGui::Text("useful text 1");

    ImGui::SliderFloat("float", &f, 0.f, 1.f);
    ImGui::ColorEdit3("color picker", (float*)&clearColor);

    // true when clicked
    if(ImGui::Button("button1")) {
        counter++;
    }

    ImGui::SameLine();
    ImGui::Text("counter  = %d", counter);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
}

bool Editor::isRunning() const {
    return running;
}