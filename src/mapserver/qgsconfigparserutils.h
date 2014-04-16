/***************************************************************************
                              qgsconfigparserutils.h
                              ------------------------
  begin                : March 28, 2014
  copyright            : (C) 2014 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCONFIGPARSERUTILS_H
#define QGSCONFIGPARSERUTILS_H

#include <QStringList>

class QgsCoordinateReferenceSystem;
class QgsMapLayer;
class QgsRectangle;
class QDomDocument;
class QDomElement;
class QString;

class QgsConfigParserUtils
{
  public:
    static void appendCRSElementsToLayer( QDomElement& layerElement, QDomDocument& doc, const QStringList &crsList,
                                          const QStringList& constrainedCrsList );
    static void appendCRSElementToLayer( QDomElement& layerElement, const QDomElement& precedingElement,
                                         const QString& crsText, QDomDocument& doc );
    static void appendLayerBoundingBoxes( QDomElement& layerElem, QDomDocument& doc, const QgsRectangle& layerExtent,
                                          const QgsCoordinateReferenceSystem& layerCRS );
    /**Returns a list of supported EPSG coordinate system numbers from a layer*/
    static QStringList createCRSListForLayer( QgsMapLayer* theMapLayer );

    /**Returns default service capabilities from wms_metadata.xml if nothing else is defined*/
    static void fallbackServiceCapabilities( QDomElement& parentElement, QDomDocument& doc );

    static QList<QgsMapLayer*> layerMapToList( const QMap< int, QgsMapLayer* >& layerMap, bool reverseOrder = false );
};

#endif // QGSCONFIGPARSERUTILS_H
