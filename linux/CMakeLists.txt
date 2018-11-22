#icons
## application icon
install(FILES ${CMAKE_SOURCE_DIR}/images/icons/qgis_icon.svg RENAME qgis.svg DESTINATION share/icons/hicolor/scalable/apps)
install(FILES ${CMAKE_SOURCE_DIR}/images/icons/qgis-icon-16x16.png RENAME qgis.png DESTINATION share/icons/hicolor/16x16/apps)
install(FILES ${CMAKE_SOURCE_DIR}/images/icons/qgis-icon-512x512.png RENAME qgis.png DESTINATION share/icons/hicolor/512x512/apps)

# Install MIME type icon
install(FILES ${CMAKE_SOURCE_DIR}/images/icons/qgis_mime_icon.svg RENAME qgis-mime.svg DESTINATION share/icons/hicolor/scalable/mimetypes)
foreach(size 8x8 16x16 22x22 24x24 32x32 36x36 42x42 48x48 64x64 72x72 80x80 96x96 128x128 192x192 256x256 512x512)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/icons//qgis-mime-icon${size}.png RENAME qgis-mime.png DESTINATION share/icons/hicolor/${size}/mimetypes/)
endforeach()

# Install QGIS file formats icons
foreach(_type qgs qlr qml qpt)
  install(FILES ${CMAKE_SOURCE_DIR}/images/icons/qgis_${_type}_icon.svg RENAME qgis_${_type}.svg DESTINATION share/icons/hicolor/scalable/mimetypes/)
  foreach(size 8x8 16x16 22x22 24x24 32x32 36x36 42x42 48x48 64x64 72x72 80x80 96x96 128x128 192x192 256x256 512x512)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/icons/qgis-${_type}${size}.png RENAME qgis-${_type}.png DESTINATION share/icons/hicolor/${size}/mimetypes/)
  endforeach()
endforeach()
