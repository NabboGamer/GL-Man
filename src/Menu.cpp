#include <windows.h>

#include "Menu.hpp"

namespace {
	Texture2D prova;
    Texture2D bg;
    Texture2D load_0;
    Texture2D load_1;
    Texture2D load_2;
    Texture2D blinky;
    Texture2D pinky;
    Texture2D inky;
    Texture2D clyde;
    Texture2D coin;
    Texture2D coin2;
    Texture2D back;

    //Booleans value to ImGui windows functions
    //First window
    bool show_menu = true;
    bool show_loading = false;
    bool show_settings = false;
    bool enableWindow = false;
    //First window buttons
    bool startFocused = false;
    bool settingsFocused = false;
    bool exitFocused = false;
    //Other windows widgets
    bool offFocus = false;
    bool onFocus = false;
    bool onActiveAlias = false;
    bool offActiveAlias = true;
    bool onActiveHdr = false;
    bool offActiveHdr = true;
    bool aliasing_windows = false;
    bool hdr_windows = false;
    bool aliasingFocus = false;
    bool hdrFocus = false;
    bool hdrActive = false;
    bool aliasingActive = false;
    bool firstOpen = false;
    bool barFocus = false;
    bool barActiveAlias = false;
    bool barActiveHdr = false;
    bool backFocus = false;
    bool backFocusAlias = false;
    bool backFocusHdr = false;
    bool wasOffAlias = true;
    bool wasOffHdr = true;
    bool needToPop = false;

    float progress = 0.0f;
    int selectable_alias = 0;
    int selectable_hdr = 1;
    const char* alias_items[] = { "4x", "8x", "16x" };
    const char* hdr_items[] = { "0.25", "0.5", "0.75", "1" };

    //Textures IDs for loading window animation
    std::vector<unsigned int> textureIDs;
    unsigned int frameWidth;
    unsigned int frameHeight;
    int frameDelay = 350;  //Delay between frames

    ImGuiIO* io;

    ImFont* defaultFont;
    ImFont* myFont;
    ImFont* myFont2;
    ImFont* myFont3;

    GLFWcursor* customCursor;
}

Menu::Menu(GLFWwindow* window, const unsigned int width, const unsigned int height, bool& showGame, CustomStructs::Config& config)
    : window(window), width(width), height(height), showGame(showGame), config(config) {
}

Menu::~Menu() {
    //Clean ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    delete defaultFont;
    delete myFont;
    delete myFont2;
    delete myFont3;
}

void Menu::Init() {
    prova  = ResourceManager::LoadTexture(FileSystem::getPath("../res/textures/logo.png").c_str(),                     "logoTexture");
    bg     = ResourceManager::LoadTexture(FileSystem::getPath("../res/textures/background.png").c_str(),               "bgTexture");
    load_0 = ResourceManager::LoadTexture(FileSystem::getPath("../res/textures/progress_bar_pacman0.png").c_str(),     "frame0Texture");
    load_1 = ResourceManager::LoadTexture(FileSystem::getPath("../res/textures/progress_bar_pacman1.png").c_str(),     "frame1Texture");
    load_2 = ResourceManager::LoadTexture(FileSystem::getPath("../res/textures/progress_bar_pacman2.png").c_str(),     "frame2Texture");
    blinky = ResourceManager::LoadTexture(FileSystem::getPath("../res/textures/progress_bar_blinky.png").c_str(),      "blinkyTexture");
    pinky  = ResourceManager::LoadTexture(FileSystem::getPath("../res/textures/progress_bar_pinky.png").c_str(),       "pinkyTexture");
    inky   = ResourceManager::LoadTexture(FileSystem::getPath("../res/textures/progress_bar_inky.png").c_str(),        "inkyTexture");
    clyde  = ResourceManager::LoadTexture(FileSystem::getPath("../res/textures/progress_bar_clyde.png").c_str(),       "clydeTexture");
    coin   = ResourceManager::LoadTexture(FileSystem::getPath("../res/textures/slider_settings_disabled.png").c_str(), "coinTexture");
    coin2  = ResourceManager::LoadTexture(FileSystem::getPath("../res/textures/slider_settings_enabled.png").c_str(),  "coin2Texture");
    back   = ResourceManager::LoadTexture(FileSystem::getPath("../res/textures/back_button.png").c_str(),              "backTexture");

    textureIDs.push_back(load_0.id);
    textureIDs.push_back(load_1.id);
    textureIDs.push_back(load_2.id);
    textureIDs.push_back(load_1.id);

    frameWidth = load_0.width;
    frameHeight = load_0.height;

    //ImGui Initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    //Turn on Navigation keyboard and turn off changing of cursor
    io = &ImGui::GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard navigation
    io->ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    //Add font for imgui
    defaultFont = io->Fonts->AddFontDefault(); // Save default font
    myFont      = io->Fonts->AddFontFromFileTTF(FileSystem::getPath("../res/fonts/PAC-FONT.TTF").c_str(), 35.0f);
    myFont2     = io->Fonts->AddFontFromFileTTF(FileSystem::getPath("../res/fonts/Pacmania.ttf").c_str(), 35.0f);
    myFont3     = io->Fonts->AddFontFromFileTTF(FileSystem::getPath("../res/fonts/Pacmania.ttf").c_str(), 55.0f);

    //ImGui Style
    ImGui::StyleColorsDark();
    ImGui::SetNavCursorVisible(true);

    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0, 0, 0, 0); // Colore trasparente

    //Init ImGui for OpenGL
    ImGui_ImplGlfw_InitForOpenGL(this->window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    //Create custom cursor
    customCursor = this->createCustomCursor("../res/textures/custom_cursor.png", 16, 16); // Adjust hotspot as needed
    if (customCursor) {
        glfwSetCursor(window, customCursor);
    }
    else {
    	 LoggerManager::LogError("Using default cursor since custom cursor creation failed.");
    }

}

