/***************************************************************************
  qgsconfigurationmap.h - QgsConfigurationMap

 ---------------------
 begin                : 11.11.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCONFIGURATIONMAP_H
#define QGSCONFIGURATIONMAP_H

#include <QVariant>
#include <QDomElement>

/**
 * A configuration map holds a QVariantMap as a
 * key-value store where each value can be a string,
 * boolean, integer, double or yet another map.
 *
 * \note Added in QGIS 3.0
 * \ingroup core
 */

class CORE_EXPORT QgsConfigurationMap
{
  public:

    /**
     * Create a new empty configuration map
     */
    QgsConfigurationMap();

    /**
     * Create a new configuration map from an existing
     * QVariantMap.
     */
    QgsConfigurationMap( const QVariantMap& value );

    /**
     * Get the content of this configuration map
     * as QVariantMap.
     */
    QVariantMap get() const;

    /**
     * Set the content of this configuration map
     * to a QVariantMap.
     */
    void set( const QVariantMap& value );

    /**
     * Write this map into the provided parentElement.
     * The attribute type of the parentElement will be
     * used internally.
     */
    void toXml( QDomElement& parentElement ) const;

    /**
     * Read this map from the provided element.
     */
    void fromXml( const QDomElement& element );

  private:
    QVariantMap mValue;

    void toXml( QDomElement& parentElement, const QVariant& value ) const;

    QVariant fromXmlHelper( const QDomElement& element ) const;
};

#endif // QGSCONFIGURATIONMAP_H
