/*
 * ImGui_TextEditor.cpp
 *
 *  Created on: Jan 22, 2024
 *      Author: sunny
 */

#include "../Headers/ImGui_TextEditor.hpp"
#include <fstream>
#include <iostream>
#include <thread>

void TextEditorManager::TextEditorWindowKeyCallback(void)
{
	//printf("Key Pressed \n");
	//Stop();
	//ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	if(ImGui::IsKeyPressed(ImGuiKey_S,false))
	{
		printf("Save File \n");
		this->m_txt_editor_frame.SaveFile();

	}else if(ImGui::IsKeyPressed(ImGuiKey_O, false))
	{
		printf("Open File \n");
		this->m_txt_editor_frame.LoadFile();
	}else if(ImGui::IsKeyPressed(ImGuiKey_N, false))
	{
		/* 1. show popup(Modal Widget) window(To check save or not)
		 * 	1. if yes, save file
		 * 	2. if no, open new frame(window);
		 * */
		//ImDrawList* draw_list = ImGui::GetWindowDrawList();
		//ImDrawList*   GetForegroundDrawList(ImGuiViewport* viewport);
		//ImGuiViewport* GetViewport();
        //ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

//		NoteEditorWindow_Render();

		printf("New File \n");
		this->m_txt_editor_frame.NewFile();
		//Start();
	}
}

void TextEditorManager::PollKeyEvent()
{
	ImGuiIO& io = ImGui::GetIO();
	//printf("Key Pressed : %d \n", io.KeyCtrl);
	if(io.KeyCtrl)
	{
		//printf("Ctrl Key Pressed \n");
		TextEditorWindowKeyCallback();
	}

}
void TextEditorManager::Run(void)
{
	//while(!m_txt_editor.CheckWindowClose())
	while(GetWindowStatus())
	{
		//m_txt_editor.TextEditor_PollEvent();
		m_txt_editor.TextEditor_WaitPollEvent();

		m_txt_editor_frame.NewFrame();
		PollKeyEvent();
		//printf("Key event is finished \n");
		m_base_window = m_txt_editor.Get_BaseWindow_Handle();

		m_txt_editor_frame.Set_Menubar_Frame(m_base_window);
		m_txt_editor_frame.MenuBar();
		m_txt_editor_frame.Set_TextEditor_Frame(m_base_window);
		m_txt_editor_frame.TextEditorFrame_Rander();

		m_txt_editor.Randering();
	}

	ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
}

