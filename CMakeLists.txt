cmake_minimum_required(VERSION 3.25)

# sudo apt install libglew-dev

project(roboflex_profiler VERSION 0.1.1 DESCRIPTION "roboflex profiler")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# -------------------- 
# Resolve dependencies

find_package(SDL2 REQUIRED)
#find_package(roboflex_transport_mqtt REQUIRED)
find_package(GLEW 2.0 REQUIRED)

# Include FetchContent Module
include(FetchContent)


# download and build roboflex_core
FetchContent_Declare(roboflex_core
    GIT_REPOSITORY https://github.com/flexrobotics/roboflex.git
    GIT_TAG        main
)
set(BUILD_ROBOFLEX_PYTHON_EXT OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(roboflex_core)
# find_package(roboflex_core REQUIRED)

# download and build roboflex_core
FetchContent_Declare(roboflex_transport_mqtt
    GIT_REPOSITORY https://github.com/flexrobotics/roboflex_transport_mqtt.git
    GIT_TAG        main
)
#set(BUILD_ROBOFLEX_PYTHON_EXT OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(roboflex_transport_mqtt)
# find_package(roboflex_core REQUIRED)



# Fetch and make imgui available, and build static library ourselves
FetchContent_Declare(
  imgui
  URL https://github.com/ocornut/imgui/archive/refs/tags/v1.89.5.tar.gz
)
FetchContent_MakeAvailable(imgui)
set(IMGUI_SRC_DIR "${imgui_SOURCE_DIR}")
add_library(imgui STATIC
  ${IMGUI_SRC_DIR}/imgui.cpp
  ${IMGUI_SRC_DIR}/imgui_draw.cpp
  ${IMGUI_SRC_DIR}/imgui_tables.cpp
  ${IMGUI_SRC_DIR}/imgui_widgets.cpp
  ${IMGUI_SRC_DIR}/backends/imgui_impl_sdl2.cpp
  ${IMGUI_SRC_DIR}/backends/imgui_impl_opengl3.cpp
)
set_property(TARGET imgui PROPERTY 
    POSITION_INDEPENDENT_CODE ON
)
target_include_directories(imgui PUBLIC 
    ${IMGUI_SRC_DIR}
    ${SDL2_INCLUDE_DIRS}
)

# get and build implot the same way
FetchContent_Declare(
  implot
  URL https://github.com/epezent/implot/archive/refs/tags/v0.14.tar.gz
)
FetchContent_MakeAvailable(implot)
set(IMPLOT_SRC_DIR "${implot_SOURCE_DIR}")
add_library(implot STATIC
  ${IMPLOT_SRC_DIR}/implot.cpp
  ${IMPLOT_SRC_DIR}/implot_items.cpp
)
set_property(TARGET implot PROPERTY 
    POSITION_INDEPENDENT_CODE ON
)
target_include_directories(implot PUBLIC 
    ${IMPLOT_SRC_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${IMGUI_SRC_DIR}
)
target_compile_definitions(implot PRIVATE
    IMGUI_DEFINE_MATH_OPERATORS
)


# -------------------- 
# Define the main profiler library

add_library(roboflex_profiler STATIC
    src/profiler.cpp
    include/roboflex_profiler/profiler.h
)

# Set some properties on our library
set_property(TARGET roboflex_profiler PROPERTY 
    POSITION_INDEPENDENT_CODE ON
)

# Include directories when we compile our library
target_include_directories(roboflex_profiler PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
    $<INSTALL_INTERFACE:include>
)

# Link against the necessary libraries
target_link_libraries(roboflex_profiler PUBLIC 
    roboflex_core 
    roboflex_transport_mqtt
)


# -------------------- 
# Examples

add_executable(profile_graph examples/profile_graph.cpp)
target_link_libraries(profile_graph PRIVATE 
    roboflex_core 
    roboflex_transport_mqtt 
    roboflex_profiler
)


# -------------------- 
# install

# If you need to install the profiler library
install(TARGETS roboflex_profiler 
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
#install(FILES profiler.h DESTINATION include/roboflex_profiler)
install(DIRECTORY include/roboflex_profiler
    DESTINATION include
)


# --------------------
# Create the metrics_central executable
add_executable(metrics_central src/metrics_central.cpp src/metrics_central_impl.cpp)
target_link_libraries(metrics_central PRIVATE 
    roboflex_core 
    roboflex_profiler
    imgui
    implot
    GLEW
    GL
    SDL2
)


# --------------------
# build python bindings

add_subdirectory(python)
