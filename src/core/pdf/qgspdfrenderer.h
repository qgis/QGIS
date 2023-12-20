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

class QPainter;
class QRectF;
class QString;

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
     * Renders the PDF from the specified \a path to a \a painter.
     *
     * \throws QgsNotSupportedException on QGIS builds without PDF4Qt library support.
     */
    static bool render( const QString &path, QPainter *painter, const QRectF &rectangle, int pageIndex ) SIP_THROW( QgsNotSupportedException );

};

#endif // QGSPDFRENDERER_H
