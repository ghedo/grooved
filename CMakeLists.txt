# grooved CMakeLists.txt
# Copyright (C) 2014 Alessandro Ghedini <alessandro@ghedini.me>
# This file is released under the 2 clause BSD license, see COPYING

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(grooved C)

SET(VERSION_MAJOR 0)
SET(VERSION_MINOR 1)

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

ADD_EXECUTABLE(grooved
  deps/inih/ini.c
  src/config.c
  src/dbus.c
  src/dbus-common.c
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

#
# groovectl
#

PKG_CHECK_MODULES(GLYR libglyr REQUIRED)

ADD_EXECUTABLE(groovectl
  deps/inih/ini.c
  src/config.c
  src/dbus-common.c
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

#
# grooved-mmkeys
#

PKG_CHECK_MODULES(X11 x11)

IF (X11_FOUND)
  ADD_EXECUTABLE(grooved-mmkeys
    deps/inih/ini.c
    src/config.c
    src/dbus-common.c
    src/grooved-mmkeys.c
    src/printf.c
  )

  TARGET_LINK_LIBRARIES(grooved-mmkeys
    ${GLIB_LIBRARIES}
    ${GIO_LIBRARIES}
    ${GOBJECT_LIBRARIES}
    ${X11_LIBRARIES}
  )

  SET_TARGET_PROPERTIES(grooved-mmkeys PROPERTIES
    COMPILE_FLAGS "-Wall -g -std=gnu99 -D_GNU_SOURCE"
  )

  INSTALL(TARGETS grooved-mmkeys DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
ENDIF (X11_FOUND)

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
  ${X11_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
)
