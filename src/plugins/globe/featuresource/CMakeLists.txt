###################################################
# osgEarth plugin
###################################################

SET(OSGEARTH_PLUGIN_PREFIX "")
SET(LIB_POSTFIX ${LIB_SUFFIX})
SET(DYNAMIC_OSGEARTH "ON" )
SET(OSGEARTH_USER_DEFINED_DYNAMIC_OR_STATIC "SHARED")

# SET(CMAKE_SHARED_MODULE_PREFIX ${OSGEARTH_PLUGIN_PREFIX})
SET(TARGET_DEFAULT_PREFIX "osgdb_")
SET(TARGET_DEFAULT_LABEL_PREFIX "Plugin")

INCLUDE( OsgEarthMacroUtils )

SET(TARGET_SRC
    qgsglobefeaturesource.cpp
)

SET (TARGET_MOC_HDRS
    qgsglobefeaturesource.h
)

SET(TARGET_HDRS
    qgsglobefeaturecursor.h
    qgsglobefeatureoptions.h
    qgsglobefeatureutils.h
)

QT4_WRAP_CPP(TARGET_MOC_SRCS  ${TARGET_MOC_HDRS})

SET(TARGET_SRC ${TARGET_SRC} ${TARGET_MOC_SRCS})

SET(TARGET_COMMON_LIBRARIES ${TARGET_COMMON_LIBRARIES}
  qgis_core
  qgis_gui
  ${OSGDB_LIBRARY}
  ${OSG_LIBRARY}
  ${OSGEARTH_LIBRARY}
  ${OSGEARTHFEATURES_LIBRARY}
  ${OSGEARTHSYMBOLOGY_LIBRARY}
  ${OSGEARTHUTIL_LIBRARY}
  ${OPENTHREADS_LIBRARY}
)
SETUP_PLUGIN(osgearth_feature_qgis)

# to install public driver includes:
SET(LIB_NAME feature_qgis)
SET(LIB_PUBLIC_HEADERS ${TARGET_HDRS} ${TARGET_MOC_HDRS})
INCLUDE(ModuleInstallOsgEarthDriverIncludes OPTIONAL)
