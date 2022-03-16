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

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgsfeature.h"

#include <ogr_api.h>
#include <gdal.h>
#include <gdalwarper.h>
#include "cpl_conv.h"
#include "cpl_string.h"

class QgsCoordinateReferenceSystem;
class QgsFieldDomain;

class QTextCodec;

namespace gdal
{

  /**
   * Destroys OGR data sources.
   */
  struct OGRDataSourceDeleter
  {

    /**
     * Destroys an OGR data \a source, using the correct gdal calls.
     */
    void CORE_EXPORT operator()( OGRDataSourceH source );

  };

  /**
   * Destroys OGR geometries.
   */
  struct OGRGeometryDeleter
  {

    /**
     * Destroys an OGR \a geometry, using the correct gdal calls.
     */
    void CORE_EXPORT operator()( OGRGeometryH geometry );

  };

  /**
   * Destroys OGR field definition.
   */
  struct OGRFldDeleter
  {

    /**
     * Destroys an OGR field \a definition, using the correct gdal calls.
     */
    void CORE_EXPORT operator()( OGRFieldDefnH definition );

  };

  /**
   * Destroys OGR feature.
   */
  struct OGRFeatureDeleter
  {

    /**
     * Destroys an OGR \a feature, using the correct gdal calls.
     */
    void CORE_EXPORT operator()( OGRFeatureH feature );

  };

  /**
   * Closes and cleanups GDAL dataset.
   */
  struct GDALDatasetCloser
  {

    /**
     * Destroys an gdal \a dataset, using the correct gdal calls.
     */
    void CORE_EXPORT operator()( GDALDatasetH datasource );

  };

  /**
   * Closes and cleanups GDAL warp options.
   */
  struct GDALWarpOptionsDeleter
  {

    /**
     * Destroys GDAL warp \a options, using the correct gdal calls.
     */
    void CORE_EXPORT operator()( GDALWarpOptions *options );

  };

  /**
   * Scoped OGR data source.
   */
  using ogr_datasource_unique_ptr = std::unique_ptr< std::remove_pointer<OGRDataSourceH>::type, OGRDataSourceDeleter >;

  /**
   * Scoped OGR geometry.
   */
  using ogr_geometry_unique_ptr = std::unique_ptr< std::remove_pointer<OGRGeometryH>::type, OGRGeometryDeleter >;

  /**
   * Scoped OGR field definition.
   */
  using ogr_field_def_unique_ptr = std::unique_ptr< std::remove_pointer<OGRFieldDefnH>::type, OGRFldDeleter >;

  /**
   * Scoped OGR feature.
   */
  using ogr_feature_unique_ptr = std::unique_ptr< std::remove_pointer<OGRFeatureH>::type, OGRFeatureDeleter >;

  /**
   * Scoped GDAL dataset.
   */
  using dataset_unique_ptr = std::unique_ptr< std::remove_pointer<GDALDatasetH>::type, GDALDatasetCloser >;

  /**
   * Performs a fast close of an unwanted GDAL dataset handle by deleting the underlying
   * data store. Use when the resultant dataset is no longer required, e.g. as a result
   * of user cancellation of an operation.
   *
   * Requires a gdal \a dataset pointer, the corresponding gdal \a driver and underlying
   * dataset file \a path.
   */
  void CORE_EXPORT fast_delete_and_close( dataset_unique_ptr &dataset, GDALDriverH driver, const QString &path );

  /**
   * Scoped GDAL warp options.
   */
  using warp_options_unique_ptr = std::unique_ptr< GDALWarpOptions, GDALWarpOptionsDeleter >;
}

/**
 * \ingroup core
 * \class QgsOgrUtils
 * \brief Utilities for working with OGR features and layers
 *
 * Contains helper utilities for assisting work with both OGR features and layers.
 * \note not available in Python bindings
 * \since QGIS 2.16
 */
class CORE_EXPORT QgsOgrUtils
{
  public:

    /**
     * Converts an OGRField \a value of the specified \a type into a QVariant.
     * \since QGIS 3.20
     */
    static QVariant OGRFieldtoVariant( const OGRField *value, OGRFieldType type );

    /**
     * Converts a QVariant to an OGRField value.
     *
     * \since QGIS 3.26
     */
    static std::unique_ptr<OGRField> variantToOGRField( const QVariant &value );

