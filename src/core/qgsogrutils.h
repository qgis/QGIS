/***************************************************************************
                             qgsogrutils.h
                             -------------
    begin                : February 2016
    copyright            : (C) 2016 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOGRUTILS_H
#define QGSOGRUTILS_H

#include "qgsfeature.h"

#include <ogr_api.h>
#include "cpl_conv.h"
#include "cpl_string.h"

/** \ingroup core
 * \class QgsOgrUtils
 * \brief Utilities for working with OGR features and layers
 *
 * Contains helper utilities for assisting work with both OGR features and layers.
 * \note Added in version 2.16
 * \note not available in Python bindings
 */
class CORE_EXPORT QgsOgrUtils
{
  public:

    /** Reads an OGR feature and converts it to a QgsFeature.
     * @param ogrFet OGR feature handle
     * @param fields fields collection corresponding to feature
     * @param encoding text encoding
     * @return valid feature if read was successful
     */
    static QgsFeature readOgrFeature( OGRFeatureH ogrFet, const QgsFields &fields, QTextCodec* encoding );

    /** Reads an OGR feature and returns a corresponding fields collection.
     * @param ogrFet OGR feature handle
     * @param encoding text encoding
     * @returns fields collection if read was successful
     */
    static QgsFields readOgrFields( OGRFeatureH ogrFet, QTextCodec* encoding );

    /** Retrieves an attribute value from an OGR feature.
     * @param ogrFet OGR feature handle
     * @param fields fields collection corresponding to feature
     * @param attIndex index of attribute to retrieve
     * @param encoding text encoding
     * @param ok optional storage for success of retrieval
     * @returns attribute converted to a QVariant object
     * @see readOgrFeatureAttributes()
     */
    static QVariant getOgrFeatureAttribute( OGRFeatureH ogrFet, const QgsFields &fields, int attIndex, QTextCodec* encoding, bool* ok = 0 );

    /** Reads all attributes from an OGR feature into a QgsFeature.
     * @param ogrFet OGR feature handle
     * @param fields fields collection corresponding to feature
     * @param feature QgsFeature to store attributes in
     * @param encoding text encoding
     * @returns true if attribute read was successful
     * @see getOgrFeatureAttribute()
     */
    static bool readOgrFeatureAttributes( OGRFeatureH ogrFet, const QgsFields &fields, QgsFeature& feature, QTextCodec* encoding );

    /** Reads the geometry from an OGR feature into a QgsFeature.
     * @param ogrFet OGR feature handle
     * @param feature QgsFeature to store geometry in
     * @returns true if geometry read was successful
     * @see readOgrFeatureAttributes()
     * @see ogrGeometryToQgsGeometry()
     */
    static bool readOgrFeatureGeometry( OGRFeatureH ogrFet, QgsFeature& feature );

    /** Converts an OGR geometry representation to a QgsGeometry object
     * @param geom OGR geometry handle
     * @returns new QgsGeometry object, if conversion was successful
     * @see readOgrFeatureGeometry()
     */
    static QgsGeometry* ogrGeometryToQgsGeometry( OGRGeometryH geom );

    /** Attempts to parse a string representing a collection of features using OGR. For example, this method can be
     * used to convert a GeoJSON encoded collection to a list of QgsFeatures.
     * @param string string to parse
     * @param fields fields collection to use for parsed features (@see stringToFields())
     * @param encoding text encoding
     * @returns list of parsed features, or an empty list if no features could be parsed
     * @see stringToFields()
     */
    static QgsFeatureList stringToFeatureList( const QString& string, const QgsFields& fields, QTextCodec* encoding );

    /** Attempts to retrieve the fields from a string representing a collection of features using OGR.
     * @param string string to parse
     * @param encoding text encoding
     * @returns retrieved fields collection, or an empty list if no fields could be determined from the string
     * @see stringToFeatureList()
     */
    static QgsFields stringToFields( const QString& string, QTextCodec* encoding );
};

#endif // QGSOGRUTILS_H
