cmake_minimum_required(VERSION 3.5.0)

project(DecisionEngineConstrainedResource_UnitTests LANGUAGES CXX)

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

    include(${_this_path}/../cmake/DecisionEngineConstrainedResource.cmake)

    build_tests(
        FILES
            # TODO ${_this_path}/CalculatedResultSystem_UnitTest.cpp
            # TODO ${_this_path}/CalculatedWorkingSystem_UnitTest.cpp
            ${_this_path}/Condition_UnitTest.cpp
            ${_this_path}/ConstrainedResource_UnitTest.cpp
            ${_this_path}/PermutationGenerator_UnitTest.cpp
            ${_this_path}/PermutationGeneratorFactory_UnitTest.cpp
            ${_this_path}/Request_UnitTest.cpp
            # TODO ${_this_path}/Resource_UnitTest.cpp
            # TODO ${_this_path}/ResultSystem_UnitTest.cpp
            # TODO ${_this_path}/StandardPermutationGenerator_UnitTest.cpp
            # TODO ${_this_path}/WorkingSystem_UnitTest.cpp

        PRECOMPILED_LIBRARY_HEADERS
            DecisionEngineConstrainedResource

        LINK_LIBRARIES
            DecisionEngineConstrainedResource
    )
endfunction()

Impl()
