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
 * \class QgsJSONExporter
 * \brief Handles exporting QgsFeature features to GeoJSON features.
 * \note Added in version 2.16
 */

class CORE_EXPORT QgsJSONExporter
{
  public:

    /** Constructor for QgsJSONExporter.
     * @param precision maximum number of decimal places to use for geometry coordinates
     * @param includeGeometry set to false to avoid including the geometry representation in the JSON output
     * @param includeAttributes set to false to avoid including any attribute values in the JSON output
     */
    QgsJSONExporter( int precision = 17, bool includeGeometry = true, bool includeAttributes = true );

    /** Sets the maximum number of decimal places to use in geometry coordinates.
     * @param precision number of decimal places
     * @see precision()
     */
    void setPrecision( int precision ) { mPrecision = precision; }

    /** Returns the maximum number of decimal places to use in geometry coordinates.
     * @see setPrecision()
     */
    int precision() const { return mPrecision; }

    /** Sets whether to include geometry in the JSON exports.
     * @param includeGeometry set to false to prevent geometry inclusion
     * @see includeGeometry()
     */
    void setIncludeGeometry( bool includeGeometry ) { mIncludeGeometry = includeGeometry; }

    /** Returns whether geometry will be included in the JSON exports.
     * @see setIncludeGeometry()
     */
    bool includeGeometry() const { return mIncludeGeometry; }

    /** Sets whether to include attributes in the JSON exports.
     * @param includeAttributes set to false to prevent attribute inclusion
     * @see includeAttributes()
     */
    void setIncludeAttributes( bool includeAttributes ) { mIncludeAttributes = includeAttributes; }

    /** Returns whether attributes will be included in the JSON exports.
     * @see setIncludeAttributes()
     */
    bool includeAttributes() const { return mIncludeAttributes; }

    /** Sets the list of attributes to include in the JSON exports.
     * @param attributes list of attribute indexes, or an empty list to include all
     * attributes
     * @see attributes()
     * @see setExcludedAttributes()
     * @note Attributes excluded via setExcludedAttributes() take precedence over
     * attributes specified by this method.
     */
    void setAttributes( const QgsAttributeList& attributes ) { mAttributeIndexes = attributes; }

    /** Returns the list of attributes which will be included in the JSON exports, or
     * an empty list if all attributes will be included.
     * @see setAttributes()
     * @see excludedAttributes()
     * @note Attributes excluded via excludedAttributes() take precedence over
     * attributes returned by this method.
     */
    QgsAttributeList attributes() const { return mAttributeIndexes; }

    /** Sets a list of attributes to specifically exclude from the JSON exports. Excluded attributes
     * take precedence over attributes included via setAttributes().
     * @param attributes list of attribute indexes to exclude
     * @see excludedAttributes()
     * @see setAttributes()
     */
    void setExcludedAttributes( const QgsAttributeList& attributes ) { mExcludedAttributeIndexes = attributes; }

    /** Returns a list of attributes which will be specifically excluded from the JSON exports. Excluded attributes
     * take precedence over attributes included via attributes().
     * @see setExcludedAttributes()
     * @see attributes()
     */
    QgsAttributeList excludedAttributes() const { return mExcludedAttributeIndexes; }

    /** Returns a GeoJSON string representation of a feature.
     * @param feature feature to convert
     * @param extraProperties map of extra attributes to include in feature's properties
     * @param id optional ID to use as GeoJSON feature's ID instead of input feature's ID. If omitted, feature's
     * ID is used.
     * @returns GeoJSON string
     */
    QString exportFeature( const QgsFeature& feature,
                           const QVariantMap& extraProperties = QVariantMap(),
                           const QVariant& id = QVariant() ) const;

  private:

    //! Maximum number of decimal places for geometry coordinates
    int mPrecision;

    //! List of attribute indexes to include in export, or empty list to include all attributes
    //! @see mExcludedAttributeIndexes
    QgsAttributeList mAttributeIndexes;

    //! List of attribute indexes to exclude from export
    QgsAttributeList mExcludedAttributeIndexes;

    //! Whether to include geometry in JSON export
    bool mIncludeGeometry;

    //! Whether to include attributes in JSON export
    bool mIncludeAttributes;

};

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

    /** Encodes a value to a JSON string representation, adding appropriate quotations and escaping
     * where required.
     * @param value value to encode
     * @returns encoded value
     */
    static QString encodeValue( const QVariant& value );

};

#endif // QGSJSONUTILS_H
