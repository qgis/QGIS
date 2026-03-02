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


#include "qgis.h"
#include "qgis_core.h"

#include <QPair>

#define SIP_NO_FILE

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
     *
     * Blank segments format is expected to be in the form (((2.90402 7.36,11.8776 30.4499),()),((2 7))) with 3 levels
     * of parenthesis like MultiPolygon to deal with multi parts and inner rings. Empty opening-closing parenthesis are allowed
     * to define the lack of blank segments for some multi part or inner rings.
     *
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
