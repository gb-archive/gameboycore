########################################################################################################################
### ImGui                                                                                                            ###
########################################################################################################################
find_package(SFML REQUIRED COMPONENTS system)

set(IMGUI_SOURCES
    # Core
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_demo.cpp   # Generally its advised to leave this in for reference
    # Binding
    imgui-sfml/imgui-SFML.cpp
    # Addons
    imgui/addons/imguitabwindow/imguitabwindow.cpp
    imgui_club/imgui_memory_editor/imgui_memory_editor.h
)

add_library(imgui STATIC
    ${IMGUI_SOURCES}
)

target_include_directories(imgui PUBLIC
    imgui/
    imgui-sfml/
    imgui_club/
    ${SFML_INCLUDE_DIR}
)

target_link_libraries(imgui PUBLIC
    ${SFML_LIBRARIES}
)

target_compile_features(imgui PUBLIC cxx_std_14)
target_compile_definitions(imgui PRIVATE IMGUI_USER_CONFIG="imconfig-SFML.h")