    /**
     * Reads an OGR feature and converts it to a QgsFeature.
     * \param ogrFet OGR feature handle
     * \param fields fields collection corresponding to feature
     * \param encoding text encoding
     * \returns valid feature if read was successful
     */
    static QgsFeature readOgrFeature( OGRFeatureH ogrFet, const QgsFields &fields, QTextCodec *encoding );

    /**
     * Reads an OGR feature and returns a corresponding fields collection.
     * \param ogrFet OGR feature handle
     * \param encoding text encoding
     * \returns fields collection if read was successful
     */
    static QgsFields readOgrFields( OGRFeatureH ogrFet, QTextCodec *encoding );

    /**
     * Retrieves an attribute value from an OGR feature.
     * \param ogrFet OGR feature handle
     * \param fields fields collection corresponding to feature
     * \param attIndex index of attribute to retrieve
     * \param encoding text encoding
     * \param ok optional storage for success of retrieval
     * \returns attribute converted to a QVariant object
     * \see readOgrFeatureAttributes()
     */
    static QVariant getOgrFeatureAttribute( OGRFeatureH ogrFet, const QgsFields &fields, int attIndex, QTextCodec *encoding, bool *ok = nullptr );

    /**
     * Retrieves an attribute value from an OGR feature, using a provided \a field definition.
     * \param ogrFet OGR feature handle
     * \param field definition of corresponding field
     * \param attIndex index of attribute to retrieve from \a ogrFet
     * \param encoding text encoding
     * \param ok optional storage for success of retrieval
     * \returns attribute converted to a QVariant object
     * \see readOgrFeatureAttributes()
     *
     * \since QGIS 3.10.1
     */
    static QVariant getOgrFeatureAttribute( OGRFeatureH ogrFet, const QgsField &field, int attIndex, QTextCodec *encoding, bool *ok = nullptr );

    /**
     * Reads all attributes from an OGR feature into a QgsFeature.
     * \param ogrFet OGR feature handle
     * \param fields fields collection corresponding to feature
     * \param feature QgsFeature to store attributes in
     * \param encoding text encoding
     * \returns TRUE if attribute read was successful
     * \see getOgrFeatureAttribute()
     */
    static bool readOgrFeatureAttributes( OGRFeatureH ogrFet, const QgsFields &fields, QgsFeature &feature, QTextCodec *encoding );

    /**
     * Reads the geometry from an OGR feature into a QgsFeature.
     * \param ogrFet OGR feature handle
     * \param feature QgsFeature to store geometry in
     * \returns TRUE if geometry read was successful
     * \see readOgrFeatureAttributes()
     * \see ogrGeometryToQgsGeometry()
     */
    static bool readOgrFeatureGeometry( OGRFeatureH ogrFet, QgsFeature &feature );

    /**
     * Converts an OGR geometry representation to a QgsGeometry object
     * \param geom OGR geometry handle
     * \returns QgsGeometry object. If conversion was not successful the geometry
     * will be empty.
     * \see readOgrFeatureGeometry()
     */
    static QgsGeometry ogrGeometryToQgsGeometry( OGRGeometryH geom );

    /**
     * Attempts to parse a string representing a collection of features using OGR. For example, this method can be
     * used to convert a GeoJSON encoded collection to a list of QgsFeatures.
     * \param string string to parse
     * \param fields fields collection to use for parsed features (\see stringToFields())
     * \param encoding text encoding
     * \returns list of parsed features, or an empty list if no features could be parsed
     * \see stringToFields()
     */
    static QgsFeatureList stringToFeatureList( const QString &string, const QgsFields &fields, QTextCodec *encoding );

    /**
     * Attempts to retrieve the fields from a string representing a collection of features using OGR.
     * \param string string to parse
     * \param encoding text encoding
     * \returns retrieved fields collection, or an empty list if no fields could be determined from the string
     * \see stringToFeatureList()
     */
    static QgsFields stringToFields( const QString &string, QTextCodec *encoding );

    /**
     * Converts a c string list to a QStringList. Presumes a null terminated string list.
     *
     * \since QGIS 3.2
     */
    static QStringList cStringListToQStringList( char **stringList );

    /**
     * Converts a OGRwkbGeometryType to QgsWkbTypes::Type
     *
     * \since QGIS 3.4.9
     */
    static QgsWkbTypes::Type ogrGeometryTypeToQgsWkbType( OGRwkbGeometryType ogrGeomType );

    /**
     * Returns a WKT string corresponding to the specified OGR \a srs object.
     *
     * The WKT string format will be selected using the most appropriate format (usually WKT2 if GDAL 3 is available).
     *
     * \since QGIS 3.10.1
     */
    static QString OGRSpatialReferenceToWkt( OGRSpatialReferenceH srs );

