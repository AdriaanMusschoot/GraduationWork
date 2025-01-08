cmake_minimum_required(VERSION 3.6)

project(RaymarchingOptimizations VERSION 1.0 LANGUAGES CXX)
set(TARGET_NAME RaymarchingOptimizations)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
include(FetchContent)

set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ProjectFiles)

set(PROJECT_SOURCE_FILES ${PROJECT_DIR}/main.cpp
    ${PROJECT_DIR}/Timer.h
    ${PROJECT_DIR}/Timer.cpp

    ${PROJECT_DIR}/SDFObjects.h
    ${PROJECT_DIR}/SDFObjects.cpp

    ${PROJECT_DIR}/SdEngine.h
    ${PROJECT_DIR}/SdEngine.cpp

    ${PROJECT_DIR}/Scene.h
    ${PROJECT_DIR}/Scene.cpp

    ${PROJECT_DIR}/Renderer.h
    ${PROJECT_DIR}/Renderer.cpp

    ${PROJECT_DIR}/ColorRGB.h
    ${PROJECT_DIR}/Camera.h

    ${PROJECT_DIR}/GUI.h
    ${PROJECT_DIR}/GUI.cpp
  
    ${PROJECT_DIR}/Misc.h 
   
    ${PROJECT_DIR}/BVHNode.h 
    ${PROJECT_DIR}/BVHNode.cpp

    ${PROJECT_DIR}/Scenes.h
    ${PROJECT_DIR}/Scenes.cpp
)

include(FetchContent)
# add glm
FetchContent_Declare(
  glm
  URL https://github.com/g-truc/glm/releases/download/0.9.9.8/glm-0.9.9.8.zip
  DOWNLOAD_NO_PROGRESS ON
  DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/downloads
)
FetchContent_MakeAvailable(glm)

FetchContent_GetProperties(glm)
if(NOT glm_POPULATED)
  FetchContent_Populate(glm)
endif()

FetchContent_Declare(
    SDL2
    URL https://www.libsdl.org/release/SDL2-devel-2.28.5-VC.zip 
    DOWNLOAD_NO_PROGRESS ON
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/downloads 
)

FetchContent_GetProperties(SDL2)
if(NOT SDL2_POPULATED)
    FetchContent_Populate(SDL2)
    set(SDL2_INCLUDE_DIR ${sdl2_SOURCE_DIR}/include)

    set(SDL2_LIBRARIES "${sdl2_SOURCE_DIR}/lib/x64/SDL2.lib;${sdl2_SOURCE_DIR}/lib/x64/SDL2main.lib")
    set(SDL2_LIBRARY_DLL "${sdl2_SOURCE_DIR}/lib/x64/SDL2.dll")
endif()

Set(IMGUI_SOURCE_FILES
  3rdParty/imgui/imgui.cpp
  3rdParty/imgui/imgui_draw.cpp
  3rdParty/imgui/imgui_widgets.cpp
  3rdParty/imgui/imgui_tables.cpp
  3rdParty/imgui/imgui_impl_sdl2.cpp
  3rdParty/imgui/imgui_impl_sdlrenderer2.cpp
  3rdParty/imgui/imgui_plot.cpp
)


add_executable(${TARGET_NAME} ${PROJECT_SOURCE_FILES} ${IMGUI_SOURCE_FILES})

target_include_directories(${TARGET_NAME} PRIVATE
    ${SDL2_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/imgui
)

target_link_libraries(${TARGET_NAME} PRIVATE
    ${SDL2_LIBRARIES}
)

target_link_libraries(${TARGET_NAME} PUBLIC glm::glm)

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy "${SDL2_LIBRARY_DLL}" ${CMAKE_BINARY_DIR}
)