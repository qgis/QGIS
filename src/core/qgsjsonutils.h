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

#include "qgis_core.h"
#include "qgsfeature.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsfields.h"

#include <QPointer>
#include <QJsonObject>

#ifndef SIP_RUN
#include "json_fwd.hpp"
using namespace nlohmann;
#endif

class QTextCodec;

/**
 * \ingroup core
 * \class QgsJsonExporter
 * \brief Handles exporting QgsFeature features to GeoJSON features.
 *
 * Note that geometries will be automatically reprojected to WGS84 to match GeoJSON spec
 * if either the source vector layer or source CRS is set.
 */

class CORE_EXPORT QgsJsonExporter
{
  public:

    /**
     * Constructor for QgsJsonExporter.
     * \param vectorLayer associated vector layer (required for related attribute export)
     * \param precision maximum number of decimal places to use for geometry coordinates,
     *  the RFC 7946 GeoJSON specification recommends limiting coordinate precision to 6
     */
    QgsJsonExporter( QgsVectorLayer *vectorLayer = nullptr, int precision = 6 );

    /**
     * Sets the maximum number of decimal places to use in geometry coordinates.
     * The RFC 7946 GeoJSON specification recommends limiting coordinate precision to 6
     * \param precision number of decimal places
     * \see precision()
     */
    void setPrecision( int precision ) { mPrecision = precision; }

    /**
     * Returns the maximum number of decimal places to use in geometry coordinates.
     * \see setPrecision()
     */
    int precision() const { return mPrecision; }

    /**
     * Sets whether to include geometry in the JSON exports.
     * \param includeGeometry set to FALSE to prevent geometry inclusion
     * \see includeGeometry()
     */
    void setIncludeGeometry( bool includeGeometry ) { mIncludeGeometry = includeGeometry; }

    /**
     * Returns whether geometry will be included in the JSON exports.
     * \see setIncludeGeometry()
     */
    bool includeGeometry() const { return mIncludeGeometry; }

    /**
     * Sets whether to include attributes in the JSON exports.
     * \param includeAttributes set to FALSE to prevent attribute inclusion
     * \see includeAttributes()
     */
    void setIncludeAttributes( bool includeAttributes ) { mIncludeAttributes = includeAttributes; }

    /**
     * Returns whether attributes will be included in the JSON exports.
     * \see setIncludeAttributes()
     */
    bool includeAttributes() const { return mIncludeAttributes; }

    /**
     * Sets whether to include attributes of features linked via references in the JSON exports.
     * \param includeRelated set to TRUE to include attributes for any related child features
     * within the exported properties element.
     * \note associated vector layer must be set with setVectorLayer()
     * \see includeRelated()
     */
    void setIncludeRelated( bool includeRelated ) { mIncludeRelatedAttributes = includeRelated; }

    /**
     * Returns whether attributes of related (child) features will be included in the JSON exports.
     * \see setIncludeRelated()
     */
    bool includeRelated() const { return mIncludeRelatedAttributes; }

    /**
     * Sets whether to print original names of attributes or aliases if
     * defined.
     * \since QGIS 3.6
     */
    void setAttributeDisplayName( bool displayName ) { mAttributeDisplayName = displayName; }

    /**
     * Returns whether original names of attributes or aliases are printed.
     * \since QGIS 3.6
     */

    bool attributeDisplayName() const { return mAttributeDisplayName; }

    /**
     * Sets the associated vector layer (required for related attribute export). This will automatically
     * update the sourceCrs() to match.
     * \param vectorLayer vector layer
     * \see vectorLayer()
     */
    void setVectorLayer( QgsVectorLayer *vectorLayer );

    /**
     * Returns the associated vector layer, if set.
     * \see setVectorLayer()
     */
    QgsVectorLayer *vectorLayer() const;

    /**
     * Sets the source CRS for feature geometries. The source CRS must be set if geometries are to be
     * correctly automatically reprojected to WGS 84, to match GeoJSON specifications.
     * \param crs source CRS for input feature geometries
     * \note the source CRS will be overwritten when a vector layer is specified via setVectorLayer()
     * \see sourceCrs()
     */
    void setSourceCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Returns the source CRS for feature geometries. The source CRS must be set if geometries are to be
     * correctly automatically reprojected to WGS 84, to match GeoJSON specifications.
     * \see setSourceCrs()
     */
    QgsCoordinateReferenceSystem sourceCrs() const;

    /**
     * Sets whether geometries should be transformed in EPSG 4326 (default
     * behavior) or just keep as it is.
     * \since QGIS 3.12
     */
    void setTransformGeometries( bool activate ) { mTransformGeometries = activate; }

    /**
     * Sets the list of attributes to include in the JSON exports.
     * \param attributes list of attribute indexes, or an empty list to include all
     * attributes
     * \see attributes()
     * \see setExcludedAttributes()
     * \note Attributes excluded via setExcludedAttributes() take precedence over
     * attributes specified by this method.
     */
    void setAttributes( const QgsAttributeList &attributes ) { mAttributeIndexes = attributes; }

