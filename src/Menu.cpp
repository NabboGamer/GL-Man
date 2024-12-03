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
    bool showMenu = true;
    bool showLoading = false;
    bool showSettings = false;
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
    bool aliasingWindow = false;
    bool hdrWindow = false;
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
    int selectableAlias = 0;
    int selectableHdr = 1;
    const char* aliasItems[] = { "4x", "8x", "16x" };
    const char* hdrItems[] = { "0.25", "0.5", "0.75", "1" };

    //Textures IDs for loading window animation
    std::vector<unsigned int> textureIDs;
    float frameWidth;
    float frameHeight;
    int frameDelay = 350;  //Delay between frames

    int execution = 0;
   
    ImGuiIO* io;

    ImFont* defaultFont;
    ImFont* myFont;
    ImFont* myFont2;
    ImFont* myFont3;

    GLFWcursor* customCursor;

    float scaleX;
    float scaleY;
}

Menu::Menu(GLFWwindow* window, const unsigned int width, const unsigned int height, bool& showGame, CustomStructs::Config& config)
    : window(window), width(width), height(height), showGame(showGame), config(config) {
    scaleX = static_cast<float>(width) / 1920.0f;
    scaleY = static_cast<float>(height) / 1080.0f;
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

    frameWidth = static_cast<float>(load_0.width) * scaleX;
    frameHeight = static_cast<float>(load_0.height) * scaleY;

    //ImGui Initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    //Turn on Navigation keyboard and turn off changing of cursor
    io = &ImGui::GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard navigation
    io->ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    //Add font for imgui
    defaultFont = io->Fonts->AddFontDefault(); // Save default font
    myFont      = io->Fonts->AddFontFromFileTTF(FileSystem::getPath("../res/fonts/PAC-FONT.TTF").c_str(), 35.0f*scaleX);
    myFont2     = io->Fonts->AddFontFromFileTTF(FileSystem::getPath("../res/fonts/Pacmania.ttf").c_str(), 35.0f*scaleX);
    myFont3     = io->Fonts->AddFontFromFileTTF(FileSystem::getPath("../res/fonts/Pacmania.ttf").c_str(), 55.0f*scaleX);

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

void Menu::styleOperativeWidget(int page) const {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f * scaleX);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 4.0f * scaleX); 
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.54f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));

    if (page == 1 || page == 3) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
    }
    else if (page == 2) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));

        if (!enableWindow)
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));
    }
}

void Menu::styleDecorativeWidget() const {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f * scaleX);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));
    ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);
}

void Menu::colorOperativeWidget(ImVec2 buttonPos, bool& focused, bool& actived, int page) const {
    ImGui::SetCursorPos(buttonPos);
    if (page == 1) {
        if (!focused)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
    }
    else if (page == 2) {
        if (!focused)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));
        else if (actived)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
    }
    else if (page == 3) {
        if (!focused && !actived)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.6f, 1.0f));
        else if (actived && !focused)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
    }
        
}

void Menu::clearStyle(int page, int colors, int vars) const { 
    ImGui::PopStyleColor(colors);
    ImGui::PopStyleVar(vars);
    if (page == 1 || page == 2)
        ImGui::PopFont();
    if(page == 1 || page == 3)
        ImGui::PopItemFlag();
}

void Menu::styleWindow(ImVec2 pos, ImVec2 size) const {
    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 5.0f * scaleX);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 15.0f * scaleX);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.7f, 0.7f, 0.7f, 0.5f));
    ImGui::PushFont(myFont);
}

void Menu::drawDecorativeWidget(ImVec2 pos, const char* label, ImVec2 dim, int type) const {
    ImGui::SetCursorPos(pos);
    if (type == 1) {
        ImGui::Button(label, dim);
    }
    else if (type == 2) {
        ImGui::Text(label);

    }
}

