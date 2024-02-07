/***************************************************************************
                          qgspdfrenderer.cpp
                             -------------------
    begin                : December 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspdfrenderer.h"
#ifdef HAVE_PDF4QT
#include "pdfdocumentreader.h"
#include "pdfrenderer.h"
#include "pdffont.h"
#include "pdfcms.h"
#else
#include "qgsexception.h"
#include <QObject>
#endif

#include <QRectF>
#include <QPainter>

#ifdef HAVE_PDF4QT
class PdfDocumentContainer
{
  public:
    PdfDocumentContainer( const QString &path )
      : reader( nullptr, []( bool * )->QString {return QString(); }, true, false )
      , document( reader.readFromFile( path ) )
      , modifiedDocument( &document, nullptr )
      , fontCache( 1000, 1000 )
    {
      fontCache.setDocument( modifiedDocument );
      renderer = std::make_unique< pdf::PDFRenderer >( &document,
                 &fontCache,
                 &pdfCms,
                 nullptr,
                 pdf::PDFRenderer::Features(),
                 meshQualitySettings );
    }
    pdf::PDFDocumentReader reader;
    pdf::PDFDocument document;
    pdf::PDFModifiedDocument modifiedDocument;
    pdf::PDFFontCache fontCache;
    pdf::PDFCMSGeneric pdfCms;
    pdf::PDFMeshQualitySettings  meshQualitySettings;
    std::unique_ptr< pdf::PDFRenderer > renderer;
};
#endif

QgsPdfRenderer::QgsPdfRenderer( const QString &path )
  : mPath( path )
{
#ifdef HAVE_PDF4QT
  mDocumentContainer = std::make_unique< PdfDocumentContainer >( path );
#endif
}

QgsPdfRenderer::~QgsPdfRenderer() = default;

#ifdef HAVE_PDF4QT
int QgsPdfRenderer::pageCount() const
{
  const pdf::PDFCatalog *catalog = mDocumentContainer->document.getCatalog();
  return catalog->getPageCount();
}
#else
int QgsPdfRenderer::pageCount() const
{
  throw QgsNotSupportedException( QObject::tr( "Rendering PDF requires a QGIS build with PDF4Qt library support" ) );
}
#endif

#ifdef HAVE_PDF4QT
QRectF QgsPdfRenderer::pageMediaBox( int pageNumber ) const
{
  if ( pageNumber < 0 || pageNumber >= pageCount() )
    return QRectF();

  const pdf::PDFCatalog *catalog = mDocumentContainer->document.getCatalog();
  return catalog->getPage( pageNumber )->getMediaBox();
}
#else
QRectF QgsPdfRenderer::pageMediaBox( int ) const
{
  throw QgsNotSupportedException( QObject::tr( "Rendering PDF requires a QGIS build with PDF4Qt library support" ) );
}
#endif

#ifdef HAVE_PDF4QT
bool QgsPdfRenderer::render( QPainter *painter, const QRectF &rectangle, int pageIndex )
{
  mDocumentContainer->renderer->render( painter, rectangle, pageIndex );
  return true;
}
#else
bool QgsPdfRenderer::render( QPainter *, const QRectF &, int )
{
  throw QgsNotSupportedException( QObject::tr( "Rendering PDF requires a QGIS build with PDF4Qt library support" ) );
}

#endif
