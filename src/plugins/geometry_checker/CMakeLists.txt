
########################################################
# Files

SET (geometrychecker_SRCS
    qgsgeometrychecker.cpp
    qgsgeometrycheckerplugin.cpp
    qgsgeometrycheckfactory.cpp
    checks/qgsgeometryanglecheck.cpp
    checks/qgsgeometryareacheck.cpp
    checks/qgsgeometrydegeneratepolygoncheck.cpp
    checks/qgsgeometryduplicatecheck.cpp
    checks/qgsgeometryduplicatenodescheck.cpp
    checks/qgsgeometrycheck.cpp
    checks/qgsgeometrygapcheck.cpp
    checks/qgsgeometryholecheck.cpp
    checks/qgsgeometrymultipartcheck.cpp
    checks/qgsgeometrycontainedcheck.cpp
    checks/qgsgeometryoverlapcheck.cpp
    checks/qgsgeometrysegmentlengthcheck.cpp
    checks/qgsgeometryselfintersectioncheck.cpp
    checks/qgsgeometrytypecheck.cpp
    ui/qgsgeometrycheckerdialog.cpp
    ui/qgsgeometrycheckersetuptab.cpp
    ui/qgsgeometrycheckerresulttab.cpp
    ui/qgsgeometrycheckfixdialog.cpp
    ui/qgsgeometrycheckerfixsummarydialog.cpp
    utils/qgsfeaturepool.cpp
    utils/qgsgeomutils.cpp
)

SET (geometrychecker_HDRS
    checks/qgsgeometrycheck.h
    qgsgeometrycheckerplugin.h
    qgsgeometrycheckfactory.h
    utils/qgsfeaturepool.h
    utils/qgsgeomutils.h
)

SET (geometrychecker_MOC_HDRS
    qgsgeometrychecker.h
    qgsgeometrycheckerplugin.h
    checks/qgsgeometryanglecheck.h
    checks/qgsgeometryareacheck.h
    checks/qgsgeometrycheck.h
    checks/qgsgeometrydegeneratepolygoncheck.h
    checks/qgsgeometryduplicatecheck.h
    checks/qgsgeometryduplicatenodescheck.h
    checks/qgsgeometrygapcheck.h
    checks/qgsgeometryholecheck.h
    checks/qgsgeometrymultipartcheck.h
    checks/qgsgeometrycontainedcheck.h
    checks/qgsgeometryoverlapcheck.h
    checks/qgsgeometrysegmentlengthcheck.h
    checks/qgsgeometryselfintersectioncheck.h
    checks/qgsgeometrysliverpolygoncheck.h
    checks/qgsgeometrytypecheck.h
    ui/qgsgeometrycheckerdialog.h
    ui/qgsgeometrycheckersetuptab.h
    ui/qgsgeometrycheckerresulttab.h
    ui/qgsgeometrycheckfixdialog.h
    ui/qgsgeometrycheckerfixsummarydialog.h
)

SET (geometrychecker_UIS
    ui/qgsgeometrycheckersetuptab.ui
    ui/qgsgeometrycheckerresulttab.ui
    ui/qgsgeometrycheckerfixsummarydialog.ui
)

SET (geometrychecker_RCCS
    pluginres.qrc
)

########################################################
# Build

QT4_WRAP_UI (geometrychecker_UIS_H  ${geometrychecker_UIS})

QT4_WRAP_CPP (geometrychecker_MOC_SRCS  ${geometrychecker_MOC_HDRS})

QT4_ADD_RESOURCES(geometrychecker_RCC_SRCS ${geometrychecker_RCCS})

ADD_LIBRARY (geometrycheckerplugin   MODULE ${geometrychecker_HDRS} ${geometrychecker_SRCS} ${geometrychecker_MOC_SRCS} ${geometrychecker_RCC_SRCS} ${geometrychecker_UIS_H})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core
     ../../core/geometry
     ../../core/symbology-ng
     ../../gui
     ..
)
INCLUDE_DIRECTORIES(SYSTEM
     ${GEOS_INCLUDE_DIR}
     ${GDAL_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(geometrycheckerplugin
  qgis_core
  qgis_gui
)

########################################################
# Install

INSTALL(TARGETS geometrycheckerplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

