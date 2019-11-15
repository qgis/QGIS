# Find SSH
# ~~~~~~~~~~
# CMake module to search for SSH library
#
# If it's found it sets SSH_FOUND to TRUE
# and following variables are set:
#   SSH_INCLUDE_DIR
#   SSH_LIBRARY
#


FIND_PATH(SSH_INCLUDE_DIR libssh/libssh.h /usr/local/include /usr/include)
FIND_LIBRARY(SSH_LIBRARY NAMES libssh PATHS /usr/local/lib /usr/lib)

IF (SSH_INCLUDE_DIR AND SSH_LIBRARY)
    SET(SSH_FOUND TRUE)
    MESSAGE(STATUS "Found SSH: ${SSH_LIBRARY}")
ELSE (SSH_INCLUDE_DIR AND SSH_LIBRARY)
    MESSAGE(SSH_INCLUDE_DIR=${SSH_INCLUDE_DIR})
    MESSAGE(SSH_LIBRARY=${SSH_LIBRARY})
    MESSAGE("Could not find SSH")
ENDIF (SSH_INCLUDE_DIR AND SSH_LIBRARY)
