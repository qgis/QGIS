/***************************************************************************
                          qgspdfrenderer.h
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

#ifndef QGSPDFRENDERER_H
#define QGSPDFRENDERER_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgsconfig.h"
#include <QString>
#include <memory>

#ifdef HAVE_PDF4QT
#endif

class QPainter;
class QRectF;
class PdfDocumentContainer;


/**
 * \class QgsPdfRenderer
 * \ingroup core
 * \brief Utility class for rendering PDF documents.
 *
 * This functionality is not available on all platforms -- it requires a build
 * with the PDF4Qt library support enabled. On other platforms calling these
 * methods will raise a QgsNotSupportedException.
 *
 * \since QGIS 3.36
 */
class CORE_EXPORT QgsPdfRenderer
{
  public:

    /**
     * Constructs a PDF renderer for the file at the specified \a path.
     */
    QgsPdfRenderer( const QString &path );
    ~QgsPdfRenderer();

    QgsPdfRenderer( const QgsPdfRenderer &other ) = delete;
    QgsPdfRenderer &operator=( const QgsPdfRenderer &other ) = delete;

    /**
     * Returns the file path of the associated PDF file.
     */
    QString path() const { return mPath; }

    /**
     * Returns the number of pages in the PDF.
     *
     * \throws QgsNotSupportedException on QGIS builds without PDF4Qt library support.
     */
    int pageCount() const SIP_THROW( QgsNotSupportedException );

    /**
     * Returns the media box for the specified page. Units are in PDF points.
     *
     * \throws QgsNotSupportedException on QGIS builds without PDF4Qt library support.
     */
    QRectF pageMediaBox( int pageNumber ) const SIP_THROW( QgsNotSupportedException );

    /**
     * Renders the PDF from the specified \a path to a \a painter.
     *
     * The \a painterRect argument specifies the target rectangle for the PDF page in
     * \a painter coordinates.
     *
     * \throws QgsNotSupportedException on QGIS builds without PDF4Qt library support.
     */
    bool render( QPainter *painter, const QRectF &painterRect, int pageIndex ) SIP_THROW( QgsNotSupportedException );

  private:

#ifdef SIP_RUN
    QgsPdfRenderer( const QgsPdfRenderer &other );
#endif

    QString mPath;

#ifdef HAVE_PDF4QT
    std::unique_ptr< PdfDocumentContainer> mDocumentContainer;
#endif
};

#endif // QGSPDFRENDERER_H
