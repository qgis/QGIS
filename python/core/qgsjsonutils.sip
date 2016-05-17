/** \ingroup core
 * \class QgsJSONExporter
 * \brief Handles exporting QgsFeature features to GeoJSON features.
 *
 * Note that geometries will be automatically reprojected to WGS84 to match GeoJSON spec
 * if either the source vector layer or source CRS is set.
 * \note Added in version 2.16
 */

class QgsJSONExporter
{
%TypeHeaderCode
#include <qgsjsonutils.h>
%End
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
    void setPrecision( int precision );

    /** Returns the maximum number of decimal places to use in geometry coordinates.
     * @see setPrecision()
     */
    int precision() const;

    /** Sets whether to include geometry in the JSON exports.
     * @param includeGeometry set to false to prevent geometry inclusion
     * @see includeGeometry()
     */
    void setIncludeGeometry( bool includeGeometry );

    /** Returns whether geometry will be included in the JSON exports.
     * @see setIncludeGeometry()
     */
    bool includeGeometry() const;

    /** Sets whether to include attributes in the JSON exports.
     * @param includeAttributes set to false to prevent attribute inclusion
     * @see includeAttributes()
     */
    void setIncludeAttributes( bool includeAttributes );

    /** Returns whether attributes will be included in the JSON exports.
     * @see setIncludeAttributes()
     */
    bool includeAttributes() const;

    /** Sets whether to include attributes of features linked via references in the JSON exports.
     * @param includeRelated set to true to include attributes for any related child features
     * within the exported properties element.
     * @note associated vector layer must be set with setVectorLayer()
     * @see includeRelated()
     */
    void setIncludeRelated( bool includeRelated );

    /** Returns whether attributes of related (child) features will be included in the JSON exports.
     * @see setIncludeRelated()
     */
    bool includeRelated() const;

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
    void setAttributes( const QgsAttributeList& attributes );

    /** Returns the list of attributes which will be included in the JSON exports, or
     * an empty list if all attributes will be included.
     * @see setAttributes()
     * @see excludedAttributes()
     * @note Attributes excluded via excludedAttributes() take precedence over
     * attributes returned by this method.
     */
    QgsAttributeList attributes() const;

    /** Sets a list of attributes to specifically exclude from the JSON exports. Excluded attributes
     * take precedence over attributes included via setAttributes().
     * @param attributes list of attribute indexes to exclude
     * @see excludedAttributes()
     * @see setAttributes()
     */
    void setExcludedAttributes( const QgsAttributeList& attributes );

    /** Returns a list of attributes which will be specifically excluded from the JSON exports. Excluded attributes
     * take precedence over attributes included via attributes().
     * @see setExcludedAttributes()
     * @see attributes()
     */
    QgsAttributeList excludedAttributes() const;

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

    QgsJSONExporter( const QgsJSONExporter& );

};


/** \ingroup core
 * \class QgsJSONUtils
 * \brief Helper utilities for working with JSON and GeoJSON conversions.
 * \note Added in version 2.16
 */

class QgsJSONUtils
{
%TypeHeaderCode
#include <qgsjsonutils.h>
%End

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
