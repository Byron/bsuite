cmake_minimum_required(VERSION 2.8.3)
cmake_policy(SET CMP0011 OLD)

include(configure.cmake)

# ADD ACTUAL PROJECTS
######################
include(fun.cmake)

add_subdirectory(src)


