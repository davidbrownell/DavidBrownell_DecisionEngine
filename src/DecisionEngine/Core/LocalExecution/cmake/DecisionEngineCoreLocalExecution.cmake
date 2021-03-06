cmake_minimum_required(VERSION 3.5.0)

set(CMAKE_MODULE_PATH "$ENV{DEVELOPMENT_ENVIRONMENT_CMAKE_MODULE_PATH}")

if(NOT WIN32)
    string(REPLACE ":" ";" CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}")
endif()

include(BuildHelpers)

function(Impl)
    get_filename_component(_this_path ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)

    include(${_this_path}/../../Components/cmake/DecisionEngineCoreComponents.cmake)

    build_library(
        NAME
            DecisionEngineCoreLocalExecution

        FILES
            ${_this_path}/../Configuration.cpp
            ${_this_path}/../Configuration.h
            ${_this_path}/../Engine.cpp
            ${_this_path}/../Engine.h
            ${_this_path}/../FingerprinterFactory.cpp
            ${_this_path}/../FingerprinterFactory.h
            ${_this_path}/../LocalExecution.h

        PRECOMPILED_HEADERS
            ${_this_path}/../LocalExecution.h

        PUBLIC_INCLUDE_DIRECTORIES
            ${_this_path}/../../../..

        PUBLIC_LINK_LIBRARIES
            DecisionEngineCoreComponents
    )
endfunction()

Impl()
