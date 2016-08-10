/***************************************************************************
    qgssymbologyconversion.h
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
#ifndef QGSSYMBOLOGYCONVERSION_H
#define QGSSYMBOLOGYCONVERSION_H

class QDomNode;

class QgsFeatureRenderer;

#include "qgis.h"

#include <Qt>

/** \ingroup core
 * This class is not a part of public API, it is intended only for compatibility with older versions of QGIS (1.x) */
class CORE_EXPORT QgsSymbologyConversion
{
  public:

    /** Read old renderer definition from XML and create matching new renderer */
    static QgsFeatureRenderer* readOldRenderer( const QDomNode& layerNode, QgsWkbTypes::GeometryType geomType );

    static QString penStyle2QString( Qt::PenStyle penstyle );
    static Qt::PenStyle qString2PenStyle( const QString& string );
    static QString brushStyle2QString( Qt::BrushStyle brushstyle );
    static Qt::BrushStyle qString2BrushStyle( const QString& string );
};

#endif // QGSSYMBOLOGYCONVERSION_H
