#    Copyright (C) 2022-2023 Jakub Melka
#
#    This file is part of PDF4QT.
#
#    PDF4QT is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    with the written consent of the copyright owner, any later version.
#
#    PDF4QT is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public License
#    along with PDF4QT.  If not, see <https://www.gnu.org/licenses/>.

add_library(Pdf4QtLibCore SHARED
    sources/pdfglobal.h
    sources/pdfaction.cpp
    sources/pdfaction.h
    sources/pdfalgorithmlcs.cpp
    sources/pdfalgorithmlcs.h
    sources/pdfannotation.cpp
    sources/pdfannotation.h
    sources/pdfblendfunction.cpp
    sources/pdfblendfunction.h
    sources/pdfccittfaxdecoder.cpp
    sources/pdfccittfaxdecoder.h
    sources/pdfcertificatemanager.cpp
    sources/pdfcertificatemanager.h
    sources/pdfcms.cpp
    sources/pdfcms.h
    sources/pdfdiff.cpp
    sources/pdfdiff.h
    sources/pdfdocumentbuilder.cpp
    sources/pdfdocumentbuilder.h
    sources/pdfdocumentmanipulator.cpp
    sources/pdfdocumentmanipulator.h
    sources/pdfdocumenttextflow.cpp
    sources/pdfdocumenttextflow.h
    sources/pdfdocumenttextfloweditormodel.cpp
    sources/pdfdocumenttextfloweditormodel.h
    sources/pdfdocumentwriter.cpp
    sources/pdfdocumentwriter.h
    sources/pdfexecutionpolicy.cpp
    sources/pdfexecutionpolicy.h
    sources/pdffile.cpp
    sources/pdffile.h
    sources/pdfform.cpp
    sources/pdfform.h
    sources/pdficontheme.cpp
    sources/pdficontheme.h
    sources/pdfjavascriptscanner.cpp
    sources/pdfjavascriptscanner.h
    sources/pdfjbig2decoder.cpp
    sources/pdfjbig2decoder.h
    sources/pdfmultimedia.cpp
    sources/pdfmultimedia.h
    sources/pdfobject.cpp
    sources/pdfobject.h
    sources/pdfobjecteditormodel.cpp
    sources/pdfobjecteditormodel.h
    sources/pdfobjectutils.cpp
    sources/pdfobjectutils.h
    sources/pdfoptimizer.cpp
    sources/pdfoptimizer.h
    sources/pdfoptionalcontent.cpp
    sources/pdfoptionalcontent.h
    sources/pdfoutline.cpp
    sources/pdfoutline.h
    sources/pdfpagenavigation.cpp
    sources/pdfpagenavigation.h
    sources/pdfpagetransition.cpp
    sources/pdfpagetransition.h
    sources/pdfpainterutils.cpp
    sources/pdfpainterutils.h
    sources/pdfparser.cpp
    sources/pdfparser.h
    sources/pdfdocument.cpp
    sources/pdfdocument.h
    sources/pdfdocumentreader.cpp
    sources/pdfdocumentreader.h
    sources/pdfpattern.cpp
    sources/pdfpattern.h
    sources/pdfplugin.cpp
    sources/pdfplugin.h
    sources/pdfprogress.cpp
    sources/pdfprogress.h
    sources/pdfredact.cpp
    sources/pdfredact.h
    sources/pdfsecurityhandler.cpp
    sources/pdfsecurityhandler.h
    sources/pdfsignaturehandler.cpp
    sources/pdfsignaturehandler.h
    sources/pdfsnapper.cpp
    sources/pdfsnapper.h
    sources/pdfstructuretree.cpp
    sources/pdfstructuretree.h
    sources/pdftextlayout.cpp
    sources/pdftextlayout.h
    sources/pdftransparencyrenderer.cpp
    sources/pdftransparencyrenderer.h
    sources/pdfutils.cpp
    sources/pdfutils.h
    sources/pdfxfaengine.cpp
    sources/pdfxfaengine.h
    sources/pdfxreftable.cpp
    sources/pdfxreftable.h
    sources/pdfvisitor.cpp
    sources/pdfvisitor.h
    sources/pdfencoding.cpp
    sources/pdfencoding.h
    sources/pdfcatalog.cpp
    sources/pdfcatalog.h
    sources/pdfpage.cpp
    sources/pdfpage.h
    sources/pdfstreamfilters.cpp
    sources/pdfstreamfilters.h
    sources/pdfcolorspaces.cpp
    sources/pdfcolorspaces.h
    sources/pdfrenderer.cpp
    sources/pdfrenderer.h
    sources/pdfpagecontentprocessor.cpp
    sources/pdfpagecontentprocessor.h
    sources/pdfpainter.cpp
    sources/pdfpainter.h
    sources/pdffunction.cpp
    sources/pdffunction.h
    sources/pdfnametounicode.cpp
    sources/pdfnametounicode.h
    sources/pdffont.cpp
    sources/pdffont.h
    sources/pdfimage.cpp
    sources/pdfimage.h
    sources/pdfdocumentsanitizer.h
    sources/pdfdocumentsanitizer.cpp
    sources/pdfimageconversion.h
    sources/pdfimageconversion.cpp
    sources/pdfcolorconvertor.h
    sources/pdfcolorconvertor.cpp
    sources/pdftextlayoutgenerator.h
    sources/pdftextlayoutgenerator.cpp
    sources/pdfwidgetsnapshot.cpp
    sources/pdfwidgetsnapshot.h
    cmaps.qrc
    sources/pdfcertificatestore.h
    sources/pdfcertificatestore.cpp
)

