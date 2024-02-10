#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl3.h"
#include "imgui/imgui_impl_sdlrenderer3.h"
#include "tinyfiledialogs/tinyfiledialogs.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "types.h"
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
    bool IsMetadataWindowOpen = false;

    bool ReloadImage = argc > 1;
    bool OpenNewImage = false;
    char* ImagePath;
    if (ReloadImage) ImagePath = argv[1];
    int w,h,x,y;
    float ZoomLevel = 1.0f;
    bool MouseImageMovement = false;
    float OffsetX = 0.0f;
    float OffsetY = 0.0f;
    bool ResetView = false;
    ParamList Metadata;

    Uint64 FPSDisplayAmount = 0; //Number shown on UI, refreshes every second 
    Uint64 FPSCount = 0;         //Number that goes up by one each frame
    Uint64 FPSLastTime = SDL_GetTicks();

    while (MainLoop){

        // Event handling
        SDL_PollEvent(&event);
        ImGui_ImplSDL3_ProcessEvent(&event);
        ImGuiIO& io = ImGui::GetIO();
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
                if (io.WantCaptureMouse)break; //Do not zoom if scrolling ui
                if (event.wheel.y > 0) ZoomLevel += 0.1f;
                else ZoomLevel -= 0.1f;
                if (ZoomLevel < 0.1f) ZoomLevel = 0.1f;
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (io.WantCaptureMouse)break;
                if (event.button.button == SDL_BUTTON_LEFT) MouseImageMovement = true;
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (event.button.button == SDL_BUTTON_LEFT) MouseImageMovement = false;
                break;
            case SDL_EVENT_MOUSE_MOTION:
                if (MouseImageMovement){
                    OffsetX += event.motion.xrel;
                    OffsetY += event.motion.yrel;
                }
                break;
        }

        // Dear ImGui rendering
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    
        if(ImGui::BeginMainMenuBar()){
            if (ImGui::BeginMenu("File")){
                ImGui::MenuItem("Open file", NULL, &OpenNewImage);
                ImGui::MenuItem("Reload current image",NULL,&ReloadImage);
                ImGui::MenuItem("Quit", NULL, &MainLoop);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Image")){
                ImGui::MenuItem("Reset View", NULL, &ResetView);
                ImGui::MenuItem("Metadata", NULL, &IsMetadataWindowOpen);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Debug")){
                ImGui::MenuItem("Dear ImGui Demo Window", NULL, &IsImGuiDemoWindowOpen);
                ImGui::Value("FPS", (int)FPSDisplayAmount);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        if (IsImGuiDemoWindowOpen)ImGui::ShowDemoWindow(&IsImGuiDemoWindowOpen);

        if (IsMetadataWindowOpen){
            if (ImGui::Begin("Image Metadata", &IsMetadataWindowOpen)){
                if (ImGui::BeginTable("metatable", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)){
                    ImGui::TableSetupColumn("Property");
                    ImGui::TableSetupColumn("Value");
                    ImGui::TableHeadersRow();

                    for (int i=0; i<Metadata.length(); i++){ //the length function here takes care of checking if parameters and values are the same length
                        ImGui::TableNextColumn();
                        ImGui::Text(Metadata.parameters[i]);
                        ImGui::TableNextColumn();
                        ImGui::Text(Metadata.values[i]);
                    }

                    ImGui::EndTable();
                }

            }
            ImGui::End();
        }

        ImGui::Render();

        //SDL Rendering start
        SDL_SetRenderDrawColor(renderer, (Uint8)0, (Uint8)0, (Uint8)0, (Uint8)255);
        SDL_RenderClear(renderer);

        ///Bool Conditions - Generally set by imgui

        // Open a new image
        if (OpenNewImage){
            OpenNewImage = false;
            const char *const patterns[2] = {"*.bmp", "*.tga"};
            ImagePath = tinyfd_openFileDialog("Open an image...", "", 2, patterns, "Image Files", 0);
            if (ImagePath) {
                ReloadImage = true;
                ResetView = true;
            }
        }

        // Image Reloading if needed
        if (ReloadImage){
            Metadata.clear();
            if (argc > 1) surface = ImageGetSurface(ImagePath, &Metadata);
            else surface = NULL;
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (texture == NULL){
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not render image"); // Replace this with a text, or an imgui popup?
            }

            ReloadImage = false;
        }

        if (ResetView){
            ResetView = false;
            OffsetX = 0.0f;
            OffsetY = 0.0f;
            ZoomLevel = 1.0f;
        }

        /// Texture Rendering
        
        SDL_QueryTexture(texture, NULL, NULL, &w, &h);
        texr.w = w * ZoomLevel;
        texr.h = h * ZoomLevel;
        SDL_GetWindowSize(window, &x, &y);
        texr.x = x/2 - texr.w/2 + OffsetX;
        texr.y = y/2 - texr.h/2 + OffsetY;

        SDL_SetRenderViewport(renderer, NULL);
        SDL_RenderTexture(renderer, texture, NULL, &texr);
        
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);

        /// FPS Count - Very end of the frame

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