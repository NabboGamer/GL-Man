#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Filesystem.hpp"
#include "ResourceManager.hpp"
#include "LoggerManager.hpp"
#include "Texture2D.hpp"
#include "Game.hpp"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "custom_types.hpp"

class Menu {

public:

	explicit Menu(GLFWwindow* window, unsigned int width, unsigned int height, bool& showGame, CustomStructs::Config& config);
	~Menu();

	void Init();
	void Render(double deltaTime, Game* GLMan) const;

private:

	GLFWwindow*            window;
	unsigned int           width;
	unsigned int           height;
	bool&                  showGame;
	CustomStructs::Config& config;

	GLFWcursor* createCustomCursor(const char* imagePath, int hotspotX, int hotspotY);
	void styleOperativeWidget(int page) const;
	void styleDecorativeWidget() const;
	void colorOperativeWidget(ImVec2 buttonPos, bool& focused, bool& actived, int page) const;
	void clearStyle(int page, int colors, int vars) const;
	void styleWindow(ImVec2 pos, ImVec2 size) const;
	void drawDecorativeWidget(ImVec2 pos, const char* label, ImVec2 dim, int type) const;
};