cmake_minimum_required(VERSION 3.16)

project(hw_overlay VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

find_package(Qt6
    COMPONENTS
        Quick
        Graphs
    REQUIRED
)

qt_standard_project_setup(REQUIRES 6.8)

FetchContent_Declare(
    lfreist-hwinfo
    GIT_REPOSITORY https://github.com/lfreist/hwinfo.git
    GIT_TAG 636598733f56e6de4809c9102289f83bed9271ef # Dec 22 2025
)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(lfreist-hwinfo)
FetchContent_MakeAvailable(googletest)

# Individual classes
qt_add_library(procdata
    STATIC
    procdata.h
    procdata.cpp
)
target_link_libraries(procdata
    Qt6::Quick
    pdh
)

qt_add_library(datamanager
    STATIC
    datamanager.h
    datamanager.cpp
)
target_link_libraries(datamanager
    PUBLIC
        procdata
        lfreist-hwinfo::hwinfo
)

qt_add_executable(apphw_overlay
    main.cpp
)

qt_add_qml_module(apphw_overlay
    URI hw_overlay
    QML_FILES
        qml/Main.qml
        qml/MemoryUsage.qml
        qml/CpuUsage.qml
        qml/GraphHeading.qml
        qml/CommonGraph.qml
    SOURCES
        main.cpp
        datamanager.h
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(apphw_overlay PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.apphw_overlay
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(apphw_overlay
    PRIVATE
        Qt6::Quick
        Qt6::Graphs
        lfreist-hwinfo::hwinfo
        procdata
        datamanager

)
target_link_options(apphw_overlay
    PRIVATE
    "-static" "-Wl,--subsystem,windows"
)

include(GNUInstallDirs)
install(TARGETS apphw_overlay
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Tests
enable_testing()
add_executable(test_errors
    test_errors.cpp
)
target_link_libraries(test_errors
    GTest::gtest_main
    GTest::gmock
    Qt6::Quick
    Qt6::Graphs
    procdata
    datamanager
)
target_compile_options(test_errors
    PUBLIC
    /Zi
)

include(GoogleTest)
gtest_add_tests(TARGET test_errors)
