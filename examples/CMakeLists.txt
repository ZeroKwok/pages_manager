# This file is part of the PagesManager distribution.
# Copyright (c) 2018-2023 zero.kwok@foxmail.com
# 
# This is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 3 of
# the License, or (at your option) any later version.
# 
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this software; 
# If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.10)

project(example_for_pages_manager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED Widgets Core)

# 注意:
# pages_manager.hpp 需要生成MOC文件, 因此需要显示生成一下
qt5_wrap_cpp(QtMocFiles ${PAGES_MANAGER_INCLUDE_DIR}/pages_manager.hpp)
add_executable(${PROJECT_NAME} pages_manager.cpp ${QtMocFiles})

target_link_libraries(${PROJECT_NAME} pages_manager Qt5::Widgets Qt5::Core)

if (WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES 
        LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()