    /**
     * Returns a QgsCoordinateReferenceSystem corresponding to the specified OGR \a srs object, or an invalid
     * QgsCoordinateReferenceSystem if \a srs could not be converted.
     *
     * \since QGIS 3.10.1
     */
    static QgsCoordinateReferenceSystem OGRSpatialReferenceToCrs( OGRSpatialReferenceH srs );

    /**
     * Returns a OGRSpatialReferenceH corresponding to the specified \a crs object.
     *
     * \note Caller must release the returned object with OSRRelease.
     *
     * \since QGIS 3.22
     */
    static OGRSpatialReferenceH crsToOGRSpatialReference( const QgsCoordinateReferenceSystem &crs );

    /**
     * Reads the encoding of the shapefile at the specified \a path (where \a path is the
     * location of the ".shp" file).
     *
     * This method considers both the CPG specified encoding and the DBF LDID encoding
     * (priority goes to CPG based encoding)
     *
     * \see readShapefileEncodingFromCpg()
     * \see readShapefileEncodingFromLdid()
     * \since QGIS 3.12
     */
    static QString readShapefileEncoding( const QString &path );

    /**
     * Reads the encoding of the shapefile at the specified \a path (where \a path is the
     * location of the ".shp" file), from the CPG specified encoding.
     *
     * Return an empty string if CPG based encoding was not found.
     *
     * \see readShapefileEncoding()
     * \since QGIS 3.12
     */
    static QString readShapefileEncodingFromCpg( const QString &path );

    /**
     * Reads the encoding of the shapefile at the specified \a path (where \a path is the
     * location of the ".shp" file), from the DBF LDID encoding.
     *
     * Return an empty string if LDID based encoding was not found.
     *
     * \see readShapefileEncoding()
     * \since QGIS 3.12
     */
    static QString readShapefileEncodingFromLdid( const QString &path );

    /**
     * Parses an OGR style \a string to a variant map containing the style string components.
     *
     * \since QGIS 3.20
     */
    static QVariantMap parseStyleString( const QString &string );

    /**
     * Creates a new QgsSymbol matching an OGR style \a string.
     *
     * \since QGIS 3.20
     */
    static std::unique_ptr< QgsSymbol > symbolFromStyleString( const QString &string, Qgis::SymbolType type ) SIP_FACTORY;

    /**
     * Converts an OGR field type and sub type to the best matching QVariant::Type equivalent.
     *
     * \param ogrType OGR field type
     * \param ogrSubType OGR field sub type
     * \param variantType will be set to matching QVariant type
     * \param variantSubType will be set to matching QVariant sub type, for list, map and other complex OGR field types.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.26
     */
    static void ogrFieldTypeToQVariantType( OGRFieldType ogrType, OGRFieldSubType ogrSubType, QVariant::Type &variantType, QVariant::Type &variantSubType ) SIP_SKIP;

    /**
     * Converts an QVariant type to the best matching OGR field type and sub type.
     *
     * \param variantType QVariant field type
     * \param ogrType will be set to matching OGR type
     * \param ogrSubType will be set to matching OGR sub type
     *
     * \note Not available in Python bindings
     * \since QGIS 3.26
     */
    static void variantTypeToOgrFieldType( QVariant::Type variantType, OGRFieldType &ogrType, OGRFieldSubType &ogrSubType ) SIP_SKIP;

    /**
     * Converts a string to a variant, using the provider OGR field \a type and \a subType to determine the most appropriate
     * variant type.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.26
     */
    static QVariant stringToVariant( OGRFieldType type, OGRFieldSubType subType, const QString &string ) SIP_SKIP;

#ifndef SIP_RUN
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,3,0)

    /**
     * Converts an OGR field domain definition to a QgsFieldDomain equivalent.
     *
     * \note Requires GDAL >= 3.3
     * \note Not available in Python bindings
     * \since QGIS 3.26
     */
    static std::unique_ptr< QgsFieldDomain > convertFieldDomain( OGRFieldDomainH domain );

    /**
     * Converts a QGIS field domain definition to an OGR field domain equivalent.
     *
     * \note Requires GDAL >= 3.3
     * \note Not available in Python bindings
     * \since QGIS 3.26
     */
    static OGRFieldDomainH convertFieldDomain( const QgsFieldDomain *domain );
#endif
#endif
};

#endif // QGSOGRUTILS_H
