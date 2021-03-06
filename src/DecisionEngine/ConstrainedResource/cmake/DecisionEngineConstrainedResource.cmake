cmake_minimum_required(VERSION 3.5.0)

set(CMAKE_MODULE_PATH "$ENV{DEVELOPMENT_ENVIRONMENT_CMAKE_MODULE_PATH}")

if(NOT WIN32)
    string(REPLACE ":" ";" CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}")
endif()

include(BuildHelpers)

function(Impl)
    get_filename_component(_this_path ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)

    include(${_this_path}/../../Core/Components/cmake/DecisionEngineCoreComponents.cmake)

    build_library(
        NAME
            DecisionEngineConstrainedResource

        FILES
            ${_this_path}/../CalculatedResultSystem.cpp
            ${_this_path}/../CalculatedResultSystem.h
            ${_this_path}/../CalculatedWorkingSystem.cpp
            ${_this_path}/../CalculatedWorkingSystem.h
            ${_this_path}/../Condition.cpp
            ${_this_path}/../Condition.h
            ${_this_path}/../ConstrainedResource.h
            ${_this_path}/../PermutationGenerator.cpp
            ${_this_path}/../PermutationGenerator.h
            ${_this_path}/../PermutationGeneratorFactory.cpp
            ${_this_path}/../PermutationGeneratorFactory.h
            ${_this_path}/../Request.cpp
            ${_this_path}/../Request.h
            ${_this_path}/../Resource.cpp
            ${_this_path}/../Resource.h
            ${_this_path}/../ResultSystem.cpp
            ${_this_path}/../ResultSystem.h
            ${_this_path}/../StandardPermutationGenerator.cpp
            ${_this_path}/../StandardPermutationGenerator.h
            ${_this_path}/../WorkingSystem.cpp
            ${_this_path}/../WorkingSystem.h

        PRECOMPILED_HEADERS
            ${_this_path}/../ConstrainedResource.h

        PUBLIC_INCLUDE_DIRECTORIES
            ${_this_path}/../../..

        PUBLIC_LINK_LIBRARIES
            DecisionEngineCoreComponents
    )
endfunction()

Impl()
