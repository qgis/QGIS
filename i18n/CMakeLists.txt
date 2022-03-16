find_package(${QT_VERSION_BASE} COMPONENTS LinguistTools REQUIRED)
set(QT_LRELEASE_EXECUTABLE ${QT_VERSION_BASE}::lrelease)

macro(ADD_TRANSLATION_FILES _sources )
    foreach (_current_FILE ${ARGN})
      get_filename_component(_in ${_current_FILE} ABSOLUTE)
      get_filename_component(_basename ${_current_FILE} NAME_WE)

      set(_out ${QGIS_OUTPUT_DIRECTORY}/i18n/${_basename}.qm)

      add_custom_command(
         OUTPUT ${_out}
         COMMAND ${QT_LRELEASE_EXECUTABLE}
         ARGS -silent ${_in} -qm ${_out}
         DEPENDS ${_in}
      )

      set(${_sources} ${${_sources}} ${_out} )
   endforeach (_current_FILE)
endmacro(ADD_TRANSLATION_FILES)

# make sure the output directory exists
file(MAKE_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/i18n)


set(TS_FILES qgis_ar.ts qgis_bg.ts qgis_bs.ts qgis_ca.ts qgis_cs.ts qgis_da.ts qgis_de.ts qgis_el.ts qgis_en_US.ts qgis_es.ts qgis_et.ts qgis_eu.ts qgis_fa.ts qgis_fi.ts qgis_fr.ts qgis_gl.ts qgis_hu.ts qgis_is.ts qgis_it.ts qgis_ja.ts qgis_ko.ts qgis_ky.ts qgis_lt.ts qgis_lv.ts qgis_nb.ts qgis_nl.ts qgis_pl.ts qgis_pt_BR.ts qgis_pt_PT.ts qgis_ro.ts qgis_ru.ts qgis_sc.ts qgis_sv.ts qgis_tr.ts qgis_uk.ts qgis_vi.ts qgis_zh-Hans.ts qgis_zh-Hant.ts)

ADD_TRANSLATION_FILES (QM_FILES ${TS_FILES})

if (UNIX AND NOT APPLE AND PYQT5_FOUND)
  add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/org.qgis.qgis.desktop ${CMAKE_BINARY_DIR}/org.qgis.qgis.appdata.xml
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMAND ${Python_EXECUTABLE}
    ARGS ${CMAKE_SOURCE_DIR}/scripts/ts2appinfo.py "${CMAKE_BINARY_DIR}"
    COMMENT "Updating appinfo files..."
    DEPENDS ${QM_FILES} ${CMAKE_SOURCE_DIR}/linux/org.qgis.qgis.desktop.in ${CMAKE_SOURCE_DIR}/linux/org.qgis.qgis.appdata.xml.in
  )

  set(MD_FILES ${CMAKE_BINARY_DIR}/org.qgis.qgis.desktop ${CMAKE_BINARY_DIR}/org.qgis.qgis.appdata.xml)

  install(FILES ${CMAKE_BINARY_DIR}/org.qgis.qgis.desktop DESTINATION share/applications)
  install(FILES ${CMAKE_BINARY_DIR}/org.qgis.qgis.appdata.xml DESTINATION share/metainfo)
endif()

# creating a custom target is needed to make the files build
# "ALL" means that it will be run by default
add_custom_target (translations ALL DEPENDS ${QM_FILES} ${MD_FILES})

# first compile sources, then compile translations
if (WITH_DESKTOP)
  add_dependencies (translations ${QGIS_APP_NAME})
endif()

install (FILES ${QM_FILES}
         DESTINATION ${QGIS_DATA_DIR}/i18n)
