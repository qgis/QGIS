#############################################################
# sources

set(QGIS_3D_SRCS
  qgs3d.cpp
  qgs3daxis.cpp
  qgs3daxissettings.cpp
  qgsaabb.cpp
  qgsabstract3dengine.cpp
  qgsabstractvectorlayer3drenderer.cpp
  qgs3danimationsettings.cpp
  qgs3dexportobject.cpp
  qgs3dmapexportsettings.cpp
  qgs3dmapscene.cpp
  qgs3dmapsettings.cpp
  qgs3dmaptool.cpp
  qgs3dmapcanvas.cpp
  qgs3drendercontext.cpp
  qgs3dsceneexporter.cpp
  qgs3dutils.cpp
  qgs3dwiredmesh_p.cpp
  qgscameracontroller.cpp
  qgscamerapose.cpp
  qgsfeature3dhandler_p.cpp
  qgsfgutils.cpp
  qgsframegraph.cpp
  qgsgeotransform.cpp
  qgsgltf3dutils.cpp
  qgsimagetexture.cpp
  qgslayoutitem3dmap.cpp
  qgsoffscreen3dengine.cpp
  qgsraycastingutils.cpp
  qgsrulebased3drenderer.cpp
  qgsrulebasedchunkloader_p.cpp
  qgstessellatedpolygongeometry.cpp
  qgstiledscenechunkloader_p.cpp
  qgstiledscenelayer3drenderer.cpp
  qgstilingscheme.cpp
  qgsvectorlayer3drenderer.cpp
  qgsvectorlayerchunkloader_p.cpp
  qgsmeshlayer3drenderer.cpp
  qgswindow3dengine.cpp
  qgsskyboxentity.cpp
  qgsskyboxsettings.cpp
  qgspostprocessingentity.cpp
  qgsrenderpassquad.cpp
  qgsambientocclusionrenderentity.cpp
  qgsambientocclusionblurentity.cpp
  qgspreviewquad.cpp
  qgsshadowsettings.cpp
  qgscolorramptexture.cpp
  qgsrubberband3d.cpp
  qgsambientocclusionsettings.cpp

  qgspointcloudlayer3drenderer.cpp
  qgspointcloudlayerchunkloader_p.cpp
  qgsvirtualpointcloudentity_p.cpp

  chunks/qgschunkboundsentity_p.cpp
  chunks/qgschunkedentity.cpp
  chunks/qgschunklist_p.cpp
  chunks/qgschunkloader.cpp
  chunks/qgschunknode.cpp
  chunks/qgschunkqueuejob.cpp

  lights/qgsdirectionallightsettings.cpp
  lights/qgslightsource.cpp
  lights/qgspointlightsettings.cpp

  materials/qgsabstractmaterialsettings.cpp
  materials/qgsgoochmaterialsettings.cpp
  materials/qgsmaterial.cpp
  materials/qgsmaterialregistry.cpp
  materials/qgsmetalroughmaterial.cpp
  materials/qgsmetalroughmaterialsettings.cpp
  materials/qgsnullmaterialsettings.cpp
  materials/qgsphongmaterialsettings.cpp
  materials/qgsphongtexturedmaterial.cpp
  materials/qgsphongtexturedmaterialsettings.cpp
  materials/qgssimplelinematerialsettings.cpp
  materials/qgstexturematerial.cpp

  processing/qgs3dalgorithms.cpp
  processing/qgsalgorithmtessellate.cpp

  symbols/qgsbillboardgeometry.cpp
  symbols/qgsline3dsymbol.cpp
  symbols/qgsline3dsymbol_p.cpp
  symbols/qgslinematerial_p.cpp
  symbols/qgslinevertexdata_p.cpp
  symbols/qgsmesh3dsymbol.cpp
  symbols/qgspoint3dbillboardmaterial.cpp
  symbols/qgspoint3dsymbol.cpp
  symbols/qgspoint3dsymbol_p.cpp
  symbols/qgspolygon3dsymbol.cpp
  symbols/qgspolygon3dsymbol_p.cpp
  symbols/qgspointcloud3dsymbol.cpp
  symbols/qgspointcloud3dsymbol_p.cpp

  terrain/qgs3dterrainregistry.cpp
  terrain/qgsabstractterrainsettings.cpp
  terrain/qgsdemterraingenerator.cpp
  terrain/qgsdemterrainsettings.cpp
  terrain/qgsdemterraintilegeometry_p.cpp
  terrain/qgsdemterraintileloader_p.cpp
  terrain/qgsflatterraingenerator.cpp
  terrain/qgsflatterrainsettings.cpp
  terrain/qgsmeshterrainsettings.cpp
  terrain/qgsonlineterraingenerator.cpp
  terrain/qgsonlinedemterrainsettings.cpp
  terrain/qgsquantizedmeshterraingenerator.cpp
  terrain/qgsquantizedmeshterrainsettings.cpp
  terrain/qgsterraindownloader.cpp
  terrain/qgsterrainentity.cpp
  terrain/qgsterraingenerator.cpp
  terrain/qgsterraintexturegenerator_p.cpp
  terrain/qgsterraintextureimage_p.cpp
  terrain/qgsterraintileloader.cpp

  mesh/qgsmesh3dgeometry_p.cpp
  mesh/qgsmesh3dentity_p.cpp
  mesh/qgsmesh3dmaterial_p.cpp
  mesh/qgsmeshterraingenerator.cpp
)

