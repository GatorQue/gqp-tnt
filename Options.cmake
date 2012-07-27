# Filename: Options.cmake
# Description: Define the options used for this project.
# Modification Log:
# 2012-02-04 Initial version
#

# Project options
set_option(TNT_ENABLED TRUE BOOL "Build 'TnT' project?")
set_option(TNT_BUILD_DOCS TRUE BOOL "Build 'TnT' documentation?")

# Define the external libraries this project depends on
set(TNT_DEPS TMXPARSER SFML GQE)

