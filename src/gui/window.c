#include "window.h"

#include "fonts.h"

#include <dcimgui/dcimgui_impl_sdl3.h>
#include <dcimgui/dcimgui_impl_sdlgpu3.h>

#include <globals.h>

bool gui_window_init(Gui* gui) {
    // FIXME: handle DPI correctly
    // TODO: see if still needs to draw one frame hidden on other platforms, linux wayland doesnt
    // FIXME: load and save window size from imgui.ini
    // FIXME: use data path for imgui.ini
    gui->window = SDL_CreateWindow(
        "F95Checker WIP C Rewrite",
        1280,
        720,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN);
    if(gui->window == NULL) {
        gui_perror("SDL_CreateWindow()");
        return false;
    }
    SDL_SetWindowMinimumSize(gui->window, 720, 400);
    SDL_SetWindowIcon(gui->window, gui->icons.icon);

    gui->window_gpu = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_METALLIB,
        true, // FIXME: should this be disabled in release builds?
        NULL);
    if(gui->window_gpu == NULL) {
        gui_perror("SDL_CreateGPUDevice()");
        SDL_DestroyWindow(gui->window);
        return false;
    }

    if(!SDL_ClaimWindowForGPUDevice(gui->window_gpu, gui->window)) {
        gui_perror("SDL_ClaimWindowForGPUDevice()");
        SDL_DestroyGPUDevice(gui->window_gpu);
        SDL_DestroyWindow(gui->window);
        return false;
    }
    gui_window_set_vsync(gui, settings->vsync);

    IMGUI_CHECKVERSION();
    ImGui_CreateContext(NULL);
    gui->io = ImGui_GetIO();
    gui->style = ImGui_GetStyle();
    gui->window_state.prev_size = (Vec2){0, 0};
    gui->window_state.scroll_energy = (ImVec2){0.0f, 0.0f};

    gui_fonts_init(gui);

    ImGui_ImplSDL3_InitForSDLGPU(gui->window);
    ImGui_ImplSDLGPU3_InitInfo init_info = {};
    init_info.Device = gui->window_gpu;
    init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(gui->window_gpu, gui->window);
    init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;
    ImGui_ImplSDLGPU3_Init(&init_info);

    return true;
}

void gui_window_show(Gui* gui) {
    if(SDL_GetWindowFlags(gui->window) & SDL_WINDOW_HIDDEN) {
        SDL_ShowWindow(gui->window);
    }
    if(SDL_GetWindowFlags(gui->window) & SDL_WINDOW_MINIMIZED) {
        SDL_RestoreWindow(gui->window);
    }
    SDL_RaiseWindow(gui->window);
}

void gui_window_hide(Gui* gui) {
    SDL_HideWindow(gui->window);
}

void gui_window_set_vsync(Gui* gui, bool vsync) {
    SDL_SetGPUSwapchainParameters(
        gui->window_gpu,
        gui->window,
        SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
        vsync ? SDL_GPU_PRESENTMODE_VSYNC : SDL_GPU_PRESENTMODE_MAILBOX);
}

void gui_window_process_event(Gui* gui, SDL_Event* event) {
    if(event->type == SDL_EVENT_MOUSE_WHEEL &&
       event->window.windowID == SDL_GetWindowID(gui->window)) {
        // Handle wheel events locally to apply scroll settings
        event->wheel.x *= settings->scroll_amount;
        event->wheel.y *= settings->scroll_amount;
        if(settings->scroll_smooth) {
            // Immediately stop if direction changes
            if(gui->window_state.scroll_energy.x * event->wheel.x < 0.0f) {
                gui->window_state.scroll_energy.x = 0.0f;
            }
            if(gui->window_state.scroll_energy.y * event->wheel.y < 0.0f) {
                gui->window_state.scroll_energy.y = 0.0f;
            }
        }
        gui->window_state.scroll_energy.x += event->wheel.x;
        gui->window_state.scroll_energy.y += event->wheel.y;
    }
    ImGui_ImplSDL3_ProcessEvent(event);
}

bool gui_window_should_draw(Gui* gui) {
    return (SDL_GetWindowFlags(gui->window) & (SDL_WINDOW_HIDDEN | SDL_WINDOW_MINIMIZED)) == 0;
}

