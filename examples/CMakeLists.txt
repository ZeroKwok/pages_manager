# Copyright (c) 2022-2024 Zero <zero.kwok@foxmail.com>
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required(VERSION 3.10)

project(example_for_pages_manager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED Widgets Core)

if (NOT PAGES_MANAGER_INCLUDE_DIR)
    set(PAGES_MANAGER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)
endif()

# 注意:
# pages_manager.hpp 需要生成MOC文件, 因此需要显示生成一下
qt5_wrap_cpp(QtMocFiles ${PAGES_MANAGER_INCLUDE_DIR}/pages_manager.hpp)
add_executable(${PROJECT_NAME} pages_manager.cpp ${QtMocFiles})

target_link_libraries(${PROJECT_NAME} pages_manager Qt5::Widgets Qt5::Core)

if (WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES 
        LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()