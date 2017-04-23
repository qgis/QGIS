/***************************************************************************
            qgsogrproviderutils.h Data provider for ESRI shapefile format
                    Formerly known as qgsshapefileprovider.h
begin                : Oct 29, 2003
copyright            : (C) 2003 by Gary E.Sherman
email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOGRPROVIDERUTILS_H
#define QGSOGRPROVIDERUTILS_H

#include <ogr_api.h>

class QgsOgrProviderUtils
{
  public:

    /**
     * QgsOgrProviderUtils Function to use to read Layer/Geometry
     *
     * \see QgsOgrProviderUtils::OGRGetLayerNameWrapper()
     * \see QgsOgrProviderUtils::OGRGetLayerIndexWrapper()
     * \see QgsOgrProviderUtils::OGRGetGeometryFeatureWrapper()
     * \since QGIS 3.0
     */
    enum OgrGetType
    {
      UnknownGetType = -1,
      LayerNameType = 0,
      LayerIndexType = 1,
      LayerGeometryIndexType = 2
    };
    static void setRelevantFields( OGRLayerH ogrLayer, int fieldCount, bool fetchGeometry, const QgsAttributeList &fetchAttributes, bool firstAttrIsFid );
    static OGRLayerH setSubsetString( OGRLayerH layer, OGRDataSourceH ds, QTextCodec *encoding, const QString &subsetString );
    static QByteArray quotedIdentifier( QByteArray field, const QString &ogrDriverName );

    /** Return OGR geometry type
     * Retrieves Geometry-Type
     * \note     *
     * Up to Gdal 2.* each geometry was 1 layer and could be retrieved using an index
     * Starting with Gdal 2.0, each table is one layer, that can contain more than 1 geometry
     * - Spatialite and GML
     * Care should be taken when using this with Gdal 2.* and more than 1 geometry exists.
     *
     * Some ogr drivers (e.g. GML,KML) are not able to determinee the geometry type of a layer
     * 'subLayers()' will call this function in such cases.
     * \see QgsOgrProvider::subLayers()
     *
     * Logic changes:
     * The Layer name will be used to insure that the correct feature is being searched for
     * - avoiding a LINESTRING being set as a POLYGON, when a POLYGON was found first
     * Some ogr drivers (KML) may contain different Geometry-Types in 1 Layer
     * - the Geometry Field Index will be used to retrieve the proper Geometry-Type
     * \note
     * Some ogr drivers (GML)  must read the first row to determine the GeometryType
     * - some may contain more than 1 geometry
     * \param ogrLayer Layer containing the Geometry
     * \param geomIndex Geometry-index in Layer to be retrieved
     * \see OGRGetSubLayersWrapper()
     * \see QgsOgrProvider::loadFields()
     * \see QgsOgrLayerItem *dataItemForLayer()
     * \returns geomType OGRwkbGeometryType
     * \since QGIS 3.0
     */
    static OGRwkbGeometryType getOgrGeomType( OGRLayerH ogrLayer, int geomIndex );
    static QString wkbGeometryTypeName( OGRwkbGeometryType type );
    static OGRwkbGeometryType wkbGeometryTypeFromName( const QString &typeName ) ;
    //! Get single flatten geometry type
    static OGRwkbGeometryType wkbSingleFlattenWrapper( OGRwkbGeometryType type );
    static OGRwkbGeometryType wkbFlattenWrapper( OGRwkbGeometryType eType );

    /** Retrieves list of valid sublayers of the Dataset, using different Gdal versions (compile,runtime) pre 2.0 and after
     * Sub-layers handled by this provider, in order from bottom to top
     * \param ogrDataSource Data Source
     * \returns subLayers list of valid sublayers of the Dataset
     * \note
     * Logic changes:
     * For non-Database-drivers, such as GML,KML
     * - duplicate layer-names can exist, therefore all layers must be listed to check againt duplicate Layer-Names
     *-- the skipping-logic, when searching for a specific layer has been removed.
     * The created SubLayer string now has 6 fields (ogr_get_type was added)
     * - layer_id:layer_name:feature_count:geometry_type:geometry_name:field_geometry_id:ogr_get_type
     * -- ogr_get_type=0: OGRGetLayerNameWrapper() ; 1: OGRGetLayerIndexWrapper
     * Goal: avoid (the costly) calling of this function when not needed
     * - QgsOgrProvider::OGRGetLayerWrapper should be able to open a Layer without any further checks
     * -- if not [caused possibly by an external change of the DataSource], it will re-read the DataSource and attempt to resove the issue
     * \see QgsOgrProvider::OGRGetLayerWrapper()
     * ogr_get_type: removal and replacement of layer_id with -1 (layer-name is unique)
     * - is done after the User has selected the Layers to be loaded
     * - QgsOgrProvider will not receive this value, but will assume when:
     * - layer_id > = 0 : The Layer-Id must be used because the Layer-Name is not unique
     * \see QgsSublayersDialog::LayerDefinitionList QgsSublayersDialog::selection()
     * For cases where the geometry type could not be reliably determined
     * - all the features of the Layer where checked until something was found
     * -- with Gdal 2.* (where more than 1 geometry can exist for a Layer),
     * -- also with KML where different types of geometries can exist
     * --> the first found setting would be taken, making a LINESTRING to become a POLYGON and thus causing confusion
     * - this has been replaced by calling the existing 'getOgrGeomType', which will determine the geometry type correctly
     * \see QgsOgrProvider::getOgrGeomType()
     * the promotion of CurvePolygons to Polygons, CompoundCurves to LineStrings and CircularStrings
     * - was not documented in a way that was understable and removed
     * -- if this is not being delt with correctly, should be added inside 'getOgrGeomType'
     * OGRGetLayerWrapper will now call this function during the first call in QgsOgrProvider::open
     * \see QgsOgrProvider::OGRGetLayerWrapper()
     * \see QgsOgrProvider::open()
     *
     * When running with Gdal 2.*
     * -  'SpatialViews' in spatialite,  cannot be determined, thus invalid layers will turn up
     * \since QGIS 3.0
     */
    static QStringList OGRGetSubLayersWrapper( OGRDataSourceH ogrDataSource );

    /** Returns Sub-layer string, using different Gdal versions (compile,runtime) pre 2.0 and after
     * Checks Sub-layers for duplicate entries,
     * \param sLayerName name of Layer to be opened
     * \param iLayerIndex index of Layer to be opened
     * \param listSubLayers List of SubLayers created with OGRGetSubLayerStringWrapper
     * \returns sSubLayerString adapted sublayer-string to be used by  OGRGetSubLayerStringWrapper
     * \note
     *
     * Terminolgy: Databases: 'table_name(geometry_column_name)' OGR: 'layer_name(feature_name)'
     * Only in cases where the table/layer has more than 1 geometry column/feature
     * - must the geometry_column/feature name be supplied
     * - in such cases retrieving a layer by name must be used, using the 'layer_name(feature_name)' syntax.
     * In cases where the table/layer has one 1 geometry column/feature
     * - must also retrieve a layer by name  using the 'layer_name' syntax. (ogr_get_type=0)
     * \see OGRGetLayerNameWrapper()
     * For non-Database-drivers, such as GML,KML
     * - duplicate layer-names can exist
     * - in such cases retrieving a layer by index must be used (ogr_get_type=1)
     * \see OGRGetLayerIndexWrapper()
     * 'iLayerIndex' will be changed by this function
     * - if the Layer-Name is unique and the index returned by OGR is different than the given value
     * -- the changed value will be stored in QgsOgrProvider
     * \see QgsOgrProvider::OGRGetLayerWrapper()
     * - Layer without geometries can exist, but will be ignored
     * - Layer with a defined geometry, but is empty (no rows) , will be loaded so that new geometries can be added
     * \since QGIS 3.0
     */
    static QString OGRGetSubLayerWrapper( QString sLayerName, long &lLayerIndex, const QStringList &listSubLayers );

    /** Retrieves OgrLayer with an layer-name, using different Gdal versions (compile,runtime) pre 2.0 and after
     * \param ogrDataSource Data Source
     * \param sSubLayerString name of Layer
     * \returns OGRLayerH if found
     * \note
     *
     * Up to Gdal 2.* each geometry was 1 layer and could be retrieved using an index
     * Starting with Gdal 2.0, each table is one layer, that can contain more than 1 geometry
     * - OGR_DS_GetLayerByName has been deprecated, but is available
     * Retrieving with an index should be avoided where praticable
     * \see OGRGetLayerIndexWrapper()
     * For drivers such as sqlite (spatialite), that may contain more than one geometry,
     * retrieving layer by index must NOT be used.
     * Starting with Gdal 2.0, a special syntax must be used for such geometries
     *  - 'table_name(field_name)'
     *  - 'field_name': may be empty (GML,KML) and is NOT needed for tables with only 1 geometry
     * \see QgsOgrProvider::subLayers()
     *
     * If the given Layer-Name is found and is unique, OGRGetLayerNameWrapper will be called
     * - if the loading failed, an attempt will be made to load the layer with OGRGetLayerIndexWrapper
     * \see QgsOgrProviderUtils::OGRGetLayerNameWrapper()
     *
     * If the given Layer-Name is found and is not unique, OGRGetLayerIndexWrapper will be called
     * - checking is done that the given index is valid, if not will be replaced by the value returned by 'subLayers'
     * -- this may happen if the id is stored in a project and the Layer has been externaly changed. OGR may return a different id.
     * \see QgsOgrProviderUtils::OGRGetLayerIndexWrapper()
     *
     * If Layer has been successfully loaded, the used values will be stored in 'mSubLayerString'
     * - if 'OGRGetLayerNameWrapper' was used to successfully load the Layer, the index value in 'mSubLayerString' will be set to '-1'
     * \see mSubLayerString
     *
     * If this is being called after the initial calling
     * - 'mSubLayerString' will be read: field 0=layer-id, field 1: layer-name
     * If Layer-id >= 0:  will QgsOgrProviderUtils::OGRGetLayerIndexWrapper() be called
     * If Layer-id < 0:  will QgsOgrProviderUtils::OGRGetLayerNameWrapper() be called
     *
     * Since this function cannot be called from another class (QgsOgrProvider being 'const')
     * - 'mSubLayerString' should be read to determine which QgsOgrProviderUtils function should be called.
     * \see QgsOgrFeatureIterator::QgsOgrFeatureIterator
     * \since QGIS 3.0
     */
    static OGRLayerH OGRGetSubLayerStringWrapper( OGRDataSourceH ogrDataSource, QString sSubLayerString );

    /** General Parse function SubLayerString for reading and writing
     * \note
     *  If subLayerString is empty, the given values will be used to format the string returned
     *  - otherwise the given values will be filled with the values from the parsed subLayerString
     * \param subLayerString Data Source
     * \param layerIndex index of Layer
     * \param layerName name of Layer
     * \param featuresCounted number of features of Layer
     * \param ogrGeometryType geometry type to be read
     * \param geometryName geometry name to be read
     * \param geometryIndex geometry field index to be read
     * \param ogrType ogr retrievel method to be used
     * \returns QString::null on error
     * \since QGIS 3.0
     */
    static QString OGRParseSubLayerStringWrapper( QString const subLayerString,
        int &layerIndex,
        QString &layerName,
        qlonglong &featuresCounted,
        OGRwkbGeometryType &ogrGeometryType,
        QString &geometryName,
        int &geometryIndex,
        QgsOgrProviderUtils::OgrGetType &ogrType );

    /** Retrieves OgrLayer with an index, using different Gdal versions (compile,runtime) pre 2.0 and after
     * \param ogrDataSource Data Source
     * \param mLayerIndex index of Layer
     * \returns OGRLayerH if found
     * \note
     *
     * Up to Gdal 2.* each geometry was 1 layer and could be retrieved using an index
     * Starting with Gdal 2.0, each table is one layer, that can contain more than 1 geometry
     * - OGR_DS_GetLayer has been deprecated, but is available
     * Retrieving with an index should be avoided where praticable
     * For drivers such as GML and KML, where Layer-Names may NOT be unique,
     * is a case where retrieving by index must be used.
     * \since QGIS 3.0
     */
    static OGRLayerH OGRGetLayerIndexWrapper( OGRDataSourceH ogrDataSource, long lLayerIndex );

    /** Retrieves OgrLayer with an layer-name, using different Gdal versions (compile,runtime) pre 2.0 and after
     * \param ogrDataSource Data Source
     * \param mLayerName name of Layer
     * \returns OGRLayerH if found
     * \note
     *
     * Up to Gdal 2.* each geometry was 1 layer and could be retrieved using an index
     * Starting with Gdal 2.0, each table is one layer, that can contain more than 1 geometry
     * - OGR_DS_GetLayerByName has been deprecated, but is available
     * Retrieving with an index should be avoided where praticable
     * \see OGRGetLayerIndexWrapper()
     * For drivers such as sqlite (spatialite), that may contain more than one geometry,
     * retrieving layer by index must NOT be used.
     * Starting with Gdal 2.0, a special syntax must be used for such geometries
     *  - 'table_name(field_name)'
     *  - 'field_name': may be empty (GML,KML) and is NOT needed for tables with only 1 geometry
     * \see QgsOgrProvider::subLayers()
     * \since QGIS 3.0
     */
    static OGRLayerH OGRGetLayerNameWrapper( OGRDataSourceH ogrDataSource, QString sLayerName );

    /** Retrieves OgrLayer with an layer-name and Geomertry field-name
     * 'table_name(field_name)' format is only valid as a OGR layer name for the SQLite driver
     *  - for other drivers where more than 1 geometry field exist
     * -> a layer must be created based on the value returned by for GetGeomFieldIndex()
     * \param ogrDataSource Data Source
     * \param mLayerName name of Layer
     * \param mGeometry name of Layer
     * \returns OGRLayerH if found
     * \note
     * If the mLayerName contains a name in the 'table_name(field_name)' format
     * - the 'table_name' will extracted to mLayerName and 'field_name' to mGeometryName
     * \since QGIS 3.0
     */
    static OGRLayerH OGRGetGeometryNameWrapper( OGRDataSourceH ogrDataSource, QString sLayerName, int &iGeometryIndex, QString iGeometryName = QString::null );

    /** Retrieves OgrGeometry from OgrFeaturer with an layer-name and Geomertry field-name
     *  - different Drivers store the Geometry(s) differently when more than one Geometry exist in a TABLE or File
     * -> if the mGeometryIndex is not contained in the SubLayerString an attempt will be made to retrieved it using the GeomertyName
     * \note
     * - a SQlite [spatialite] table with two geometries will always have OGR_F_GetGeomFieldCount(fetch_feature) == 1 (not 2)
     * --> the mGeometryIndex should NOT be used, since the Feature only contains 1 Geometry
     * - a GML file with two geometries will always have OGR_F_GetGeomFieldCount(fetch_feature) == 2
     * --> and the mGeometryIndex must be used  when > 0 [0 can use by both methods]
     * \param fetch_feature Ogr-Feature to retrieve the Information from
     * \param sSubLayerString Collected Data from the Data-Source
     * \returns OGRGeometryH if found
     * \see QgsOgrFeatureIterator::readFeature
     * \since QGIS 3.0
     */
    static OGRGeometryH OGRGetGeometryFeatureWrapper( OGRFeatureH fetch_feature, QString sSubLayerString );

    /** Retrieves amount of Layers found in the Datasource, using different Gdal versions (compile,runtime) pre 2.0 and after
     * \param ogrDataSource Data Source
     * \returns amount as 'long' value
     * \note
     * \since QGIS 3.0
     */
    static long OGRGetLayerCountWrapper( OGRDataSourceH ogrDataSource );

    /** Retrieves amount of Features (geometries) found in the Layer, using different Gdal versions (compile,runtime) pre 2.0 and after
     * \param ogrDataSource Data Source
     * \returns amount as 'long' value
     * \note
     *
     * Up to Gdal 2.* the result was returned as an integer
     * - the result should always be 1
     * - sometimes 2 (or more) will be returned (with same geomety data):
     * -- when a spatialite 'SpatialView' (where only 1 geometry is supported)
     * -- and the registered geometry is NOT the first geometry listed in the VIEW definition
     * - these geometries be ingnore since they cannot be rendered properly
     * \see QgsOgrProvider::subLayers()
     * Starting with Gdal 2.0, the SpatialView- problem cannot be isolated and ignored
     * - the result will returned the amount of geometries contained in the layer
     * -- for a spatialite 'SpatialView': this should be 1, for a 'SpatialTable' >= 1
     * <a href="https://trac.osgeo.org/gdal/ticket/6659L">gdal-ticket from 2016-08-23 </a>
     *
     * When running with Gdal 2.*
     * - the returned qlonglong value will be returned
     * \since QGIS 3.0
     */
    static qlonglong OGRGetFeatureCountWrapper( OGRLayerH layer, int bForce );

    /** Determines if the Geometry-Type contains a Z-value, using different Gdal versions (compile,runtime) pre 2.0 and after
     * \param eType geometry Type
     * \returns true or false
     * \note
     * Starting with Gdal 2.0, the macro calls the function OGR_GT_HasZ
     *
     * Compiling and Runtime:
     * When running with Gdal 2.*
     * - the result of wkbHasZ will be returned
     * \since QGIS 3.0
     */
    static int wkbHasZWrapper( OGRwkbGeometryType eType );

    /** Determines if the Geometry-Type contains a M-value, using different Gdal versions (compile,runtime) pre 2.0 and after
     * \param eType geometry Type
     * \returns true or false
     * \note
     *
     * Until Gdal 2.0 M-values were not supported
     * - the correct result should be returned if M-values are supported
     * - older gdal version not supporting M-values will have [-2147483646]
     * -- false will be returned
     * \see QgsOgrProvider::subLayers()
     * Starting with Gdal 2.1, the macro calls the function OGR_GT_HasM
     *
     * Compiling and Runtime:
     * When running with Gdal 2.*
     * - the result of wkbHasZ will be returned
     * \since QGIS 3.0
     */
    static int wkbHasMWrapper( OGRwkbGeometryType eType );

    /** Quote a value for placement in a SQL string.
     */
    static QString quotedValue( const QVariant &value );

    static OGRDataSourceH OGROpenWrapper( const char *pszPath, bool bUpdate, OGRSFDriverH *phDriver );
    static void OGRDestroyWrapper( OGRDataSourceH ogrDataSource );
};

#endif // QGSOGRPROVIDERUTILS_H
