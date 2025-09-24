# Copyright (c) 2025
# Distributed under the MIT software license.

#[=======================================================================[
FindQuickJS
-----------

Finds the QuickJS headers and libraries.

This is a wrapper around find_package()/pkg_check_modules() commands that:
 - facilitates searching in various build environments
 - prints a standard log message

#]=======================================================================]

# Locate header
find_path(QuickJS_INCLUDE_DIR quickjs.h
  PATHS /usr/include /usr/local/include
  PATH_SUFFIXES quickjs
  NO_DEFAULT_PATH
)

# Locate library (searches populated system library folders for any arch)
find_library(QuickJS_LIBRARY quickjs
  PATHS /usr/lib /usr/local/lib
        /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}
        /usr/local/lib/${CMAKE_LIBRARY_ARCHITECTURE}
        /lib /lib/${CMAKE_LIBRARY_ARCHITECTURE}
  PATH_SUFFIXES quickjs
  NO_DEFAULT_PATH
)

# If found, set variables and provide target
if(QuickJS_INCLUDE_DIR AND QuickJS_LIBRARY)
  set(QuickJS_FOUND TRUE)
  set(QuickJS_INCLUDE_DIRS "${QuickJS_INCLUDE_DIR}")
  set(QuickJS_LIBRARIES "${QuickJS_LIBRARY}")

  if(NOT TARGET quickjs::quickjs)
    add_library(quickjs::quickjs INTERFACE IMPORTED)
    set_target_properties(quickjs::quickjs PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${QuickJS_INCLUDE_DIRS}"
      INTERFACE_LINK_LIBRARIES "${QuickJS_LIBRARIES}"
    )
    target_link_libraries(core_interface INTERFACE quickjs::quickjs)
  endif()
else()
  set(QuickJS_FOUND FALSE)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QuickJS REQUIRED_VARS QuickJS_FOUND)
mark_as_advanced(QuickJS_INCLUDE_DIR QuickJS_LIBRARY)
