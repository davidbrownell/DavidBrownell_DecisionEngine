cmake_minimum_required(VERSION 3.5.0)

project(DecisionEngineCoreComponents_UnitTests LANGUAGES CXX)

set(CMAKE_MODULE_PATH "$ENV{DEVELOPMENT_ENVIRONMENT_CMAKE_MODULE_PATH}")

if(NOT WIN32)
    string(REPLACE ":" ";" CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}")
endif()

include(BuildHelpers)

set(CppCommon_STATIC_CRT ON CACHE BOOL "" FORCE)
set(CppCommon_NO_ADDRESS_SPACE_LAYOUT_RANDOMIZATION ON CACHE BOOL "" FORCE)

function(Impl)
    get_filename_component(_this_path ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)

    include(${_this_path}/../cmake/DecisionEngineCoreComponents.cmake)

    build_tests(
        FILES
            ${_this_path}/Condition_UnitTest.cpp

        LINK_LIBRARIES
            DecisionEngineCoreComponents
    )
endfunction()

Impl()
