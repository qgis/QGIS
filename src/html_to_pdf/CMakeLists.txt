########################################################
# Files

set(QGIS_HTML_TO_PDF_SRCS
  main.cpp
  qgshtmltopdf.cpp
  ${CMAKE_BINARY_DIR}/qgsconfig.h
)

########################################################
# Build

win32_icon(QGIS_HTML_TO_PDF_SRCS)
win32_version_info("QGIS HTML to PDF conversion tool" "qgis_html_to_pdf" QGIS_HTML_TO_PDF_SRCS)

add_executable (qgis_html_to_pdf ${QGIS_HTML_TO_PDF_SRCS})
if (QGISPOSTFIX)
  set_target_properties(qgis_html_to_pdf PROPERTIES RUNTIME_OUTPUT_NAME qgis_html_to_pdf${QGISPOSTFIX})
endif()

target_include_directories(qgis_html_to_pdf PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(qgis_html_to_pdf
  ${QT_VERSION_BASE}::Core
  ${QT_VERSION_BASE}::Gui
  ${QT_VERSION_BASE}::WebEngineCore
)

set_target_properties(qgis_html_to_pdf PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_LIBEXEC_SUBDIR}
)

# install

install (TARGETS qgis_html_to_pdf
  RUNTIME DESTINATION ${QGIS_LIBEXEC_DIR}
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

