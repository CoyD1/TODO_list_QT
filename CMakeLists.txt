cmake_minimum_required(VERSION 3.19)
project(TODO_lists_QT LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

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
    src/serializer.h
    src/serializer.cpp
    src/message.h
    src/message.cpp
    src/taskserver.h
    src/taskserver.cpp
    src/networkclient.h
    src/networkclient.cpp
)

target_link_libraries(TODO_lists_QT
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
)

install(TARGETS TODO_lists_QT
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_generate_deploy_app_script(
        TARGET TODO_lists_QT
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script})
endif()
