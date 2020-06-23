cmake_minimum_required(VERSION 3.5.0)

project(DecisionEngineCoreLocalExecution_UnitTests LANGUAGES CXX)

set(CppCommon_STATIC_CRT ON CACHE BOOL "" FORCE)
set(CppCommon_NO_ADDRESS_SPACE_LAYOUT_RANDOMIZATION ON CACHE BOOL "" FORCE)

set(CMAKE_MODULE_PATH "$ENV{DEVELOPMENT_ENVIRONMENT_CMAKE_MODULE_PATH}")

if(NOT WIN32)
    string(REPLACE ":" ";" CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}")
endif()

include(BuildHelpers)

enable_testing()

function(Impl)
    get_filename_component(_this_path ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)

    include(${_this_path}/../cmake/DecisionEngineCoreLocalExecution.cmake)

    build_tests(
        FILES
            ${_this_path}/Configuration_UnitTest.cpp
            ${_this_path}/Engine_UnitTest.cpp
            ${_this_path}/FingerprinterFactory_UnitTest.cpp
            ${_this_path}/LocalExecution_UnitTest.cpp

        PRECOMPILED_LIBRARY_HEADERS
            DecisionEngineCoreLocalExecution

        LINK_LIBRARIES
            DecisionEngineCoreLocalExecution
    )
endfunction()

Impl()