set(QGIS_3D_HDRS
  processing/qgs3dalgorithms.h
  qgs3d.h
  qgs3danimationsettings.h
  qgs3daxis.h
  qgs3daxissettings.h
  qgs3dmapscene.h
  qgs3dmapsettings.h
  qgs3dmaptool.h
  qgs3drendercontext.h
  qgs3dmapsceneentity.h
  qgs3dsceneexporter.h
  qgs3dtypes.h
  qgs3dutils.h
  qgs3dmapcanvas.h
  qgs3dwiredmesh_p.h
  qgsaabb.h
  qgsabstract3dengine.h
  qgsabstractvectorlayer3drenderer.h
  qgscameracontroller.h
  qgscamerapose.h
  qgsfgutils.h
  qgsframegraph.h
  qgsgeotransform.h
  qgsgltf3dutils.h
  qgslayoutitem3dmap.h
  qgsmeshlayer3drenderer.h
  qgsoffscreen3dengine.h
  qgsraycastingutils.h
  qgsrulebased3drenderer.h
  qgstessellatedpolygongeometry.h
  qgstiledscenechunkloader_p.h
  qgstiledscenelayer3drenderer.h
  qgstilingscheme.h
  qgsvectorlayer3drenderer.h
  qgswindow3dengine.h
  qgsskyboxentity.h
  qgsskyboxsettings.h
  qgspostprocessingentity.h
  qgspreviewquad.h
  qgsshadowsettings.h
  qgspointcloudlayer3drenderer.h
  qgsrenderpassquad.h
  qgsambientocclusionrenderentity.h
  qgsambientocclusionblurentity.h
  qgsambientocclusionsettings.h

  lights/qgsdirectionallightsettings.h
  lights/qgslightsource.h
  lights/qgspointlightsettings.h

  materials/qgsabstractmaterialsettings.h
  materials/qgsgoochmaterialsettings.h
  materials/qgsmaterial.h
  materials/qgsmaterialregistry.h
  materials/qgsmetalroughmaterial.h
  materials/qgsmetalroughmaterialsettings.h
  materials/qgsnullmaterialsettings.h
  materials/qgsphongmaterialsettings.h
  materials/qgsphongtexturedmaterial.h
  materials/qgsphongtexturedmaterialsettings.h
  materials/qgssimplelinematerialsettings.h
  materials/qgstexturematerial.h

  symbols/qgsbillboardgeometry.h
  symbols/qgsline3dsymbol.h
  symbols/qgsmesh3dsymbol.h
  symbols/qgspoint3dbillboardmaterial.h
  symbols/qgspoint3dsymbol.h
  symbols/qgspolygon3dsymbol.h
  symbols/qgspointcloud3dsymbol.h

  terrain/qgs3dterrainregistry.h
  terrain/qgsabstractterrainsettings.h
  terrain/qgsdemterraingenerator.h
  terrain/qgsdemterrainsettings.h
  terrain/qgsflatterraingenerator.h
  terrain/qgsflatterrainsettings.h
  terrain/qgsmeshterrainsettings.h
  terrain/qgsonlineterraingenerator.h
  terrain/qgsonlinedemterrainsettings.h
  terrain/qgsquantizedmeshterraingenerator.h
  terrain/qgsquantizedmeshterrainsettings.h
  terrain/qgsterraindownloader.h
  terrain/qgsterraingenerator.h
  terrain/qgsterraintileloader.h
  terrain/qgsterrainentity.h

  chunks/qgschunkloader.h
  chunks/qgschunkqueuejob.h
  chunks/qgschunknode.h
  chunks/qgschunkedentity.h

  mesh/qgsmeshterraingenerator.h
)

set(QGIS_3D_PRIVATE_HDRS
  qgsrulebasedchunkloader_p.h
  qgsvectorlayerchunkloader_p.h
  qgspointcloudlayerchunkloader_p.h
  qgsvirtualpointcloudentity_p.h
  chunks/qgschunkboundsentity_p.h
  chunks/qgschunklist_p.h
  symbols/qgsline3dsymbol_p.h
  symbols/qgslinematerial_p.h
  symbols/qgslinevertexdata_p.h
  symbols/qgspoint3dsymbol_p.h
  symbols/qgspolygon3dsymbol_p.h
  symbols/qgspointcloud3dsymbol_p.h
  terrain/qgsdemterraintilegeometry_p.h
  terrain/qgsdemterraintileloader_p.h
  terrain/qgsterraintexturegenerator_p.h
  terrain/qgsterraintextureimage_p.h
  terrain/qgsterraintileentity_p.h
  mesh/qgsmesh3dentity_p.h
  mesh/qgsmesh3dgeometry_p.h
  mesh/qgsmesh3dmaterial_p.h
  mesh/qgsmeshterraintileloader_p.h
  qgscolorramptexture.h
)