include(GenerateExportHeader)

GENERATE_EXPORT_HEADER(Pdf4QtLibCore
                       EXPORT_MACRO_NAME
                       PDF4QTLIBCORESHARED_EXPORT
                       EXPORT_FILE_NAME "${CMAKE_BINARY_DIR}/${INSTALL_INCLUDEDIR}/pdf4qtlibcore_export.h")

target_link_libraries(Pdf4QtLibCore PRIVATE ${QT_VERSION_BASE}::Core ${QT_VERSION_BASE}::Gui ${QT_VERSION_BASE}::Xml ${QT_VERSION_BASE}::Svg)
target_link_libraries(Pdf4QtLibCore PRIVATE ${LCMS2_LIBRARIES})
target_link_libraries(Pdf4QtLibCore PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(Pdf4QtLibCore PRIVATE ZLIB::ZLIB)
target_link_libraries(Pdf4QtLibCore PRIVATE ${FREETYPE_LIBRARIES})
target_link_libraries(Pdf4QtLibCore PRIVATE ${Fontconfig_LIBRARIES})
target_link_libraries(Pdf4QtLibCore PRIVATE openjp2)
target_link_libraries(Pdf4QtLibCore PRIVATE JPEG::JPEG)

target_include_directories(Pdf4QtLibCore PRIVATE SYSTEM
                           ${FREETYPE_INCLUDE_DIRS}
                           ${Fontconfig_INCLUDE_DIRS})

if(PDF4QT_ENABLE_OPENGL)
    target_link_libraries(Pdf4QtLibCore PRIVATE Qt6::OpenGL)
endif()

if(LINUX_GCC)
    #target_link_libraries(Pdf4QtLibCore PUBLIC TBB::tbb)
endif()

if(MINGW)
    target_link_libraries(Pdf4QtLibCore PRIVATE Secur32 Mscms Gdi32 User32 crypt32)
endif()

target_include_directories(Pdf4QtLibCore INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/sources)
target_include_directories(Pdf4QtLibCore PUBLIC ${CMAKE_BINARY_DIR}/${INSTALL_INCLUDEDIR})

set_target_properties(Pdf4QtLibCore PROPERTIES
                      VERSION ${PDF4QT_VERSION}
                      SOVERSION ${PDF4QT_VERSION}
                      LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PDF4QT_INSTALL_LIB_DIR}
                      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PDF4QT_INSTALL_BIN_DIR})

install(TARGETS Pdf4QtLibCore RUNTIME DESTINATION ${PDF4QT_INSTALL_BIN_DIR} LIBRARY DESTINATION ${PDF4QT_INSTALL_LIB_DIR})
