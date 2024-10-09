cmake_minimum_required(VERSION 3.5)

if(ESP_PLATFORM)
    idf_component_register(
        SRCS "LList.h"
        INCLUDE_DIRS "."
    )
endif()

project(LinkedList VERSION 1.4.0)