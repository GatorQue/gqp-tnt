# Filename: CMakeLists.txt
# Description: CMake project file for the Example project. This is an
#   example project file from which you can see how things work all work
#   together to provide you the ability to create video games using both
#   SFML, GQE, and/or Thor together. I have tried to make it as painless as
#   possible to add your own projects. Just edit a few items in this file to
#   get your own project started.
# Modification Log:
# 2012-02-06 Initial version
# 2012-02-11 Fix typo and added source files checking
# 2012-04-03 Fix missing libraries when using modules without components
# 2012-04-07 Fix case sensitivity issues
#

# Set our project name, which is the name of the current directory
project(${SUBPROJECT_CMAKE_DIR})

# Establish our version numbers for this project
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_NAME_VERSION_MINOR 0)
set(PROJECT_NAME_VERSION_PATCH 0)
set(PROJECT_NAME_VERSION
  "${PROJECT_NAME_VERSION_MAJOR}.${PROJECT_NAME_VERSION_MINOR}.${PROJECT_NAME_VERSION_PATCH}")

# Include the top level macros to be used by all projects
include(${TOPLEVEL_DIR}/cmake/Macros.cmake)

# Include the top level functions to be used by all projects
include(${TOPLEVEL_DIR}/cmake/Functions.cmake)

# Include the top level options to be used by all projects
include(${TOPLEVEL_DIR}/cmake/Options.cmake)

# Include our options file
include(${PROJECT_SOURCE_DIR}/Options.cmake)

# Loop through this projects external dependencies and Find each of them
set(${SUBPROJECT_NAME}_LIBS)
foreach(EXTERNAL_DEP ${${SUBPROJECT_NAME}_DEPS})
  # Get the uppercase version of the subdirectory name
  string(TOUPPER ${EXTERNAL_DEP} EXTERNAL_NAME)

  # Try to find this external dependency package
  find_package(${EXTERNAL_DEP} REQUIRED COMPONENTS ${${EXTERNAL_NAME}_COMPONENTS})

  # Add the include directories to our list of include directories
  include_directories(${${EXTERNAL_NAME}_INCLUDE_DIR})

  # Add the definitions to our list of definitions
  if(${EXTERNAL_NAME}_DEFS)
    add_definitions(${${EXTERNAL_NAME}_DEFS})
  endif(${EXTERNAL_NAME}_DEFS)

  # Add the libraries listed for this dependency
  set(${SUBPROJECT_NAME}_LIBS ${${SUBPROJECT_NAME}_LIBS} ${${EXTERNAL_NAME}_LIBRARIES})

  # Look for dependencies to this external dependency
  foreach(SUBEXTERNAL_DEP ${${EXTERNAL_NAME}_DEPS})
    # Get the uppercase version of the subdirectory name
    string(TOUPPER ${SUBEXTERNAL_DEP} SUBEXTERNAL_NAME)

    # Try to find this external dependency package
    find_package(${SUBEXTERNAL_DEP} REQUIRED COMPONENTS ${${SUBEXTERNAL_NAME}_COMPONENTS})

    # Add the include directories to our list of include directories
    include_directories(${${SUBEXTERNAL_NAME}_INCLUDE_DIR})

    # Add the definitions to our list of definitions
    if(${SUBEXTERNAL_NAME}_DEFS)
      add_definitions(${${SUBEXTERNAL_NAME}_DEFS})
    endif(${SUBEXTERNAL_NAME}_DEFS)

    # Add the libraries listed for this dependency
    set(${SUBPROJECT_NAME}_LIBS ${${SUBPROJECT_NAME}_LIBS} ${${SUBEXTERNAL_NAME}_LIBRARIES})

    # Loop through each component and get the libraries
    foreach(COMPONENT_NAME ${${SUBEXTERNAL_NAME}_COMPONENTS})
      # Build a list of libraries to link against for this project
      set(${SUBPROJECT_NAME}_LIBS ${${SUBPROJECT_NAME}_LIBS} ${${SUBEXTERNAL_NAME}_${COMPONENT_NAME}_LIBRARY})
    endforeach(COMPONENT_NAME ${${SUBEXTERNAL_NAME}_COMPONENTS})
  endforeach()

  # Loop through each component and get the libraries
  foreach(COMPONENT_NAME ${${EXTERNAL_NAME}_COMPONENTS})
    # Add our original depency libraries to the list
    set(${SUBPROJECT_NAME}_LIBS ${${SUBPROJECT_NAME}_LIBS} ${${EXTERNAL_NAME}_${COMPONENT_NAME}_LIBRARY})
  endforeach(COMPONENT_NAME ${${EXTERNAL_NAME}_COMPONENTS})
endforeach()