/*TextEditor Core Window Start*/
void TextEditor::Initialize(void)
{
    glfwSetErrorCallback(glfw_error_callback);
    if(!glfwInit())
    	assert(0);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    m_base_window = glfwCreateWindow(1280, 720, "Dear Note Editor", nullptr, nullptr);
    if(m_base_window == nullptr)
        assert(0);

    glfwMakeContextCurrent(m_base_window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    m_base_window_context = ImGui::CreateContext();
    ImGuiIO& m_base_io = ImGui::GetIO(); (void)m_base_io;
    m_base_io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //m_base_io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    //m_base_io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;        // Enable Menubar
    //m_base_io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (m_base_io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends install all callbackfunctions of the GLFWwindow
    ImGui_ImplGlfw_InitForOpenGL(m_base_window, true);
    ImGui_ImplOpenGL3_Init(m_glsl_version);

    ImGui::SetCurrentContext(m_base_window_context);

}

void TextEditor::Randering(void)
{
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	// Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(this->m_base_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
    //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
    ImGuiIO& m_base_io = ImGui::GetIO(); (void)m_base_io;
    if (m_base_io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }

    ImGui::SetCurrentContext(this->m_base_window_context);

    glfwSwapBuffers(this->m_base_window);

}


GLFWwindow* TextEditor::Get_BaseWindow_Handle(void)
{
	return this->m_base_window;
}

void TextEditor::NewFrame(void)
{
	//printf("TextEditor::NewFrame()\n");
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void TextEditor::EndFrame(void)
{
	printf("TextEditor::EndFrame()\n");
    // Cleanup
	ImGui::EndFrame();
}
/*Text Editor Frame Start*/
void TextEditorFrame::Set_TextEditor_Frame(GLFWwindow* window)
{
	//printf("Set_TextEditor_Frame()\n");
    int width, height;

	/*Fit window size to screen*/
    glfwGetWindowSize(window, &width, &height);

	float topbarboarder = ImGui::GetFrameHeight();
    float topboarderSpacing = ImGui::GetFrameHeightWithSpacing();
	ImGui::SetNextWindowSize(ImVec2(width, height-topbarboarder));
	ImGui::SetNextWindowPos(ImVec2(0, topbarboarder));
	this->m_text_width = width;
	this->m_text_height = height-topboarderSpacing;
}

void TextEditorFrame::TextEditorFrame_Rander(void)
{
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoDocking;
	bool open = true;

	std::string *str_buffer = &this->m_string;
	ImGuiInputTextCallbackData data;
	data.EventFlag = ImGuiInputTextFlags_CallbackEdit;
	data.Flags = ImGuiInputTextFlags_CallbackEdit;
	const char* fileName = this->m_textfile_name.c_str();
	ImGui::Begin(fileName,&open,flags);

	ImGui::InputTextMultiline("##note", str_buffer,
			ImVec2(-FLT_MIN, this->m_text_height),ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_CallbackEdit,
			nullptr,(void *)(&data));

	ImGui::End();
}


void TextEditorFrame::SaveFile(void)
{
	std::ofstream myfile;
	std::string text_filename;

	printf("Enter File Name : ");
	std::cin >> text_filename;

	myfile.open (text_filename);
	myfile << this->m_string;
	myfile.close();

}

void TextEditorFrame::LoadFile(void)
{
	printf("Load File \n");
}

void TextEditorFrame::NewFile(void)
{
	//printf("New File \n");
	//ImGui::End();
	printf("OpenPopup!!! \n");
	//ImGui::OpenPopup("Save ?");
	m_openSaveModal = true;
	DrawSaveModal();
	this->m_string.clear();

	this->Set_TextEditor_Filename("UnTitled");
	TextEditorFrame_Rander();

}

/*Menubar Frame Start*/
void TextEditorFrame::Set_Menubar_Frame(GLFWwindow* window)
{
	//printf("Set_Menubar_Frame()\n");

    int width, height; //ImGuiDockNodeFlags_NoTabBar
	/*Fit window size to screen*/
    glfwGetWindowSize(window, &width, &height);
	ImGui::SetNextWindowSize(ImVec2(width, height));
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	//printf("window size : %d, %d\n", width, height);
}

void TextEditorFrame::MenuEdit(void)
{
	if(ImGui::BeginMenu("Edit"))
	{
		if(ImGui::MenuItem("Undo", "CTRL+Z"))
		{
			printf("Undo \n");
		}
		else if(ImGui::MenuItem("Redo", "CTRL+Y", false, false))
		{
			printf("Redo \n");
		}  // Disabled item
		ImGui::Separator();
		if(ImGui::MenuItem("Cut", "CTRL+X"))
		{
			printf("cut \n");
		}
		else if(ImGui::MenuItem("Copy", "CTRL+C"))
		{
			printf("copy \n");
		}
		else if(ImGui::MenuItem("Paste", "CTRL+V"))
		{
			printf("paste \n");
		}
		ImGui::EndMenu();//end Edit
	}
}

void TextEditorFrame::MenuFile(void)
{
	if (ImGui::BeginMenu("File"))
	{
		if(ImGui::MenuItem("New"))
		{
			printf("open new file \n");
			NewFile();
		}//end new
		else if(ImGui::MenuItem("Open", "Ctrl+O"))
		{
			printf("Open file \n");
			LoadFile();
		}//end open
		else if(ImGui::BeginMenu("Open Recent"))
		{
			ImGui::MenuItem("fish_hat.c");
			ImGui::MenuItem("fish_hat.inl");
			ImGui::MenuItem("fish_hat.h");
			if(ImGui::BeginMenu("More.."))
			{
				ImGui::MenuItem("Hello");
				ImGui::MenuItem("Sailor");
				if(ImGui::BeginMenu("Recurse.."))
				{
					//ShowExampleMenuFile();
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}//end more
			ImGui::EndMenu();
		}//end open recent
		if(ImGui::MenuItem("Save", "Ctrl+S"))
		{
			printf("save file \n");
			SaveFile();
		}//end save
		else if(ImGui::MenuItem("Save As.."))
		{
			printf("save file as \n");
		}//end save as
		ImGui::EndMenu();//end File
	}//end file

}

void TextEditorFrame::MenuBar(void)
{
	//printf("MenuBar()\n");
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_MenuBar|ImGuiWindowFlags_NoDocking;
	bool open = true;
    //Main body of the Demo window starts here.
    ImGui::Begin("Menubar",&open,flags);
	{
		if(ImGui::BeginMenuBar())
		{
			MenuFile();
			MenuEdit();
		}
		ImGui::EndMenuBar();
	}
	ImGui::End();
}

void TextEditorFrame::DrawSaveModal(void)
{
	ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.00f, 1.00f);
	ImGuiID id = ImGui::GetID("Save ?");
//	ImGui::BeginChild(id, ImVec2(80,80), true);
	if(m_openSaveModal==true)
	{
		ImGui::OpenPopup("Save ?");
		m_openSaveModal = false;
	}
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	if(ImGui::BeginPopupModal("Save ?", NULL, ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoDocking))
	{
		ImGui::Text("Are you sure to save ?");
		ImGui::Separator();
		if(ImGui::Button("Yes",ImVec2(80,0)))
		{
			//printf("Save File \n");
			SaveFile();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if(ImGui::Button("No",ImVec2(80,0)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if(ImGui::Button("Cancel",ImVec2(80,0)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	//ImGui::EndChild();
}

