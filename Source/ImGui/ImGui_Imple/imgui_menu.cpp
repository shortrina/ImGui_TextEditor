/*
 * imGui_menu.cpp
 *
 *  Created on: Jan 4, 2024
 *      Author: sunny
 */

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "../Headers/imgui.h"
#include "ImGui_FileManager.hpp"
#include "ImGui_NoteManager.hpp"

#ifndef IMGUI_DISABLE
// System includes
#include <ctype.h>          // toupper
#include <limits.h>         // INT_MIN, INT_MAX
#include <math.h>           // sqrtf, powf, cosf, sinf, floorf, ceilf
#include <stdio.h>          // vsnprintf, sscanf, printf
#include <stdlib.h>         // NULL, malloc, free, atoi
#include <stdint.h>         // intptr_t
#if !defined(_MSC_VER) || _MSC_VER >= 1800
#include <inttypes.h>       // PRId64/PRIu64, not avail in some MinGW headers.
#endif

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (disable: 4127)     // condition expression is constant
#pragma warning (disable: 4996)     // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#pragma warning (disable: 26451)    // [Static Analyzer] Arithmetic overflow : Using operator 'xxx' on a 4 byte value and then casting the result to an 8 byte value. Cast the value to the wider type before calling operator 'xxx' to avoid overflow(io.2).
#endif

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#if __has_warning("-Wunknown-warning-option")
#pragma clang diagnostic ignored "-Wunknown-warning-option"         // warning: unknown warning group 'xxx'                     // not all warnings are known by all Clang versions and they tend to be rename-happy.. so ignoring warnings triggers new warnings on some configuration. Great!
#endif
#pragma clang diagnostic ignored "-Wunknown-pragmas"                // warning: unknown warning group 'xxx'
#pragma clang diagnostic ignored "-Wold-style-cast"                 // warning: use of old-style cast                           // yes, they are more terse.
#pragma clang diagnostic ignored "-Wdeprecated-declarations"        // warning: 'xx' is deprecated: The POSIX name for this..   // for strdup used in demo code (so user can copy & paste the code)
#pragma clang diagnostic ignored "-Wint-to-void-pointer-cast"       // warning: cast to 'void *' from smaller integer type
#pragma clang diagnostic ignored "-Wformat-security"                // warning: format string is not a string literal
#pragma clang diagnostic ignored "-Wexit-time-destructors"          // warning: declaration requires an exit-time destructor    // exit-time destruction order is undefined. if MemFree() leads to users code that has been disabled before exit it might cause problems. ImGui coding style welcomes static/globals.
#pragma clang diagnostic ignored "-Wunused-macros"                  // warning: macro is not used                               // we define snprintf/vsnprintf on Windows so they are available, but not always used.
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"  // warning: zero as null pointer constant                   // some standard header variations use #define NULL 0
#pragma clang diagnostic ignored "-Wdouble-promotion"               // warning: implicit conversion from 'float' to 'double' when passing argument to function  // using printf() is a misery with this as C++ va_arg ellipsis changes float to double.
#pragma clang diagnostic ignored "-Wreserved-id-macro"              // warning: macro name is a reserved identifier
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"  // warning: implicit conversion from 'xxx' to 'float' may lose precision
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wpragmas"                  // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"      // warning: cast to pointer from integer of different size
#pragma GCC diagnostic ignored "-Wformat-security"          // warning: format string is not a string literal (potentially insecure)
#pragma GCC diagnostic ignored "-Wdouble-promotion"         // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"               // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#pragma GCC diagnostic ignored "-Wmisleading-indentation"   // [__GNUC__ >= 6] warning: this 'if' clause does not guard this statement      // GCC 6.0+ only. See #883 on GitHub.
#endif

// Play it nice with Windows users (Update: May 2018, Notepad now supports Unix-style carriage returns!)
#ifdef _WIN32
#define IM_NEWLINE  "\r\n"
#else
#define IM_NEWLINE  "\n"
#endif

// Helpers
#if defined(_MSC_VER) && !defined(snprintf)
#define snprintf    _snprintf
#endif
#if defined(_MSC_VER) && !defined(vsnprintf)
#define vsnprintf   _vsnprintf
#endif

// Format specifiers for 64-bit values (hasn't been decently standardized before VS2013)
#if !defined(PRId64) && defined(_MSC_VER)
#define PRId64 "I64d"
#define PRIu64 "I64u"
#elif !defined(PRId64)
#define PRId64 "lld"
#define PRIu64 "llu"
#endif

// Helpers macros
// We normally try to not use many helpers in imgui_demo.cpp in order to make code easier to copy and paste,
// but making an exception here as those are largely simplifying code...
// In other imgui sources we can use nicer internal functions from imgui_internal.h (ImMin/ImMax) but not in the demo.
#define IM_MIN(A, B)            (((A) < (B)) ? (A) : (B))
#define IM_MAX(A, B)            (((A) >= (B)) ? (A) : (B))
#define IM_CLAMP(V, MN, MX)     ((V) < (MN) ? (MN) : (V) > (MX) ? (MX) : (V))


