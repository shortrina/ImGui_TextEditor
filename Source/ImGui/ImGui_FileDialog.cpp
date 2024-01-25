/*
 * ImGui_FileDialog.cpp
 *
 *  Created on: Jan 23, 2024
 *      Author: sunny
 */

#include "../Hearders/ImGui_FileDialog.hpp"

bool FileDialog::Draw(bool* open)
{
	if(*open == false)
	{
		return false;
	}else
	{
		ImGui::OpenPopup(title.c_str());
	}

    bool done = false;
    title = (type == Type::OPEN) ? "Open File" : "Save File";
    ImGui::SetNextWindowSize(ImVec2(660.0f, 410.0f), ImGuiCond_Once);
    ImGui::SetNextWindowSizeConstraints(ImVec2(410, 410), ImVec2(1080, 410));
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if(ImGui::BeginPopupModal(title.c_str(), open, ImGuiWindowFlags_NoCollaps|ImGuiWindowFlags_NoDocking))
    {
    	if(currentFiles.empty() && currentDirectories.empty() || refresh)
		{
    		refresh = false;
    		currentIndex = 0;
    		currentFiles.clear();
    		currentDirectories.clear();
    		for(const auto& entry : std::filesystem::directory_iterator(directoryPath))
    		{
				if(entry.is_directory()) currentDirectories.push_back(entry) : currentFiles.push_back(entry);
    		}
		}//end currentfile

    	//Path
    	ImGui::Text("Path : %s", directoryPath.string().c_str());
    	ImGui::BeginChild("##browser",ImVec2(ImGui::GetWindowContentRegionWidth(),300.0f),true,ImGuiWindowFlags_None);
    	size_t index = 0;

    	//Parent
    	if(directoryPath.has_parent_path())
		{
			if(ImGui::Selectable("..",currentIndex == index, ImGuiSelectableFlags_AllowDoubleClick),ImVec2(ImGui::GetWindowContentRegionWidth(),0))
			{
				currentIndex = index;
				if(ImGui::IsMouseDoubleClicked(0))
				{
					directoryPath = directoryPath.parent_path();
					refresh = true;
				}

			}
			index++;
		}//end parent

		//Directories
		for(const auto& dir : currentDirectories )
		{
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 210, 0, 255));
			if(ImGui::Selectable(dir.path().filename().string().c_str(),currentIndex == index, ImGuiSelectableFlags_AllowDoubleClick),ImVec2(ImGui::GetWindowContentRegionWidth(),0))
			{
				currentIndex = index;
				if(ImGui::IsMouseDoubleClicked(0))
				{
					directoryPath = dir.path();
					refresh = true;
				}
			}
			ImGui::PopStyleColor();
			index++;
		}//end Directories

		//Files
		for(const auto& files : currentFiles)
		{
			if(ImGui::Selectable(files.path().filename().string().c_str(),currentIndex == index, ImGuiSelectableFlags_AllowDoubleClick),ImVec2(ImGui::GetWindowContentRegionWidth(),0))
			{
				currentIndex = index;
				if(ImGui::IsMouseDoubleClicked(0))
				{
					currentIndex = index;
					fileName = files.path().filename();
				}
			}
			index++;
		}//end Files
		ImGui::EndChild();


		//Draw FileNames
		size_t fileNameSize = fileName.string().size();
		if(fileNameSize >= bufferSize)
		{
			fileNameSize = bufferSize;
		}
		std::memcpy(buffer,fileName.string().c_str(),fileNameSize);
		buffer[bufferSize-1] = '/0';

		ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth());
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 200, 0, 255));
		if(ImGui::InputText("File Name",buffer,bufferSize))
		{
			fileName = std::string(buffer);
			currentIndex = 0;
		}
		ImGui::PopStyleColor();
		if(ImGui::Button("Cancel"))
		{
			refresh = false;
			currentIndex = 0;
			currentFiles.clear();
			currentDirectories.clear();
			*open = false;
		}
		ImGui::SameLine();
		resultPath = directoryPath / fileName;
		if(type == Type::OPEN)
		{
			if(ImGui::Button("Open"))
			{
				if(std::filesystem::exists(resultPath)&&fileName.string.rfind(".") != std::string::npos &&
					fileName.string.substr(fileName.string.rfind(".") + 1) == fileFormat)
				{
					refresh= false;
					currentIndex = 0;
					currentFiles.clear();
					currentDirectories.clear();
					*open = false;
					done = true;
				}
				else
				{
					printf("File Format does not valid\n");
					//TODO: Show Error(popup)
				}
			}//end Button Open
		}//end Open
		else if(type == Type::SAVE)
		{
			const auto beforeFormatCheck = resultPath.string();
			bool isFormatCorrect = false;
			if(auto dot = fileName.string.rfind("."); dot != std::string::npos)
			{
				resultPath = resultPath.string() + fileFormat;
			}
			else if(fileName.string.substr(dot) != fileFormat)
			{
				resultPath = resultPath.string() + fileFormat;
			}
			else
			{
				isFormatCorrect = true;
			}
			if(ImGui::Button("Save"))
			{
				if(std::filesystem::exists(beforeFormatCheck)==true && isFormatCorrect==false)
				{
					printf("Same File Name and File Format\n");
				}
				else if(std::filesystem::exists(resultPath)==false)
				{
					refresh= false;
					currentIndex = 0;
					currentFiles.clear();
					currentDirectories.clear();
					*open = false;
					done = true;
				}
				else
				{
					ImGui::OpenPopup("Override?");

				}
			}//end Button Save
			ImGui::SetNextWindowPos(ImGui::GetCurrentWindow()->Rect().GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
			if(ImGui::BeginPopupModal("Override?",nullptr,ImGuiWindowFlags_NResize))
			{
				ImGui::Text("File already exists! Do you want to override?");
				ImGui::Separator();
				if(ImGui::Button("Yes",ImVec2(50,0)))
				{
					refresh= false;
					currentIndex = 0;
					currentFiles.clear();
					currentDirectories.clear();
					*open = false;
					done = true;
				}
				ImGui::SameLine();
				if(ImGui::Button("No",ImVec2(50,0)))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}//end Modal
		}//end Save
	        ImGui::EndPopup();
    }//end of BeginPopupModal
    return done;
}




