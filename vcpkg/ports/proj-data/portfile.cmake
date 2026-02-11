set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO OSGeo/PROJ-data
    REF "${VERSION}"
    SHA512 945445434ef272eda0e52fb8f486b773a0a5064cab0a158597422543544992562ab4302155c81499b70574022ccb56b2d54489f657a0d1dcc8a190d6e9c56227
    HEAD_REF master
)

file(READ "${SOURCE_PATH}/agency.json" agency_json)
string(JSON num_entries LENGTH "${agency_json}")

math(EXPR max_index "${num_entries} - 1")

foreach(index RANGE "${max_index}")
    string(JSON agency_id GET "${agency_json}" "${index}" id)
    file(GLOB agency_files "${SOURCE_PATH}/${agency_id}/*")
    foreach(agency_file IN LISTS agency_files)
        if(NOT IS_DIRECTORY "${agency_file}")
            file(INSTALL FILES "${agency_file}" DESTINATION "${CURRENT_PACKAGES_DIR}/share/proj")
        endif()
    endforeach()
endforeach()

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/copyright_and_licenses.csv")