# Now add the project source files and targets
file(GLOB_RECURSE ${SUBPROJECT_NAME}_SOURCES
  ${PROJECT_SOURCE_DIR}/src/*.c++
  ${PROJECT_SOURCE_DIR}/src/*.cpp
  ${PROJECT_SOURCE_DIR}/src/*.cxx
  ${PROJECT_SOURCE_DIR}/src/*.cc
  ${PROJECT_SOURCE_DIR}/src/*.c
  ${PROJECT_SOURCE_DIR}/src/*.hpp
  ${PROJECT_SOURCE_DIR}/src/*.hxx
  ${PROJECT_SOURCE_DIR}/src/*.hh
  ${PROJECT_SOURCE_DIR}/src/*.h)

# Add executable if source files were found
if(${SUBPROJECT_NAME}_SOURCES)

  # Add a executable target to build the source files
  if(WINDOWS)
    add_executable(${PROJECT_NAME} WIN32 ${${SUBPROJECT_NAME}_SOURCES})
  else(WINDOWS)
    add_executable(${PROJECT_NAME} ${${SUBPROJECT_NAME}_SOURCES})
  endif(WINDOWS)

  # Set the debug suffix
  set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX -d)

  # For GCC 4.x on Windows, apply the BUILD_STATIC_STD_LIBS option if it is enabled
  if(WINDOWS AND COMPILER_GCC AND BUILD_STATIC_STD_LIBS)
    if(${GCC_VERSION} MATCHES "4\\..*")
      set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-static-libgcc -static-libstdc++")
    endif()
  endif()

  # Add the 3rdparty/external libraries to our target's linker list
  if(${SUBPROJECT_NAME}_LIBS)
    target_link_libraries(${PROJECT_NAME} ${${SUBPROJECT_NAME}_LIBS})
  endif(${SUBPROJECT_NAME}_LIBS)

  # If on windows, we want to copy the binaries to our build directory
  if(WINDOWS)
    set(RELEASE_BINARIES)
    set(DEBUG_BINARIES)
    # Loop through each dependency looking for DLL files to copy
    foreach(EXTERNAL_NAME ${${SUBPROJECT_NAME}_DEPS})

      # Loop through each binary directory and add a copy directory command
      foreach(BINARY_DIR ${${EXTERNAL_NAME}_BIN_DIR})
        # Add each binary directory to copy as a custom command
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${BINARY_DIR}
          ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
          VERBATIM)
      endforeach(BINARY_DIR ${${EXTERNAL_NAME}_BIN_DIR})

      # Loop through each library provided by this module
      foreach(LIBRARY_NAME ${${EXTERNAL_NAME}_LIBS})
        # Get the release and debug names of each library
        get_target_property(RELEASE_FILENAME ${LIBRARY_NAME} LOCATION)
        get_target_property(DEBUG_FILENAME ${LIBRARY_NAME} DEBUG_LOCATION)

        # Get the release and debug filename extension for each library
        get_filename_component(RELEASE_EXT ${RELEASE_FILENAME} EXT)
        get_filename_component(DEBUG_EXT ${DEBUG_FILENAME} EXT)

        # Add libraries to project binaries list
        if(RELEASE_EXT MATCHES ".dll")
          set(RELEASE_BINARIES ${RELEASE_BINARIES} ${RELEASE_FILENAME})
        endif(RELEASE_EXT MATCHES ".dll")
        if(DEBUG_EXT MATCHES ".dll")
          set(DEBUG_BINARIES ${DEBUG_BINARIES} ${DEBUG_FILENAME})
        endif(DEBUG_EXT MATCHES ".dll")
      endforeach()

      # Get the subexternal dependencies too
      foreach(SUBEXTERNAL_NAME ${${EXTERNAL_NAME}_DEPS})

        # Loop through each binary directory and add a copy directory command
        foreach(BINARY_DIR ${${SUBEXTERNAL_NAME}_BIN_DIR})
          # Add each binary directory to copy as a custom command
          add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${BINARY_DIR}
            ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
            VERBATIM)
        endforeach(BINARY_DIR ${${SUBEXTERNAL_NAME}_BIN_DIR})

        # Loop through each library provided by this module
        foreach(LIBRARY_NAME ${${SUBEXTERNAL_NAME}_LIBS})
          # Get the release and debug names of each library
          get_target_property(RELEASE_FILENAME ${LIBRARY_NAME} LOCATION)
          get_target_property(DEBUG_FILENAME ${LIBRARY_NAME} DEBUG_LOCATION)

          # Get the release and debug filename extension for each library
          get_filename_component(RELEASE_EXT ${RELEASE_FILENAME} EXT)
          get_filename_component(DEBUG_EXT ${DEBUG_FILENAME} EXT)

          # Add these libraries to the list of binaries to copy
          if(RELEASE_EXT MATCHES ".dll")
            set(RELEASE_BINARIES ${RELEASE_BINARIES} ${RELEASE_FILENAME})
          endif(RELEASE_EXT MATCHES ".dll")
          if(DEBUG_EXT MATCHES ".dll")
            set(DEBUG_BINARIES ${DEBUG_BINARIES} ${DEBUG_FILENAME})
          endif(DEBUG_EXT MATCHES ".dll")
        endforeach()
      endforeach(SUBEXTERNAL_NAME ${${EXTERNAL_NAME}_DEPS})
    endforeach(EXTERNAL_NAME ${${PROJECT_NAME}_DEPS})

    # Create a script to copy the above files to the projects binary directory post build
    ScriptCopyBinaries(${PROJECT_BINARY_DIR}/copy.cmake .
      ${RELEASE_BINARIES}
      ${DEBUG_BINARIES})

    # Create a custom command to call the copy.cmake script created above
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -DCONFIG:STRING=$<CONFIGURATION> -P ${PROJECT_BINARY_DIR}/copy.cmake
      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
      VERBATIM)
  endif(WINDOWS)

  # Copy resources folder to build directory after build
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${PROJECT_SOURCE_DIR}/resources
    ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/resources
    VERBATIM)
endif(${SUBPROJECT_NAME}_SOURCES)

# Add docmentation folder if it exists
if(EXISTS ${PROJECT_SOURCE_DIR}/doc AND DOXYGEN_FOUND AND ${SUBPROJECT_NAME}_BUILD_DOCS)
  set(PROJECT_DOC_DIR ${TOPLEVEL_DIR}/doc/${PROJECT_NAME})
  add_subdirectory(${PROJECT_SOURCE_DIR}/doc)
endif(EXISTS ${PROJECT_SOURCE_DIR}/doc AND DOXYGEN_FOUND AND ${SUBPROJECT_NAME}_BUILD_DOCS)

