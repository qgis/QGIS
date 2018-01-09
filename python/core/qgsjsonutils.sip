/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsjsonutils.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsJsonExporter
{
%Docstring
 Handles exporting QgsFeature features to GeoJSON features.

Note that geometries will be automatically reprojected to WGS84 to match GeoJSON spec
if either the source vector layer or source CRS is set.

.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgsjsonutils.h"
%End
  public:

    QgsJsonExporter( QgsVectorLayer *vectorLayer = 0, int precision = 6 );
%Docstring
Constructor for QgsJsonExporter.

:param vectorLayer: associated vector layer (required for related attribute export)
:param precision: maximum number of decimal places to use for geometry coordinates,
the RFC 7946 GeoJSON specification recommends limiting coordinate precision to 6
%End

    void setPrecision( int precision );
%Docstring
Sets the maximum number of decimal places to use in geometry coordinates.
The RFC 7946 GeoJSON specification recommends limiting coordinate precision to 6

:param precision: number of decimal places

.. seealso:: :py:func:`precision`
%End

    int precision() const;
%Docstring
Returns the maximum number of decimal places to use in geometry coordinates.

.. seealso:: :py:func:`setPrecision`
%End

    void setIncludeGeometry( bool includeGeometry );
%Docstring
Sets whether to include geometry in the JSON exports.

:param includeGeometry: set to false to prevent geometry inclusion

.. seealso:: :py:func:`includeGeometry`
%End

    bool includeGeometry() const;
%Docstring
Returns whether geometry will be included in the JSON exports.

.. seealso:: :py:func:`setIncludeGeometry`
%End

    void setIncludeAttributes( bool includeAttributes );
%Docstring
Sets whether to include attributes in the JSON exports.

:param includeAttributes: set to false to prevent attribute inclusion

.. seealso:: :py:func:`includeAttributes`
%End

    bool includeAttributes() const;
%Docstring
Returns whether attributes will be included in the JSON exports.

.. seealso:: :py:func:`setIncludeAttributes`
%End

    void setIncludeRelated( bool includeRelated );
%Docstring
Sets whether to include attributes of features linked via references in the JSON exports.

:param includeRelated: set to true to include attributes for any related child features
within the exported properties element.

.. note::

   associated vector layer must be set with setVectorLayer()

.. seealso:: :py:func:`includeRelated`
%End

    bool includeRelated() const;
%Docstring
Returns whether attributes of related (child) features will be included in the JSON exports.

.. seealso:: :py:func:`setIncludeRelated`
%End

    void setVectorLayer( QgsVectorLayer *vectorLayer );
%Docstring
Sets the associated vector layer (required for related attribute export). This will automatically
update the sourceCrs() to match.

:param vectorLayer: vector layer

.. seealso:: :py:func:`vectorLayer`
%End

    QgsVectorLayer *vectorLayer() const;
%Docstring
Returns the associated vector layer, if set.

.. seealso:: :py:func:`setVectorLayer`
%End

    void setSourceCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
Sets the source CRS for feature geometries. The source CRS must be set if geometries are to be
correctly automatically reprojected to WGS 84, to match GeoJSON specifications.

:param crs: source CRS for input feature geometries

.. note::

   the source CRS will be overwritten when a vector layer is specified via setVectorLayer()

.. seealso:: :py:func:`sourceCrs`
%End

    QgsCoordinateReferenceSystem sourceCrs() const;
%Docstring
Returns the source CRS for feature geometries. The source CRS must be set if geometries are to be
correctly automatically reprojected to WGS 84, to match GeoJSON specifications.

.. seealso:: :py:func:`setSourceCrs`
%End

    void setAttributes( const QgsAttributeList &attributes );
%Docstring
Sets the list of attributes to include in the JSON exports.

:param attributes: list of attribute indexes, or an empty list to include all
attributes

.. seealso:: :py:func:`attributes`

.. seealso:: :py:func:`setExcludedAttributes`

.. note::

   Attributes excluded via setExcludedAttributes() take precedence over
attributes specified by this method.
%End

    QgsAttributeList attributes() const;
%Docstring
Returns the list of attributes which will be included in the JSON exports, or
an empty list if all attributes will be included.

.. seealso:: :py:func:`setAttributes`

.. seealso:: :py:func:`excludedAttributes`

.. note::

   Attributes excluded via excludedAttributes() take precedence over
attributes returned by this method.
%End

    void setExcludedAttributes( const QgsAttributeList &attributes );
%Docstring
Sets a list of attributes to specifically exclude from the JSON exports. Excluded attributes
take precedence over attributes included via setAttributes().

:param attributes: list of attribute indexes to exclude

.. seealso:: :py:func:`excludedAttributes`

.. seealso:: :py:func:`setAttributes`
%End

    QgsAttributeList excludedAttributes() const;
%Docstring
Returns a list of attributes which will be specifically excluded from the JSON exports. Excluded attributes
take precedence over attributes included via attributes().

.. seealso:: :py:func:`setExcludedAttributes`

.. seealso:: :py:func:`attributes`
%End

    QString exportFeature( const QgsFeature &feature,
                           const QVariantMap &extraProperties = QVariantMap(),
                           const QVariant &id = QVariant() ) const;
%Docstring
Returns a GeoJSON string representation of a feature.

:param feature: feature to convert
:param extraProperties: map of extra attributes to include in feature's properties
:param id: optional ID to use as GeoJSON feature's ID instead of input feature's ID. If omitted, feature's
ID is used.

:return: GeoJSON string

.. seealso:: :py:func:`exportFeatures`
%End


    QString exportFeatures( const QgsFeatureList &features ) const;
%Docstring
Returns a GeoJSON string representation of a list of features (feature collection).

:param features: features to convert

:return: GeoJSON string

.. seealso:: :py:func:`exportFeature`
%End

};


