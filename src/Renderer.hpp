#pragma once

#include <cstdint>
#include <string>

#include "SDL.h"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

#include <opencv2/opencv.hpp>

#include "SimComponentBase.hpp"

class Renderer : public SimComponentBase
{
public:
    Renderer(class Simulator* simulator, std::string dest_ip);
    ~Renderer();

    void update();

private:
    SDL_Texture* convertCV_matToSDL_Texture(cv::Mat& mat);

    /// Chip simulator window name
    std::string sim_chip_window_ = "Chip Simulator";

    /// Marble simulator window name
    std::string sim_marble_window_ = "Marble Simulator";

    // Window size
    int32_t win_width_;
    int32_t win_height_;

    SDL_Window* win_;
    SDL_Renderer* renderer_;
    
    ImGuiContext* imgui_context_;

    ImGuiIO* io_;

public:
    int32_t sim_width_;
    int32_t sim_height_;

    /// Destination ip address for mouse event
    std::string dest_ip_;
};