void gui_window_new_frame(Gui* gui) {
    UNUSED(gui);

    // Apply smooth scrolling
    ImVec2 scroll_now;
    if(settings->scroll_smooth) {
        if(ABS(gui->window_state.scroll_energy.x) > 0.01f) {
            scroll_now.x = gui->window_state.scroll_energy.x * gui->io->DeltaTime *
                           settings->scroll_smooth_speed;
            gui->window_state.scroll_energy.x -= scroll_now.x;
        } else {
            // Cutoff smoothing when it's basically stopped
            scroll_now.x = 0.0f;
            gui->window_state.scroll_energy.x = 0.0f;
        }
        if(ABS(gui->window_state.scroll_energy.y) > 0.01f) {
            scroll_now.y = gui->window_state.scroll_energy.y * gui->io->DeltaTime *
                           settings->scroll_smooth_speed;
            gui->window_state.scroll_energy.y -= scroll_now.y;
        } else {
            // Cutoff smoothing when it's basically stopped
            scroll_now.y = 0.0f;
            gui->window_state.scroll_energy.y = 0.0f;
        }
    } else {
        scroll_now = gui->window_state.scroll_energy;
        gui->window_state.scroll_energy = (ImVec2){0, 0};
    }
    gui->io->MouseWheel = scroll_now.y;
    gui->io->MouseWheelH = -scroll_now.x;

    // Hand cursor when hovering buttons and similar
    if(ImGui_IsAnyItemHovered()) {
        ImGui_SetMouseCursor(ImGuiMouseCursor_Hand);
    }

    ImGui_ImplSDLGPU3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui_NewFrame();

    ImGui_SetNextWindowPos((ImVec2){0.0f, 0.0f}, ImGuiCond_Once);
    Vec2 window_size;
    SDL_GetWindowSize(gui->window, &window_size.x, &window_size.y);
    if(window_size.x != gui->window_state.prev_size.x ||
       window_size.y != gui->window_state.prev_size.y) {
        ImGui_SetNextWindowSize((ImVec2){(f32)window_size.x, (f32)window_size.y}, ImGuiCond_Always);
        gui->window_state.prev_size = window_size;
    }

    ImGui_PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui_Begin(
        "F95Checker",
        NULL,
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse);
    ImGui_PopStyleVar();
}

void gui_window_render(Gui* gui) {
    ImGui_End();

    ImGui_Render();
    ImDrawData* draw_data = ImGui_GetDrawData();

    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(gui->window_gpu);
    SDL_GPUTexture* swapchain_texture = NULL;
    SDL_AcquireGPUSwapchainTexture(command_buffer, gui->window, &swapchain_texture, NULL, NULL);

    if(swapchain_texture != NULL) {
        Imgui_ImplSDLGPU3_PrepareDrawData(draw_data, command_buffer);

        SDL_GPUColorTargetInfo color_target_infos = {};
        color_target_infos.texture = swapchain_texture;
        color_target_infos.load_op = SDL_GPU_LOADOP_DONT_CARE;
        color_target_infos.store_op = SDL_GPU_STOREOP_STORE;
        color_target_infos.mip_level = 0;
        color_target_infos.layer_or_depth_plane = 0;
        color_target_infos.cycle = false;
        SDL_GPURenderPass* render_pass =
            SDL_BeginGPURenderPass(command_buffer, &color_target_infos, 1, NULL);

        ImGui_ImplSDLGPU3_RenderDrawData(draw_data, command_buffer, render_pass);

        SDL_EndGPURenderPass(render_pass);

        SDL_SubmitGPUCommandBuffer(command_buffer);
    } else {
        SDL_CancelGPUCommandBuffer(command_buffer);
    }
}

void gui_window_free(Gui* gui) {
    ImGui_ImplSDLGPU3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    gui_fonts_free(gui);
    ImGui_DestroyContext(NULL);

    SDL_ReleaseWindowFromGPUDevice(gui->window_gpu, gui->window);
    SDL_DestroyGPUDevice(gui->window_gpu);
    SDL_DestroyWindow(gui->window);
}
