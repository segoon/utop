cmake_minimum_required(VERSION 3.11)  # FetchContent

project(uctl)

include(FetchContent)
include(CheckIPOSupported)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v3.0.0
)
FetchContent_MakeAvailable(ftxui)

find_package(
  Boost
    COMPONENTS
    program_options
    filesystem
)

set(SOURCES  
	src/top.cpp
  )


add_library(
  ${PROJECT_NAME}_lib
  ${SOURCES}
)
target_link_libraries(${PROJECT_NAME}_lib
  PUBLIC
    ftxui::screen
    ftxui::dom
    ftxui::component
    Boost::program_options
    Boost::filesystem
)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME}
  PRIVATE ${PROJECT_NAME}_lib
)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

add_subdirectory(tests)