// Helper to wire demo markers located in code to an interactive browser
typedef void (*ImGuiDemoMarkerCallback)(const char* file, int line, const char* section, void* user_data);
extern ImGuiDemoMarkerCallback      GImGuiDemoMarkerCallback;
extern void*                        GImGuiDemoMarkerCallbackUserData;
ImGuiDemoMarkerCallback             GImGuiDemoMarkerCallback = NULL;
void*                               GImGuiDemoMarkerCallbackUserData = NULL;
#define IMGUI_DEMO_MARKER(section)  do { if (GImGuiDemoMarkerCallback != NULL) GImGuiDemoMarkerCallback(__FILE__, __LINE__, section, GImGuiDemoMarkerCallbackUserData); } while (0)

void imgui_window_marker(char *section){IMGUI_DEMO_MARKER(section);};
//-----------------------------------------------------------------------------
// [SECTION] Forward Declarations, Helpers
//-----------------------------------------------------------------------------

#if !defined(IMGUI_DISABLE_DEMO_WINDOWS)

static void ShowConsole(bool* p_open)
{
    //static ExampleAppConsole console; //AppConsole Class

	static ImGui_FileExplore fileExplore;
	IMGUI_DEMO_MARKER("FileManager");
	fileExplore.ImGui_FileWindowDraw("FileManager", p_open);
    //console.Draw("Example: Console", p_open);
}

static void ShowNoteManager(bool* p_open)
{
	static ImGui_NoteManager noteManager;
	IMGUI_DEMO_MARKER("NoteManager");
	noteManager.ImGui_OpenNewFile("newfile",p_open);
}

/*
 *
 * */
void ImGui::ShowDemoWindow(const char* name, bool* p_open)
{
    IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing Dear ImGui context. Refer to examples app!");

    // Examples Apps (accessible from the "Examples" menu)
    static bool show_app_open_directory = false;
    static bool show_app_main_menu_bar = true;
    static bool show_app_note_manager = true;


    if (show_app_open_directory)             ShowConsole(&show_app_open_directory);


    ImGuiWindowFlags window_flags = 0;
    static bool no_titlebar = true;
    static bool no_menu = true;
    static bool no_docking = true;

    if(no_titlebar)        window_flags |= ImGuiWindowFlags_NoTitleBar;
    if(!no_menu)           window_flags |= ImGuiWindowFlags_MenuBar;
    if(no_docking)         window_flags |= ImGuiWindowFlags_NoDocking;

    // We specify a default position/size in case there's no data in the .ini file.
    // We only do it to make the demo applications a little more welcoming, but typically this isn't required.
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
//    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
//    ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
      //ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_Always);
      ImGui::SetNextWindowSize(ImVec2(1280, 720), ImGuiCond_None);
#if 0
     //Main body of the Demo window starts here.
    if (!ImGui::Begin(name, p_open, window_flags))
    {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }
#endif
    //ImGui::PushItemWidth(ImGui::GetFontSize() * -12);

    /*Main Menu Bar*/
    if (ImGui::BeginMainMenuBar())
     {
         //printf("Muenu Bar Open :: %u \n", );
     	if (ImGui::BeginMenu("File"))
         {
             if (ImGui::MenuItem("New"))
             {

             }
             if(ImGui::MenuItem("Open", "Ctrl+O", &show_app_open_directory))
             //if (ImGui::MenuItem("Open", "Ctrl+O"))
             {
             	//system("nautilus $");
             }
             if (ImGui::BeginMenu("Open Recent"))
             {
                 ImGui::MenuItem("fish_hat.c");
                 ImGui::MenuItem("fish_hat.inl");
                 ImGui::MenuItem("fish_hat.h");
                 if (ImGui::BeginMenu("More.."))
                 {
                     ImGui::MenuItem("Hello");
                     ImGui::MenuItem("Sailor");
                     if (ImGui::BeginMenu("Recurse.."))
                     {
                         //ShowExampleMenuFile();
                         ImGui::EndMenu();
                     }
                     ImGui::EndMenu();
                 }
                 ImGui::EndMenu();
             }
             if (ImGui::MenuItem("Save", "Ctrl+S"))//First save
             {}
             if (ImGui::MenuItem("Save As.."))//change file name or change directory
             {}

         	ImGui::EndMenu();//end File
         }
         if (ImGui::BeginMenu("Edit"))
         {
             if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
             if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
             ImGui::Separator();
             if (ImGui::MenuItem("Cut", "CTRL+X")) {}
             if (ImGui::MenuItem("Copy", "CTRL+C")) {}
             if (ImGui::MenuItem("Paste", "CTRL+V")) {}
             ImGui::EndMenu();
         }
         ImGui::EndMainMenuBar();
     }//BeginMainMenuBar();

    ShowNoteManager(&show_app_note_manager);

    //ImGui::PopItemWidth();
//    ImGui::End();

}//end of ShowDemoWindow()

/*
 *
 */

/*Setting Font */

#endif
#endif
