/*
 * TxApp.hpp
 *
 *  Created on: Jan 24, 2024
 *      Author: sunny
 */

#ifndef HEADERS_TXAPP_HPP_
#define HEADERS_TXAPP_HPP_

#include <memory>
#include <deque>
#include <iostream>

#include "TxGuiApp.hpp"

class TxApp : public TxGuiApp
{
public:
	TxApp();
	virtual ~TxApp() final = default;

	void Update() override;
    //void TestBasic() const;

private:
	  std::string m_tmpfile_name;
	  std::string m_string;
	  bool m_open_fileDialog ;
	  bool m_newfile_save;



	  //void SelectTab(const char* windowName) const;
	  std::string version{ "v0.1.0" };
	  void PollKeyEvent(void);
	  void Menu(void);
	  void MenuFile(void);
	  void MenuEdit(void);

	  void Editor(void);

	  void SaveFile(void);
	  void LoadFile(void);
	  void NewFile(void);


	  //FileDialog fileDialog;
	  //bool fileDialogOpen = false;
	  //void DrawFileDialog();
	  //bool hasFile = false;
	  //std::filesystem::path filePath;

	  bool openSaveModal = false;
	  int stateSaveModal = 0; // 1: from new, 2: from open, 3: from e
	  void DrawSaveModal();
	  void EditorWindowKeyCallback(void);
	  //void SaveToFile(const std::string& fName, const std::string& fPath);

	  //void LoadFromFile();
	  //int iCurrentDoc{ 0 };
	  //int iSavedDoc{ 0 };

	  //void UndoRedoSave();
	  //void ResetDocDeque();
	  //bool openAbout = false;
	//  void DrawAbout();
};

#endif /* HEADERS_TXAPP_HPP_ */
