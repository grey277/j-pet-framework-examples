set(projectName JPetRecoImageTools)

######################################################################
### this section should not need to be modified for a new project
######################################################################
project(${projectName} CXX) # using only C++

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${projectName}/JPetRecoImageTools.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/${projectName}/JPetSinogramType.cpp)
set(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/${projectName}/JPetFilterCosine.h
            ${CMAKE_CURRENT_SOURCE_DIR}/${projectName}/JPetFilterHamming.h
            ${CMAKE_CURRENT_SOURCE_DIR}/${projectName}/JPetFilterInterface.h
            ${CMAKE_CURRENT_SOURCE_DIR}/${projectName}/JPetFilterNone.h
            ${CMAKE_CURRENT_SOURCE_DIR}/${projectName}/JPetFilterRamLak.h
            ${CMAKE_CURRENT_SOURCE_DIR}/${projectName}/JPetFilterRidgelet.h
            ${CMAKE_CURRENT_SOURCE_DIR}/${projectName}/JPetFilterSheppLogan.h
            ${CMAKE_CURRENT_SOURCE_DIR}/${projectName}/JPetRecoImageTools.h
            ${CMAKE_CURRENT_SOURCE_DIR}/${projectName}/JPetSinogramType.h)
######################################################################
### Configure FFTW(based on: https://github.com/egpbos/findFFTW)
######################################################################
configure_file(downloadFindFFTW.cmake.in findFFTW-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/findFFTW-download )
if(result)
    message(FATAL_ERROR "CMake step for findFFTW failed: ${result}")
    else()
    message("CMake step for findFFTW completed (${result}).")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/findFFTW-download )
if(result)
    message(FATAL_ERROR "Build step for findFFTW failed: ${result}")
endif()

set(findFFTW_DIR ${CMAKE_CURRENT_BINARY_DIR}/findFFTW-src)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${findFFTW_DIR}")


find_package(FFTW REQUIRED DOUBLE_LIB)
include_directories(${FFTW_INCLUDE_DIRS})

# unit tests
set(TESTS_DIR ${CMAKE_CURRENT_BINARY_DIR}/tests)
file(MAKE_DIRECTORY ${TESTS_DIR})
foreach(test_source ${UNIT_TEST_SOURCES})
  get_filename_component(test ${test_source} NAME_WE)
  generate_root_dictionaries(test_dictionaries SOURCES ${test_source})
  list(APPEND test_binaries ${test}.x)
  add_executable(${test}.x EXCLUDE_FROM_ALL ${test_source} ${SOURCES}
    ${test_dictionaries}
    )
  set_target_properties(${test}.x PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TESTS_DIR} )
  target_link_libraries(${test}.x
    JPetFramework
    ${Boost_LIBRARIES}
    ${FFTW_LIBRARIES}
    )
endforeach()

add_custom_command(OUTPUT ${TESTS_DIR}/unitTestData
  COMMAND ln -s ${CMAKE_SOURCE_DIR}/unitTestData ${TESTS_DIR}/unitTestData
)

add_custom_target(tests_fbp DEPENDS ${test_binaries} ${TESTS_DIR}/unitTestData)


set(DICTIONARY_NAME G__ImageReconstruction)
set(HEADERS_WITH_DICTIONARY_REQUIRED
  JPetRecoImageTools/JPetSinogramType.h
  )

set(LINKDEF_NAME ImageReconstruction_LinkDef.h)

get_target_property(DICTIONARY_INCLUDES JPetFramework::JPetFramework INTERFACE_INCLUDE_DIRECTORIES)
list(APPEND DICTIONARY_INCLUDES /home/grey/jpet/j-pet-framework/build/install/include/)
foreach(dict ${DICTIONARY_INCLUDES})
  include_directories(${dict})  # Ugly include directories from JPetFramework for dictionaries
endforeach(dict ${DICTIONARY_INCLUDES})

include_directories(ROOT_BUG) # Fix bug in NewMacros Root file
ROOT_GENERATE_DICTIONARY(${DICTIONARY_NAME} ${HEADERS_WITH_DICTIONARY_REQUIRED}
  LINKDEF ${LINKDEF_NAME})

add_library(${projectName} SHARED ${SOURCES} ${DICTIONARY_NAME}.cxx ${HEADERS})

add_library(${projectName} SHARED ${SOURCES} ${DICTIONARY_NAME}.cxx ${HEADERS})
target_link_libraries(${projectName}
  PRIVATE
    ${FFTW_LIBRARIES})
set_target_properties(${projectName} PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_include_directories(${projectName} PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${projectName}>
                           $<BUILD_INTERFACE:${FFTW_INCLUDE_DIRS}>)
target_link_libraries(${projectName} PRIVATE JPetFramework::JPetFramework ${FFTW_LIBRARIES})
