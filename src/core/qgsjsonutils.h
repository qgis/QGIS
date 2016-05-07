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
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"

class QTextCodec;
class QgsVectorLayer;

/** \ingroup core
 * \class QgsJSONExporter
 * \brief Handles exporting QgsFeature features to GeoJSON features.
 *
 * Note that geometries will be automatically reprojected to WGS84 to match GeoJSON spec
 * if either the source vector layer or source CRS is set.
 * \note Added in version 2.16
 */

class CORE_EXPORT QgsJSONExporter
{
  public:

    /** Constructor for QgsJSONExporter.
     * @param vectorLayer associated vector layer (required for related attribute export)
     * @param precision maximum number of decimal places to use for geometry coordinates
     */
    QgsJSONExporter( const QgsVectorLayer* vectorLayer = nullptr, int precision = 17 );

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

    /** Sets whether to include attributes of features linked via references in the JSON exports.
     * @param includeRelated set to true to include attributes for any related child features
     * within the exported properties element.
     * @note associated vector layer must be set with setVectorLayer()
     * @see includeRelated()
     */
    void setIncludeRelated( bool includeRelated ) { mIncludeRelatedAttributes = includeRelated; }

    /** Returns whether attributes of related (child) features will be included in the JSON exports.
     * @see setIncludeRelated()
     */
    bool includeRelated() const { return mIncludeRelatedAttributes; }

    /** Sets the associated vector layer (required for related attribute export). This will automatically
     * update the sourceCrs() to match.
     * @param vectorLayer vector layer
     * @see vectorLayer()
     */
    void setVectorLayer( const QgsVectorLayer* vectorLayer );

    /** Returns the associated vector layer, if set.
     * @see setVectorLayer()
     */
    QgsVectorLayer* vectorLayer() const;

    /** Sets the source CRS for feature geometries. The source CRS must be set if geometries are to be
     * correctly automatically reprojected to WGS 84, to match GeoJSON specifications.
     * @param crs source CRS for input feature geometries
     * @note the source CRS will be overwritten when a vector layer is specified via setVectorLayer()
     * @see sourceCrs()
     */
    void setSourceCrs( const QgsCoordinateReferenceSystem& crs );

    /** Returns the source CRS for feature geometries. The source CRS must be set if geometries are to be
     * correctly automatically reprojected to WGS 84, to match GeoJSON specifications.
     * @see setSourceCrs()
     */
    const QgsCoordinateReferenceSystem& sourceCrs() const;

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
     * @see exportFeatures()
     */
    QString exportFeature( const QgsFeature& feature,
                           const QVariantMap& extraProperties = QVariantMap(),
                           const QVariant& id = QVariant() ) const;


    /** Returns a GeoJSON string representation of a list of features (feature collection).
     * @param features features to convert
     * @returns GeoJSON string
     * @see exportFeature()
     */
    QString exportFeatures( const QgsFeatureList& features ) const;

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

    //! Whether to include attributes from related features in JSON export
    bool mIncludeRelatedAttributes;

    //! Layer ID of associated vector layer. Required for related attribute export.
    QString mLayerId;

    QgsCoordinateReferenceSystem mCrs;

    QgsCoordinateTransform mTransform;

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

    /** Exports all attributes from a QgsFeature as a JSON map type.
     * @param feature feature to export
     */
    static QString exportAttributes( const QgsFeature& feature );

};

#endif // QGSJSONUTILS_H
