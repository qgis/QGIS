########################################################
# Files

SET (IMAGE_RCCS images.qrc)

########################################################
# Build

QT5_ADD_RESOURCES(IMAGE_RCC_SRCS ${IMAGE_RCCS})

########################################################
# Continue on to subdirs
ADD_SUBDIRECTORY(icons)
ADD_SUBDIRECTORY(themes)
ADD_SUBDIRECTORY(svg)