    /**
     * Returns the list of attributes which will be included in the JSON exports, or
     * an empty list if all attributes will be included.
     * \see setAttributes()
     * \see excludedAttributes()
     * \note Attributes excluded via excludedAttributes() take precedence over
     * attributes returned by this method.
     */
    QgsAttributeList attributes() const { return mAttributeIndexes; }

    /**
     * Sets a list of attributes to specifically exclude from the JSON exports. Excluded attributes
     * take precedence over attributes included via setAttributes().
     * \param attributes list of attribute indexes to exclude
     * \see excludedAttributes()
     * \see setAttributes()
     */
    void setExcludedAttributes( const QgsAttributeList &attributes ) { mExcludedAttributeIndexes = attributes; }

    /**
     * Returns a list of attributes which will be specifically excluded from the JSON exports. Excluded attributes
     * take precedence over attributes included via attributes().
     * \see setExcludedAttributes()
     * \see attributes()
     */
    QgsAttributeList excludedAttributes() const { return mExcludedAttributeIndexes; }

    /**
     * Returns a GeoJSON string representation of a feature.
     * \param feature feature to convert
     * \param extraProperties map of extra attributes to include in feature's properties
     * \param id optional ID to use as GeoJSON feature's ID instead of input feature's ID. If omitted, feature's
     * ID is used.
     * \param indent number of indentation spaces for generated JSON (defaults to none)
     * \returns GeoJSON string
     * \see exportFeatures()
     * \see exportFeatureToJsonObject()
     */
    QString exportFeature( const QgsFeature &feature,
                           const QVariantMap &extraProperties = QVariantMap(),
                           const QVariant &id = QVariant(),
                           int indent = -1 ) const;

    /**
     * Returns a QJsonObject representation of a feature.
     * \param feature feature to convert
     * \param extraProperties map of extra attributes to include in feature's properties
     * \param id optional ID to use as GeoJSON feature's ID instead of input feature's ID. If omitted, feature's
     * ID is used.
     * \returns json object
     * \see exportFeatures()
     */
    json exportFeatureToJsonObject( const QgsFeature &feature,
                                    const QVariantMap &extraProperties = QVariantMap(),
                                    const QVariant &id = QVariant() ) const SIP_SKIP;


    /**
     * Returns a GeoJSON string representation of a list of features (feature collection).
     * \param features features to convert
     * \param indent number of indentation spaces for generated JSON (defaults to none)
     * \returns GeoJSON string
     * \see exportFeature()
     */
    QString exportFeatures( const QgsFeatureList &features, int indent = -1 ) const;

    /**
     * Returns a JSON object representation of a list of features (feature collection).
     * \param features features to convert
     * \returns json object
     * \see exportFeatures()
     * \since QGIS 3.10
     */
    json exportFeaturesToJsonObject( const QgsFeatureList &features ) const SIP_SKIP;

    /**
     * Set the destination CRS for feature geometry transformation to \a destinationCrs, this defaults to EPSG:4326
     * and it is only effective when the automatic geometry transformation is active (it is by default).
     *
     * \see setTransformGeometries()
     * \see  setSourceCrs()
     *
     * \since QGIS 3.30
     */
    void setDestinationCrs( const QgsCoordinateReferenceSystem &destinationCrs );

  private:

    //! Maximum number of decimal places for geometry coordinates
    int mPrecision;

    /**
     * List of attribute indexes to include in export, or empty list to include all attributes
     * \see mExcludedAttributeIndexes
     */
    QgsAttributeList mAttributeIndexes;

    //! List of attribute indexes to exclude from export
    QgsAttributeList mExcludedAttributeIndexes;

    //! Whether to include geometry in JSON export
    bool mIncludeGeometry = true;

    //! Whether to include attributes in JSON export
    bool mIncludeAttributes = true;

    //! Whether to include attributes from related features in JSON export
    bool mIncludeRelatedAttributes = false;

    //! Associated vector layer. Required for related attribute export.
    QPointer< QgsVectorLayer > mLayer;

    QgsCoordinateReferenceSystem mCrs;

    QgsCoordinateTransform mTransform;

    bool mAttributeDisplayName = false;

    bool mTransformGeometries = true;

    QgsCoordinateReferenceSystem mDestinationCrs;
};

/**
 * \ingroup core
 * \class QgsJsonUtils
 * \brief Helper utilities for working with JSON and GeoJSON conversions.
 */

class CORE_EXPORT QgsJsonUtils
{
    Q_GADGET

  public:

    /**
     * Attempts to parse a GeoJSON \a string to a collection of features.
     * It is possible to specify \a fields to parse specific fields, if not provided, no fields will be included.
     * An \a encoding can be specified which defaults to UTF-8 if it is `nullptr`.
     * \returns a list of parsed features, or an empty list if no features could be parsed
     * \see stringToFields()
     * \note this function is a wrapper around QgsOgrUtils::stringToFeatureList()
     */
    static QgsFeatureList stringToFeatureList( const QString &string, const QgsFields &fields = QgsFields(), QTextCodec *encoding SIP_PYARGREMOVE6 = nullptr );

