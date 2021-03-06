# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

project(OrbitQt CXX)

add_executable(OrbitQt)

target_compile_options(OrbitQt PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(
  OrbitQt
  PRIVATE CallTreeViewItemModel.h
          CallTreeWidget.h
          CopyKeySequenceEnabledTreeView.h
          CutoutWidget.h
          deploymentconfigurations.h
          ElidedLabel.h
          Error.h
          eventloop.h
          orbitaboutdialog.h
          orbitcodeeditor.h
          orbitdataviewpanel.h
          orbitdisassemblydialog.h
          orbiteventiterator.h
          orbitglwidget.h
          orbitglwidgetwithheader.h
          orbitlivefunctions.h
          orbitmainwindow.h
          orbitsamplingreport.h
          OrbitStartupWindow.h
          orbittablemodel.h
          orbittreeview.h
          opengldetect.h
          processlauncherwidget.h
          resource.h
          servicedeploymanager.h
          TutorialContent.h
          TutorialOverlay.h)

target_sources(
  OrbitQt
  PRIVATE CallTreeViewItemModel.cpp
          CallTreeWidget.cpp
          CallTreeWidget.ui
          Error.cpp
          ElidedLabel.cpp
          MainThreadExecutorImpl.cpp
          StatusListenerImpl.cpp
          deploymentconfigurations.cpp
          main.cpp
          orbitaboutdialog.cpp
          orbitaboutdialog.ui
          orbitcodeeditor.cpp
          orbitdataviewpanel.cpp
          orbitdisassemblydialog.cpp
          orbiteventiterator.cpp
          orbiteventiterator.ui
          orbitglwidget.cpp
          orbitglwidgetwithheader.cpp
          orbitlivefunctions.cpp
          orbitmainwindow.cpp
          orbitsamplingreport.cpp
          OrbitStartupWindow.cpp
          orbittablemodel.cpp
          orbittreeview.cpp
          opengldetect.cpp
          processlauncherwidget.cpp
          servicedeploymanager.cpp
          TutorialContent.cpp
          TutorialOverlay.cpp
          TutorialOverlay.ui
          ../icons/orbiticons.qrc
          ../images/orbitimages.qrc)

target_include_directories(OrbitQt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
  OrbitQt
  PRIVATE OrbitCore
          OrbitGl
          OrbitGgp
          OrbitSshQt
          OrbitVersion
          Qt5::Widgets
          Qt5::Core
          qtpropertybrowser::qtpropertybrowser)

if(WITH_CRASH_HANDLING)
  target_sources(
    OrbitQt
    PRIVATE CrashHandler.cpp
    PUBLIC CrashHandler.h CrashOptions.h)
  target_link_libraries(OrbitQt PRIVATE crashpad::crashpad)
  target_compile_definitions(OrbitQt PRIVATE ORBIT_CRASH_HANDLING)

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CrashOptions.cpp.in
                 ${CMAKE_CURRENT_BINARY_DIR}/CrashOptions.cpp)
  target_sources(OrbitQt PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/CrashOptions.cpp")
endif()

if(WIN32)
  set_target_properties(OrbitQt PROPERTIES WIN32_EXECUTABLE ON)

  include(${CMAKE_SOURCE_DIR}/cmake/version.cmake)
  GenerateVersionFile("${CMAKE_CURRENT_BINARY_DIR}/OrbitQt.rc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/OrbitQt.rc.in" OrbitQt)
  target_sources(OrbitQt PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/OrbitQt.rc)

  get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
  get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
  find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

  add_custom_command(
    TARGET OrbitQt
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E env PATH="${_qt_bin_dir}"
            "${WINDEPLOYQT_EXECUTABLE}" --pdb "$<TARGET_FILE:OrbitQt>"
    COMMENT "Running windeployqt...")
endif()

set_target_properties(OrbitQt PROPERTIES OUTPUT_NAME "Orbit")
set_target_properties(OrbitQt PROPERTIES AUTOMOC ON)
set_target_properties(OrbitQt PROPERTIES AUTOUIC ON)
set_target_properties(OrbitQt PROPERTIES AUTORCC ON)
strip_symbols(OrbitQt)

add_executable(OrbitQtTests)

target_compile_options(OrbitQtTests PRIVATE ${STRICT_COMPILE_FLAGS})

target_sources(OrbitQtTests
        PRIVATE
                CutoutWidget.h
                TutorialOverlay.h)
target_sources(OrbitQtTests
        PRIVATE StatusListenerImplTest.cpp
                StatusListenerImpl.cpp
                TutorialOverlayTest.cpp
                TutorialOverlay.cpp
                TutorialOverlay.ui
                ../images/orbitimages.qrc)

target_include_directories(OrbitQtTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
  OrbitQtTests
  PRIVATE OrbitCore
          OrbitGl
          Qt5::Widgets
          Qt5::Core
          GTest::Main)

set_target_properties(OrbitQtTests PROPERTIES AUTOMOC ON)
set_target_properties(OrbitQtTests PROPERTIES AUTOUIC ON)
set_target_properties(OrbitQtTests PROPERTIES AUTORCC ON)

if(WIN32)
  get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
  get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
  find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

  add_custom_command(
    TARGET OrbitQtTests
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E env PATH="${_qt_bin_dir}"
            "${WINDEPLOYQT_EXECUTABLE}" --pdb "$<TARGET_FILE:OrbitQtTests>"
    COMMENT "Running windeployqt...")
endif()


set(test_properties "")

# QT_QPA_PLATFORM=offscreen is currently not supported under Windows.
# So if requested we disable the test all the way.
# This is mainly a work-around for the CI, especially the presubmit checks.
if (WIN32 AND "$ENV{QT_QPA_PLATFORM}" STREQUAL "offscreen")
  message(STATUS "Disabling OrbitQt-tests since they don't work in a headless setup")
  list(APPEND test_properties DISABLED TRUE)
endif()

if (NOT WIN32)
  # On Linux we can always run the tests with offscreen rendering. That won't do any harm
  # and it avoid test failures in headless environments like SSH-sessions.
  list(APPEND test_properties ENVIRONMENT QT_QPA_PLATFORM=offscreen)
endif()

register_test(OrbitQtTests PROPERTIES ${test_properties})
