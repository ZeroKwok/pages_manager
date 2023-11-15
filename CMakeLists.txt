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

project(pages_manager VERSION 0.1.2)
option(PAGES_MANAGER_BUILD_EXAMPLE "Compile the example" ON)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(PAGES_MANAGER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(PAGES_MANAGER_BUILD_EXAMPLE)
    add_subdirectory(examples)
endif()