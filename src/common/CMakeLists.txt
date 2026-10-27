#
# This file is part of the YggdrasilCore Project. See AUTHORS file for Copyright information
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#

CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES
  # Exclude
  ${CMAKE_CURRENT_SOURCE_DIR}/Debugging
  ${CMAKE_CURRENT_SOURCE_DIR}/Platform
  ${CMAKE_CURRENT_SOURCE_DIR}/Collision
  ${CMAKE_CURRENT_SOURCE_DIR}/Navigation
  ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders)

if (BUILD_APPLICATION_WORLDSERVER OR BUILD_TOOLS_MAPS)
  unset(PRIVATE_SOURCES)
  CollectSourceFiles(
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE_SOURCES
    # Exclude
    ${CMAKE_CURRENT_SOURCE_DIR}/Debugging
    ${CMAKE_CURRENT_SOURCE_DIR}/Platform
    ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders)
endif()

# Manually set sources for Debugging directory as we don't want to include WheatyExceptionReport in common project
# It needs to be included both in authserver and worldserver for the static global variable to be properly initialized
# and to handle crash logs on windows
list(APPEND PRIVATE_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/Debugging/CrashHandler.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Debugging/CrashHandler.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Debugging/Errors.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Debugging/Errors.h)

if (USE_COREPCH)
  set(PRIVATE_PCH_HEADER PrecompiledHeaders/commonPCH.h)
endif()

# Group sources
GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

add_library(common
  ${PRIVATE_SOURCES})

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES
  # Exclude
  ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders)

target_include_directories(common
  PUBLIC
    # Provide the binary dir for all child targets
    ${CMAKE_BINARY_DIR}
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(common
  PRIVATE
    ycore-core-interface
  PUBLIC
    boost
    argon2
    sfmt
    utf8cpp
    openssl
    threads
    jemalloc
    stdfs
    fmt)

# Symbolized crash backtraces. libdw is optional: without it the crash handler still prints
# every frame as module+offset, which can be resolved offline with addr2line.
if (UNIX)
  find_path(LIBDW_INCLUDE_DIR NAMES libdwfl.h PATH_SUFFIXES elfutils)
  find_library(LIBDW_LIBRARY NAMES dw)

  if (LIBDW_INCLUDE_DIR AND LIBDW_LIBRARY)
    target_compile_definitions(common PRIVATE YC_HAVE_LIBDW=1)
    target_include_directories(common PRIVATE ${LIBDW_INCLUDE_DIR})
    target_link_libraries(common PUBLIC ${LIBDW_LIBRARY})
    message(STATUS "Found libdw: crash reports will contain symbolized backtraces")
  else()
    message(STATUS "libdw not found: crash reports will only contain module offsets")
  endif()

  if (CMAKE_DL_LIBS)
    target_link_libraries(common PUBLIC ${CMAKE_DL_LIBS})
  endif()
endif()

if (BUILD_APPLICATION_WORLDSERVER OR BUILD_TOOLS_MAPS)
  target_link_libraries(common
    PUBLIC
      g3dlib
      Detour)
endif()

set_target_properties(common
  PROPERTIES
    FOLDER
      "server")

# Generate precompiled header
if (USE_COREPCH)
  add_cxx_pch(common ${PRIVATE_PCH_HEADER})
endif ()
