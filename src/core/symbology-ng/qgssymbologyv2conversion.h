/***************************************************************************
    qgssymbologyv2conversion.h
    ---------------------
    begin                : December 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSYMBOLOGYV2CONVERSION_H
#define QGSSYMBOLOGYV2CONVERSION_H

class QDomNode;

class QgsFeatureRendererV2;

#include "qgis.h"

#include <Qt>

/** This class is not a part of public API, it is intended only for compatibility with older versions of QGIS (1.x) */
class CORE_EXPORT QgsSymbologyV2Conversion
{
  public:

    /** Read old renderer definition from XML and create matching new renderer */
    static QgsFeatureRendererV2* readOldRenderer( const QDomNode& layerNode, QGis::GeometryType geomType );


    static QString penStyle2QString( Qt::PenStyle penstyle );
    static Qt::PenStyle qString2PenStyle( QString string );
    static QString brushStyle2QString( Qt::BrushStyle brushstyle );
    static Qt::BrushStyle qString2BrushStyle( QString string );
};

#endif // QGSSYMBOLOGYV2CONVERSION_H
