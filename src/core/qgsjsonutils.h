/***************************************************************************
    qgsjsonutils.h
     -------------
    Date                 : May 206
    Copyright            : (C) 2016 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSJSONUTILS_H
#define QGSJSONUTILS_H

#include "qgsfeature.h"

class QTextCodec;

/** \ingroup core
 * \class QgsJSONUtils
 * \brief Helper utilities for working with JSON and GeoJSON conversions.
 * \note Added in version 2.16
 */

class CORE_EXPORT QgsJSONUtils
{
  public:

    /** Attempts to parse a GeoJSON string to a collection of features.
     * @param string GeoJSON string to parse
     * @param fields fields collection to use for parsed features
     * @param encoding text encoding
     * @returns list of parsed features, or an empty list if no features could be parsed
     * @see stringToFields()
     * @note this function is a wrapper around QgsOgrUtils::stringToFeatureList()
     */
    static QgsFeatureList stringToFeatureList( const QString& string, const QgsFields& fields, QTextCodec* encoding );

    /** Attempts to retrieve the fields from a GeoJSON string representing a collection of features.
     * @param string GeoJSON string to parse
     * @param encoding text encoding
     * @returns retrieved fields collection, or an empty list if no fields could be determined from the string
     * @see stringToFeatureList()
     * @note this function is a wrapper around QgsOgrUtils::stringToFields()
     */
    static QgsFields stringToFields( const QString& string, QTextCodec* encoding );

    /** Returns a GeoJSON string representation of a feature.
     * @param feature feature to convert
     * @param precision maximum number of decimal places to use for geometry coordinates
     * @param attrIndexes list of attribute indexes to include in GeoJSON, or an empty list to include
     * all attributes
     * @param includeGeom set to false to avoid including the geometry representation in the JSON output
     * @param includeAttributes set to false to avoid including any attribute values in the JSON output
     * @param id optional ID to use as GeoJSON feature's ID instead of input feature's ID. If omitted, feature's
     * ID is used.
     * @returns GeoJSON string
     */
    static QString featureToGeoJSON( const QgsFeature& feature,
                                     int precision = 17,
                                     const QgsAttributeList& attrIndexes = QgsAttributeList(),
                                     bool includeGeom = true,
                                     bool includeAttributes = true,
                                     const QVariant& id = QVariant() );

    /** Encodes a value to a JSON string representation, adding appropriate quotations and escaping
     * where required.
     * @param value value to encode
     * @returns encoded value
     */
    static QString encodeValue( const QVariant& value );

};

#endif // QGSJSONUTILS_H
