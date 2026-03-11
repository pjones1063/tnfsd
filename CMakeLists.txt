cmake_minimum_required(VERSION 3.16)
project(TnfsTrayApp VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

file(GLOB GUI_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.h")
file(GLOB TNFS_SOURCES CONFIGURE_DEPENDS "src/tnfsd/*.c" "src/tnfsd/*.h")

add_executable(TnfsTrayApp ${GUI_SOURCES} ${TNFS_SOURCES} res/resources.qrc)

set_target_properties(TnfsTrayApp PROPERTIES WIN32_EXECUTABLE TRUE MACOSX_BUNDLE TRUE)

target_include_directories(TnfsTrayApp PRIVATE src/tnfsd)
target_link_libraries(TnfsTrayApp PRIVATE Qt6::Widgets Qt6::Network)

if(WIN32)
    # ADDED NEED_BSDCOMPAT HERE:
    target_compile_definitions(TnfsTrayApp PRIVATE WIN32 NEED_ERRTABLE NEED_BSDCOMPAT)
elseif(APPLE)
    target_compile_definitions(TnfsTrayApp PRIVATE UNIX NEED_ERRTABLE)
else()
    target_compile_definitions(TnfsTrayApp PRIVATE UNIX NEED_BSDCOMPAT NEED_ERRTABLE)
endif()

install(TARGETS TnfsTrayApp DESTINATION bin)
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "tnfstrayapp")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Paul")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "FujiNet Protocol Server for Qt6")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
include(CPack)