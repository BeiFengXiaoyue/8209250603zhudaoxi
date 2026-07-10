cmake_minimum_required(VERSION 3.16)
project(MyQtApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 生成 compile_commands.json 供 Clangd 使用
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

function(find_qt_dir PREFIX RESULT_VAR)
    file(GLOB QT_DIRS LIST_DIRECTORIES true "${PREFIX}/[0-9]*.[0-9]*.[0-9]*")
    set(BEST_DIR "")
    set(BEST_VER "")
    foreach(DIR ${QT_DIRS})
        get_filename_component(VER "${DIR}" NAME)
        if(VER MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$")
            if(BEST_DIR STREQUAL "" OR VER VERSION_GREATER BEST_VER)
                set(BEST_DIR "${DIR}")
                set(BEST_VER "${VER}")
            endif()
        endif()
    endforeach()
    set(${RESULT_VAR} "${BEST_DIR}" PARENT_SCOPE)
endfunction()

find_qt_dir("C:/Qt" QT_BASE_DIR)
if(NOT QT_BASE_DIR)
    find_qt_dir("D:/Qt" QT_BASE_DIR)
endif()

if(QT_BASE_DIR)
    set(CMAKE_PREFIX_PATH "${QT_BASE_DIR}/mingw_64")
    get_filename_component(QT_VER "${QT_BASE_DIR}" NAME)
    message(STATUS "Qt auto-detected: ${QT_BASE_DIR} (version ${QT_VER})")
else()
    message(FATAL_ERROR "Qt not found under C:/Qt or D:/Qt.")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Network)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# 自动收集项目根目录下所有 .cpp 和 .h 文件
file(GLOB_RECURSE PROJECT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)
# 排除 build 目录中的文件
list(FILTER PROJECT_SOURCES EXCLUDE REGEX "${CMAKE_BINARY_DIR}/.*")

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} resources.qrc)
set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)

# 如果存在 QSS 文件则复制到构建输出目录
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/common/style/common.qss")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/common/style/common.qss
        DESTINATION ${CMAKE_BINARY_DIR})
    message(STATUS "common/style/common.qss copied to build directory")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)

find_program(WINDEPLOYQT windeployqt
    HINTS "${Qt${QT_VERSION_MAJOR}_DIR}/../../../bin"
    NO_DEFAULT_PATH
)
if(NOT WINDEPLOYQT)
    message(FATAL_ERROR "windeployqt not found")
endif()

add_custom_target(run
    COMMAND ${CMAKE_COMMAND} -E echo "=== windeployqt ==="
    COMMAND ${WINDEPLOYQT} "./bin/${PROJECT_NAME}.exe"
    COMMAND ${CMAKE_COMMAND} -E echo "=== Launching ${PROJECT_NAME} ==="
    COMMAND "./bin/${PROJECT_NAME}.exe"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    DEPENDS ${PROJECT_NAME}
    COMMENT "Deploy and run ${PROJECT_NAME}"
)
