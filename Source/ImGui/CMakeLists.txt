
add_library(ImGui OBJECT imgui.cpp
                        ../Headers/imgui.h
                        ../Headers/TxGuiApp.hpp
                        ../Headers/TxApp.hpp
                        ../Headers/imconfig.h
                        ../Headers/imgui_stdlib.h
                        ../Headers/imstb_truetype.h
                        ../Headers/imgui_internal.h
                        ../Headers/imstb_rectpack.h
                        ../Headers/imstb_textedit.h
                        ../Headers/imgui_impl_glfw.h
                        ../Headers/imgui_impl_opengl3.h                        
                        TxGuiApp.cpp
                        TxApp.cpp
						imgui_draw.cpp
						imgui_tables.cpp 
						imgui_widgets.cpp						
						imgui_stdlib.cpp
						ImGui_Imple/imgui_impl_glfw.cpp
						ImGui_Imple/imgui_impl_opengl3.cpp	
						)


