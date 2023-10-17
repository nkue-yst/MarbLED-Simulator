#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

#include "osc/OscOutboundPacketStream.h"
#include "ip/IpEndpointName.h"
#include "ip/UdpSocket.h"

#include "Renderer.hpp"
#include "Color.hpp"
#include "Common.hpp"
#include "Simulator.hpp"
#include "Socket.hpp"

#define IMGUI_MARGIN 17
#define IMGUI_TITLE_BAR_HEIGHT 18
#define IMGUI_MENU_BAR_HEIGHT 19

#define BOARD_WIDTH  18
#define BOARD_HEIGHT 18

#define SETTING_PANEL_WIDTH  250

#define MINIMUM_SIM_WIDTH 300
#define MINIMUM_WIN_WIDTH (MINIMUM_SIM_WIDTH + SETTING_PANEL_WIDTH)
#define MINIMUM_WIN_HEIGHT 400

#define PIXEL_SIZE  20
#define PIXEL_PITCH 5

Renderer::Renderer(Simulator* simulator, std::string dest_ip)
    : SimComponentBase(simulator)
    , sim_width_(MINIMUM_SIM_WIDTH)
    , sim_height_((MINIMUM_WIN_HEIGHT - IMGUI_TITLE_BAR_HEIGHT - IMGUI_MARGIN * 4) / 2)
    , dest_ip_(dest_ip)
{
    // Initialize SDL system
    SDL_Init(SDL_INIT_VIDEO);

    // Initialize OpenGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // Create window
    SDL_WindowFlags win_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    this->win_width_  = MINIMUM_WIN_WIDTH;
    this->win_height_ = MINIMUM_WIN_HEIGHT;

    this->win_ = SDL_CreateWindow(
        "MarbLED Simulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        this->win_width_,
        this->win_height_,
        win_flags
    );

    // Create renderer
    this->gl_context_ = SDL_GL_CreateContext(this->win_);
    SDL_GL_MakeCurrent(this->win_, this->gl_context_);
    SDL_GL_SetSwapInterval(1);
    this->renderer_ = SDL_CreateRenderer(this->win_, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

    // Setup ImGui
    const char* glsl_version = "#version 130";
    IMGUI_CHECKVERSION();
    this->imgui_context_ = ImGui::CreateContext();
    
    ImGui_ImplSDL2_InitForOpenGL(this->win_, this->gl_context_);
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui::StyleColorsDark();

    this->io_ = &ImGui::GetIO();
    this->io_->Fonts->Build();

    this->update();

    printLog("Init Renderer", true);
}

Renderer::~Renderer()
{
    SDL_DestroyRenderer(this->renderer_);
    SDL_DestroyWindow(this->win_);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_Quit();

    printLog("Destroy Renderer", true);
}

void Renderer::update()
{
    /////////////////////////
    ///// Frame counter /////
    /////////////////////////
    static uint64_t frame = 0;
    ++frame;
    //std::cout << frame << ": End the frame." << std::endl;

    bool win_resizing = false;

    // Terminate input and mouse event
    SDL_Event ev;
    while (SDL_PollEvent(&ev))
    {
        ImGui_ImplSDL2_ProcessEvent(&ev);

        // Terminate event
        if (ev.type == SDL_QUIT)
        {
            this->getParent()->setQuitFlag(true);
        }
        else if (ev.type == SDL_WINDOWEVENT && ev.window.event == SDL_WINDOWEVENT_CLOSE && ev.window.windowID == SDL_GetWindowID(this->win_))
        {
            this->getParent()->setQuitFlag(true);
        }

        /////////////////////////////
        ///// Mouse click event /////
        /////////////////////////////
        // When mouse left button is pressed
        // if (ev.button.button == SDL_BUTTON_LEFT || (ev.motion.state & SDL_BUTTON_LMASK != 0))
        // {
        //     UdpTransmitSocket sock(IpEndpointName(this->dest_ip_.c_str(), 9000));
// 
        //     char buff[1024];
        //     osc::OutboundPacketStream p(buff, 1024);
// 
        //     // Check window range
        //     if (IMGUI_MARGIN / 2 < ev.button.x && ev.button.x < IMGUI_MARGIN / 2 + this->pixel_size_ * this->getParent()->getLedWidth())
        //     {
        //         int32_t pos_x = (ev.button.x - IMGUI_MARGIN / 2) / this->pixel_size_;
        //         int32_t pos_y = -1;
// 
        //         if (IMGUI_MARGIN / 2 + IMGUI_MENU_BAR_HEIGHT + IMGUI_TITLE_BAR_HEIGHT < ev.button.y && ev.button.y < IMGUI_MARGIN / 2 + IMGUI_MENU_BAR_HEIGHT + IMGUI_TITLE_BAR_HEIGHT + this->pixel_size_ * this->getParent()->getLedHeight())
        //         {
        //             pos_y = (ev.button.y - IMGUI_TITLE_BAR_HEIGHT - IMGUI_MENU_BAR_HEIGHT - IMGUI_MARGIN / 2) / this->pixel_size_;
        //         }
        //         else if (IMGUI_MARGIN + IMGUI_MARGIN / 2 + IMGUI_MENU_BAR_HEIGHT + IMGUI_TITLE_BAR_HEIGHT + this->pixel_size_ * this->getParent()->getLedHeight() < ev.button.y && ev.button.y < IMGUI_MARGIN + IMGUI_MARGIN / 2 + IMGUI_MENU_BAR_HEIGHT + IMGUI_TITLE_BAR_HEIGHT + this->pixel_size_ * // this->getParent()->getLedHeight() * 2)
        //         {
        //             pos_y = (ev.button.y - IMGUI_TITLE_BAR_HEIGHT - IMGUI_MENU_BAR_HEIGHT - IMGUI_MARGIN - IMGUI_MARGIN / 2 - this->pixel_size_ * this->getParent()->getLedHeight()) / this->pixel_size_;
        //         }
// 
        //         if (pos_y != -1)
        //         {
        //             if (ev.button.type == SDL_MOUSEBUTTONDOWN || ev.motion.type == SDL_MOUSEMOTION)
        //             {
        //                 p << osc::BeginBundleImmediate
        //                     << osc::BeginMessage("/touch/0/point")
        //                         << pos_x
        //                         << pos_y
        //                     << osc::EndMessage
        //                 << osc::EndBundle;
        //                 sock.Send(p.Data(), p.Size());
        //             }
        //             else if (ev.button.type == SDL_MOUSEBUTTONUP)
        //             {
        //                 p << osc::BeginBundleImmediate
        //                     << osc::BeginMessage("/touch/0/delete")
        //                     << osc::EndMessage
        //                 << osc::EndBundle;
        //                 sock.Send(p.Data(), p.Size());
        //             }
        //         }
        //     }
        // }

        if (ev.type == SDL_WINDOWEVENT && ev.window.event == SDL_WINDOWEVENT_RESIZED)
        {
            win_resizing = true;
        }
    }

    SDL_SetWindowMinimumSize(this->win_, MINIMUM_WIN_WIDTH, MINIMUM_WIN_HEIGHT);

    if (win_resizing)
    {
        //std::cout << frame << ": Skip the frame." << std::endl;
        return;
    }

    // Start new ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    //////////////////////////////
    ///// Draw setting panel /////
    //////////////////////////////
    ImGui::SetNextWindowPos(ImVec2(this->win_width_ - SETTING_PANEL_WIDTH, 0.f));
    ImGui::SetNextWindowSize(ImVec2(SETTING_PANEL_WIDTH, this->win_height_), ImGuiCond_Always);
    ImGui::Begin("Simulator settings", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    // Slider for size of LED matrix
    // static int32_t led_width  = 64;
    // static int32_t led_height = 32;
    // ImGui::TextUnformatted("LED Size");
    // ImGui::SliderInt("Width",  &led_width,  BOARD_WIDTH,  BOARD_WIDTH  * 10);
    // ImGui::SliderInt("Height", &led_height, BOARD_HEIGHT, BOARD_HEIGHT * 10);

    // ImGui::TextUnformatted("");

    // Radio button to select marble thickness
    static int32_t marble_thickness = 6;
    ImGui::TextUnformatted("Marble Worktop Thickness");
    ImGui::RadioButton("6mm", &marble_thickness, 6);
    ImGui::SameLine();
    ImGui::RadioButton("9mm", &marble_thickness, 9);
    ImGui::SameLine();
    ImGui::RadioButton("11mm", &marble_thickness, 11);

    ImGui::TextUnformatted("");

    // Slider for room brightness
    static int32_t room_brightness = 50;
    ImGui::TextUnformatted("Room Brightness");
    ImGui::SliderInt("", &room_brightness, 0, 100);
    ImGui::SameLine();
    if (ImGui::Button("Reset"))
    {
        room_brightness = 50;
    }

    // End draw setting panel
    ImGui::End();

    ///////////////////////////////////
    ///// Create simulation image /////
    ///////////////////////////////////
    uint32_t width  = this->getParent()->getLedWidth();
    uint32_t height = this->getParent()->getLedHeight();

    // Set background color
    cv::Scalar bg_color = cv::Scalar(room_brightness + 50, room_brightness + 50, room_brightness + 50);

    // Simulator image before processing
    cv::Mat sim_chip_img = cv::Mat(
        PIXEL_SIZE * height + PIXEL_PITCH * (height + 1),
        PIXEL_SIZE * width  + PIXEL_PITCH * (width  + 1),
        CV_8UC3,
        bg_color
    );

    // Draw chips
    std::vector<Color> color_mat = this->getParent()->getColors();

    for (uint32_t y = 0; y < height; ++y)
    {
        for (uint32_t x = 0; x < width; ++x)
        {
            Color p_color = color_mat[x + width * y];

            if (!(p_color.r == 0 && p_color.g == 0 && p_color.b == 0))
            {
                cv::circle(
                    sim_chip_img,
                    cv::Point(
                        PIXEL_SIZE * (x + 0.5) + PIXEL_PITCH * (x + 1),
                        PIXEL_SIZE * (y + 0.5) + PIXEL_PITCH * (y + 1)
                    ),
                    static_cast<int32_t>(
                        PIXEL_SIZE / 2
                    ),
                    cv::Scalar(
                        p_color.r,
                        p_color.g,
                        p_color.b
                    ),
                    -1
                );
            }
        }
    }

    // cv::imshow("Simulator - Origin", sim_chip_img);
    // cv::waitKey(16);

    ////////////////////////////////////
    ///// Draw led-chip simulation /////
    ////////////////////////////////////
    SDL_GetWindowSize(this->win_, &this->win_width_, &this->win_height_);

    this->sim_width_  = this->win_width_ - SETTING_PANEL_WIDTH;
    this->sim_height_ = this->win_height_ / 2;

    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
    ImGui::SetNextWindowSize(ImVec2(this->sim_width_, this->sim_height_), ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("MarbLED: Simulator", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

    /////////////////////////////////////////////
    ///// Convert and draw simulation image /////
    /////////////////////////////////////////////
    this->sim_width_ -= this->sim_width_ % 4;    // Round the width of the simulator image to a multiple of 4

    // Keep the image aspect rate
    double width_ratio  = static_cast<double>(this->sim_width_) / sim_chip_img.cols;
    double height_ratio = static_cast<double>(this->sim_height_) / sim_chip_img.rows;

    double resize_rate = std::min(width_ratio, height_ratio);

    cv::Mat resized_img;    // Simulator image after resizing
    cv::resize(
        sim_chip_img,
        resized_img,
        cv::Size(),
        resize_rate,
        resize_rate
    );

    // cv::imshow("Simulator - resized", resized_img);
    // cv::waitKey(16);

    GLuint simulation_img1 = this->convertCVmatToGLtexture(&resized_img);
    ImGui::Image((void*)(uintptr_t)simulation_img1, ImVec2(resized_img.cols, resized_img.rows));

    // End led-chip simulation
    ImGui::End();

    /////////////////////////////////////
    ///// Drawing marble simulation /////
    /////////////////////////////////////
    ImGui::SetNextWindowPos(ImVec2(0.f, sim_height_));
    ImGui::SetNextWindowSize(ImVec2(this->sim_width_, this->sim_height_), ImGuiCond_Always);
    ImGui::Begin("Marble Simulation", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    // Convert and draw simulation image
    int32_t kernel_size;
    switch (marble_thickness)
    {
    case 6:
        kernel_size = 37;
        break;
    case 9:
        kernel_size = 41;
        break;
    case 11:
        kernel_size = 55;
        break;
    default:
        break;
    }

    cv::Mat sim_marble_img;
    cv::GaussianBlur(sim_chip_img, sim_marble_img, cv::Size(kernel_size, kernel_size), 0);

    cv::resize(
        sim_marble_img,
        resized_img,
        cv::Size(),
        resize_rate,
        resize_rate
    );

    GLuint simulation_img2 = this->convertCVmatToGLtexture(&resized_img);
    ImGui::Image((void*)(uintptr_t)simulation_img2, ImVec2(resized_img.cols, resized_img.rows));

    // End marble simulation
    ImGui::End();
    ImGui::PopStyleVar();

    /////////////////////
    ///// Rendering /////
    /////////////////////
    ImGui::Render();

    glViewport(0, 0, (int)this->io_->DisplaySize.x, (int)this->io_->DisplaySize.y);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(this->win_);

    glDeleteTextures(1, &simulation_img1);
    glDeleteTextures(1, &simulation_img2);

    // Control frame rate
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
}

GLuint Renderer::convertCVmatToGLtexture(cv::Mat* mat)
{
    GLuint texture_id;

    cv::Mat temp_mat;
    cv::cvtColor((*mat), temp_mat, cv::COLOR_RGB2BGR);

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, temp_mat.cols, temp_mat.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, temp_mat.ptr());

    return texture_id;
}