    /**
     * Attempts to retrieve the fields from a GeoJSON  \a string representing a collection of features.
     * An \a encoding can be specified which defaults to UTF-8 if it is `nullptr`.
     * \returns retrieved fields collection, or an empty list if no fields could be determined from the string
     * \see stringToFeatureList()
     * \note this function is a wrapper around QgsOgrUtils::stringToFields()
     */
    static QgsFields stringToFields( const QString &string, QTextCodec *encoding SIP_PYARGREMOVE6 = nullptr );

    /**
     * Encodes a value to a JSON string representation, adding appropriate quotations and escaping
     * where required.
     * \param value value to encode
     * \returns encoded value
     */
    Q_INVOKABLE static QString encodeValue( const QVariant &value );

    /**
     * Exports all attributes from a QgsFeature as a JSON map type.
     * \param feature feature to export
     * \param layer optional associated vector layer. If specified, this allows
     * richer export utilising settings like the layer's fields widget configuration.
     * \param attributeWidgetCaches optional widget configuration cache. Can be used
     * to speed up exporting the attributes for multiple features from the same layer.
     */
    static QString exportAttributes( const QgsFeature &feature, QgsVectorLayer *layer = nullptr,
                                     const QVector<QVariant> &attributeWidgetCaches = QVector<QVariant>() );

    /**
     * Exports all attributes from a QgsFeature as a json object.
     * \param feature feature to export
     * \param layer optional associated vector layer. If specified, this allows
     * richer export utilising settings like the layer's fields widget configuration.
     * \param attributeWidgetCaches optional widget configuration cache. Can be used
     * to speed up exporting the attributes for multiple features from the same layer.
     * \note Not available in Python bindings
     * \since QGIS 3.8
     */
    static json exportAttributesToJsonObject( const QgsFeature &feature, QgsVectorLayer *layer = nullptr,
        const QVector<QVariant> &attributeWidgetCaches = QVector<QVariant>() ) SIP_SKIP;

    /**
     * Parse a simple array (depth=1)
     * \param json the JSON to parse
     * \param type optional variant type of the elements, if specified (and not Invalid),
     *        the array items will be converted to the type, and discarded if
     *        the conversion is not possible.
     */
    Q_INVOKABLE static QVariantList parseArray( const QString &json, QVariant::Type type = QVariant::Invalid );

    /**
     * Parses a GeoJSON "geometry" value to a QgsGeometry object.
     *
     * Returns a null geometry if the geometry could not be parsed.
     *
     * \note Not available in Python bindings.
     * \since QGIS 3.36
     */
    static QgsGeometry geometryFromGeoJson( const json &geometry ) SIP_SKIP;

    /**
     * Parses a GeoJSON "geometry" value to a QgsGeometry object.
     *
     * Returns a null geometry if the geometry could not be parsed.
     *
     * \since QGIS 3.36
     */
    static QgsGeometry geometryFromGeoJson( const QString &geometry );

    /**
     * Converts a QVariant \a v to a json object
     * \note Not available in Python bindings
     * \since QGIS 3.8
     */
    static json jsonFromVariant( const QVariant &v ) SIP_SKIP;

    /**
     * Converts JSON \a jsonString to a QVariant, in case of parsing error an invalid QVariant is returned and an
     * error is logged to the message log.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.8
     */
    static QVariant parseJson( const std::string &jsonString ) SIP_SKIP;

    /**
     * Converts JSON \a jsonString to a QVariant, in case of parsing error an invalid QVariant is returned
     * and the \a error argument is populated accordingly.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.24
     */
    static QVariant parseJson( const std::string &jsonString, QString &error ) SIP_SKIP;

    /**
     * Converts JSON \a jsonString to a QVariant, in case of parsing error an invalid QVariant is returned.
     * \note Not available in Python bindings
     * \since QGIS 3.8
     */
    static QVariant parseJson( const QString &jsonString ) SIP_SKIP;

    /**
     * Converts a JSON \a value to a QVariant, in case of parsing error an invalid QVariant is returned.
     * \note Not available in Python bindings
     * \since QGIS 3.36
     */
    static QVariant jsonToVariant( const json &value ) SIP_SKIP;

    /**
     * Add \a crs information entry in \a json object regarding old GeoJSON specification format
     * if it differs from OGC:CRS84 or EPSG:4326.
     * According to new specification RFC 7946, coordinate reference system for all GeoJSON coordinates
     * is assumed to be OGC:CRS84 but when user specifically request a different CRS, this method
     * adds this information in the JSON output
     */
    static void addCrsInfo( json &value, const QgsCoordinateReferenceSystem &crs ) SIP_SKIP;

};

#endif // QGSJSONUTILS_H
