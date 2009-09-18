########################################################
# Files

SET (IMAGE_RCCS  images.qrc)

########################################################
# Build

QT4_ADD_RESOURCES(IMAGE_RCC_SRCS ${IMAGE_RCCS})

########################################################
# Continue on to subdirs
SUBDIRS (north_arrows splash icons themes svg)
