# Macros/functions for finding QCA's qcatool utility and qca-ossl plugin
# ~~~~~~~~~~~~~~~~
# When FIND_QCATOOL is run, will define:
#
#   QCATOOL_EXECUTABLE - Path to QCA's qcatool utility
#
# NOTE: FIND_QCAOSSL_PLUGIN_CPP requires Qt and QCA packages to be found
#
# Copyright (c) 2014, Larry Shaffer, <larrys (at) dakotacarto (dot) com>>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

function(FIND_QCAOSSL_PLUGIN_CPP PLUGIN_REQUIRED)

  # requires Qt and QCA packages to be found
  if(QT_INCLUDE_DIR AND QT_QTCORE_INCLUDE_DIR AND QT_QTCORE_LIBRARY
     AND QCA_INCLUDE_DIR AND QCA_LIBRARY
     AND NOT CMAKE_CROSSCOMPILING)

    # NOTE: boolean result when compiled executable is run
    set(CODE
      "
      #include <QtCrypto>
      #include <QCoreApplication>

      int main( int argc, char** argv )
      {
        QCA::Initializer init;
        QCoreApplication app( argc, argv );
        if ( !QCA::isSupported( \"cert\", \"qca-ossl\" ) )
        {
          return 0;
        }
        return 1;
      }
      "
    )
    set(TESTCPP "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/qcaossl.cpp")
    file(WRITE ${TESTCPP} "${CODE}")

    set(QCA_INCLUDE_DIRECTORIES "-DINCLUDE_DIRECTORIES:STRING=${QT_INCLUDE_DIR};${QT_QTCORE_INCLUDE_DIR};${QCA_INCLUDE_DIR}")
    set(QCA_LINK_LIBRARIES "-DLINK_LIBRARIES:STRING=${QT_QTCORE_LIBRARY};${QCA_LIBRARY}")

    try_run(RUN_RESULT COMPILE_RESULT
      ${CMAKE_BINARY_DIR} ${TESTCPP}
      CMAKE_FLAGS "${QCA_INCLUDE_DIRECTORIES}" "${QCA_LINK_LIBRARIES}"
      COMPILE_OUTPUT_VARIABLE COMPILE_OUTPUT
    )

    set(_msg "QCA OpenSSL plugin not found (run-time/unit-test dependency)")

    if(NOT COMPILE_RESULT)
      message(STATUS "QCA OpenSSL plugin C++ check failed to compile")
      if(${PLUGIN_REQUIRED})
        message(STATUS "QCA OpenSSL plugin C++ check compile output:")
        message(STATUS "${COMPILE_OUTPUT}")
        message(FATAL_ERROR ${_msg})
      else()
        message(STATUS ${_msg})
      endif()
    else()
      if(NOT RUN_RESULT)
        if(${PLUGIN_REQUIRED})
          message(FATAL_ERROR ${_msg})
        else()
          message(STATUS ${_msg})
        endif()
      else()
        message(STATUS "Found QCA OpenSSL plugin")
      endif()
    endif()

  else()
    message(STATUS "QtCore/QCA include/lib variables missing or CMake is cross-compiling,")
    message(STATUS "  skipping QCA OpenSSL plugin C++ check")
  endif()

endfunction(FIND_QCAOSSL_PLUGIN_CPP PLUGIN_REQUIRED)


function(FIND_QCATOOL TOOL_REQUIRED)
  if(NOT QCATOOL_EXECUTABLE)

    if(MSVC)
      find_program(QCATOOL_EXECUTABLE NAMES qcatool.exe qcatool2.exe
        PATHS
          $ENV{LIB_DIR}/bin
          $ENV{OSGEO4W_ROOT}/bin
      )
    else()
      find_program(QCATOOL_EXECUTABLE NAMES qcatool qcatool2)
    endif()

    if(NOT QCATOOL_EXECUTABLE)
      set(_msg "QCA's qcatool utility not found - aborting")
      if(${TOOL_REQUIRED})
        message(FATAL_ERROR ${_msg})
      else()
        message(STATUS ${_msg})
      endif()
    endif()

  else()

    get_filename_component(_qcatool ${QCATOOL_EXECUTABLE} REALPATH)
    if(NOT EXISTS "${_qcatool}")
      set(_msg "QCA's qcatool utility not found at: ${QCATOOL_EXECUTABLE}")
      if(${TOOL_REQUIRED})
        message(FATAL_ERROR ${_msg})
      else()
        message(STATUS ${_msg})
      endif()
    endif()

  endif(NOT QCATOOL_EXECUTABLE)

endfunction(FIND_QCATOOL TOOL_REQUIRED)


function(FIND_QCAOSSL_PLUGIN PLUGIN_REQUIRED)

  get_filename_component(_qcatool ${QCATOOL_EXECUTABLE} REALPATH)

  if(EXISTS "${_qcatool}")
    execute_process(COMMAND "${_qcatool}" plugins OUTPUT_VARIABLE _qca_plugins)
    # message(STATUS ${_qca_plugins})

    if(NOT "${_qca_plugins}" MATCHES "qca-ossl")
      set(_msg "QCA OpenSSL plugin not found (run-time/unit-test dependency)")
      if(${PLUGIN_REQUIRED})
        message(FATAL_ERROR ${_msg})
      else()
        message(STATUS ${_msg})
      endif()
    else()
      message(STATUS "Found QCA OpenSSL plugin")
    endif()
  endif()

endfunction(FIND_QCAOSSL_PLUGIN PLUGIN_REQUIRED)


function(FIND_QCA_PLUGIN_DIR DIR_REQUIRED)

  FIND_QCATOOL(1)
  get_filename_component(_qcatool ${QCATOOL_EXECUTABLE} REALPATH)

  if(EXISTS "${_qcatool}")
    execute_process(COMMAND "${_qcatool}" plugins OUTPUT_VARIABLE _qca_plugins)
    #message(STATUS ${_qca_plugins})
    string(REGEX REPLACE "\n" ";" _qca_plugins_list "${_qca_plugins}")
    #message(STATUS "_qca_plugins_list:  ${_qca_plugins_list}")

    if(NOT "${_qca_plugins}" MATCHES "Available Providers")
      set(_msg "QCA plugin directory not found")
      if(${DIR_REQUIRED})
        message(FATAL_ERROR ${_msg})
      else()
        message(STATUS ${_msg})
      endif()
    else()

      set(QCA_PLUGIN_DIR)
      foreach(_plugin_dir ${_qca_plugins_list})
        string(STRIP "${_plugin_dir}" _plugin_dir)
        if(EXISTS "${_plugin_dir}" AND IS_DIRECTORY "${_plugin_dir}" AND NOT QCA_PLUGIN_DIR)
          file(GLOB qca_dylibs "${_plugin_dir}/crypto/libqca*")
          if(qca_dylibs)
            set(QCA_PLUGIN_DIR "${_plugin_dir}")
          endif()
        endif()
      endforeach()

      if(QCA_PLUGIN_DIR)
        set(QCA_PLUGIN_DIR "${QCA_PLUGIN_DIR}" PARENT_SCOPE)
        message(STATUS "Found QCA plugin directory: ${QCA_PLUGIN_DIR}")
      else()
        set(_msg "QCA plugin directory not found")
        if(${DIR_REQUIRED})
          message(FATAL_ERROR ${_msg})
        else()
          message(STATUS ${_msg})
        endif()
      endif()

    endif()
  endif()

endfunction(FIND_QCA_PLUGIN_DIR DIR_REQUIRED)
