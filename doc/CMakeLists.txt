# Filename: CMakeLists.txt
# Description: This file is responsible for creating Doxygen documentation of
#   your project.  All you need to do is use Doxygen comments in your code and
#   when you build your project, you will have Doxygen documentation created
#   automatically for your project under the following directory:
#   <path-to-MyProject>/doc/<YourProjectName>
# Modification Log:
# 2012-01-14 Initial version
#

# Add some paths specifically for Mac OSX to find doxygen
if(MACOSX)
    # Add some path to search doxygen in more directories.
    set(ADDITIONAL_PATHS
        /Developer/Applications/Doxygen.app/Contents/Resources
        /Developer/Applications/Doxygen.app/Contents/MacOS
        $ENV{HOME}/Applications/Doxygen.app/Contents/Resources
        $ENV{HOME}/Applications/Doxygen.app/Contents/MacOS
        $ENV{HOME}/Applications/Developer/Doxygen.app/Contents/Resources
        $ENV{HOME}/Applications/Developer/Doxygen.app/Contents/MacOS)

    set(CMAKE_PROGRAM_PATH ${CMAKE_PROGRAM_PATH} ${ADDITIONAL_PATHS})
endif()
find_package(Doxygen)

if(DOXYGEN_FOUND)
  # see if we can generate the CHM documentation
  if(WINDOWS)
    # if HHC is found, we can generate the CHM (compressed HTML) output
    find_program(DOXYGEN_HHC_PROGRAM
                 NAMES hhc.exe
                 PATHS "c:/Program Files/HTML Help Workshop"
                 DOC "HTML Help Compiler program")
    if(DOXYGEN_HHC_PROGRAM)
        set(DOXYGEN_GENERATE_HTMLHELP YES)
    else()
        set(DOXYGEN_GENERATE_HTMLHELP NO)
    endif()
  else()
    set(DOXYGEN_HHC_PROGRAM)
    set(DOXYGEN_GENERATE_HTMLHELP NO)
  endif()

  # configure the source Doxyfile by copying it and replacing all @variables@
  configure_file(${PROJECT_SOURCE_DIR}/doc/Doxyfile.in
    ${PROJECT_DOC_DIR}/Doxyfile
    @ONLY)

  # copy the files needed by the documentation
  #configure_file(${PROJECT_SOURCE_DIR}/doc/doxygen.css ${PROJECT_DOC_DIR}/html/doxygen.css COPYONLY)
  #configure_file(${PROJECT_SOURCE_DIR}/doc/logo.jpg    ${PROJECT_DOC_DIR}/html/logo.jpg    COPYONLY)
  #configure_file(${PROJECT_SOURCE_DIR}/doc/logo-bg.jpg ${PROJECT_DOC_DIR}/html/logo-bg.jpg COPYONLY)

  # Create documentation build target
  add_custom_target(doc-${PROJECT_NAME} ALL
    DEPENDS ${TOUCH_DIR}/${PROJECT_NAME}-doc)

  # Create command to build documentation
  add_custom_command(OUTPUT ${TOUCH_DIR}/${PROJECT_NAME}-doc
    COMMENT "Creating documentation for '${PROJECT_NAME}'"
    COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_DOC_DIR}/Doxyfile
    COMMAND ${CMAKE_COMMAND} -E touch ${TOUCH_DIR}/${PROJECT_NAME}-doc
    WORKING_DIRECTORY ${PROJECT_DOC_DIR}
    VERBATIM)
endif(DOXYGEN_FOUND)
