cmake_minimum_required(VERSION 3.19)
project(TODO_lists_QT LANGUAGES CXX)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

qt_standard_project_setup()

qt_add_executable(TODO_lists_QT
    WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/mainwindow.ui
    src/task.h
    src/task.cpp
    src/taskmanager.h
    src/taskmanager.cpp
    src/mainwindow.h
    src/mainwindow.cpp
)

target_link_libraries(TODO_lists_QT
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt6::Network
)

install(TARGETS TODO_lists_QT
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET TODO_lists_QT
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
