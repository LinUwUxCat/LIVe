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

// Hints
#ifdef __linux__
    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0"); //There is no need for compositor bypass in an image viewer.
#endif //__linux__


    // SDL window and renderer
    window = SDL_CreateWindow("LIVe", 800, 600, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, NULL);

    // Dear ImGui Init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
    
    bool MainLoop = true;

    bool IsImGuiDemoWindowOpen = false;
    bool IsMetadataWindowOpen = false;
    bool IsSettingsWindowOpen = false;

    bool ReloadImage = argc > 1;
    bool OpenNewImage = false;
    bool IsImageLoaded = false;
    bool SaveToBMP = false;
    char* ImagePath;
    if (ReloadImage) ImagePath = argv[1];
    float w,h;
    int x,y;
    float ZoomLevel = 1.0f;
    bool MouseImageMovement = false;
    float OffsetX = 0.0f;
    float OffsetY = 0.0f;
    bool ResetView = false;
    ParamList Metadata;
    SDL_FColor BgColor = {0.05,0.05,0.05,1.0};

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
                ImGui::MenuItem("Save Image to BMP", NULL, &SaveToBMP);
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
                ImGui::MenuItem("Background Color", NULL, &IsSettingsWindowOpen);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        if (IsImGuiDemoWindowOpen)ImGui::ShowDemoWindow(&IsImGuiDemoWindowOpen);
        
        if (IsSettingsWindowOpen){
            if (ImGui::Begin("Background Color", &IsSettingsWindowOpen)){
                ImGui::ColorEdit4("Background Color", &BgColor.r);
            }
            ImGui::End();
        }

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
        SDL_SetRenderDrawColor(renderer, BgColor.r*255, BgColor.g*255, BgColor.b*255, BgColor.a*255);
        SDL_RenderClear(renderer);

        ///Bool Conditions - Generally set by imgui

        // Open a new image
        if (OpenNewImage){
            OpenNewImage = false;
            const char *const patterns[3] = {"*.bmp", "*.tga", "*.dds"};
            ImagePath = tinyfd_openFileDialog("Open an image...", "", 3, patterns, "Image Files", 0);
            if (ImagePath) {
                ReloadImage = true;
                ResetView = true;
            }
        }

        // Image Reloading if needed
        if (ReloadImage){
            Metadata.clear();
            if (IsImageLoaded){ //Don't do this on first load.
                if (surface->flags & SDL_SURFACE_PREALLOCATED){
                    void* pixels = surface->pixels;
                    SDL_DestroySurface(surface);
                    SDL_free(pixels);
                } else SDL_DestroySurface(surface);
                
            }
            surface = ImageGetSurface(ImagePath, &Metadata);
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (texture == NULL){
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not render image"); // Replace this with a text, or an imgui popup?
            } else IsImageLoaded = true;

            ReloadImage = false;
            ResetView = true;
        }

        if (ResetView){
            ResetView = false;
            OffsetX = 0.0f;
            OffsetY = 0.0f;
            ZoomLevel = 1.0f;
            SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST); //TODO-SETTING
        }

        if (SaveToBMP){
            SaveToBMP = false;
            const char* const patterns[1] = {"*.bmp"};
            char* path = tinyfd_saveFileDialog("Save to...", "", 1, patterns, "BMP Files");
            SDL_SaveBMP(surface, path);
        }

        /// Texture Rendering
        
        SDL_GetTextureSize(texture, &w, &h);
        texr.w = w * ZoomLevel;
        texr.h = h * ZoomLevel;
        SDL_GetWindowSize(window, &x, &y);
        texr.x = x/2 - texr.w/2 + OffsetX;
        texr.y = y/2 - texr.h/2 + OffsetY;

        SDL_SetRenderViewport(renderer, NULL);
        SDL_RenderTexture(renderer, texture, NULL, &texr);
        
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
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