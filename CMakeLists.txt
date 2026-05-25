cmake_minimum_required(VERSION 3.16)
project(MediaViewer VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core Gui Widgets Multimedia MultimediaWidgets Svg
)

set(SOURCES
    main.cpp
    MainWindow.cpp
    ImageViewer.cpp
    AudioPlayer.cpp
    VideoPlayer.cpp
    PlaylistWidget.cpp
)

set(HEADERS
    MainWindow.h
    ImageViewer.h
    AudioPlayer.h
    VideoPlayer.h
    PlaylistWidget.h
)

if(WIN32)
    add_executable(MediaViewer WIN32 ${SOURCES} ${HEADERS})
else()
    add_executable(MediaViewer ${SOURCES} ${HEADERS})
endif()

target_link_libraries(MediaViewer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    Qt6::Svg
)

if(WIN32)
    include(GNUInstallDirs)
    install(TARGETS MediaViewer
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
    if(TARGET Qt6::windeployqt)
        add_custom_command(TARGET MediaViewer POST_BUILD
            COMMAND Qt6::windeployqt
            ARGS "$<TARGET_FILE:MediaViewer>" --dir "$<TARGET_FILE_DIR:MediaViewer>"
        )
    endif()
endif()
