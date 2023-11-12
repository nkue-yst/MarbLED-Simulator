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

#define DEBUG std::cout<<"DEBUG: "<<__FILE__<<":"<<__LINE__<<std::endl

#define BOARD_WIDTH  18
#define BOARD_HEIGHT 18

#define SETTING_PANEL_WIDTH  250

#define MINIMUM_SIM_WIDTH 300
#define MINIMUM_WIN_WIDTH (MINIMUM_SIM_WIDTH + SETTING_PANEL_WIDTH)
#define MINIMUM_WIN_HEIGHT 400

#define PIXEL_SIZE  20
#define PIXEL_PITCH 5

#define OUTPUT_BUFFER_SIZE 1024

Renderer::Renderer(Simulator* simulator, std::string dest_ip)
    : SimComponentBase(simulator)
    , sim_width_(MINIMUM_SIM_WIDTH)
    , sim_height_(MINIMUM_WIN_HEIGHT / 2)
    , dest_ip_(dest_ip)
{
    // Initialize SDL system
    SDL_Init(SDL_INIT_VIDEO);

    // Create window
    SDL_WindowFlags win_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
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
    this->renderer_ = SDL_CreateRenderer(this->win_, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

    // Setup ImGui
    IMGUI_CHECKVERSION();
    this->imgui_context_ = ImGui::CreateContext();
    
    ImGui_ImplSDLRenderer2_Init(this->renderer_);
    ImGui_ImplSDL2_InitForSDLRenderer(this->win_, this->renderer_);
    
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

    ImGui_ImplSDLRenderer2_Shutdown();
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
    static double resize_rate = 1.0;

    // Terminate input and mouse event
    SDL_Event ev;
    while (SDL_PollEvent(&ev))
    {
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
        if (ev.button.button == SDL_BUTTON_LEFT || ev.motion.state & SDL_BUTTON_LMASK != 0)
        {
            // Get mouse position
            int32_t mouse_x = ev.button.x;
            int32_t mouse_y = ev.button.y;

            // Check simulator window range
            if ((0 <= mouse_x && mouse_x <= this->sim_width_) &&
                    ((0 < mouse_y && mouse_y < this->sim_height_) || (this->win_height_ / 2 < mouse_y && mouse_y < this->win_height_ / 2 + this->sim_height_)))
            {
                // Convert mouse position y-axis
                if (0 <= mouse_x && mouse_x <= this->sim_width_)
                {
                    if (this->win_height_ / 2 < mouse_y && mouse_y <= this->win_height_)
                    {
                        mouse_y -= this->win_height_ / 2;
                    }

                    // Convert mouse position to LED position
                    uint32_t led_x = static_cast<uint32_t>(mouse_x / resize_rate / (PIXEL_SIZE + PIXEL_PITCH));
                    uint32_t led_y = static_cast<uint32_t>(mouse_y / resize_rate / (PIXEL_SIZE + PIXEL_PITCH));

                    // std::cout << "Mouse click: (" << led_x << ", " << led_y << ")" << std::endl;

                    // Send OSC message
                    UdpTransmitSocket sock(IpEndpointName(this->dest_ip_.c_str(), 9000));
                    char buffer[OUTPUT_BUFFER_SIZE];
                    osc::OutboundPacketStream p(buffer, OUTPUT_BUFFER_SIZE);

                    if (ev.button.type == SDL_MOUSEBUTTONDOWN || ev.motion.type == SDL_MOUSEMOTION)
                    {
                        p << osc::BeginBundleImmediate
                            << osc::BeginMessage("/touch/0/point")
                                << static_cast<int32_t>(led_x)
                                << static_cast<int32_t>(led_y)
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

        // When window is resized
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
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    //////////////////////////////
    ///// Draw setting panel /////
    //////////////////////////////
    ImGui::SetNextWindowPos(ImVec2(this->win_width_ - SETTING_PANEL_WIDTH, 0.f));
    ImGui::SetNextWindowSize(ImVec2(SETTING_PANEL_WIDTH, this->win_height_), ImGuiCond_Always);
    ImGui::Begin("Simulator settings", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

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

            ////////////////////////////////
            ///// Draw chips for debug /////
            ////////////////////////////////
            // cv::circle(
            //     sim_chip_img,
            //     cv::Point(
            //         PIXEL_SIZE * (x + 0.5) + PIXEL_PITCH * (x + 1),
            //         PIXEL_SIZE * (y + 0.5) + PIXEL_PITCH * (y + 1)
            //     ),
            //     static_cast<int32_t>(
            //         PIXEL_SIZE / 2
            //     ),
            //     cv::Scalar(
            //         255,
            //         128,
            //         0
            //     ),
            //     -1
            // );
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

    // Keep the image aspect rate
    double width_ratio  = static_cast<double>(this->sim_width_) / sim_chip_img.cols;
    double height_ratio = static_cast<double>(this->sim_height_) / sim_chip_img.rows;

    resize_rate = std::min(width_ratio, height_ratio);

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

    SDL_Texture* simulation_img1 = this->convertCV_matToSDL_Texture(resized_img);
    ImGui::Image(simulation_img1, ImVec2(resized_img.cols, resized_img.rows));

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

    SDL_Texture* simulation_img2 = this->convertCV_matToSDL_Texture(resized_img);
    ImGui::Image(simulation_img2, ImVec2(resized_img.cols, resized_img.rows));

    // End marble simulation
    ImGui::End();
    ImGui::PopStyleVar();

    /////////////////////
    ///// Rendering /////
    /////////////////////
    ImGui::Render();

    SDL_RenderClear(this->renderer_);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(this->renderer_);

    // Re-set simulator image size
    SDL_QueryTexture(simulation_img1, NULL, NULL, &this->sim_width_, &this->sim_height_);

    // Control frame rate
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
}

SDL_Texture* Renderer::convertCV_matToSDL_Texture(cv::Mat& mat)
{
    // Create a new surface from the cv::Mat
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
        (void*)mat.data,
        mat.cols,
        mat.rows,
        mat.channels() * 8,
        mat.step,
        0xff0000,
        0x00ff00,
        0x0000ff,
        0
    );

    // Create a new texture from the surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(this->renderer_, surface);

    // Free the surface
    SDL_FreeSurface(surface);

    return texture;
}
