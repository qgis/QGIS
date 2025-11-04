if(VCPKG_TARGET_IS_OSX)
  if(VCPKG_TARGET_ARCHITECTURE STREQUAL "arm64")
    vcpkg_download_distfile(BASIC_PACKAGE
      URLS https://download.oracle.com/otn_software/mac/instantclient/233023/instantclient-basic-macos.arm64-${VERSION}-1.dmg
      FILENAME instantclient-basic-arm.dmg
      SHA512 beb9ed7c3bf61721eff4c6cd375aeb0deaf5941e9297ca8b4c5efd9d5e154a9052445e0f3612ff2b5ddfa50cd40c83eda6ead8159b98810ae644ec60f41ef82c
    )
    
    vcpkg_download_distfile(SDK_PACKAGE
      URLS https://download.oracle.com/otn_software/mac/instantclient/233023/instantclient-sdk-macos.arm64-${VERSION}.dmg
      FILENAME instantclient-sdk-arm.dmg
      SHA512 bb8e9e7b7f11281b7897271ae92a7e2fa485075c9b027e18823e639dd419f3ebddcfcdfa2df62bed9ae92d908cae622d51dfc86ca93e6242c74cae49e2b7b34a
    )
  else()
    # 19.16.0.0.0 is the latest release for x64 at the moment on https://www.oracle.com/database/technologies/instant-client/macos-intel-x86-downloads.html
    vcpkg_download_distfile(BASIC_PACKAGE
      URLS https://download.oracle.com/otn_software/mac/instantclient/1916000/instantclient-basic-macos.x64-19.16.0.0.0dbru.dmg
      FILENAME instantclient-basic-x64.dmg
      SHA512 91deb1c0f12d14cf0217567c3018aa2f922b248afb948858075bd9175115aa077cc797a4c5698b52c01b14aa8d205424f25e295e1845b94adab7c3d6e2e9364c
    )
    
    vcpkg_download_distfile(SDK_PACKAGE
      URLS https://download.oracle.com/otn_software/mac/instantclient/1916000/instantclient-sdk-macos.x64-19.16.0.0.0dbru.dmg
      FILENAME instantclient-sdk-x64.dmg
      SHA512 c910b764003bccd6f61cbf740d74865dbec0f9ab67898f78947e779ef5ee913e20f975b967b7bc31f31c78a0b8c63414c632de51c4f1c51746f0c4f3054a02c9
    )
  endif()
    
  vcpkg_execute_required_process(
    COMMAND "hdiutil" "mount" "${BASIC_PACKAGE}" "-mountpoint" "${CURRENT_BUILDTREES_DIR}/instantclient-basic"
    WORKING_DIRECTORY ${CURRENT_PACKAGES_DIR}
    LOGNAME hdiutil-mount-basic
  )
  vcpkg_execute_required_process(
    COMMAND "hdiutil" "mount" "${SDK_PACKAGE}" "-mountpoint" "${CURRENT_BUILDTREES_DIR}/instantclient-sdk"
    WORKING_DIRECTORY ${CURRENT_PACKAGES_DIR}
    LOGNAME hdiutil-mount-sdk
  )

function(install_versioned_libraries)
    # Parse arguments
    set(options)
    set(oneValueArgs TARGET_DIR)
    set(multiValueArgs LIBRARIES)
    cmake_parse_arguments(INSTALL "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    # Check for required arguments
    if(NOT DEFINED INSTALL_TARGET_DIR)
        message(FATAL_ERROR "TARGET_DIR must be specified")
    endif()
    
    if(NOT DEFINED INSTALL_LIBRARIES)
        message(FATAL_ERROR "LIBRARIES must be specified")
    endif()
    
    # Make sure target directory exists
    file(MAKE_DIRECTORY ${INSTALL_TARGET_DIR})
    
    foreach(lib ${INSTALL_LIBRARIES})
        # Get the real file (full version)
        get_filename_component(lib_real_path ${lib} REALPATH)
        get_filename_component(lib_filename ${lib} NAME)
        get_filename_component(lib_real_filename ${lib_real_path} NAME)
        
        # Extract the base name (without extension)
        string(REGEX REPLACE "\\..*$" "" lib_basename ${lib_filename})
        
        message(STATUS "Installing ${lib_real_path} as ${lib_basename}.dylib")
        
        # Copy the actual library file directly to the target with the base name
        configure_file(${lib_real_path} ${INSTALL_TARGET_DIR}/${lib_basename}.dylib COPYONLY)
    endforeach()
endfunction()


  install_versioned_libraries(
    TARGET_DIR "${CURRENT_PACKAGES_DIR}/lib"
    LIBRARIES
      "${CURRENT_BUILDTREES_DIR}/instantclient-basic/libocci.dylib" 
      "${CURRENT_BUILDTREES_DIR}/instantclient-basic/libclntsh.dylib"
  )

  file(INSTALL
          "${CURRENT_BUILDTREES_DIR}/instantclient-sdk/sdk/include"
      DESTINATION
          "${CURRENT_PACKAGES_DIR}/"
  )
  
  # Handle copyright
  file(INSTALL "${CURRENT_BUILDTREES_DIR}/instantclient-basic/BASIC_LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)

  vcpkg_execute_required_process(
    COMMAND "hdiutil" "detach" "${CURRENT_BUILDTREES_DIR}/instantclient-basic"
    WORKING_DIRECTORY ${CURRENT_PACKAGES_DIR}
    LOGNAME hdiutil-detach-basic
  )
  vcpkg_execute_required_process(
    COMMAND "hdiutil" "detach" "${CURRENT_BUILDTREES_DIR}/instantclient-sdk"
    WORKING_DIRECTORY ${CURRENT_PACKAGES_DIR}
    LOGNAME hdiutil-detach-sdk
  )
endif()
