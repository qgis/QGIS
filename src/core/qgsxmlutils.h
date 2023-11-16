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

class QgsRectangle;
class QgsBox3D;

#include <QDomElement>
#include <QMetaEnum>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"



/**
 * \ingroup core
 * \brief Assorted helper methods for reading and writing chunks of XML
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
    static Qgis::DistanceUnit readMapUnits( const QDomElement &element );

    static QgsRectangle readRectangle( const QDomElement &element );

    /**
     * Decodes a DOM element to a 3D box.
     * \param element DOM document
     * \returns decoded 3D box
     * \since QGIS 3.36
     */
    static QgsBox3D readBox3D( const QDomElement &element );

    /* writing */

    /**
     * Encodes a distance unit to a DOM element.
     * \param units units to encode
     * \param doc DOM document
     * \returns element containing encoded units
     * \see readMapUnits()
     */
    static QDomElement writeMapUnits( Qgis::DistanceUnit units, QDomDocument &doc );

    /**
     * Encodes a 3D box to a DOM element.
     * \param box 3D box to encode
     * \param doc DOM document
     * \param elementName name of the DOM element
     * \returns element containing encoded 3D box
     * \since QGIS 3.36
     */
    static QDomElement writeBox3D( const QgsBox3D &box, QDomDocument &doc, const QString &elementName = QStringLiteral( "extent3D" ) );

    /**
     * Encodes a rectangle to a DOM element.
     * \param rect rectangle to encode
     * \param doc DOM document
     * \param elementName name of the DOM element
     * \returns element containing encoded rectangle
     */
    static QDomElement writeRectangle( const QgsRectangle &rect, QDomDocument &doc, const QString &elementName = QStringLiteral( "extent" ) );

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

    /**
     * Read a flag value from an attribute of the element.
     * \param element the element to read the attribute from
     * \param attributeName the attribute name
     * \param defaultValue the default value as a flag
     * \note The flag value is a text as returned by \see QMetaEnum::valueToKeys.
     *       The flag must have been declared with Q_ENUM macro.
     * \since QGIS 3.4
     */
    template<class T> static T readFlagAttribute( const QDomElement &element, const QString &attributeName, T defaultValue ) SIP_SKIP
    {
      T value = defaultValue;
      // Get source categories
      const QMetaEnum metaEnum = QMetaEnum::fromType<T>();
      const QString sourceCategoriesStr( element.attribute( attributeName, metaEnum.valueToKeys( static_cast<int>( defaultValue ) ) ) );
      if ( metaEnum.isValid() )
      {
        bool ok = false;
        const int newValue = metaEnum.keysToValue( sourceCategoriesStr.toUtf8().constData(), &ok );
        if ( ok )
          value = static_cast<T>( newValue );
      }
      return value;
    }
};


#endif // QGSXMLUTILS_H
