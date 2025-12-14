cmake_minimum_required(VERSION 3.16)
project(SalesSystem_)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 设置SQLite3路径
set(SQLITE3_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/third_party/sqlite/include")
set(SQLITE3_LIBRARY "${PROJECT_SOURCE_DIR}/third_party/sqlite/lib/sqlite3.dll")

# 创建SQLite3目标
add_library(SQLite3::SQLite3 UNKNOWN IMPORTED)
set_target_properties(SQLite3::SQLite3 PROPERTIES
        IMPORTED_LOCATION "${SQLITE3_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${SQLITE3_INCLUDE_DIR}"
)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

add_executable(SalesSystem_ WIN32 main.cpp
        sqlite/database.cpp
        sale/salesManager.cpp
        sale/stockManager.cpp
        sale/saleStruct.cpp
        sale/shoppingCart.cpp
        qt/mainwindow.cpp
        qt/simulate.cpp
        qt/simulate.h
        qt/simulate.ui
        qt/manualadddialog.cpp
        qt/manualadddialog.h
        qt/settlementdialog.cpp
        qt/settlementdialog.h
        qt/historydialog.cpp
        qt/historydialog.h
        qt/historydialog.ui
)
target_include_directories(SalesSystem_ PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/qt
        ${CMAKE_CURRENT_SOURCE_DIR}/sale
        ${CMAKE_CURRENT_SOURCE_DIR}/sqlite
)
target_link_libraries(SalesSystem_
        Qt::Core
        Qt::Gui
        Qt::Widgets
        SQLite3::SQLite3
)

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core Gui Widgets)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()
