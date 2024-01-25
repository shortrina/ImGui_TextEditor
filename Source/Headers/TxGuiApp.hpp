/*
 * TxGuiApp.hpp
 *
 *  Created on: Jan 24, 2024
 *      Author: sunny
 */

#ifndef TXGUIAPP_HPP_
#define TXGUIAPP_HPP_

#include <iostream>
#include <fstream>
#include <mutex>
#include <string>
#include <stdio.h>


#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGui_FileDialog.hpp"
#include "imgui_stdlib.h"
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

class TxGuiApp {
public:
	explicit TxGuiApp(std::string_view name);
	virtual ~TxGuiApp();

	void Run();

	virtual void Update() = 0;
    void SetTitle(std::string_view name);
//    void SetAsterisk(bool flag);
    std::string GetTitle() const { return title; }
//    bool GetAsterisk() const { return titleAsterisk; }
protected:

    inline static bool attemptToClose = false;
    inline static bool timeToClose = false;

    ImFont* fontRobotoMedium;
    ImFont* fontRobotoRegular;
    ImFont* fontMaterialIcons;
    ImFont* fontLarge;
    ImFont* iconLarge;

private:
    void UpdateTitle();
    std::string title{ "" };
    //bool titleAsterisk = false;
    std::string appName{ "" };
    GLFWwindow* window;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    static void glfw_error_callback(int error, const char* description);
    static void window_close_callback(GLFWwindow* window);

};

#endif /* TXGUIAPP_HPP_ */
