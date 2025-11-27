/***************************************************************************
    blanksegmentutils.h
    ---------------------
    begin                : 2025/11/05
    copyright            : (C) 2025 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSBLANKSEGMENTUTILS_H
#define QGSBLANKSEGMENTUTILS_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgis.h"
#include <QPair>

class QgsRenderContext;

/**
 * \ingroup core
 * \brief Helper class to manipulate blank segments
 * \since QGIS 4.0
*/
class CORE_EXPORT QgsBlankSegmentUtils
{
  public:

    typedef QList<QPair<double, double>> BlankSegments;

    /**
     * Parse blank segments string representation \a strBlankSegments
     * The blank segments are expected to be expressed in \a unit and converted in pixels regarding render context \a renderContext
     * \a error is populated with a descritive message if the string representation is not well formatted
     * Returns a list of start and end distance expressed in pixels for each part and rings
     *
     * \since QGIS 4.0
     */
    static QList<QList<BlankSegments>> parseBlankSegments( const QString &strBlankSegments, const QgsRenderContext &renderContext, Qgis::RenderUnit unit, QString &error );

  private:

};

#endif
