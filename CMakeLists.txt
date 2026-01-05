cmake_minimum_required(VERSION 3.16)

project(hmon VERSION 0.1 LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

FetchContent_Declare(
    lfreist-hwinfo
    GIT_REPOSITORY https://github.com/lfreist/hwinfo.git
    GIT_TAG 636598733f56e6de4809c9102289f83bed9271ef # Dec 22 2025
)

FetchContent_MakeAvailable(lfreist-hwinfo)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(apphmon
    main.cpp
)

qt_add_qml_module(apphmon
    URI hmon
    QML_FILES
        Main.qml
    SOURCES headers/datamanager.hpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(apphmon PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.apphmon
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(apphmon
    PRIVATE Qt6::Quick
    lfreist-hwinfo::hwinfo
)

target_include_directories(apphmon
    PUBLIC ./headers
)

target_sources(apphmon
    PRIVATE src/datamanager.cpp
    src/procdatasource.cpp
)

include(GNUInstallDirs)
install(TARGETS apphmon
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
