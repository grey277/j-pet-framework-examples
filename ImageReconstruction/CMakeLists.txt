################################################################################
## Data analysis project based on J-PET Framework
## Created by J-PET Framework developers 2016-2018
##
## Description:
##   Image reconstruction for Large Barrel setup
################################################################################
cmake_minimum_required(VERSION 3.1...3.14)

if(${CMAKE_VERSION} VERSION_LESS 3.14)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.14)
endif()

################################################################################
## Project name
set(projectName ImageReconstruction)

################################################################################
## Auxiliary files
set(AUXILIARY_FILES
  userParams.json
  PARAMETERS.md
  README.md
  run.sh
)

set(ROOT_SCRIPTS
  rootlogon.C
  rootalias.C
)

################################################################################
## Binary, header and source files definitions
set(projectBinary ${projectName}.x)
project(${projectName} CXX)

add_subdirectory(Reconstruction)

include_directories(Reconstruction/JPetRecoImageTools)

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)
file(GLOB MAIN_CPP main.cpp)
file(GLOB UNIT_TEST_SOURCES *Test.cpp)
list(REMOVE_ITEM SOURCES ${UNIT_TEST_SOURCES})
file(GLOB SOURCES_WITHOUT_MAIN *.cpp)
list(REMOVE_ITEM SOURCES_WITHOUT_MAIN ${UNIT_TEST_SOURCES})
list(REMOVE_ITEM SOURCES_WITHOUT_MAIN ${MAIN_CPP})

################################################################################
## Add options from j-pet-mlem as compile flags removing PET_ prefix
## j-pet-mlem sets some parameters of the CMake build by using definitions,
## that are limited to project, it is necessary to copy them here
# configuration time options
set(PET_MAX_DETECTORS 512
    CACHE INTEGER
    "maximum number of detectors allowed in the scanner (impacts performance)"  )
set(PET_GRANULARITY_TYPE "WARP"
    CACHE STRING
    "warp, thread (or simple just for testing) granularity"  )
set(PET_USE_STATISTICS OFF
    CACHE BOOL
    "collect statistics and print them after program is finished"  )
set(PET_CACHE_ELLIPSE_PIXELS OFF
    CACHE BOOL
    "shared memory pixel buffer in error ellipse"  )
set(PET_USE_SENSITIVITY ON
    CACHE BOOL
    "use sensitivity when calculating reconstruction"  )
set(PET_USE_KERNEL ON
    CACHE BOOL
    "use kernel (when off it uses constant 1 for kernel value)"  )
set(PET_THROW_ON_ZERO_DENOMINATOR OFF
    CACHE BOOL
    "throw on zero denominator (event outside FOV)"  )
set(PET_MAX_PIXELS_PER_THREAD 12
    CACHE INTEGER
    "chosen arbitrarily, no more than 11 pixels per thread in current config"  )
set(PET_WARP_SIZE 32
    CACHE INTEGER
    "normally it should not be changed to anything else than 32"  )
set(PET_MAX_THREADS_PER_BLOCK 512
    CACHE INTEGER
    "not used in current implementation"  )
set(PET_USE_FAST_TEXT_PARSER ON
    CACHE BOOL
    "use fast text parser for loading events (around ~5x faster than C++)"  )
get_cmake_property(vars VARIABLES)
foreach(var ${vars})
  if(var MATCHES "^PET_" AND NOT var MATCHES "_DIR$")
    string(REGEX REPLACE "^PET_" "" name ${var})
    if(name MATCHES "_TYPE$")
      string(REGEX REPLACE "_TYPE$" "" type_name ${name})
      add_definitions(-DUSE_${${var}}_${type_name}=1)
      add_definitions(-D${name}=${${var}})
    elseif(${var})
      if(${var} STREQUAL ON)
        add_definitions(-D${name}=1)
      else()
        add_definitions(-D${name}=${${var}})
      endif()
    else()
      add_definitions(-D${name}=0)
    endif()
  endif()
endforeach()

add_executable(${projectBinary} ${SOURCES} ${HEADERS})
target_link_libraries(${projectBinary} JPetFramework ${PNG_LIBRARIES} JPetRecoImageTools)

add_custom_target(clean_data_${projectName}
  COMMAND rm -f *.tslot.*.root *.phys.*.root *.sig.root
)
################################################################################
## Copy the auxiliary files
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${AUXILIARY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

## Create variable for list with depends files path
set(copy_depends)

################################################################################
## Copy the auxiliary files
foreach(file_i ${AUXILIARY_FILES})
  list(APPEND copy_depends ${CMAKE_CURRENT_BINARY_DIR}/${file_i})
  if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${file_i})
    set(CP_CMD copy_directory)
  else()
    set(CP_CMD copy)
  endif()
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file_i}
    POST_BUILD COMMAND ${CMAKE_COMMAND}
    ARGS -E ${CP_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/${file_i} ${CMAKE_CURRENT_BINARY_DIR}/${file_i}
  )
endforeach(file_i)

## Copy the ROOT scripts
foreach(file_i ${ROOT_SCRIPTS})
  list(APPEND copy_depends ${CMAKE_CURRENT_BINARY_DIR}/${file_i})
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file_i}
    POST_BUILD COMMAND ${CMAKE_COMMAND}
    ARGS -E ${CP_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/${file_i} ${CMAKE_CURRENT_BINARY_DIR}/${file_i}
  )
endforeach(file_i)

################################################################################
## Unit tests
set(TEST_SRC SinogramCreatorTools.cpp)
set(TESTS_DIR ${CMAKE_CURRENT_BINARY_DIR}/tests)
file(MAKE_DIRECTORY ${TESTS_DIR})
foreach(test_source ${UNIT_TEST_SOURCES})
  get_filename_component(test ${test_source} NAME_WE)
  list(APPEND test_binaries ${test}.x)
  add_executable(${test}.x EXCLUDE_FROM_ALL ${test_source} ${TEST_SRC}
    ${test_dictionaries}
    )
  set_target_properties(${test}.x PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TESTS_DIR} )
  target_link_libraries(${test}.x
    JPetFramework
    ${Boost_LIBRARIES}
    ${FFTW_LIBRARIES}
    )
endforeach()

add_custom_target(tests_ImageReconstruction DEPENDS ${test_binaries})

################################################################################
## Add new target that depends on copied files
add_custom_target(copy_files_${projectName} DEPENDS ${copy_depends})
## Add dependencies between building project and copy files
add_dependencies(${projectBinary} copy_files_${projectName})
