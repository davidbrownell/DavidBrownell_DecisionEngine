cmake_minimum_required(VERSION 3.5.0)

set(CMAKE_MODULE_PATH "$ENV{DEVELOPMENT_ENVIRONMENT_CMAKE_MODULE_PATH}")

if(NOT WIN32)
    string(REPLACE ":" ";" CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}")
endif()

include(BuildHelpers)

function(Impl)
    include(BoostCommon)

    include(BoostHelpers)
    include(CommonHelpers)

    get_filename_component(_this_path ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)

    build_library(
        NAME
            DecisionEngineCoreComponents

        FILES
            ${_this_path}/../Components.h
            ${_this_path}/../Condition.cpp
            ${_this_path}/../Condition.h
            ${_this_path}/../Score.cpp
            ${_this_path}/../Score.h

        PUBLIC_LINK_LIBRARIES
            BoostHelpers
            CommonHelpers
            ${Boost_LIBRARIES}
    )
endfunction()

Impl()
