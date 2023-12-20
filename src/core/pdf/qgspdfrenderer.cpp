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

bool QgsPdfRenderer::render( const QString &path, QPainter *painter, const QRectF &rectangle, int pageIndex )
{
  // LOTS of incorrect shortcuts here!

  pdf::PDFDocumentReader reader( nullptr, []( bool * )->QString {return QString(); }, true, false );
  pdf::PDFDocument document = reader.readFromFile( path );
  pdf::PDFModifiedDocument modifiedDocument( &document, nullptr );

  pdf::PDFFontCache fontCache( 1000, 1000 );
  fontCache.setDocument( modifiedDocument );

  pdf::PDFCMSGeneric pdfCms;
  pdf::PDFMeshQualitySettings meshQualitySettings;

  pdf::PDFRenderer renderer( &document, &fontCache, &pdfCms, nullptr, pdf::PDFRenderer::Features(), meshQualitySettings );
  renderer.render( painter, rectangle, pageIndex );
  return true;
}

#else
bool QgsPdfRenderer::render( const QString &, QPainter *, const QRectF &, int )
{
  throw QgsNotSupportedException( QObject::tr( "Rendering PDF requires a QGIS build with PDF4Qt library support" ) );
}

#endif
