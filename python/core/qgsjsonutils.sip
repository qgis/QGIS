/** \ingroup core
 * \class QgsJSONExporter
 * \brief Handles exporting QgsFeature features to GeoJSON features.
 * \note Added in version 2.16
 */

class QgsJSONExporter
{
%TypeHeaderCode
#include <qgsjsonutils.h>
%End
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

    /** Sets the list of attributes to include in the JSON exports.
     * @param attributes list of attribute indexes, or an empty list to include all
     * attributes
     * @see attributes()
     */
    void setAttributes( const QgsAttributeList& attributes );

    /** Returns the list of attributes which will be included in the JSON exports, or
     * an empty list if all attributes will be included.
     * @see setAttributes()
     */
    QgsAttributeList attributes() const;

    /** Returns a GeoJSON string representation of a feature.
     * @param feature feature to convert
     * @param id optional ID to use as GeoJSON feature's ID instead of input feature's ID. If omitted, feature's
     * ID is used.
     * @returns GeoJSON string
     */
    QString exportFeature( const QgsFeature& feature,
                           const QVariant& id = QVariant() ) const;

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
};
