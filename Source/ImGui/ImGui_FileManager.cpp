/*
 * ImGui_FileManager.cpp
 *
 *  Created on: Dec 28, 2023
 *      Author: sunny
 */
#include "ImGui_FileManager.hpp"

#include <chrono>
#include <ctime>
// Enforce cdecl calling convention for functions called by the standard library, in case compilation settings changed the default to e.g. __vectorcall
#ifndef IMGUI_CDECL
#ifdef _MSC_VER
#define IMGUI_CDECL __cdecl
#else
#define IMGUI_CDECL
#endif
#endif

/* Order of write program
 * 1. draw window.
 * 2. read directory and files from linux and show the window.
 * 3. select the directory and files from user.
 * 4. build a design. (this the las thing)*
 * */


/*
 *         IMGUI_DEMO_MARKER("Widgets/Selectables/Single Selection");
        if (ImGui::TreeNode("Selection State: Single Selection"))
        {
            static int selected = -1;
            for (int n = 0; n < 5; n++)
            {
                char buf[32];
                sprintf(buf, "Object %d", n);
                if (ImGui::Selectable(buf, selected == n))
                    selected = n;
            }
            ImGui::TreePop();
        }
 */

enum TableColumnID
{
    TableColumnID_Name,
    TableColumnID_Type,
    TableColumnID_Size,
    TableColumnID_Time
};

struct TableItem
{
	const char *name;
	const char *type;
	int size;
	std::filesystem::file_time_type time;


	// We have a problem which is affecting _only this demo_ and should not affect your code:
    // As we don't rely on std:: or other third-party library to compile dear imgui, we only have reliable access to qsort(),
    // however qsort doesn't allow passing user data to comparing function.
    // As a workaround, we are storing the sort specs in a static/global for the comparing function to access.
    // In your own use case you would probably pass the sort specs to your sorting/comparing functions directly and not use a global.
    // We could technically call ImGui::TableGetSortSpecs() in CompareWithSortSpecs(), but considering that this function is called
    // very often by the sorting algorithm it would be a little wasteful.
    static const ImGuiTableSortSpecs* s_current_sort_specs;

    static void SortWithSortSpecs(ImGuiTableSortSpecs* sort_specs, TableItem* items, int items_count)
    {
        s_current_sort_specs = sort_specs; // Store in variable accessible by the sort function.
        if (items_count > 1)
            qsort(items, (size_t)items_count, sizeof(items[0]), TableItem::CompareWithSortSpecs);
        s_current_sort_specs = NULL;
    }

    // Compare function to be used by qsort()
    static int IMGUI_CDECL CompareWithSortSpecs(const void* lhs, const void* rhs)
    {
        const TableItem* a = (const TableItem*)lhs;
        const TableItem* b = (const TableItem*)rhs;
        for (int n = 0; n < s_current_sort_specs->SpecsCount; n++)
        {
            // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
            // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
            const ImGuiTableColumnSortSpecs* sort_spec = &s_current_sort_specs->Specs[n];
            int delta = 0;
            switch (sort_spec->ColumnUserID)
            {
				case TableColumnID_Name:             delta = (strcmp(a->name, b->name));                break;
				case TableColumnID_Type:           	 delta = (strcmp(a->type, b->type));     break;
				case TableColumnID_Size:   		 	delta = (a->size - b->size);    break;
				default: IM_ASSERT(0); break;
            }
            if (delta > 0)
                return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;
            if (delta < 0)
                return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? -1 : +1;
        }

        // qsort() is instable so always return a way to differenciate items.
        // Your own compare function may want to avoid fallback on implicit sort specs e.g. a Name compare if it wasn't already part of the sort specs.
        return (a->size - b->size);
    }
};
	const ImGuiTableSortSpecs* TableItem::s_current_sort_specs = NULL;


/*Default Constructor*/
ImGui_FileExplore::ImGui_FileExplore()
{
	this->m_gui_dirContentInfo = fileManager.GetDirectoryInfo();
}

extern void imgui_window_marker(char *section);
void ImGui_FileExplore::ImGui_FileWindowDraw(const char* title, bool* p_open)
{
    ImGui::SetNextWindowSize(ImVec2(800, 500), ImGuiCond_None);

    if (!ImGui::Begin(title, p_open))
    {
        ImGui::End();
        return;
    }

    // As a specific feature guaranteed by the library, after calling Begin() the last Item represent the title bar.
    // So e.g. IsItemHovered() will return true when hovering the title bar.
    // Here we create a context menu only available from the title bar.
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Close window"))
            *p_open = false;
        ImGui::EndPopup();
    }


    ImGui_ReadDirectoryList(this->m_gui_dirContentInfo);
	ImGui_SelectDirectoryPath(this->m_gui_dirContentInfo);

    ImGui::End();
}



