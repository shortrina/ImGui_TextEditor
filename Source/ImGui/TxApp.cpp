/*
 * TxApp.cpp
 *
 *  Created on: Jan 24, 2024
 *      Author: sunny
 */

#include "../Headers/TxApp.hpp"
#include <unistd.h>

void TxApp::EditorWindowKeyCallback()
{
	//printf("Key Pressed \n");
	//Stop();
	//ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	if(ImGui::IsKeyPressed(ImGuiKey_S,false))
	{
		printf("Save File \n");
		SaveFile();

	}else if(ImGui::IsKeyPressed(ImGuiKey_O, false))
	{
		printf("Open File \n");
		LoadFile();
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
		NewFile();
		//Start();
	}
}

void TxApp::PollKeyEvent()
{
	ImGuiIO& io = ImGui::GetIO();
	//printf("Key Pressed : %d \n", io.KeyCtrl);
	if(io.KeyCtrl)
	{
		//printf("Ctrl Key Pressed \n");
		EditorWindowKeyCallback();
	}

}


TxApp::TxApp() : TxGuiApp("TxApp")
{
    SetTitle(" ");
	m_string = "";
	m_open_fileDialog = false;
	m_newfile_save = false;
	m_tmpfile_name = "tmpfile.txt";
	openSaveModal = false;
}

void TxApp::Update() //initial call
{
	ImGui::SetNextWindowSize(ImVec2(1280, 720));
	ImGui::SetNextWindowPos(ImVec2(0, 0));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_MenuBar|ImGuiWindowFlags_NoDocking;
	bool open = true;
    //Main body of the Demo window starts here.
    ImGui::Begin("TxAppSpace",&open,flags);
	{

		PollKeyEvent();
		DrawSaveModal();
		Menu();
		Editor();
	}
	ImGui::End();
}

void TxApp::Editor()
{

	float width = ImGui::GetWindowWidth();
	float height = ImGui::GetWindowHeight();
	float topbarboarder = ImGui::GetFrameHeight();
    float topboarderSpacing = ImGui::GetFrameHeightWithSpacing();
    //printf("topbarboarder : %f \n", topbarboarder);
	ImGui::SetNextWindowSize(ImVec2(1280, 720-topbarboarder));
	ImGui::SetNextWindowPos(ImVec2(0, topbarboarder));

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoDocking;
	bool open = true;

	std::string *str_buffer = &this->m_string;
	ImGuiInputTextCallbackData data;
	data.EventFlag = ImGuiInputTextFlags_CallbackEdit;
	data.Flags = ImGuiInputTextFlags_CallbackEdit;
	const char* fileName = "UnTitled";

	ImGui::BeginChild(ImGui::GetID(fileName), ImVec2(0, 0),true);

	ImGui::InputTextMultiline("##note", str_buffer,
			ImVec2(-FLT_MIN, 700),ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_CallbackEdit,
			nullptr,(void *)(&data));

	ImGui::EndChild();

}

void TxApp::Menu()
{
	if (ImGui::BeginMenuBar())
    {
        MenuFile();
        MenuEdit();
        ImGui::EndMenuBar();
    }

}

void TxApp::MenuFile()
{
   if (ImGui::BeginMenu("File"))
   {
       if (ImGui::MenuItem(u8"\ue873 New", nullptr, false, true))
       {
		   openSaveModal = true;
		   stateSaveModal = 1;
		   //NewProject();

       }
       if (ImGui::MenuItem(u8"\ueaf3 Open", nullptr, false, true))
       {
               openSaveModal = true;
               stateSaveModal = 2;
               //OpenProject();
       }
       ImGui::Separator();
       if (ImGui::MenuItem(u8"\ue161 Save", nullptr, false, true))
       {
           //SaveProject();
       }
       if (ImGui::MenuItem(u8"\ueb60 Save As...", nullptr, false, true))
       {
           //SaveProject(true);
       }
       ImGui::Separator();
       if (ImGui::MenuItem(u8"\ue9ba Exit", nullptr, false, true))
       {
           attemptToClose = true;
       }
       ImGui::EndMenu();
   }
   // Exit
   if (attemptToClose == true) // Exit, Window X, alt+f4.
   {
       attemptToClose = false;
	   openSaveModal = true;
	   stateSaveModal = 3;
   }
}

void TxApp::MenuEdit()
{
	if(ImGui::BeginMenu("Edit"))
	{
		if(ImGui::MenuItem(u8"\ue871 Undo", "CTRL+Z"))
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

void TxApp::DrawSaveModal(void)
{
	ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.00f, 1.00f);
	ImGuiID id = ImGui::GetID("Save ?");

	if(openSaveModal==true)
	{
		printf("OpenPopup is called \n");
		ImGui::OpenPopup("Save ?");
		openSaveModal = false;
	}
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.50f, 0.50f));
	if(ImGui::BeginPopupModal("Save ?", NULL, ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoDocking))
	{
		ImGui::Text("Are you sure to save ?");
		ImGui::Separator();
		if(ImGui::Button("Yes",ImVec2(80,0)))
		{
			//printf("Save File \n");
			m_newfile_save = true;
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
}

void TxApp::SaveFile()
{
	std::ofstream myfile;
	std::string text_filename;

	printf("Enter File Name : ");
	std::cin >> text_filename;

	if(m_newfile_save)
	{
		std::rename(m_tmpfile_name.c_str(), text_filename.c_str());
		m_newfile_save = false;
	}else
	{
		myfile.open (text_filename);
		myfile << this->m_string;
		myfile.close();
	}
}

void TxApp::LoadFile()
{
	printf("Load File \n");
}

void TxApp::NewFile()
{

	if(this->m_string.size()>0)
	{
		std::ofstream myfile;
		myfile.open (this->m_tmpfile_name);
		myfile << this->m_string;
		myfile.close();
	}

	openSaveModal = true;
	DrawSaveModal();
	this->m_string.clear();



}


