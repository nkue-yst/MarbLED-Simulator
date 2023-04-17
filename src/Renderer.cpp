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

#define IMGUI_MARGIN 17
#define IMGUI_TITLE_BAR_HEIGHT 18
#define IMGUI_MENU_BAR_HEIGHT 19

#define SETTING_PANEL_WIDTH  250
#define SETTING_PANEL_HEIGHT 400

Renderer::Renderer(Simulator* simulator, std::string dest_ip)
    : SimComponentBase(simulator)
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

    // Display size settings
    uint32_t display_width = 1920;
    uint32_t display_height = 1080;

    // Pixel size settings
    this->pixel_size_ = std::min(display_width / this->getParent()->getLedWidth(), display_height / this->getParent()->getLedHeight()) / 2;

    // Create window
    SDL_WindowFlags win_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
    this->win_width_ = this->pixel_size_ * this->getParent()->getLedWidth();
    this->win_height_ = this->pixel_size_ * this->getParent()->getLedHeight() * 2;

    this->win_ = SDL_CreateWindow(
        "MarbLED SDK",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        this->win_width_ + IMGUI_MARGIN + SETTING_PANEL_WIDTH,
        this->win_height_ + IMGUI_MARGIN * 2 + IMGUI_TITLE_BAR_HEIGHT + IMGUI_MENU_BAR_HEIGHT,
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
        if (ev.button.button == SDL_BUTTON_LEFT || (ev.motion.state & SDL_BUTTON_LMASK != 0))
        {
            UdpTransmitSocket sock(IpEndpointName(this->dest_ip_.c_str(), 9000));

            char buff[1024];
            osc::OutboundPacketStream p(buff, 1024);

            // Check window range
            if (IMGUI_MARGIN / 2 < ev.button.x && ev.button.x < IMGUI_MARGIN / 2 + this->pixel_size_ * this->getParent()->getLedWidth())
            {
                int32_t pos_x = (ev.button.x - IMGUI_MARGIN / 2) / this->pixel_size_;
                int32_t pos_y = -1;

                if (IMGUI_MARGIN / 2 + IMGUI_MENU_BAR_HEIGHT + IMGUI_TITLE_BAR_HEIGHT < ev.button.y && ev.button.y < IMGUI_MARGIN / 2 + IMGUI_MENU_BAR_HEIGHT + IMGUI_TITLE_BAR_HEIGHT + this->pixel_size_ * this->getParent()->getLedHeight())
                {
                    pos_y = (ev.button.y - IMGUI_TITLE_BAR_HEIGHT - IMGUI_MENU_BAR_HEIGHT - IMGUI_MARGIN / 2) / this->pixel_size_;
                }
                else if (IMGUI_MARGIN + IMGUI_MARGIN / 2 + IMGUI_MENU_BAR_HEIGHT + IMGUI_TITLE_BAR_HEIGHT + this->pixel_size_ * this->getParent()->getLedHeight() < ev.button.y && ev.button.y < IMGUI_MARGIN + IMGUI_MARGIN / 2 + IMGUI_MENU_BAR_HEIGHT + IMGUI_TITLE_BAR_HEIGHT + this->pixel_size_ * this->getParent()->getLedHeight() * 2)
                {
                    pos_y = (ev.button.y - IMGUI_TITLE_BAR_HEIGHT - IMGUI_MENU_BAR_HEIGHT - IMGUI_MARGIN - IMGUI_MARGIN / 2 - this->pixel_size_ * this->getParent()->getLedHeight()) / this->pixel_size_;
                }

                if (pos_y != -1)
                {
                    if (ev.button.type == SDL_MOUSEBUTTONDOWN || ev.motion.type == SDL_MOUSEMOTION)
                    {
                        p << osc::BeginBundleImmediate
                            << osc::BeginMessage("/touch/0/point")
                                << pos_x
                                << pos_y
                            << osc::EndMessage
                        << osc::EndBundle;
                        sock.Send(p.Data(), p.Size());
                    }
                    else if (ev.button.type == SDL_MOUSEBUTTONUP)
                    {
                        p << osc::BeginBundleImmediate
                            << osc::BeginMessage("/touch/0/delete")
                            << osc::EndMessage
                        << osc::EndBundle;
                        sock.Send(p.Data(), p.Size());
                    }
                }
            }
        }
    }

    ///////////////////////////////////
    ///// Create simulation image /////
    ///////////////////////////////////
    // Set background mat
    this->sim_chip_img_ = cv::Mat(
        this->pixel_size_ * this->getParent()->getLedHeight(),
        this->pixel_size_ * this->getParent()->getLedWidth(),
        CV_8UC3,
        cv::Scalar(120, 120, 120)
    );

    // Draw chips
    std::vector<Color> color_mat = this->getParent()->getColors();
    uint32_t width = this->getParent()->getLedWidth();
    uint32_t height = this->getParent()->getLedHeight();

    for (uint32_t y = 0; y < height; y++)
    {
        for (uint32_t x = 0; x < width; x++)
        {
            Color p_color = color_mat[x + width * y];

            if (!(p_color.r == 0 && p_color.g == 0 && p_color.b == 0))
            {
                cv::circle(
                    this->sim_chip_img_,
                    cv::Point(
                        this->pixel_size_ * x + this->pixel_size_ / 2,
                        this->pixel_size_ * y + this->pixel_size_ / 2
                    ),
                    static_cast<int32_t>(this->pixel_size_ / 2),
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

    // Start new ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    //////////////////////////////
    ///// Draw setting panel /////
    //////////////////////////////
    ImGui::SetNextWindowSize(ImVec2(SETTING_PANEL_WIDTH, this->win_height_ + IMGUI_MARGIN * 2 + IMGUI_TITLE_BAR_HEIGHT + IMGUI_MENU_BAR_HEIGHT), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(IMGUI_MARGIN + this->pixel_size_ * this->getParent()->getLedWidth(), 0.f));
    ImGui::Begin("Simulator settings", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    // Radio button to select marble thickness
    static int32_t marble_thickness = 6;
    ImGui::TextUnformatted("Marble worktop thickness");
    ImGui::RadioButton("6mm", &marble_thickness, 6);
    ImGui::SameLine();
    ImGui::RadioButton("9mm", &marble_thickness, 9);
    ImGui::SameLine();
    ImGui::RadioButton("11mm", &marble_thickness, 11);

    // End draw setting panel
    ImGui::End();

    ////////////////////////////////////
    ///// Draw led-chip simulation /////
    ////////////////////////////////////
    ImGui::SetNextWindowSize(ImVec2(this->win_width_ + IMGUI_MARGIN, this->win_height_ / 2 + IMGUI_MARGIN + IMGUI_TITLE_BAR_HEIGHT + IMGUI_MENU_BAR_HEIGHT), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
    ImGui::Begin("MarbLED: Simulator", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    /////////////////////////
    ///// Draw menu bar /////
    /////////////////////////
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save"))
            {
                auto format_date = [](const std::chrono::system_clock::time_point& tp, const std::string& format)
                {
                    std::time_t t = std::chrono::system_clock::to_time_t(tp);
                    std::tm tm = *std::localtime(&t);

                    std::ostringstream oss;
                    oss << std::put_time(&tm, format.c_str());

                    return oss.str();
                };

                auto now = std::chrono::system_clock::now();
                std::string formatted_date = format_date(now, "%Y-%m-%d-%H-%M-%S");

                cv::imwrite("Chip_" + formatted_date + ".png", this->sim_chip_img_);
                cv::imwrite("Marble_" + formatted_date + ".png", this->sim_marble_img_);
            }

            if (ImGui::MenuItem("Exit"))
            {
                this->getParent()->setQuitFlag(true);
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    // Convert and draw simulation image
    ImVec2 uv0 = ImVec2(0, 0);
    ImVec2 uv1 = ImVec2(1, 1);

    GLuint simulation_img = this->convertCVmatToGLtexture(&this->sim_chip_img_);
    ImGui::Image((void*)(uintptr_t)simulation_img, ImVec2(this->win_width_, this->win_height_ / 2), uv0, uv1);

    // End led-chip simulation
    ImGui::End();
    
    /////////////////////////////////////
    ///// Drawing marble simulation /////
    /////////////////////////////////////
    ImGui::SetNextWindowSize(ImVec2(this->win_width_ + IMGUI_MARGIN, this->win_height_ / 2 + IMGUI_MARGIN + IMGUI_MENU_BAR_HEIGHT), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(0.f, this->win_height_ / 2 + IMGUI_MARGIN + IMGUI_TITLE_BAR_HEIGHT + IMGUI_MENU_BAR_HEIGHT));
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

    cv::GaussianBlur(this->sim_chip_img_, this->sim_marble_img_, cv::Size(kernel_size, kernel_size), 0);
    simulation_img = this->convertCVmatToGLtexture(&this->sim_marble_img_);
    ImGui::Image((void*)(uintptr_t)simulation_img, ImVec2(this->win_width_, this->win_height_ / 2), uv0, uv1);

    // End marble simulation
    ImGui::End();

    /////////////////
    /// Rendering ///
    /////////////////
    ImGui::Render();

    glViewport(0, 0, (int)this->io_->DisplaySize.x, (int)this->io_->DisplaySize.y);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(this->win_);
}

GLuint Renderer::convertCVmatToGLtexture(cv::Mat* mat)
{
    GLuint texture_id;

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    cv::cvtColor((*mat), (*mat), cv::COLOR_RGB2BGR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (*mat).cols, (*mat).rows, 0, GL_RGB, GL_UNSIGNED_BYTE, (*mat).ptr());

    cv::cvtColor((*mat), (*mat), cv::COLOR_BGR2RGB);

    return texture_id;
}