set (QGIS_3D_RCCS  shaders.qrc  ../../resources/3d/textures/textures.qrc)


#############################################################
# qgis_3d library


add_library(qgis_3d SHARED ${QGIS_3D_SRCS} ${QGIS_3D_HDRS} ${QGIS_3D_RCCS} ${QGIS_3D_PRIVATE_HDRS})

if (USE_PRECOMPILED_HEADERS)
  target_precompile_headers(qgis_3d PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/src/core/qgis.h>)
endif()

set_target_properties(qgis_3d PROPERTIES UNITY_BUILD ${ENABLE_UNITY_BUILDS})

# require c++17
target_compile_features(qgis_3d PRIVATE cxx_std_17)

if (BUILD_WITH_QT6)
  target_include_directories(qgis_3d SYSTEM PUBLIC
    ${QT6_3DEXTRA_INCLUDE_DIR}
  )
else()
  target_include_directories(qgis_3d SYSTEM PUBLIC
    ${QT5_3DEXTRA_INCLUDE_DIR}
  )
endif()

target_include_directories(qgis_3d PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/chunks
  ${CMAKE_CURRENT_SOURCE_DIR}/lights
  ${CMAKE_CURRENT_SOURCE_DIR}/materials
  ${CMAKE_CURRENT_SOURCE_DIR}/mesh
  ${CMAKE_CURRENT_SOURCE_DIR}/processing
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
  ${CMAKE_CURRENT_SOURCE_DIR}/symbols
  ${CMAKE_CURRENT_SOURCE_DIR}/terrain

  ${CMAKE_BINARY_DIR}/src/3d
  ${CMAKE_SOURCE_DIR}/external/delaunator-cpp
  ${CMAKE_SOURCE_DIR}/external/tinygltf
)

target_link_libraries(qgis_3d
  ${QT_VERSION_BASE}::3DCore
  ${QT_VERSION_BASE}::3DRender
  ${QT_VERSION_BASE}::3DInput
  ${QT_VERSION_BASE}::3DLogic
  ${QT_VERSION_BASE}::3DExtras
)

GENERATE_EXPORT_HEADER(
   qgis_3d
   BASE_NAME 3D
   EXPORT_FILE_NAME qgis_3d.h
)

set(QGIS_3D_HDRS ${QGIS_3D_HDRS} ${CMAKE_CURRENT_BINARY_DIR}/qgis_3d.h)

if(NOT APPLE OR NOT QGIS_MACAPP_FRAMEWORK)
  install(FILES ${QGIS_3D_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})
else()
  set_target_properties(qgis_3d PROPERTIES
    # no moc headers, messes up PROPERTIES syntax
    CLEAN_DIRECT_OUTPUT 1
    FRAMEWORK 1
    FRAMEWORK_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}"
    MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_SOURCE_DIR}/mac/framework.info.plist.in"
    MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${COMPLETE_VERSION}
    MACOSX_FRAMEWORK_IDENTIFIER org.qgis.qgis3_3d
    BUILD_WITH_INSTALL_RPATH TRUE
    PUBLIC_HEADER "${QGIS_3D_HDRS}"
    LINK_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}"
  )
  # generated export header does not get copied with PUBLIC_HEADER files
  add_custom_command(TARGET qgis_3d
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy qgis_3d.h
            "${QGIS_OUTPUT_DIRECTORY}/${QGIS_LIB_SUBDIR}/qgis_3d.framework/Headers"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS qgis_3d.h
  )
endif()

#generate unversioned libs for android
if (NOT ANDROID)
  set_target_properties(qgis_3d PROPERTIES
    VERSION ${COMPLETE_VERSION}
    SOVERSION ${COMPLETE_VERSION}
    )
endif()

target_link_libraries(qgis_3d qgis_core)

# clang-tidy
if(CLANG_TIDY_EXE)
  set_target_properties(
    qgis_3d PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
endif()

# install

install(TARGETS qgis_3d
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR}
  FRAMEWORK DESTINATION ${QGIS_FW_SUBDIR}
  PUBLIC_HEADER DESTINATION ${QGIS_INCLUDE_DIR})

# Mac dev frameworks

if (APPLE AND QGIS_MACAPP_INSTALL_DEV)
  install(TARGETS qgis_3d FRAMEWORK DESTINATION ${QGIS_MACAPP_DEV_PREFIX})
  install(CODE "execute_process(COMMAND install_name_tool -id \"${QGIS_MACAPP_DEV_PREFIX}/qgis_3d.framework/Versions/${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/qgis_3d\" \"$ENV{DESTDIR}${QGIS_MACAPP_DEV_PREFIX}/qgis_3d.framework/qgis_3d\")")
  install(CODE "execute_process(COMMAND install_name_tool -change \"${CMAKE_INSTALL_NAME_DIR}/qgis_core.framework/Versions/${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/qgis_core\" \"${QGIS_MACAPP_DEV_PREFIX}/qgis_core.framework/Versions/${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/qgis_core\" \"$ENV{DESTDIR}${QGIS_MACAPP_DEV_PREFIX}/qgis_3d.framework/qgis_3d\")")
endif()