void Menu::Render(double deltaTime) const {
    //Initializing ImGui Frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

    //Window Dimension
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(this->width), static_cast<float>(this->height)));
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));

    //Menu window
    if (show_menu) {
        //Background
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::Begin("My name is window, ImGui window", (bool*)0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);
        ImGui::GetBackgroundDrawList()->AddImage(bg.id, ImVec2(0, 0), ImVec2(static_cast<float>(this->width), static_cast<float>(this->height)));

        //Logo
        ImGui::SetCursorPos(ImVec2(600.0f, 100.0f));
        ImGui::Image(prova.id, ImVec2(static_cast<float>(prova.width), static_cast<float>(prova.height)));

        //Start Button
        ImGui::SetCursorPos(ImVec2(740.0f, 500.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 4.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.54f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));
        if (!startFocused)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));

        ImGui::PushFont(myFont);

        if (ImGui::Button("", ImVec2(400, 100))) {
            show_menu = false;
            show_loading = true;
        }

        if (ImGui::IsItemHovered() && (io->MouseDelta.x != 0.0f || io->MouseDelta.y != 0.0f)) {
            ImGui::SetKeyboardFocusHere(-1);
        }

        if (ImGui::IsItemFocused()) {
            startFocused = true;
            settingsFocused = false;
            exitFocused = false;
        }

        ImGui::SetCursorPos(ImVec2(760.0f, 510.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));
        ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);
        ImGui::Button("start", ImVec2(357, 80));
        ImGui::PopStyleColor(10);
        ImGui::PopStyleVar(3);
        ImGui::PopItemFlag();

        //Settings Button
        ImGui::SetCursorPos(ImVec2(740.0f, 650.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 4.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.54f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));
        if (!settingsFocused)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));

        if (ImGui::Button(" ", ImVec2(400, 100))) {
            show_menu = false;
            show_settings = true;
        }

        if (ImGui::IsItemHovered() && (io->MouseDelta.x != 0.0f || io->MouseDelta.y != 0.0f)) {
            ImGui::SetKeyboardFocusHere(-1);
        }

        if (ImGui::IsItemFocused()) {
            startFocused = false;
            settingsFocused = true;
            exitFocused = false;
        }

        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar(2);
        ImGui::SetCursorPos(ImVec2(760.0f, 660.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));
        ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);
        ImGui::Button("settings", ImVec2(357, 80));
        ImGui::PopStyleColor(5);
        ImGui::PopStyleVar();
        ImGui::PopItemFlag();

        //Exit Button
        ImGui::SetCursorPos(ImVec2(740.0f, 800.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 4.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.54f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));
        if (!exitFocused)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));

        if (ImGui::Button("  ", ImVec2(400, 100))) {
            glfwSetWindowShouldClose(window, true);
        }

        if (ImGui::IsItemHovered() && (io->MouseDelta.x != 0.0f || io->MouseDelta.y != 0.0f)) {
            ImGui::SetKeyboardFocusHere(-1);
        }

        if (ImGui::IsItemFocused()) {
            startFocused = false;
            settingsFocused = false;
            exitFocused = true;
        }

        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar(1);
        ImGui::SetCursorPos(ImVec2(760.0f, 810.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));
        ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);
        ImGui::Button("exit", ImVec2(357, 80));
        ImGui::PopStyleColor(5);
        ImGui::PopStyleVar(2);
        ImGui::PopItemFlag();
        ImGui::PopFont();

        //Credential Text
        ImGui::SetCursorPos(ImVec2(40.0f, 1000.0f));
        ImGui::PushFont(myFont2);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
        ImGui::Text("created by: mps & rst");
        ImGui::SetCursorPos(ImVec2(1800.0f, 1000.0f));
        ImGui::Text("v 1.00");
        ImGui::PopStyleColor();
        ImGui::PopFont();

        if ((ImGui::GetFocusID() == ImGui::GetID("")) && ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
            ImGui::ClearActiveID();
            ImGui::SetFocusID(ImGui::GetID("  "), ImGui::GetCurrentWindow());
        }

        if ((ImGui::GetFocusID() == ImGui::GetID("  ")) && ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
            ImGui::ClearActiveID();
            ImGui::SetFocusID(ImGui::GetID(""), ImGui::GetCurrentWindow());
        }

        ImGui::End();
    }

    //Loading window
    if (show_loading) {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        progress += 0.01f * ImGui::GetIO().DeltaTime * 3.75f;
        constexpr auto progressBarColor = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
        constexpr ImVec2 loadSize(150.0f, 150.0f);
        constexpr ImVec2 imageSize(100.0f, 100.0f);

        ImGui::Begin("My name is window, ImGui window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);

        // Progress Bar
        ImGui::SetCursorPos(ImVec2(310, 530));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 4.0f);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.54f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, progressBarColor);
        ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f), "");
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
        ImGui::PushFont(myFont);
        const float loadPosX = 300.0f + progress * 1200;
        constexpr float loadPosY = 460.0f;
        const auto loadPos = ImVec2(loadPosX, loadPosY);
        constexpr auto blinkyPos = ImVec2(500.0f, 490.0f);
        constexpr auto inkyPos = ImVec2(750.0f, 490.0f);
        constexpr auto clydePos = ImVec2(1000.0f, 490.0f);
        constexpr auto pinkyPos = ImVec2(1250.0f, 490.0f);
        static size_t currentFrame = 0;
        static auto lastFrameTime = std::chrono::steady_clock::now();

        // Calculating time to update progress bar
        const auto now = std::chrono::steady_clock::now();
        const auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFrameTime).count();

        // Changing frame for animation
        if (elapsedTime > frameDelay) {
            currentFrame = (currentFrame + 1) % textureIDs.size();
            lastFrameTime = now;
        }

        // Rendering actual frame
        //Ghosts
        ImGui::GetWindowDrawList()->AddImage(textureIDs[currentFrame], loadPos, ImVec2(loadPos.x + loadSize.x, loadPos.y + loadSize.y));
        if (loadPosX + 60.0f < blinkyPos.x)
            ImGui::GetWindowDrawList()->AddImage(blinky.id, blinkyPos, ImVec2(blinkyPos.x + imageSize.x, blinkyPos.y + imageSize.y));
        if (loadPosX + 60.0f < pinkyPos.x)
            ImGui::GetWindowDrawList()->AddImage(pinky.id, pinkyPos, ImVec2(pinkyPos.x + imageSize.x, pinkyPos.y + imageSize.y));
        if (loadPosX + 60.0f < inkyPos.x)
            ImGui::GetWindowDrawList()->AddImage(inky.id, inkyPos, ImVec2(inkyPos.x + imageSize.x, inkyPos.y + imageSize.y));
        if (loadPosX + 60.0f < clydePos.x)
            ImGui::GetWindowDrawList()->AddImage(clyde.id, clydePos, ImVec2(clydePos.x + imageSize.x, clydePos.y + imageSize.y));

        //Bar and text
        ImGui::SetCursorPos(ImVec2(765, 600));
        ImGui::Text("loading: ");
        ImGui::SetCursorPos(ImVec2(1000, 588));
        ImGui::PopFont();
        ImGui::PushFont(myFont3);
        ImGui::Text("% .0f% %", progress * 100.0f); // Show progress as percentage

        if (progress > 1.0f) {
            ImGui_ImplGlfw_Sleep(1500);
            show_loading = false;
            this->showGame = true;
        }
        ImGui::PopStyleColor(4);
        ImGui::PopFont();
        ImGui::PopStyleVar(2);
        ImGui::End();
    }

    //Settings window
    if (show_settings) {
        //Background
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::Begin("My name is window, ImGui window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);
        ImGui::GetBackgroundDrawList()->AddImage(bg.id, ImVec2(0, 0), ImVec2(static_cast<float>(this->width), static_cast<float>(this->height)));
        ImGui::SetCursorPos(ImVec2(40.0f, 1020.0f));
        ImGui::PushFont(myFont2);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
        ImGui::Text("created by: mps & rst");
        ImGui::SetCursorPos(ImVec2(1800.0f, 1020.0f));
        ImGui::Text("v 1.00");
        ImGui::PopStyleColor();
        ImGui::PopFont();

        //Logo
        constexpr ImVec2 imagePos(50.0f, 80.0f);
        constexpr ImVec2 imageSize(400.0f, 200.0f);
        ImGui::GetWindowDrawList()->AddImage(prova.id, imagePos, ImVec2(imagePos.x + imageSize.x, imagePos.y + imageSize.y));

        //First window
        ImGui::SetNextWindowPos(ImVec2(186.0f, 350.0f));
        ImGui::SetNextWindowFocus();
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 5.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 15.0f);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.7f, 0.7f, 0.7f, 0.5f));

        ImGui::BeginChild("ciao", ImVec2(750, 650), ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

        if (!enableWindow) {
            backFocusHdr = false;
            backFocusAlias = false;
        }

        //Anti-aliasing button
        ImGui::SetCursorPos(ImVec2(175.0f, 200.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 4.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.54f, 1.0f));

        if (!enableWindow)
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));

        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));

        if (!aliasingFocus) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));
        }
        else if (aliasingActive)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));

        ImGui::PushFont(myFont);

        if (ImGui::IsWindowAppearing()) {
            ImGui::ClearActiveID();
            ImGui::SetFocusID(ImGui::GetID("alias"), ImGui::GetCurrentWindow());
            io->ConfigNavCursorVisibleAlways = true;
        }

        if (enableWindow) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            needToPop = true;
        }

        if (ImGui::Button("alias", ImVec2(400, 100))) {
            enableWindow = true;
            aliasingActive = true;
            hdrActive = false;
            firstOpen = true;
        }

        if (needToPop) {
            ImGui::PopItemFlag();
            needToPop = false;
        }

        if (ImGui::IsItemHovered() && (io->MouseDelta.x != 0.0f || io->MouseDelta.y != 0.0f)) {
            ImGui::SetKeyboardFocusHere(-1);
        }

        if (ImGui::IsItemFocused() || ImGui::IsItemHovered()) {
            aliasingFocus = true;
            hdrFocus = false;
            aliasing_windows = true;
            hdr_windows = false;
            backFocus = false;
        }

        ImGui::SetCursorPos(ImVec2(195.0f, 210.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));
        ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);
        ImGui::Button("anti-aliasing", ImVec2(357, 80));
        ImGui::PopStyleColor(10);
        ImGui::PopStyleVar(3);
        ImGui::PopItemFlag();

        //Hdr Button
        ImGui::SetCursorPos(ImVec2(175.0f, 350.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 4.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.54f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));

        if (!enableWindow)
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));

        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));

        if (!hdrFocus)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));
        else if (hdrActive)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));

        if (enableWindow) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            needToPop = true;
        }

        if (ImGui::Button("hd", ImVec2(400, 100))) {
            enableWindow = true;
            aliasingActive = false;
            hdrActive = true;
            firstOpen = true;
        }

        if (needToPop) {
            ImGui::PopItemFlag();
            needToPop = false;
        }

        if (ImGui::IsItemHovered() && (io->MouseDelta.x != 0.0f || io->MouseDelta.y != 0.0f)) {
            ImGui::SetKeyboardFocusHere(-1);
        }

        if (ImGui::IsItemFocused() || ImGui::IsItemHovered()) {
            aliasingFocus = false;
            hdrFocus = true;
            aliasing_windows = false;
            hdr_windows = true;
            backFocus = false;
        }

        ImGui::PopStyleColor(5);
        ImGui::PopStyleVar(2);
        ImGui::SetCursorPos(ImVec2(195.0f, 360.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));
        ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);
        ImGui::Button("hdr", ImVec2(357, 80));
        ImGui::PopStyleColor(5);
        ImGui::PopStyleVar();
        ImGui::PopItemFlag();
        ImGui::PopFont();

        //Keyboard configuration
        if ((ImGui::GetFocusID() == ImGui::GetID("back")) && ImGui::IsKeyPressed(ImGuiKey_UpArrow) && !enableWindow) {
            ImGui::ClearActiveID();
            ImGui::SetFocusID(ImGui::GetID("hd"), ImGui::GetCurrentWindow());
        }

        if ((ImGui::GetFocusID() == ImGui::GetID("hd")) && ImGui::IsKeyPressed(ImGuiKey_DownArrow) && !enableWindow) {
            ImGui::ClearActiveID();
            ImGui::SetFocusID(ImGui::GetID("back"), ImGui::GetCurrentWindow());
        }

        if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Backspace) && !enableWindow) {
            show_settings = false;
            show_menu = true;
        }

        //Back button
        ImGui::SetCursorPos(ImVec2(50.0f, 50.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 4.0f);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.54f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));

        if (!backFocus) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));
        }
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));

        if (enableWindow) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            needToPop = true;
        }

        if (ImGui::ImageButton("back", back.id, ImVec2(100.0f, 50.0f))) {
            show_settings = false;
            show_menu = true;
        }

        if (ImGui::IsItemHovered() && (io->MouseDelta.x != 0.0f || io->MouseDelta.y != 0.0f)) {
            ImGui::SetKeyboardFocusHere(-1);
        }

        if (ImGui::IsItemFocused() || ImGui::IsItemHovered()) {
            aliasingFocus = false;
            hdrFocus = false;
            aliasing_windows = false;
            hdr_windows = false;
            backFocus = true;
        }

        if (needToPop) {
            ImGui::PopItemFlag();
            needToPop = false;
        }

        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar(2);

        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);

        //Anti-aliasing settings windows
        if (aliasing_windows) {
            //Window style
            ImGui::SetNextWindowPos(ImVec2(965.0f, 350.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 5.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 15.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.7f, 0.7f, 0.7f, 0.5f));
            ImGui::BeginChild("ciao2", ImVec2(750, 650), ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);

            if (aliasingFocus && enableWindow)
                ImGui::SetWindowFocus();

            //Off button
            ImGui::SetCursorPos(ImVec2(410.0f, 200.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 4.0f);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.54f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));

            if (!offFocus && !offActiveAlias)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));
            else if (offActiveAlias && !offFocus)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));
            else
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));

            ImGui::PushFont(myFont);

            if (!enableWindow) {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                needToPop = true;
            }

            if (firstOpen) {
                ImGui::SetFocusID(ImGui::GetID("no"), ImGui::GetCurrentWindow());
                firstOpen = false;
            }

            if (ImGui::Button("no", ImVec2(200, 80))) {
                barFocus = false;
                offActiveAlias = true;
                onActiveAlias = false;
                barActiveAlias = false;
                this->config.useMSAA = false;
            }

            if (ImGui::IsItemHovered() && (io->MouseDelta.x != 0.0f || io->MouseDelta.y != 0.0f)) {
                ImGui::SetKeyboardFocusHere(-1);
                wasOffAlias = true;
            }

            if (ImGui::IsItemFocused()) {
                onFocus = false;
                offFocus = true;
                wasOffAlias = true;
                backFocusAlias = false;
            }

            if (needToPop) {
                ImGui::PopItemFlag();
                needToPop = false;
            }

            ImGui::SetCursorPos(ImVec2(435.0f, 210.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));
            ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);
            ImGui::Button("off", ImVec2(157, 60));
            ImGui::PopStyleColor(9);
            ImGui::PopStyleVar(3);
            ImGui::PopItemFlag();

            //On button
            ImGui::SetCursorPos(ImVec2(140.0f, 200.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 4.0f);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.54f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));

            if (!onFocus && !onActiveAlias)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));
            else if (onActiveAlias && !onFocus)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));
            else
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));

            if (!enableWindow) {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                needToPop = true;
            }

            if (ImGui::Button("si", ImVec2(200, 80))) {
                onActiveAlias = true;
                offActiveAlias = false;
                barFocus = true;
                ImGui::ActivateItemByID(ImGui::GetID("##discrete"));
                this->config.useMSAA = true;
            }

            if (ImGui::IsItemHovered() && (io->MouseDelta.x != 0.0f || io->MouseDelta.y != 0.0f)) {
                ImGui::SetKeyboardFocusHere(-1);
                wasOffAlias = false;
            }

            if (ImGui::IsItemFocused()) {
                onFocus = true;
                offFocus = false;
                backFocusAlias = false;
                wasOffAlias = false;
            }

            if (needToPop) {
                ImGui::PopItemFlag();
                needToPop = false;
            }

            //Slider-grab
            ImGui::PopStyleColor(4);
            ImGui::PopStyleVar(2);
            ImGui::SetCursorPos(ImVec2(165.0f, 210.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));
            ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);
            ImGui::Button("on", ImVec2(157, 60));
            ImGui::PopStyleColor(5);
            ImGui::PopStyleVar();
            ImGui::PopItemFlag();
            ImGui::PopFont();

            ImGui::SetCursorPos(ImVec2(143.0f, 350.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 10.0f));

            if (!barActiveAlias) {
                ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.988f, 0.812f, 0.0f, 0.5f));
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 1.0f, 0.5f));
            }
            else {
                ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
            }

            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));

            ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);

            if (!barFocus) {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                needToPop = true;
            }

            ImGui::PushItemWidth(450.0f);

            if (ImGui::SliderInt("##discrete", &selectable_alias, 0, IM_ARRAYSIZE(alias_items) - 1, "", ImGuiSliderFlags_NoInput)) {
                switch (selectable_alias) {
                case 0:
                    this->config.useMSAA = true;
                    this->config.numSampleMSAA = 4;
                    //Anti-Aliasing 4x
                    break;
                case 1:
                    this->config.useMSAA = true;
                    this->config.numSampleMSAA = 8;
                    //Anti-Aliasing 8x
                    break;
                case 2:
                    this->config.useMSAA = true;
                    this->config.numSampleMSAA = 16;
                    //Anti-Aliasing 16x
                    break;
				default:
                    this->config.useMSAA = false;
                }
            }

            ImGui::PopItemWidth();

            if (ImGui::IsItemFocused()) {
                barFocus = true;
                onFocus = false;
                backFocusAlias = false;
            	barActiveAlias = true;
            }

            if (barFocus && (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsMouseReleased(0))) {
                ImGui::ClearActiveID();
                ImGui::SetFocusID(ImGui::GetID("si"), ImGui::GetCurrentWindow());
                barFocus = false;
            }

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(6);

            if (needToPop) {
                ImGui::PopItemFlag();
                needToPop = false;
            }

            ImGui::PopItemFlag();

            //Keyboard configuration
            if ((ImGui::GetFocusID() == ImGui::GetID("si")) && ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
                ImGui::ClearActiveID();
                ImGui::SetFocusID(ImGui::GetID("no"), ImGui::GetCurrentWindow());
            }

            if ((ImGui::GetFocusID() == ImGui::GetID("no")) && ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
                ImGui::ClearActiveID();
                ImGui::SetFocusID(ImGui::GetID("si"), ImGui::GetCurrentWindow());
            }

            if ((ImGui::GetFocusID() == ImGui::GetID("si")) && ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
                ImGui::ClearActiveID();
                ImGui::SetFocusID(ImGui::GetID("no"), ImGui::GetCurrentWindow());
            }

            if ((ImGui::GetFocusID() == ImGui::GetID("no")) && ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
                ImGui::ClearActiveID();
                ImGui::SetFocusID(ImGui::GetID("si"), ImGui::GetCurrentWindow());
            }

            if (((ImGui::GetFocusID() == ImGui::GetID("no")) || (ImGui::GetFocusID() == ImGui::GetID("si"))) && (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || ImGui::IsKeyPressed(ImGuiKey_DownArrow))) {
                ImGui::ClearActiveID();
                ImGui::SetFocusID(ImGui::GetID("backAlias"), ImGui::GetCurrentWindow());
            }

            if ((ImGui::GetFocusID() == ImGui::GetID("backAlias")) && (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || ImGui::IsKeyPressed(ImGuiKey_DownArrow)) && (!onFocus && !offFocus)) {
                ImGui::ClearActiveID();
                if (wasOffAlias)
                    ImGui::SetFocusID(ImGui::GetID("no"), ImGui::GetCurrentWindow());
                else
                    ImGui::SetFocusID(ImGui::GetID("si"), ImGui::GetCurrentWindow());
            }

            if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
                aliasingActive = false;
                onFocus = false;
                offFocus = false;
                backFocusAlias = false;
                enableWindow = false;
            }

            //Images and text of slider-grab
            constexpr ImVec2 imageSize(80.0f, 80.0f);
            ImGui::PushFont(myFont3);
            if (!barFocus)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.988f, 0.812f, 0.0f, 0.5f));
            else
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));

            ImGui::SetCursorPos(ImVec2(255.0f, 400.0f));
            ImGui::Text("4x");

            ImGui::SetCursorPos(ImVec2(420.0f, 400.0f));
            ImGui::Text("8x");

            ImGui::SetCursorPos(ImVec2(585.0f, 400.0f));
            ImGui::Text("16x");

        	ImVec2 imagePos(1210.0f, 675.0f);
            if (!barActiveAlias)
                ImGui::GetWindowDrawList()->AddImage(coin.id, imagePos, ImVec2(imagePos.x + imageSize.x, imagePos.y + imageSize.y));
            else
                ImGui::GetWindowDrawList()->AddImage(coin2.id, imagePos, ImVec2(imagePos.x + imageSize.x, imagePos.y + imageSize.y));

            constexpr ImVec2 imagePos1(1375.0f, 675.0f);
            if (!barActiveAlias)
                ImGui::GetWindowDrawList()->AddImage(coin.id, imagePos1, ImVec2(imagePos1.x + imageSize.x, imagePos1.y + imageSize.y));
            else
                ImGui::GetWindowDrawList()->AddImage(coin2.id, imagePos1, ImVec2(imagePos1.x + imageSize.x, imagePos1.y + imageSize.y));

            constexpr ImVec2 imagePos2(1535.0f, 675.0f);
            if (!barActiveAlias)
                ImGui::GetWindowDrawList()->AddImage(coin.id, imagePos2, ImVec2(imagePos2.x + imageSize.x, imagePos2.y + imageSize.y));
            else
                ImGui::GetWindowDrawList()->AddImage(coin2.id, imagePos2, ImVec2(imagePos2.x + imageSize.x, imagePos2.y + imageSize.y));

            //Back button
            ImGui::SetCursorPos(ImVec2(50.0f, 50.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 4.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.54f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));

            if (!backFocusAlias) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));
            }
            else
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));

            if (!enableWindow) {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                needToPop = true;
            }

            if (ImGui::ImageButton("backAlias", back.id, ImVec2(70.0f, 37.5f))) {
                aliasingActive = false;
                onFocus = false;
                offFocus = false;
                backFocusAlias = false;
                enableWindow = false;
            }

            if (ImGui::IsItemHovered() && (io->MouseDelta.x != 0.0f || io->MouseDelta.y != 0.0f)) {
                ImGui::SetKeyboardFocusHere(-1);
            }

            if (ImGui::IsItemFocused() || ImGui::IsItemHovered()) {
                onFocus = false;
                offFocus = false;
                backFocusAlias = true;
            }

            if (needToPop) {
                ImGui::PopItemFlag();
                needToPop = false;
            }

            ImGui::PopFont();
            ImGui::PopStyleColor(5);
            ImGui::PopStyleVar(2);
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
        }


        //Hdr window settings
        if (hdr_windows) {
            //Window style
            ImGui::SetNextWindowPos(ImVec2(965.0f, 350.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 5.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 15.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.7f, 0.7f, 0.7f, 0.5f));
            ImGui::BeginChild("ciao2", ImVec2(750, 650), ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);

            if (hdrFocus && enableWindow)
                ImGui::SetWindowFocus();

            //Off button
            ImGui::SetCursorPos(ImVec2(410.0f, 200.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 4.0f);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.54f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));

            if (!offFocus && !offActiveHdr)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));
            else if (offActiveHdr && !offFocus)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));
            else
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));

            ImGui::PushFont(myFont);

            if (!enableWindow) {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                needToPop = true;
            }

            if (firstOpen) {
                ImGui::SetFocusID(ImGui::GetID("no"), ImGui::GetCurrentWindow());
                firstOpen = false;
            }

            if (ImGui::Button("no", ImVec2(200, 80))) {
                barFocus = false;
                offActiveHdr = true;
                onActiveHdr = false;
                barActiveHdr = false;
                this->config.useHDR = false;
                //Disattiva anti-aliasing
            }

            if (ImGui::IsItemHovered() && (io->MouseDelta.x != 0.0f || io->MouseDelta.y != 0.0f)) {
                ImGui::SetKeyboardFocusHere(-1);
                wasOffHdr = true;
            }

            if (ImGui::IsItemFocused()) {
                onFocus = false;
                offFocus = true;
                backFocusHdr = false;
                wasOffHdr = true;
            }

            if (needToPop) {
                ImGui::PopItemFlag();
                needToPop = false;
            }

            ImGui::SetCursorPos(ImVec2(435.0f, 210.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));
            ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);
            ImGui::Button("off", ImVec2(157, 60));
            ImGui::PopStyleColor(9);
            ImGui::PopStyleVar(3);
            ImGui::PopItemFlag();

            //On button
            ImGui::SetCursorPos(ImVec2(140.0f, 200.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 4.0f);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.54f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));

            if (!onFocus && !onActiveHdr)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));
            else if (onActiveHdr && !onFocus)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));
            else
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));

            if (!enableWindow) {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                needToPop = true;
            }

            if (ImGui::Button("si", ImVec2(200, 80))) {
                onActiveHdr = true;
                offActiveHdr = false;
                barFocus = true;
                ImGui::ActivateItemByID(ImGui::GetID("##discrete"));
                this->config.useHDR = true;
            }

            if (ImGui::IsItemHovered() && (io->MouseDelta.x != 0.0f || io->MouseDelta.y != 0.0f)) {
                ImGui::SetKeyboardFocusHere(-1);
                wasOffHdr = false;
            }

            if (ImGui::IsItemFocused()) {
                onFocus = true;
                offFocus = false;
                backFocusHdr = false;
                wasOffHdr = false;
            }

            if (needToPop) {
                ImGui::PopItemFlag();
                needToPop = false;
            }

            ImGui::PopStyleColor(4);
            ImGui::PopStyleVar(2);
            ImGui::SetCursorPos(ImVec2(165.0f, 210.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));
            ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);
            ImGui::Button("on", ImVec2(157, 60));
            ImGui::PopStyleColor(5);
            ImGui::PopStyleVar();
            ImGui::PopItemFlag();
            ImGui::PopFont();

            //Slider-grab
            ImGui::SetCursorPos(ImVec2(143.0f, 350.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 10.0f));

            if (!barActiveHdr) {
                ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.988f, 0.812f, 0.0f, 0.5f));
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 1.0f, 0.5f));
            }
            else {
                ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
            }

            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));

            ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);

            if (!barFocus) {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                needToPop = true;
            }

            ImGui::PushItemWidth(450.0f);

            if (ImGui::SliderInt("##discrete", &selectable_hdr, 0, IM_ARRAYSIZE(hdr_items) - 1, "", ImGuiSliderFlags_NoInput)) {
                switch (selectable_hdr) {
                case 0:
                    this->config.useHDR = true;
                    this->config.exposure= 0.25f;
                    //hdr 0.25
                    break;
                case 1:
                    this->config.useHDR = true;
                    this->config.exposure = 0.50f;
                    //hdr 0.5
                    break;
                case 2:
                    this->config.useHDR = true;
                    this->config.exposure = 0.75f;
                    //hdr 0.75
                    break;
                case 3:
                    this->config.useHDR = true;
                    this->config.exposure = 1.00f;
                    //hdr 1
                    break;
                default:
                    this->config.useHDR = false;
                }
            }

            ImGui::PopItemWidth();

            if (ImGui::IsItemFocused()) {
                barFocus = true;
                onFocus = false;
                backFocusHdr = false;
                barActiveHdr = true;
            }

            if (barFocus && (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsMouseReleased(0))) {
                ImGui::ClearActiveID();
                ImGui::SetFocusID(ImGui::GetID("si"), ImGui::GetCurrentWindow());
                barFocus = false;
            }

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(6);

            if (needToPop) {
                ImGui::PopItemFlag();
                needToPop = false;
            }

            ImGui::PopItemFlag();

            //Keyboard configurations
            if ((ImGui::GetFocusID() == ImGui::GetID("si")) && ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
                ImGui::ClearActiveID();
                ImGui::SetFocusID(ImGui::GetID("no"), ImGui::GetCurrentWindow());
            }

            if ((ImGui::GetFocusID() == ImGui::GetID("no")) && ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
                ImGui::ClearActiveID();
                ImGui::SetFocusID(ImGui::GetID("si"), ImGui::GetCurrentWindow());
            }

            if ((ImGui::GetFocusID() == ImGui::GetID("si")) && ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
                ImGui::ClearActiveID();
                ImGui::SetFocusID(ImGui::GetID("no"), ImGui::GetCurrentWindow());
            }

            if ((ImGui::GetFocusID() == ImGui::GetID("no")) && ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
                ImGui::ClearActiveID();
                ImGui::SetFocusID(ImGui::GetID("si"), ImGui::GetCurrentWindow());
            }

            if (((ImGui::GetFocusID() == ImGui::GetID("no")) || (ImGui::GetFocusID() == ImGui::GetID("si"))) && (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || ImGui::IsKeyPressed(ImGuiKey_DownArrow))) {
                ImGui::ClearActiveID();
                ImGui::SetFocusID(ImGui::GetID("backHdr"), ImGui::GetCurrentWindow());
            }

            if ((ImGui::GetFocusID() == ImGui::GetID("backHdr")) && (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || ImGui::IsKeyPressed(ImGuiKey_DownArrow)) && (!offFocus && !onFocus)) {
                ImGui::ClearActiveID();
                if (wasOffHdr)
                    ImGui::SetFocusID(ImGui::GetID("no"), ImGui::GetCurrentWindow());
                else
                    ImGui::SetFocusID(ImGui::GetID("si"), ImGui::GetCurrentWindow());
            }

            if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
                hdrActive = false;
                onFocus = false;
                offFocus = false;
                backFocusHdr = false;
            	enableWindow = false;
            }

            //Images and text for slider-grab
            ImVec2 imageSize(80.0f, 80.0f);
            ImGui::PushFont(myFont3);

            if (!barFocus)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.988f, 0.812f, 0.0f, 0.5f));
            else
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));

            ImGui::SetCursorPos(ImVec2(180.0f, 400.0f));
            ImGui::Text("0.25");

            ImGui::SetCursorPos(ImVec2(320.0f, 400.0f));
            ImGui::Text("0.5");

            ImGui::SetCursorPos(ImVec2(430.0f, 400.0f));
            ImGui::Text("0.75");

            ImGui::SetCursorPos(ImVec2(600.0f, 400.0f));
            ImGui::Text("1");

            ImVec2 imagePos(1175.0f, 675.0f);
            if (!barActiveHdr)
                ImGui::GetWindowDrawList()->AddImage(coin.id, imagePos, ImVec2(imagePos.x + imageSize.x, imagePos.y + imageSize.y));
            else
                ImGui::GetWindowDrawList()->AddImage(coin2.id, imagePos, ImVec2(imagePos.x + imageSize.x, imagePos.y + imageSize.y));

            constexpr ImVec2 imagePos1(1295.0f, 675.0f);
            if (!barActiveHdr)
                ImGui::GetWindowDrawList()->AddImage(coin.id, imagePos1, ImVec2(imagePos1.x + imageSize.x, imagePos1.y + imageSize.y));
            else
                ImGui::GetWindowDrawList()->AddImage(coin2.id, imagePos1, ImVec2(imagePos1.x + imageSize.x, imagePos1.y + imageSize.y));

            constexpr ImVec2 imagePos2(1415.0f, 675.0f);
            if (!barActiveHdr)
                ImGui::GetWindowDrawList()->AddImage(coin.id, imagePos2, ImVec2(imagePos2.x + imageSize.x, imagePos2.y + imageSize.y));
            else
                ImGui::GetWindowDrawList()->AddImage(coin2.id, imagePos2, ImVec2(imagePos2.x + imageSize.x, imagePos2.y + imageSize.y));

            constexpr ImVec2 imagePos3(1535.0f, 675.0f);
            if (!barActiveHdr)
                ImGui::GetWindowDrawList()->AddImage(coin.id, imagePos3, ImVec2(imagePos3.x + imageSize.x, imagePos3.y + imageSize.y));
            else
                ImGui::GetWindowDrawList()->AddImage(coin2.id, imagePos3, ImVec2(imagePos3.x + imageSize.x, imagePos3.y + imageSize.y));

            //Back button
            ImGui::SetCursorPos(ImVec2(50.0f, 50.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 4.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.54f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));

            if (!backFocusHdr) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));
            }
            else
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));

            if (!enableWindow) {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                needToPop = true;
            }

            if (ImGui::ImageButton("backHdr", back.id, ImVec2(70.0f, 37.5f))) {
                backFocusHdr = false;
                hdrActive = false;
                onFocus = false;
                offFocus = false;
                enableWindow = false;

            }

            if (ImGui::IsItemHovered() && (io->MouseDelta.x != 0.0f || io->MouseDelta.y != 0.0f)) {
                ImGui::SetKeyboardFocusHere(-1);
            }

            if (ImGui::IsItemFocused() || ImGui::IsItemHovered()) {
                onFocus = false;
                offFocus = false;
                backFocusHdr = true;
            }

            if (needToPop) {
                ImGui::PopItemFlag();
                needToPop = false;
            }

            ImGui::PopFont();
            ImGui::PopStyleColor(5);
            ImGui::PopStyleVar(2);
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
        }

        ImGui::End();
        ImGui::PopStyleColor();
    }

    //Rendering ImGui windows
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSetCursor(window, customCursor); //Ensure personalized cursor
}

GLFWcursor* Menu::createCustomCursor(const char* imagePath, const int hotspotX, const int hotspotY) {
    // create personalized hardware cursor
    int width, height, channels;
    //Loading image for cursor with stbi_image.h; since is hardware cursor image 
    //dimensions cannot be greater then 64x64
    unsigned char* imageData = stbi_load(imagePath, &width, &height, &channels, 4); // Force RGBA

    if (!imageData) {
        LoggerManager::LogError("Failed to load cursor image: {}", imagePath);
        return nullptr;
    }

    // Create GLFW image structure
    GLFWimage glfwImage;
    glfwImage.width = width;
    glfwImage.height = height;
    glfwImage.pixels = imageData;

    // Create GLFW cursor
    GLFWcursor* customCursorLocal = glfwCreateCursor(&glfwImage, hotspotX, hotspotY);
    if (!customCursorLocal) {
        LoggerManager::LogError("Failed to create custom cursor!");
    }
    else {
        LoggerManager::LogDebug("Custom cursor created successfully.");
    }

    // Free the image data (no longer needed after cursor creation)
    stbi_image_free(imageData);

    return customCursorLocal;
}
