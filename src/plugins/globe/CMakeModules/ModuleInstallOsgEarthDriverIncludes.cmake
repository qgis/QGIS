# Required Vars:
# ${LIB_NAME}
# ${LIB_PUBLIC_HEADERS}

SET(INSTALL_INCDIR include)

# FIXME: Do not run for OS X framework
INSTALL(
    FILES        ${LIB_PUBLIC_HEADERS}
    DESTINATION ${INSTALL_INCDIR}/osgEarthDrivers/${LIB_NAME}
)