const char* ImGui_FileExplore::to_string(const std::filesystem::file_time_type& ftime)
{

    // Convert the file_time_type to a more easily printable representation
    std::chrono::system_clock::time_point timePoint = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());

    // Convert the time_point to a std::time_t value
    std::time_t time = std::chrono::system_clock::to_time_t(timePoint);

    // Format and print the last modified time
    std::strftime(this->time_buffer, sizeof(this->time_buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time));
    //std::cout << "Last modified time: " << this->time_buffer << std::endl;
    return this->time_buffer;

}

//Top Area, Select directory or file used by path
void ImGui_FileExplore::ImGui_SelectDirectoryPath(FileManager::directory_content_info &dirContentInfo)
{
    //IMGUI_DEMO_MARKER("Widgets/Selectables/Single Selection");
    //IMGUI_DEMO_MARKER("Widgets/List Boxes");
	char marker[] = "Widgets/Selectables";
	imgui_window_marker(marker);

//	std::cout<<"ImGui_SelectDirectoryPath"<<std::endl;

    static ImGuiTableFlags flags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable
        | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti
        | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody
        | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY
        | ImGuiTableFlags_SizingStretchProp;
    static ImGuiTableColumnFlags columns_base_flags = ImGuiTableColumnFlags_None;

    enum ContentsType { CT_Text, CT_Button, CT_SmallButton, CT_FillButton, CT_Selectable, CT_SelectableSpanRow };
    static int contents_type = CT_SelectableSpanRow;
    static int items_count = this->m_gui_dirContentInfo.dirList.size();

    static int freeze_cols = 1;
    static int freeze_rows = 1;

    static ImVec2 outer_size_value = ImVec2(0.0f, TEXT_BASE_HEIGHT * 12);
    static float row_min_height = 0.0f; // Auto
    static float inner_width_with_scroll = 0.0f; // Auto-extend
    static bool outer_size_enabled = false;
    static bool show_headers = true;
    static bool show_wrapped_text = false;
    static ImGuiChildFlags child_flags =  ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY;

    static int item_current_idx = 0; // Here we store our selection data as an index.
	int dirPathList = m_gui_dirContentInfo.rootPathList.size();
    const char* combo_preview_value = m_gui_dirContentInfo.dirList[0].currentPath.c_str();  // Pass in the preview value visible before opening the combo (it could be anything)


    ImGui::Spacing();
    ImGui::BeginChild("FileConsole", ImVec2(700, 100), child_flags, ImGuiWindowFlags_None);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20, 7));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(100, 10)); // indenting of object
    ImGui::Indent();
    ImGui::Indent();
    ImGui::SameLine();
    if (ImGui::BeginCombo("DIR", combo_preview_value, ImGuiComboFlags_WidthFitPreview))
    {
        for (int n = 0; n < dirPathList; n++)
        {
            const bool is_selected = (item_current_idx == n);
            if (ImGui::Selectable(m_gui_dirContentInfo.rootPathList[n].c_str(), is_selected))
                item_current_idx = n;

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::PopStyleVar(2);
    //ImGui::PopStyleVar();
    ImGui::EndChild();
    ImGui::Spacing();

    // Update item list if we changed the number of items
    // Table Part
    const ImDrawList* parent_draw_list = ImGui::GetWindowDrawList();
    const int parent_draw_list_draw_cmd_count = parent_draw_list->CmdBuffer.Size;
    ImVec2 table_scroll_cur, table_scroll_max; // For debug display
    const ImDrawList* table_draw_list = NULL;  // "

    // Update direcotry and file  list
    static ImVector<TableItem> table_item;
    static ImVector<int> selection;
    static bool items_need_sort = false;
    if (table_item.Size != items_count)
    {
    	table_item.resize(items_count, TableItem());
        for (int n = 0; n < items_count; n++)
        {
            const int template_n = n % this->m_gui_dirContentInfo.dirList.size();
            TableItem& item = table_item[n];
            item.name = this->m_gui_dirContentInfo.dirList[template_n].name.c_str();
            item.size = this->m_gui_dirContentInfo.dirList[template_n].number_subitem;
            item.type = this->m_gui_dirContentInfo.dirList[template_n].extension.c_str();
            item.time = this->m_gui_dirContentInfo.dirList[template_n].last_modified_time;
        }
    }//end if


    // Submit table
    const float inner_width_to_use = (flags & ImGuiTableFlags_ScrollX) ? inner_width_with_scroll : 0.0f;
    if (ImGui::BeginTable("table_advanced", 4, flags, outer_size_enabled ? outer_size_value : ImVec2(0, 0), inner_width_to_use))
    {
        // Declare columns
        // We use the "user_id" parameter of TableSetupColumn() to specify a user id that will be stored in the sort specifications.
        // This is so our sort function can identify a column given our own identifier. We could also identify them based on their index!

        ImGui::TableSetupColumn("Name",         columns_base_flags | ImGuiTableColumnFlags_WidthFixed, 0.0f, TableColumnID_Name);
        ImGui::TableSetupColumn("Type",       columns_base_flags | ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, TableColumnID_Type);
        ImGui::TableSetupColumn("Size",     columns_base_flags | ImGuiTableColumnFlags_PreferSortDescending, 0.0f, TableColumnID_Size);
        ImGui::TableSetupColumn("Time",       columns_base_flags |  ImGuiTableColumnFlags_NoSort);
        ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);

        // Sort our data if sort specs have been changed!
        ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs();
        if (sort_specs && sort_specs->SpecsDirty)
            items_need_sort = true;
        if (sort_specs && items_need_sort && table_item.Size > 1)
        {
            TableItem::SortWithSortSpecs(sort_specs, table_item.Data, table_item.Size);
            sort_specs->SpecsDirty = false;
        }
        items_need_sort = false;

        // Take note of whether we are currently sorting based on the Quantity field,
        // we will use this to trigger sorting when we know the data of this column has been modified.
        const bool sorts_specs_using_quantity = (ImGui::TableGetColumnFlags(0) & ImGuiTableColumnFlags_IsSorted) != 0;

        // Show headers
        if (show_headers && (columns_base_flags & ImGuiTableColumnFlags_AngledHeader) != 0)
            ImGui::TableAngledHeadersRow();
        if (show_headers)
            ImGui::TableHeadersRow();

        // Show data
        // FIXME-TABLE FIXME-NAV: How we can get decent up/down even though we have the buttons here?
        ImGui::PushButtonRepeat(true);
#if 1
        // Demonstrate using clipper for large vertical lists
        ImGuiListClipper clipper;
        clipper.Begin(table_item.Size);
        while (clipper.Step())
        {
            for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
#else
        // Without clipper
        {
            for (int row_n = 0; row_n < items.Size; row_n++)
#endif
            {
                TableItem* item = &table_item[row_n];
                //if (!filter.PassFilter(item->Name))
                //    continue;

                const bool item_is_selected = selection.contains(item->name[0]);
                ImGui::PushID(item->name);
                ImGui::TableNextRow(ImGuiTableRowFlags_None, row_min_height);

                // For the demo purpose we can select among different type of items submitted in the first column
                ImGui::TableSetColumnIndex(0);
                char label[32];
                sprintf(label, "%s", item->name);
                if (contents_type == CT_Selectable || contents_type == CT_SelectableSpanRow)
                {
                    ImGuiSelectableFlags selectable_flags = (contents_type == CT_SelectableSpanRow) ? ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap : ImGuiSelectableFlags_None;
                    if (ImGui::Selectable(label, item_is_selected, selectable_flags, ImVec2(0, row_min_height)))
                    {
                        if (ImGui::GetIO().KeyCtrl)
                        {
                            if (item_is_selected)
                                selection.find_erase_unsorted(item->name[0]);
                            else
                                selection.push_back(item->name[0]);
                        }
                        else
                        {
                            selection.clear();
                            selection.push_back(item->name[0]);
                        }
                    }
                }

                // Here we demonstrate marking our data set as needing to be sorted again if we modified a quantity,
                // and we are currently sorting on the column showing the Quantity.
                // To avoid triggering a sort while holding the button, we only trigger it when the button has been released.
                // You will probably need a more advanced system in your code if you want to automatically sort when a specific entry changes.

					ImGui::TableSetColumnIndex(1); // description of type
					ImGui::Text("%s", item->type);

				if (ImGui::TableSetColumnIndex(2)) //size
					ImGui::Text("%d", item->size);

                /* not button
                if (ImGui::TableSetColumnIndex(2))
                {
                    if (ImGui::SmallButton("Chop")) { item->size += 1; }
                    if (sorts_specs_using_quantity && ImGui::IsItemDeactivated()) { items_need_sort = true; }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Eat")) { item->size -= 1; }
                    if (sorts_specs_using_quantity && ImGui::IsItemDeactivated()) { items_need_sort = true; }
                }*/
                if (ImGui::TableSetColumnIndex(3))//modified time
                    ImGui::Text(" %s ", to_string(item->time));

                ImGui::PopID();
            }
        }
        ImGui::PopButtonRepeat();

        // Store some info to display debug details below
        table_scroll_cur = ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());
        table_scroll_max = ImVec2(ImGui::GetScrollMaxX(), ImGui::GetScrollMaxY());
        table_draw_list = ImGui::GetWindowDrawList();
        ImGui::EndTable();
    }//end while
}//end function

void ImGui_FileExplore::ImGui_ReadDirectoryList(FileManager::directory_content_info &m_dirContentInfo)
{

	/*1. read current directory used ${PWD}*/
	m_dirContentInfo = fileManager.GetDirectoryInfo();
	//ImGui_FileManageBuildList();

	//std::cout<<"current_direcotry_path : "<<dir_info.current_directory_path<<std::endl;
	/*2. show the list current directory list*/
	//ImGui::TextWrapped((char *)dir_info.current_directory_path.c_str());

}
