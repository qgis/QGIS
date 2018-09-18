/***************************************************************************
    qgsxmlutils.h
    ---------------------
    begin                : December 2013
    copyright            : (C) 2013 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSXMLUTILS_H
#define QGSXMLUTILS_H

class QDomDocument;
class QDomElement;

class QgsRectangle;

#include "qgis_core.h"
#include "qgis.h"
#include "qgsunittypes.h"

/**
 * \ingroup core
 * Assorted helper methods for reading and writing chunks of XML
 */
class CORE_EXPORT QgsXmlUtils
{
  public:

    /* reading */

    /**
     * Decodes a distance unit from a DOM element.
     * \param element DOM element to decode
     * \returns distance units
     * \see writeMapUnits()
     */
    static QgsUnitTypes::DistanceUnit readMapUnits( const QDomElement &element );

    static QgsRectangle readRectangle( const QDomElement &element );

    /* writing */

    /**
     * Encodes a distance unit to a DOM element.
     * \param units units to encode
     * \param doc DOM document
     * \returns element containing encoded units
     * \see readMapUnits()
     */
    static QDomElement writeMapUnits( QgsUnitTypes::DistanceUnit units, QDomDocument &doc );

    static QDomElement writeRectangle( const QgsRectangle &rect, QDomDocument &doc );

    /**
     * Write a QVariant to a QDomElement.
     *
     * Supported types are
     *
     * - QVariant::Map
     * - QVariant::Int
     * - QVariant::Double
     * - QVariant::String
     * - QgsProperty (since QGIS 3.4)
     * - QgsCoordinateReferenceSystem (since QGIS 3.4)
     */
    static QDomElement writeVariant( const QVariant &value, QDomDocument &doc );

    /**
     * Read a QVariant from a QDomElement.
     */
    static QVariant readVariant( const QDomElement &element );
};


#endif // QGSXMLUTILS_H