void Menu::Render(double deltaTime, Game* GLMan) const {
    //Initializing ImGui Frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //Window Dimension
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(this->width), static_cast<float>(this->height)));
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));

    //Menu window
    if (showMenu) {
        //Background
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::Begin("My name is window, ImGui window", (bool*)0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);
        ImGui::GetBackgroundDrawList()->AddImage(bg.id, ImVec2(0, 0), ImVec2(static_cast<float>(this->width), static_cast<float>(this->height)));
        ImGui::PushFont(myFont);
       
        //Logo
        ImGui::SetCursorPos(ImVec2(600.0f*scaleX, 100.0f*scaleY));
        ImGui::Image(prova.id, ImVec2(static_cast<float>(prova.width*scaleX), static_cast<float>(prova.height*scaleY)));

        //Start Button
        styleOperativeWidget(1);
        colorOperativeWidget(ImVec2(740.0f * scaleX, 500.0f * scaleY), startFocused, startFocused, 1);
        
        if (ImGui::Button("", ImVec2(400*scaleX, 100*scaleY))) {
            showMenu = false;
            showLoading = true;
        }

        if (ImGui::IsItemHovered() && (io->MouseDelta.x != 0.0f || io->MouseDelta.y != 0.0f)) {
            ImGui::SetKeyboardFocusHere(-1);
        }

        if (ImGui::IsItemFocused()) {
            startFocused = true;
            settingsFocused = false;
            exitFocused = false;
        }

        //Settings Button
        colorOperativeWidget(ImVec2(740.0f * scaleX, 650.0f * scaleY), settingsFocused, settingsFocused, 1);

        if (ImGui::Button(" ", ImVec2(400 * scaleX, 100 * scaleY))) {
            showMenu = false;
            showSettings = true;
        }

        if (ImGui::IsItemHovered() && (io->MouseDelta.x != 0.0f || io->MouseDelta.y != 0.0f)) {
            ImGui::SetKeyboardFocusHere(-1);
        }

        if (ImGui::IsItemFocused()) {
            startFocused = false;
            settingsFocused = true;
            exitFocused = false;
        }

        //Exit Button
        colorOperativeWidget(ImVec2(740.0f * scaleX, 800.0f * scaleY), exitFocused, exitFocused, 1);

        if (ImGui::Button("  ", ImVec2(400 * scaleX, 100 * scaleY))) {
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

        //Decorative Button
        styleDecorativeWidget();
        drawDecorativeWidget(ImVec2(760.0f * scaleX, 510.0f * scaleY), "start", ImVec2(357 * scaleX, 80 * scaleY), 1);
        drawDecorativeWidget(ImVec2(760.0f * scaleX, 660.0f * scaleY), "settings", ImVec2(357 * scaleX, 80 * scaleY), 1);
        drawDecorativeWidget(ImVec2(760.0f * scaleX, 810.0f * scaleY), "exit", ImVec2(357 * scaleX, 80 * scaleY), 1);
        ImGui::PopFont();
        ImGui::PushFont(myFont2);
        drawDecorativeWidget(ImVec2(40.0f * scaleX, 1000.0f * scaleY), "created by: mps & rst", ImVec2(0.0f, 0.0f), 2);
        drawDecorativeWidget(ImVec2(1800.0f * scaleX, 1000.0f * scaleY), "v 1.00", ImVec2(0.0f, 0.0f), 2);
        clearStyle(1, 12, 3);

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
    if (showLoading) {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        if (progress < 0.6f) {
            progress += 0.01f * ImGui::GetIO().DeltaTime * 4.0f * scaleX;
        }
        else
            progress += 0.01f * ImGui::GetIO().DeltaTime * 8.0f * scaleX;

        constexpr auto progressBarColor = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
        const ImVec2 loadSize(150.0f * scaleX, 150.0f * scaleY);
        const ImVec2 imageSize(100.0f * scaleX, 100.0f * scaleY);

        ImGui::Begin("My name is window, ImGui window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);

        // Progress Bar
        styleOperativeWidget(1);
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, progressBarColor);
        ImGui::SetCursorPos(ImVec2(310 * scaleX, 530 * scaleY));
        ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f), "");
        ImGui::PushFont(myFont);
        const float loadPosX = (300.0f + progress * 1200);
        constexpr float loadPosY = 460.0f;
        const auto loadPos = ImVec2(loadPosX * scaleX, loadPosY * scaleY);
        const auto blinkyPos = ImVec2(500.0f * scaleX, 490.0f * scaleY);
        const auto inkyPos = ImVec2(750.0f * scaleX, 490.0f * scaleY);
        const auto clydePos = ImVec2(1000.0f * scaleX, 490.0f * scaleY);
        const auto pinkyPos = ImVec2(1250.0f * scaleX, 490.0f * scaleY);
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
        if (loadPosX * scaleX + 60.0f * scaleX < blinkyPos.x)
            ImGui::GetWindowDrawList()->AddImage(blinky.id, blinkyPos, ImVec2(blinkyPos.x + imageSize.x, blinkyPos.y + imageSize.y));
        if (loadPosX * scaleX + 60.0f * scaleX < pinkyPos.x)
            ImGui::GetWindowDrawList()->AddImage(pinky.id, pinkyPos, ImVec2(pinkyPos.x + imageSize.x, pinkyPos.y + imageSize.y));
        if (loadPosX * scaleX + 60.0f * scaleX < inkyPos.x)
            ImGui::GetWindowDrawList()->AddImage(inky.id, inkyPos, ImVec2(inkyPos.x + imageSize.x, inkyPos.y + imageSize.y));
        if (loadPosX * scaleX + 60.0f * scaleX < clydePos.x)
            ImGui::GetWindowDrawList()->AddImage(clyde.id, clydePos, ImVec2(clydePos.x + imageSize.x, clydePos.y + imageSize.y));

        //Bar and text
        drawDecorativeWidget(ImVec2(765 * scaleX, 600 * scaleY), "loading: ", ImVec2(0.0f, 0.0f), 2);
        ImGui::PopFont();
        ImGui::PushFont(myFont3);
         if (progress == 0.99f)
            progress += 0.01f;
        else if (progress >= 1.0f)
            progress = 1.0f;
        ImGui::SetCursorPos(ImVec2(1000 * scaleX, 588 * scaleY));
        ImGui::Text("% .0f% %", progress * 100.0f); // Show progress as percentage

        if (execution > 30) {
            if(progress <= 0.5f)
                ImGui_ImplGlfw_Sleep(500);
            if (GLMan->ContinueInit() && progress >= 1.0f) {
                progress = 1.0f;
                ImGui_ImplGlfw_Sleep(1500);
                //GLMan->ContinueInit();
                showLoading = false;
                this->showGame = true;
            }
        }
        else
            execution += 1;

        clearStyle(2, 6, 2);
        ImGui::End();
    }

    //Settings window
    if (showSettings) {
        //Background
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::Begin("My name is window, ImGui window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);
        ImGui::GetBackgroundDrawList()->AddImage(bg.id, ImVec2(0, 0), ImVec2(static_cast<float>(this->width), static_cast<float>(this->height)));
        ImGui::PushFont(myFont2);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));
        drawDecorativeWidget(ImVec2(40.0f * scaleX, 1020.0f * scaleY), "created by: mps & rst", ImVec2(0.0f, 0.0f), 2);
        drawDecorativeWidget(ImVec2(1800.0f * scaleX, 1020.0f * scaleY), "v 1.00", ImVec2(0.0f, 0.0f), 2);
        ImGui::PopFont();

        //Logo
        const ImVec2 imagePos(50.0f * scaleX, 80.0f * scaleY);
        const ImVec2 imageSize(400.0f * scaleX, 200.0f * scaleX);
        ImGui::GetWindowDrawList()->AddImage(prova.id, imagePos, ImVec2(imagePos.x + imageSize.x, imagePos.y + imageSize.y));

        //First window
        styleWindow(ImVec2(186.0f * scaleX, 350.0f * scaleY), ImVec2(750.0f * scaleX, 650.0f * scaleY));
        ImGui::SetNextWindowFocus();

        ImGui::BeginChild("ciao", ImVec2(750, 650), ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

        if (!enableWindow) {
            backFocusHdr = false;
            backFocusAlias = false;
        }

        //Anti-aliasing button
        styleOperativeWidget(2);
        colorOperativeWidget(ImVec2(175.0f * scaleX, 200.0f * scaleY), aliasingFocus, aliasingActive, 2);

        if (ImGui::IsWindowAppearing()) {
            ImGui::ClearActiveID();
            ImGui::SetFocusID(ImGui::GetID("alias"), ImGui::GetCurrentWindow());
            io->ConfigNavCursorVisibleAlways = true;
        }

        if (enableWindow) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            needToPop = true;
        }

        if (ImGui::Button("alias", ImVec2(400 * scaleX, 100 * scaleY))) {
            enableWindow = true;
            aliasingActive = true;
            hdrActive = false;
            firstOpen = true;
        }

        if (ImGui::IsItemHovered() && (io->MouseDelta.x != 0.0f || io->MouseDelta.y != 0.0f)) {
            ImGui::SetKeyboardFocusHere(-1);
        }

        if (ImGui::IsItemFocused() || ImGui::IsItemHovered()) {
            aliasingFocus = true;
            hdrFocus = false;
            aliasingWindow = true;
            hdrWindow = false;
            backFocus = false;
        }

        //HDR Button
        colorOperativeWidget(ImVec2(175.0f * scaleX, 350.0f * scaleY), hdrFocus, hdrActive, 2);

        if (ImGui::Button("hd", ImVec2(400 * scaleX, 100 * scaleY))) {
            enableWindow = true;
            aliasingActive = false;
            hdrActive = true;
            firstOpen = true;
        }

        if (ImGui::IsItemHovered() && (io->MouseDelta.x != 0.0f || io->MouseDelta.y != 0.0f)) {
            ImGui::SetKeyboardFocusHere(-1);
        }

        if (ImGui::IsItemFocused() || ImGui::IsItemHovered()) {
            aliasingFocus = false;
            hdrFocus = true;
            aliasingWindow = false;
            hdrWindow = true;
            backFocus = false;
        }

        //Back Button
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.56f, 1.0f, 1.0f));

        colorOperativeWidget(ImVec2(50.0f * scaleX, 50.0f * scaleY), backFocus, backFocus, 1);
        

        if (ImGui::ImageButton("back", back.id, ImVec2(100.0f * scaleX, 50.0f * scaleY))) {
            showSettings = false;
            showMenu = true;
        }

        if (ImGui::IsItemHovered() && (io->MouseDelta.x != 0.0f || io->MouseDelta.y != 0.0f)) {
            ImGui::SetKeyboardFocusHere(-1);
        }

        if (ImGui::IsItemFocused() || ImGui::IsItemHovered()) {
            aliasingFocus = false;
            hdrFocus = false;
            aliasingWindow = false;
            hdrWindow = false;
            backFocus = true;
        }

        if (needToPop) {
            ImGui::PopItemFlag();
            needToPop = false;
        }

        //Decorative Button
        styleDecorativeWidget();
        ImGui::SetCursorPos(ImVec2(195.0f * scaleX, 210.0f * scaleY));
        ImGui::Button("anti-aliasing", ImVec2(357 * scaleX, 80 * scaleY));
        drawDecorativeWidget(ImVec2(195.0f * scaleX, 210.0f * scaleY), "anti-aliasing", ImVec2(357 * scaleX, 80 * scaleY), 1);
        drawDecorativeWidget(ImVec2(195.0f * scaleX, 360.0f * scaleY), "hdr", ImVec2(357 * scaleX, 80 * scaleY), 1);
        clearStyle(1, 14, 3);

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
            showSettings = false;
            showMenu = true;
        }

        ImGui::EndChild();
        clearStyle(4, 2, 2);

        //Anti-aliasing settings windows
        if (aliasingWindow) {
            //Window style
            styleWindow(ImVec2(965.0f * scaleX, 350.0f * scaleY), ImVec2(750.0f * scaleX, 650.0f * scaleY));

            ImGui::BeginChild("ciao2", ImVec2(750 , 650), ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);

            if (aliasingFocus && enableWindow)
                ImGui::SetWindowFocus();

            //Off button
            styleOperativeWidget(3);
            colorOperativeWidget(ImVec2(410.0f * scaleX, 200.0f * scaleY), offFocus, offActiveAlias, 3);

            if (!enableWindow) {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                needToPop = true;
            }

            if (firstOpen) {
                ImGui::SetFocusID(ImGui::GetID("no"), ImGui::GetCurrentWindow());
                firstOpen = false;
            }

            if (ImGui::Button("no", ImVec2(200 * scaleX, 80 * scaleY))) {
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

            //On Button
            colorOperativeWidget(ImVec2(140.0f * scaleX, 200.0f * scaleY), onFocus, onActiveAlias, 3);

            if (ImGui::Button("si", ImVec2(200 * scaleX, 80 * scaleY))) {
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

            //Back Button
            colorOperativeWidget(ImVec2(50.0f * scaleX, 50.0f * scaleY), backFocusAlias, backFocusAlias, 1);

            if (ImGui::ImageButton("backAlias", back.id, ImVec2(70.0f * scaleX, 37.5f * scaleY))) {
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

            //Decorative Button
            styleDecorativeWidget();
            drawDecorativeWidget(ImVec2(435.0f * scaleX, 210.0f * scaleY), "off", ImVec2(157 * scaleX, 60 * scaleY), 1);
            drawDecorativeWidget(ImVec2(165.0f * scaleX, 210.0f * scaleY), "on", ImVec2(157 * scaleX, 60 * scaleY), 1);
            clearStyle(2, 10, 3);

            //Slider-grab
            ImGui::SetCursorPos(ImVec2(143.0f * scaleX, 350.0f * scaleY));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f * scaleX, 10.0f * scaleY));

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

            if (!barFocus) {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                needToPop = true;
            }

            ImGui::PushItemWidth(450.0f * scaleX);

            if (ImGui::SliderInt("##discrete", &selectableAlias, 0, IM_ARRAYSIZE(aliasItems) - 1, "", ImGuiSliderFlags_NoInput)) {
                switch (selectableAlias) {
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

            clearStyle(4, 6, 1);

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
            ImVec2 imageSize(80.0f * scaleX, 80.0f * scaleY);
            ImGui::PushFont(myFont3);

            if (!barFocus)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.988f, 0.812f, 0.0f, 0.5f));
            else
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));

            drawDecorativeWidget(ImVec2(255.0f * scaleX, 400.0f * scaleY), "4x", ImVec2(0.0f, 0.0f), 2);
            drawDecorativeWidget(ImVec2(420.0f * scaleX, 400.0f * scaleY), "8x", ImVec2(0.0f, 0.0f), 2);
            drawDecorativeWidget(ImVec2(585.0f * scaleX, 400.0f * scaleY), "16x", ImVec2(0.0f, 0.0f), 2);

        	ImVec2 imagePos(1210.0f * scaleX, 675.0f * scaleY);
            ImVec2 imagePos1(1375.0f * scaleX, 675.0f * scaleY);
            ImVec2 imagePos2(1535.0f * scaleX, 675.0f * scaleY);
            if (!barActiveAlias) {
                ImGui::GetWindowDrawList()->AddImage(coin.id, imagePos, ImVec2(imagePos.x + imageSize.x, imagePos.y + imageSize.y));
                ImGui::GetWindowDrawList()->AddImage(coin.id, imagePos1, ImVec2(imagePos1.x + imageSize.x, imagePos1.y + imageSize.y));
                ImGui::GetWindowDrawList()->AddImage(coin.id, imagePos2, ImVec2(imagePos2.x + imageSize.x, imagePos2.y + imageSize.y));
            }
            else {
                ImGui::GetWindowDrawList()->AddImage(coin2.id, imagePos, ImVec2(imagePos.x + imageSize.x, imagePos.y + imageSize.y));
                ImGui::GetWindowDrawList()->AddImage(coin2.id, imagePos1, ImVec2(imagePos1.x + imageSize.x, imagePos1.y + imageSize.y));
                ImGui::GetWindowDrawList()->AddImage(coin2.id, imagePos2, ImVec2(imagePos2.x + imageSize.x, imagePos2.y + imageSize.y));
            }

            clearStyle(2, 2, 2);
            ImGui::EndChild();
        }

        //Hdr window settings
        if (hdrWindow) {
            //Window style
            styleWindow(ImVec2(965.0f * scaleX, 350.0f * scaleY), ImVec2(750.0f * scaleX, 650.0f * scaleY));
            ImGui::BeginChild("ciao2", ImVec2(750, 650), ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);

            if (hdrFocus && enableWindow)
                ImGui::SetWindowFocus();

            //Off button 
            styleOperativeWidget(3);
            colorOperativeWidget(ImVec2(410.0f * scaleX, 200.0f * scaleY), offFocus, offActiveHdr, 3);

            if (!enableWindow) {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                needToPop = true;
            }

            if (firstOpen) {
                ImGui::SetFocusID(ImGui::GetID("no"), ImGui::GetCurrentWindow());
                firstOpen = false;
            }

            if (ImGui::Button("no", ImVec2(200 * scaleX, 80 * scaleY))) {
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

            //On Button
            colorOperativeWidget(ImVec2(140.0f * scaleX, 200.0f * scaleY), onFocus, onActiveHdr, 3);

            if (ImGui::Button("si", ImVec2(200 * scaleX, 80 * scaleY))) {
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

            //Back Button
            colorOperativeWidget(ImVec2(50.0f * scaleX, 50.0f * scaleX), backFocusHdr, backFocusHdr, 1);

            if (ImGui::ImageButton("backHdr", back.id, ImVec2(70.0f * scaleX, 37.5f * scaleY))) {
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

            //Decorative Button
            styleDecorativeWidget();
            drawDecorativeWidget(ImVec2(435.0f * scaleX, 210.0f * scaleY), "off", ImVec2(157 * scaleX, 60 * scaleY), 1);
            drawDecorativeWidget(ImVec2(165.0f * scaleX, 210.0f * scaleY), "on", ImVec2(157 * scaleX, 60 * scaleY), 1);
            clearStyle(1, 10, 3);

            //Slider-grab
            ImGui::SetCursorPos(ImVec2(143.0f * scaleX, 350.0f * scaleY));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f * scaleX, 10.0f * scaleY));

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

            ImGui::PushItemWidth(450.0f * scaleX);

            if (ImGui::SliderInt("##discrete", &selectableHdr, 0, IM_ARRAYSIZE(hdrItems) - 1, "", ImGuiSliderFlags_NoInput)) {
                switch (selectableHdr) {
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

            clearStyle(4, 6, 1);

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
            ImVec2 imageSize(80.0f * scaleX, 80.0f * scaleX);
            ImGui::PushFont(myFont3);

            if (!barFocus)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.988f, 0.812f, 0.0f, 0.5f));
            else
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.988f, 0.812f, 0.0f, 1.0f));

            drawDecorativeWidget(ImVec2(180.0f * scaleX, 400.0f * scaleY), "0.25", ImVec2(0.0f, 0.0f), 2);
            drawDecorativeWidget(ImVec2(320.0f * scaleX, 400.0f * scaleY), "0.5", ImVec2(0.0f, 0.0f), 2);
            drawDecorativeWidget(ImVec2(430.0f * scaleX, 400.0f * scaleY), "0.75", ImVec2(0.0f, 0.0f), 2);
            drawDecorativeWidget(ImVec2(600.0f * scaleX, 400.0f * scaleY), "1", ImVec2(0.0f, 0.0f), 2);

            ImVec2 imagePos(1175.0f * scaleX, 675.0f * scaleY);
            ImVec2 imagePos1(1295.0f * scaleX, 675.0f * scaleY);
            ImVec2 imagePos2(1415.0f * scaleX, 675.0f * scaleY);
            ImVec2 imagePos3(1535.0f * scaleX, 675.0f * scaleY);
            if (!barActiveHdr) {
                ImGui::GetWindowDrawList()->AddImage(coin.id, imagePos, ImVec2(imagePos.x + imageSize.x, imagePos.y + imageSize.y));
                ImGui::GetWindowDrawList()->AddImage(coin.id, imagePos1, ImVec2(imagePos1.x + imageSize.x, imagePos1.y + imageSize.y));
                ImGui::GetWindowDrawList()->AddImage(coin.id, imagePos2, ImVec2(imagePos2.x + imageSize.x, imagePos2.y + imageSize.y));
                ImGui::GetWindowDrawList()->AddImage(coin.id, imagePos3, ImVec2(imagePos3.x + imageSize.x, imagePos3.y + imageSize.y));
            }
            else {
                ImGui::GetWindowDrawList()->AddImage(coin2.id, imagePos, ImVec2(imagePos.x + imageSize.x, imagePos.y + imageSize.y));
                ImGui::GetWindowDrawList()->AddImage(coin2.id, imagePos1, ImVec2(imagePos1.x + imageSize.x, imagePos1.y + imageSize.y));
                ImGui::GetWindowDrawList()->AddImage(coin2.id, imagePos2, ImVec2(imagePos2.x + imageSize.x, imagePos2.y + imageSize.y));
                ImGui::GetWindowDrawList()->AddImage(coin2.id, imagePos3, ImVec2(imagePos3.x + imageSize.x, imagePos3.y + imageSize.y));
            }

            clearStyle(2, 2, 2);
            ImGui::EndChild();
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