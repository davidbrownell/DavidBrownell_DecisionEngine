cmake_minimum_required(VERSION 3.5.0)

set(CMAKE_MODULE_PATH "$ENV{DEVELOPMENT_ENVIRONMENT_CMAKE_MODULE_PATH}")

if(NOT WIN32)
    string(REPLACE ":" ";" CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}")
endif()

include(BuildHelpers)

function(Impl)
    include(BoostHelpers)
    include(CommonHelpers)

    get_filename_component(_this_path ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)

    build_library(
        NAME
            DecisionEngineCoreComponents

        FILES
            ${_this_path}/../CalculatedResultSystem.cpp
            ${_this_path}/../CalculatedResultSystem.h
            ${_this_path}/../CalculatedWorkingSystem.cpp
            ${_this_path}/../CalculatedWorkingSystem.h
            ${_this_path}/../Components.h
            ${_this_path}/../Condition.cpp
            ${_this_path}/../Condition.h
            ${_this_path}/../EngineImpl.cpp
            ${_this_path}/../EngineImpl.h
            ${_this_path}/../Fingerprinter.cpp
            ${_this_path}/../Fingerprinter.h
            ${_this_path}/../Index.cpp
            ${_this_path}/../Index.h
            ${_this_path}/../ResultSystem.cpp
            ${_this_path}/../ResultSystem.h
            ${_this_path}/../Score.cpp
            ${_this_path}/../Score.h
            ${_this_path}/../System.cpp
            ${_this_path}/../System.h
            ${_this_path}/../WorkingSystem.cpp
            ${_this_path}/../WorkingSystem.h

        PRECOMPILED_HEADERS
            ${_this_path}/../Components.h

        PUBLIC_INCLUDE_DIRECTORIES
            ${_this_path}/../../../..

        PUBLIC_LINK_LIBRARIES
            BoostHelpers
            CommonHelpers
    )
endfunction()

Impl()