class QgsJsonUtils
{
%Docstring
 Helper utilities for working with JSON and GeoJSON conversions.

.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgsjsonutils.h"
%End
  public:

    static QgsFeatureList stringToFeatureList( const QString &string, const QgsFields &fields, QTextCodec *encoding );
%Docstring
Attempts to parse a GeoJSON string to a collection of features.

:param string: GeoJSON string to parse
:param fields: fields collection to use for parsed features
:param encoding: text encoding

:return: list of parsed features, or an empty list if no features could be parsed

.. seealso:: :py:func:`stringToFields`

.. note::

   this function is a wrapper around :py:func:`QgsOgrUtils.stringToFeatureList()`
%End

    static QgsFields stringToFields( const QString &string, QTextCodec *encoding );
%Docstring
Attempts to retrieve the fields from a GeoJSON string representing a collection of features.

:param string: GeoJSON string to parse
:param encoding: text encoding

:return: retrieved fields collection, or an empty list if no fields could be determined from the string

.. seealso:: :py:func:`stringToFeatureList`

.. note::

   this function is a wrapper around :py:func:`QgsOgrUtils.stringToFields()`
%End

    static QString encodeValue( const QVariant &value );
%Docstring
Encodes a value to a JSON string representation, adding appropriate quotations and escaping
where required.

:param value: value to encode

:return: encoded value
%End

    static QString exportAttributes( const QgsFeature &feature, QgsVectorLayer *layer = 0,
                                     const QVector<QVariant> &attributeWidgetCaches = QVector<QVariant>() );
%Docstring
Exports all attributes from a QgsFeature as a JSON map type.

:param feature: feature to export
:param layer: optional associated vector layer. If specified, this allows
richer export utilising settings like the layer's fields widget configuration.
:param attributeWidgetCaches: optional widget configuration cache. Can be used
to speed up exporting the attributes for multiple features from the same layer.
%End

    static QVariantList parseArray( const QString &json, QVariant::Type type );
%Docstring
Parse a simple array (depth=1).

:param json: the JSON to parse
:param type: the type of the elements

.. versionadded:: 3.0
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsjsonutils.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
