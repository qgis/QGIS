cmake_minimum_required(VERSION 3.10.0)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

if(o2_WITH_QT6)
    find_package(Qt6 COMPONENTS Core Network REQUIRED)
else()
    find_package(Qt5 COMPONENTS Core Network REQUIRED)
endif()

set( o2_SRCS
    o2.cpp
    o2pollserver.cpp
    o2reply.cpp
    o2replyserver.cpp
    o2requestor.cpp
    o2simplecrypt.cpp
    o0jsonresponse.cpp
    o0settingsstore.cpp
    o0baseauth.cpp
)

set( o2_HDRS
    o2.h
    o2pollserver.h
    o2reply.h
    o2replyserver.h
    o2requestor.h
    o0abstractstore.h
    o0baseauth.h
    o0export.h
    o0globals.h
    o0jsonresponse.h
    o0requestparameter.h
    o0settingsstore.h
    o0simplecrypt.h
)

if(o2_WITH_OAUTH1)
    set( o2_SRCS
        ${o2_SRCS}
        o1.cpp
        o1requestor.cpp
        o1timedreply.cpp
    )
    set( o2_HDRS
        ${o2_HDRS}
        o1.h
        o1requestor.h
        o1timedreply.h
        o1freshbooks.h
    )
endif(o2_WITH_OAUTH1)

if(o2_WITH_TWITTER)
    set( o2_SRCS
        ${o2_SRCS}
        oxtwitter.cpp
    )
    set( o2_HDRS
        ${o2_HDRS}
        o1twitter.h
        oxtwitter.h
    )
endif(o2_WITH_TWITTER)

if(o2_WITH_DROPBOX)
    set( o2_HDRS
        ${o2_HDRS}
        o1dropbox.h
    )
endif(o2_WITH_DROPBOX)

if(o2_WITH_GOOGLE)
    set( o2_SRCS
        ${o2_SRCS}
        o2gft.cpp
        o2google.cpp
        o2googledevice.cpp
    )
    set( o2_HDRS
        ${o2_HDRS}
        o2gft.h
        o2google.h
        o2googledevice.h
    )
endif(o2_WITH_GOOGLE)

if(o2_WITH_VIMEO)
    set( o2_SRCS
        ${o2_SRCS}
        o2vimeo.cpp
    )
    set( o2_HDRS
        ${o2_HDRS}
        o2vimeo.h
    )
endif(o2_WITH_VIMEO)

if(o2_WITH_FACEBOOK)
    set( o2_SRCS
        ${o2_SRCS}
        o2facebook.cpp
    )
    set( o2_HDRS
        ${o2_HDRS}
        o2facebook.h
    )
endif(o2_WITH_FACEBOOK)

if(o2_WITH_SKYDRIVE)
    set( o2_SRCS
        ${o2_SRCS}
        o2skydrive.cpp
    )
    set( o2_HDRS
        ${o2_HDRS}
        o2skydrive.h
    )
endif(o2_WITH_SKYDRIVE)

if(o2_WITH_FLICKR)
    set( o2_HDRS
        ${o2_HDRS}
        o1flickr.h
    )
endif(o2_WITH_FLICKR)

if(o2_WITH_HUBIC)
    set( o2_SRCS
        ${o2_SRCS}
        o2hubic.cpp
    )
    set( o2_HDRS
        ${o2_HDRS}
        o2hubic.h
    )
endif(o2_WITH_HUBIC)

if(o2_WITH_SPOTIFY)
    set( o2_SRCS
        ${o2_SRCS}
        o2spotify.cpp
    )
    set( o2_HDRS
        ${o2_HDRS}
        o2spotify.h
    )
endif(o2_WITH_SPOTIFY)

if(o2_WITH_SURVEYMONKEY)
    set( o2_SRCS
        ${o2_SRCS}
        o2surveymonkey.cpp
    )
    set( o2_HDRS
        ${o2_HDRS}
        o2surveymonkey.h
    )
endif(o2_WITH_SURVEYMONKEY)

if(o2_WITH_SMUGMUG)
    set( o2_SRCS
        ${o2_SRCS}
        o1smugmug.cpp
    )
    set( o2_HDRS
        ${o2_HDRS}
        o1smugmug.h
    )
endif(o2_WITH_SMUGMUG)

if(o2_WITH_MSGRAPH)
    set( o2_SRCS
        ${o2_SRCS}
        o2msgraph.cpp
    )
    set( o2_HDRS
        ${o2_HDRS}
        o2msgraph.h
    )
endif(o2_WITH_MSGRAPH)

if(o2_WITH_KEYCHAIN)
    list(APPEND LINK_TARGETS ${QTKEYCHAIN_LIBRARIES})
    include_directories(SYSTEM ${QTKEYCHAIN_INCLUDE_DIRS})
    set( o2_SRCS
            ${o2_SRCS}
            o0keychainstore.cpp
            )
    set( o2_HDRS
            ${o2_HDRS}
            o0keychainstore.h
            )

endif(o2_WITH_KEYCHAIN)

if(BUILD_SHARED_LIBS AND APPLE AND POLICY CMP0042) # in CMake >= 2.8.12
    cmake_policy(SET CMP0042 OLD)
    set(CMAKE_MACOSX_RPATH OFF) # don't embed @rpath in install name
endif(BUILD_SHARED_LIBS AND APPLE AND POLICY CMP0042)

add_library( o2 ${o2_SRCS} ${o2_HDRS} )

if(BUILD_SHARED_LIBS)
    add_definitions( -DO2_SHARED_LIB )
endif(BUILD_SHARED_LIBS)

target_link_libraries( o2 Qt::Core Qt::Network ${LINK_TARGETS})

if(BUILD_SHARED_LIBS)
    if(APPLE)
      set_target_properties(o2
          PROPERTIES INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/lib
      )
    endif(APPLE)
    set_target_properties(o2
        PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${API_VERSION}
    )
else(BUILD_SHARED_LIBS)
    # needed for statically linked o2 in shared libs on x86_64
    set_target_properties(o2
        PROPERTIES POSITION_INDEPENDENT_CODE TRUE
    )
endif(BUILD_SHARED_LIBS)

install(TARGETS o2
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib${o2_LIB_SUFFIX}
    ARCHIVE DESTINATION lib${o2_LIB_SUFFIX}
)

install(FILES ${o2_HDRS}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include/o2
)

message(STATUS "Writing pkg-config file...")
configure_file(${CMAKE_CURRENT_LIST_DIR}/../o2.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/o2.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/o2.pc DESTINATION "${CMAKE_INSTALL_PREFIX}/lib${o2_LIB_SUFFIX}/pkgconfig/")

configure_file(${CMAKE_CURRENT_LIST_DIR}/../o2-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/o2-config.h @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/o2-config.h DESTINATION "${CMAKE_INSTALL_PREFIX}/include/o2")

