#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl3.h"
#include "imgui/imgui_impl_sdlrenderer3.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "image.h"

int main(int argc, char* argv[]){

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;

    SDL_Surface* surface;
    SDL_Texture* texture;
    SDL_FRect texr;

    if (SDL_Init(SDL_INIT_VIDEO) < 0){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not init SDL3");
    }

    window = SDL_CreateWindow("LIVe", 800, 600, SDL_WINDOW_RESIZABLE);

    renderer = SDL_CreateRenderer(window, NULL, 0);

    // Dear ImGui Init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
    
    bool MainLoop = true;

    bool IsImGuiDemoWindowOpen = false;

    bool ReloadImage = argc > 1;
    int w,h,x,y;
    float ZoomLevel = 1.0f;

    Uint64 FPSDisplayAmount = 0; //Number shown on UI, refreshes every second 
    Uint64 FPSCount = 0;         //Number that goes up by one each frame
    Uint64 FPSLastTime = SDL_GetTicks();

    while (MainLoop){

        // Event handling
        SDL_PollEvent(&event);
        ImGui_ImplSDL3_ProcessEvent(&event);
        switch(event.type){
            case SDL_EVENT_QUIT:
                MainLoop = false;
                break;
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                if (event.window.windowID == SDL_GetWindowID(window)){
                    MainLoop = false;
                }
                break;
            case SDL_EVENT_MOUSE_WHEEL:
                if (event.wheel.y > 0) ZoomLevel += 0.1f;
                else ZoomLevel -= 0.1f;
                if (ZoomLevel < 0.1f) ZoomLevel = 0.1f;
                break;
        }


        // Dear ImGui rendering
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    
        if(ImGui::BeginMainMenuBar()){
            if (ImGui::BeginMenu("File")){
                //ImGui::MenuItem("Open file", NULL, &hasToOpen);
                ImGui::MenuItem("Reload current image",NULL,&ReloadImage);
                ImGui::MenuItem("Quit", NULL, &MainLoop);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Info")){
                ImGui::MenuItem("Dear ImGui Demo Window", NULL, &IsImGuiDemoWindowOpen);
                ImGui::Value("FPS", (int)FPSDisplayAmount);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        if (IsImGuiDemoWindowOpen)ImGui::ShowDemoWindow(&IsImGuiDemoWindowOpen);

        ImGui::Render();

        //SDL Rendering start
        SDL_SetRenderDrawColor(renderer, (Uint8)0, (Uint8)0, (Uint8)0, (Uint8)255);
        SDL_RenderClear(renderer);


        // Image Reloading if needed
        if (ReloadImage){
            surface = ImageGetSurface(argv[1]);
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (texture == NULL){
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not render image"); // Replace this with a text, or an imgui popup?
            }


            ReloadImage = false;
        }

        SDL_QueryTexture(texture, NULL, NULL, &w, &h);
        texr.w = w * ZoomLevel;
        texr.h = h * ZoomLevel;
        SDL_GetWindowSize(window, &x, &y);
        texr.x = x/2 - texr.w/2;
        texr.y = y/2 - texr.h/2;

        // Image rendering
        SDL_SetRenderViewport(renderer, NULL);
        SDL_RenderTexture(renderer, texture, NULL, &texr);
        
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);

        if (FPSLastTime <= SDL_GetTicks() - 1000){
            FPSLastTime = SDL_GetTicks();
            FPSDisplayAmount = FPSCount;
            FPSCount = 0;
        }
        FPSCount++;
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}