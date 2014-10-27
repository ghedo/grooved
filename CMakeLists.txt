# grooved CMakeLists.txt
# Copyright (C) 2014 Alessandro Ghedini <alessandro@ghedini.me>
# This file is released under the 2 clause BSD license, see COPYING

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(grooved C)

SET(VERSION_MAJOR 0)
SET(VERSION_MINOR 2)

SET(GROOVED_VERSION ${VERSION_MAJOR}.${VERSION_MINOR})

SET(INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

INCLUDE(FindPkgConfig)

#
# grooved
#

PKG_CHECK_MODULES(DBUS dbus-1 REQUIRED)
PKG_CHECK_MODULES(GIO gio-2.0 REQUIRED)
PKG_CHECK_MODULES(GIO_UNIX gio-unix-2.0 REQUIRED)
PKG_CHECK_MODULES(GLIB glib-2.0 REQUIRED)
PKG_CHECK_MODULES(GOBJECT gobject-2.0 REQUIRED)
PKG_CHECK_MODULES(MPV mpv REQUIRED)
PKG_CHECK_MODULES(SQLITE sqlite3 REQUIRED)

FIND_PROGRAM(GDBUS_CODEGEN gdbus-codegen REQUIRED)

ADD_CUSTOM_COMMAND(
  OUTPUT            ${CMAKE_CURRENT_BINARY_DIR}/dbus-common.c
  COMMAND           ${GDBUS_CODEGEN} --generate-c-code dbus-common --interface-prefix io.github.ghedo.grooved --c-namespace Grooved ${PROJECT_SOURCE_DIR}/src/dbus-service.xml
  DEPENDS           ${PROJECT_SOURCE_DIR}/src/dbus-service.xml
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT           "Generate D-Bus code"
  VERBATIM
)

ADD_EXECUTABLE(grooved
  deps/inih/ini.c
  src/config.c
  ${CMAKE_CURRENT_BINARY_DIR}/dbus-common.c
  src/dbus-service.c
  src/grooved.c
  src/library.c
  src/player.c
  src/printf.c
)

TARGET_LINK_LIBRARIES(grooved
  ${GIO_LIBRARIES}
  ${GLIB_LIBRARIES}
  ${MPV_LIBRARIES}
  ${SQLITE_LIBRARIES}
  -lpthread
)

SET_TARGET_PROPERTIES(grooved PROPERTIES
  COMPILE_FLAGS "-Wall -g -std=gnu99 -D_GNU_SOURCE"
)

INSTALL(TARGETS grooved DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

INSTALL(FILES
  ${PROJECT_SOURCE_DIR}/docs/grooved.1
  DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1
)

INSTALL(FILES
  ${PROJECT_SOURCE_DIR}/etc/grooved.service
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/user
)

INSTALL(FILES
  ${PROJECT_SOURCE_DIR}/etc/io.github.ghedo.grooved.service
  DESTINATION ${CMAKE_INSTALL_PREFIX}/share/dbus-1/services
)

#
# groovectl
#

PKG_CHECK_MODULES(GLYR libglyr REQUIRED)

ADD_EXECUTABLE(groovectl
  deps/inih/ini.c
  src/config.c
  ${CMAKE_CURRENT_BINARY_DIR}/dbus-common.c
  src/groovectl.c
  src/printf.c
)

TARGET_LINK_LIBRARIES(groovectl
  ${GLYR_LIBRARIES}
  ${GLIB_LIBRARIES}
  ${GIO_LIBRARIES}
  ${GOBJECT_LIBRARIES}
)

SET_TARGET_PROPERTIES(groovectl PROPERTIES
  COMPILE_FLAGS "-Wall -g -std=gnu99 -D_GNU_SOURCE"
)

INSTALL(TARGETS groovectl DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

INSTALL(FILES
  ${PROJECT_SOURCE_DIR}/docs/groovectl.1
  DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1
)

INSTALL(FILES
  ${PROJECT_SOURCE_DIR}/etc/groovectl.bash-completion
  DESTINATION ${CMAKE_INSTALL_PREFIX}/share/bash-completion/completions/
  RENAME groovectl
)

INSTALL(FILES
  ${PROJECT_SOURCE_DIR}/etc/groovectl.zsh-completion
  DESTINATION ${CMAKE_INSTALL_PREFIX}/share/zsh/vendor-completions/
  RENAME _groovectl
)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}/deps
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${DBUS_INCLUDE_DIRS}
  ${GLIB_INCLUDE_DIRS}
  ${GLYR_INCLUDE_DIRS}
  ${GIO_INCLUDE_DIRS}
  ${GIO_UNIX_INCLUDE_DIRS}
  ${GOBJECT_INCLUDE_DIRS}
  ${MPV_INCLUDE_DIRS}
  ${SQLITE_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
)
