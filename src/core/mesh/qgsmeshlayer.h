/***************************************************************************
                         qgsmeshlayer.h
                         --------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHLAYER_H
#define QGSMESHLAYER_H

#include <memory>

#include "qgis_core.h"
#include "qgsinterval.h"
#include "qgsmaplayer.h"
#include "qgsmeshdataprovider.h"
#include "qgsmeshrenderersettings.h"
#include "qgsmeshtimesettings.h"
#include "qgsmeshsimplificationsettings.h"
#include "qgscoordinatetransform.h"
#include "qgsabstractprofilesource.h"

class QgsMapLayerRenderer;
struct QgsMeshLayerRendererCache;
class QgsSymbol;
class QgsTriangularMesh;
class QgsRenderContext;
struct QgsMesh;
class QgsMesh3dAveragingMethod;
class QgsMeshLayerTemporalProperties;
class QgsMeshDatasetGroupStore;
class QgsMeshEditor;
class QgsMeshLayerElevationProperties;

/**
 * \ingroup core
 *
 * \brief Represents a mesh layer supporting display of data on structured or unstructured meshes
 *
 * The QgsMeshLayer is instantiated by specifying the name of a data provider,
 * such as mdal, and url defining the specific data set to connect to.
 * The vector layer constructor in turn instantiates a QgsMeshDataProvider subclass
 * corresponding to the provider type, and passes it the url. The data provider
 * connects to the data source.
 *
 * The QgsMeshLayer provides a common interface to the different data types. It does not
 * yet support editing transactions.
 *
 * The main data providers supported by QGIS are listed below.
 *
 * \section mesh_providers Mesh data providers
 *
 * \subsection mesh_memory Memory data providerType (mesh_memory)
 *
 * The memory data provider is used to construct in memory data, for example scratch
 * data. There is no inherent persistent storage of the data. The data source uri is constructed.
 * Data can be populated by setMesh(const QString &vertices, const QString &faces), where
 * vertices and faces is comma separated coordinates and connections for mesh.
 * E.g. to create mesh with one quad and one triangle
 *
 * \code{py}
 *  uri = "1.0, 2.0 \n" \
 *      "2.0, 2.0 \n" \
 *      "3.0, 2.0 \n" \
 *      "2.0, 3.0 \n" \
 *      "1.0, 3.0 \n" \
 *      "---" \
 *      "0, 1, 3, 4 \n" \
 *      "1, 2, 3 \n"
 *
 *  scratchLayer = QgsMeshLayer(uri, "My Scratch layer", "mesh_memory")
 * \endcode
 *
 * \subsection mdal MDAL data provider (mdal)
 *
 * Accesses data using the MDAL drivers (https://github.com/lutraconsulting/MDAL). The url
 * is the MDAL connection string. QGIS must be built with MDAL support to allow this provider.

 * \code{py}
 *     uri = "test/land.2dm"
 *     scratchLayer = QgsMeshLayer(uri, "My Scratch Layer",  "mdal")
 * \endcode
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshLayer : public QgsMapLayer, public QgsAbstractProfileSource
{
    Q_OBJECT
  public:

    /**
     * Setting options for loading mesh layers.
     */
    struct LayerOptions
    {

      /**
       * Constructor for LayerOptions with optional \a transformContext.
       * \note transformContext argument was added in QGIS 3.8
       */
      explicit LayerOptions( const QgsCoordinateTransformContext &transformContext = QgsCoordinateTransformContext( ) )
        : transformContext( transformContext )
      {}

      /**
       * Coordinate transform context
       */
      QgsCoordinateTransformContext transformContext;

      /**
       * Set to TRUE if the default layer style should be loaded.
       * \since QGIS 3.22
       */
      bool loadDefaultStyle = true;

      /**
       * Controls whether the layer is allowed to have an invalid/unknown CRS.
       *
       * If TRUE, then no validation will be performed on the layer's CRS and the layer
       * layer's crs() may be invalid() (i.e. the layer will have no georeferencing available
       * and will be treated as having purely numerical coordinates).
       *
       * If FALSE (the default), the layer's CRS will be validated using QgsCoordinateReferenceSystem::validate(),
       * which may cause a blocking, user-facing dialog asking users to manually select the correct CRS for the
       * layer.
       *
       * \since QGIS 3.10
       */
      bool skipCrsValidation = false;
    };

    /**
     * Constructor - creates a mesh layer
     *
     * The QgsMeshLayer is constructed by instantiating a data provider.  The provider
     * interprets the supplied path (url) of the data source to connect to and access the
     * data.
     *
     * \param path  The path or url of the parameter.  Typically this encodes
     *               parameters used by the data provider as url query items.
     * \param baseName The name used to represent the layer in the legend
     * \param providerLib  The name of the data provider, e.g., "mesh_memory", "mdal"
     * \param options general mesh layer options
     */
    explicit QgsMeshLayer( const QString &path = QString(), const QString &baseName = QString(), const QString &providerLib = QStringLiteral( "mesh_memory" ),
                           const QgsMeshLayer::LayerOptions &options = QgsMeshLayer::LayerOptions() );

    ~QgsMeshLayer() override;

    //! QgsMeshLayer cannot be copied.
    QgsMeshLayer( const QgsMeshLayer &rhs ) = delete;
    //! QgsMeshLayer cannot be copied.
    QgsMeshLayer &operator=( QgsMeshLayer const &rhs ) = delete;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsMeshLayer: '%1' (%2)>" ).arg( sipCpp->name(), sipCpp->dataProvider() ? sipCpp->dataProvider()->name() : QStringLiteral( "Invalid" ) );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    QgsMeshDataProvider *dataProvider() override;
    const QgsMeshDataProvider *dataProvider() const override SIP_SKIP;
    QgsMeshLayer *clone() const override SIP_FACTORY;
    QgsRectangle extent() const override;
    QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) override SIP_FACTORY;
    QgsAbstractProfileGenerator *createProfileGenerator( const QgsProfileRequest &request ) override SIP_FACTORY;
    bool readSymbology( const QDomNode &node, QString &errorMessage,
                        QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories ) override;
    bool writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage,
                         const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories ) const override;
    bool writeStyle( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories ) const override;
    bool readStyle( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories ) override;
    QString encodedSource( const QString &source, const QgsReadWriteContext &context ) const override;
    QString decodedSource( const QString &source, const QString &provider, const QgsReadWriteContext &context ) const override;
    bool readXml( const QDomNode &layer_node, QgsReadWriteContext &context ) override;
    bool writeXml( QDomNode &layer_node, QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    QgsMapLayerTemporalProperties *temporalProperties() override;
    QgsMapLayerElevationProperties *elevationProperties() override;
    void reload() override;
    QStringList subLayers() const override;
    QString htmlMetadata() const override;
    bool isEditable() const override;
    bool supportsEditing() const override;
    QString loadDefaultStyle( bool &resultFlag SIP_OUT ) FINAL;

    //! Returns the provider type for this layer
    QString providerType() const;

    /**
     * Adds datasets to the mesh from file with \a path. Use the the time \a defaultReferenceTime as reference time is not provided in the file
     *
     * \param path the path to the atasets file
     * \param defaultReferenceTime reference time used if not provided in the file
     * \return whether the dataset is added
     *
     * \since QGIS 3.14
     */
    bool addDatasets( const QString &path, const QDateTime &defaultReferenceTime = QDateTime() );

    /**
     * Adds extra datasets to the mesh. Take ownership.
     *
     * \param datasetGroup the extra dataset group
     * \return whether the dataset is effectively added
     *
     * \since QGIS 3.16
     */
    bool addDatasets( QgsMeshDatasetGroup *datasetGroup SIP_TRANSFER );

    /**
     * Saves datasets group on file with the specified \a driver
     *
     * \param path the path of the file
     * \param datasetGroupIndex the index of the dataset group
     * \param driver the driver to used for saving
     * \return false if succeeds
     *
     * \since QGIS 3.16
     */
    bool saveDataset( const QString &path, int datasetGroupIndex, QString driver );

    /**
     * Returns native mesh (NULLPTR before rendering or calling to updateMesh)
     *
     * \note Not available in Python bindings
     */
    QgsMesh *nativeMesh() SIP_SKIP;

    /**
     * Returns native mesh (NULLPTR before rendering or calling to updateMesh)
     *
     * \note Not available in Python bindings
     */
    const QgsMesh *nativeMesh() const SIP_SKIP;

    /**
     * Returns triangular mesh (NULLPTR before rendering or calling to updateMesh).
     *
     * If the parameter triangleSize is provided, among the base triangular mesh
     * and the simplified triangular meshes, the one returned is which has the average triangle size just greater than triangleSize.
     * The size of a triangle is the maximum between the height and the width of the triangle bounding box
     * For default parameter (=0), it returns base triangular mesh.
     * \param minimumTriangleSize is the average size criteria in canvas map units
     * \returns triangular mesh, the layer keeps the ownership
     * \note triangular size added in QGIS 3.14
     * \note Not available in Python bindings
     */
    QgsTriangularMesh *triangularMesh( double minimumTriangleSize = 0 ) const SIP_SKIP;

    /**
     * Returns the count of levels of detail of the mesh simplification
     *
     * \note Not available in Python bindings
     * \since QGIS 3.18
     */
    int triangularMeshLevelOfDetailCount() const SIP_SKIP;

    /**
     * Returns triangular corresponding to the index of level of details.
     * If \a lodIndex is greater than the count of levels of detail, returns the last one (with lesser triangles)
     * If \a lodIndex is lesser or equal to 0, returns the original triangular mesh
     *
     * \param lodIndex the level od detail index
     *
     * \note Not available in Python bindings
     * \since QGIS 3.18
     */
    QgsTriangularMesh *triangularMeshByLodIndex( int lodIndex ) const SIP_SKIP;

    /**
     * Gets native mesh and updates (creates if it doesn't exist) the base triangular mesh
     *
     * \param transform Transformation from layer CRS to destination (e.g. map) CRS. With invalid transform, it keeps the native mesh CRS
     *
     * \since QGIS 3.14
     */
    void updateTriangularMesh( const QgsCoordinateTransform &transform = QgsCoordinateTransform() );

    /**
     * Returns native mesh (NULLPTR before rendering)
     *
     * \note Not available in Python bindings
     */
    QgsMeshLayerRendererCache *rendererCache() SIP_SKIP;

    //! Returns renderer settings
    QgsMeshRendererSettings rendererSettings() const;
    //! Sets new renderer settings
    void setRendererSettings( const QgsMeshRendererSettings &settings );

    /**
     * Returns time format settings
     *
     * \since QGIS 3.8
     */
    QgsMeshTimeSettings timeSettings() const;

    /**
     * Sets time format settings
     *
     * \since QGIS 3.8
     */
    void setTimeSettings( const QgsMeshTimeSettings &settings );

    /**
     * Returns mesh simplification settings
     *
     * \since QGIS 3.14
     */
    QgsMeshSimplificationSettings meshSimplificationSettings() const SIP_SKIP;

    /**
     * Sets mesh simplification settings
     *
     * \since QGIS 3.14
     */
    void setMeshSimplificationSettings( const QgsMeshSimplificationSettings &meshSimplificationSettings ) SIP_SKIP;

    /**
     * Returns (date) time in hours formatted to human readable form
     * \param hours time in double in hours
     * \returns formatted time string
     * \since QGIS 3.8
     */
    QString formatTime( double hours );

    /**
     * Returns the dataset groups count handle by the layer
     *
     * \since QGIS 3.16
     */
    int datasetGroupCount() const;

    /**
     * Returns the extra dataset groups count handle by the layer
     *
     * \since QGIS 3.16
     */
    int extraDatasetGroupCount() const;

    /**
     * Returns the list of indexes of dataset groups handled by the layer
     *
     * \note indexes are used to distinguish all the dataset groups handled by the layer (from dataprovider, extra dataset group,...)
     * In the layer scope, those indexes can be different from the data provider indexes.
     *
     * \since QGIS 3.16
     */
    QList<int> datasetGroupsIndexes() const;

    /**
     * Returns the list of indexes of enables dataset groups handled by the layer
     *
     * \note indexes are used to distinguish all the dataset groups handled by the layer (from dataprovider, extra dataset group,...)
     * In the layer scope, those indexes can be different from the data provider indexes.
     *
     * \since QGIS 3.16.3
     */
    QList<int> enabledDatasetGroupsIndexes() const;

    /**
     * Returns the dataset groups metadata
     *
     * \note indexes are used to distinguish all the dataset groups handled by the layer (from dataprovider, extra dataset group,...)
     * In the layer scope, those indexes can be different from the data provider indexes.
     *
     * \since QGIS 3.16
     */
    QgsMeshDatasetGroupMetadata datasetGroupMetadata( const QgsMeshDatasetIndex &index ) const;

    /**
     * Returns the dataset count in the dataset groups
     *
     * \param index index of the dataset in the group
     *
     * \note indexes are used to distinguish all the dataset groups handled by the layer (from dataprovider, extra dataset group,...)
     * In the layer scope, those indexes can be different from the data provider indexes.
     *
     * \since QGIS 3.16
     */
    int datasetCount( const QgsMeshDatasetIndex &index ) const;

    /**
     * Returns the dataset metadata
     *
     * \param index index of the dataset
     *
     * \note indexes are used to distinguish all the dataset groups handled by the layer (from dataprovider, extra dataset group,...)
     * In the layer scope, those indexes can be different from the data provider indexes.
     *
     * \since QGIS 3.16
     */
    QgsMeshDatasetMetadata datasetMetadata( const QgsMeshDatasetIndex &index ) const;

    /**
     * Returns  vector/scalar value associated with the index from the dataset
     * To read multiple continuous values, use datasetValues()
     *
     * See QgsMeshDatasetMetadata::isVector() or QgsMeshDataBlock::type()
     * to check if the returned value is vector or scalar
     *
     * Returns invalid value for DataOnVolumes
     *
     * \param index index of the dataset
     * \param valueIndex index of the value
     *
     * \note indexes are used to distinguish all the dataset groups handled by the layer (from dataprovider, extra dataset group,...)
     * In the layer scope, those indexes can be different from the data provider indexes.
     *
     * \since QGIS 3.16
     */
    QgsMeshDatasetValue datasetValue( const QgsMeshDatasetIndex &index, int valueIndex ) const;

    /**
     * Returns N vector/scalar values from the index from the dataset
     *
     * See QgsMeshDatasetMetadata::isVector() or QgsMeshDataBlock::type()
     * to check if the returned value is vector or scalar
     *
     * Returns invalid block for DataOnVolumes. Use QgsMeshLayerUtils::datasetValues() if you
     * need block for any type of data type
     *
     * \param index index of the dataset
     * \param valueIndex index of the value
     * \param count number of values to return
     *
     * \note indexes are used to distinguish all the dataset groups handled by the layer (from dataprovider, extra dataset group,...)
     * In the layer scope, those indexes can be different from the data provider indexes.
     *
     * \since QGIS 3.16
     */
    QgsMeshDataBlock datasetValues( const QgsMeshDatasetIndex &index, int valueIndex, int count ) const;

    /**
     * Returns N vector/scalar values from the face index from the dataset for 3d stacked meshes
     *
     * See QgsMeshDatasetMetadata::isVector() to check if the returned value is vector or scalar
     *
     * returns invalid block for DataOnFaces and DataOnVertices.
     *
     * \param index index of the dataset
     * \param faceIndex index of the face
     * \param count number of values to return
     *
     * \note indexes are used to distinguish all the dataset groups handled by the layer (from dataprovider, extra dataset group,...)
     * In the layer scope, those indexes can be different from the data provider indexes.
     *
     * \since QGIS 3.16
     */
    QgsMesh3dDataBlock dataset3dValues( const QgsMeshDatasetIndex &index, int faceIndex, int count ) const;

    /**
     * Returns N vector/scalar values from the face index from the dataset for 3d stacked meshes
     *
     * See QgsMeshDatasetMetadata::isVector() to check if the returned value is vector or scalar
     *
     * returns invalid block for DataOnFaces and DataOnVertices.
     */
    bool isFaceActive( const QgsMeshDatasetIndex &index, int faceIndex ) const;

    /**
     * Returns whether the faces are active for particular dataset
     *
     * \param index index of the dataset
     * \param faceIndex index of the face
     * \param count number of values to return
     *
     * \note indexes are used to distinguish all the dataset groups handled by the layer (from dataprovider, extra dataset group,...)
     * In the layer scope, those indexes are different from the data provider indexes.
     *
     * \since QGIS 3.16
     */
    QgsMeshDataBlock areFacesActive( const QgsMeshDatasetIndex &index, int faceIndex, int count ) const;

    /**
      * Interpolates the value on the given point from given dataset.
      * For 3D datasets, it uses dataset3dValue(), \n
      * For 1D datasets, it uses dataset1dValue() with \a searchRadius
      *
      * \note It uses previously cached and indexed triangular mesh
      * and so if the layer has not been rendered previously
      * (e.g. when used in a script) it returns NaN value
      * \see updateTriangularMesh
      *
      * \param index dataset index specifying group and dataset to extract value from
      * \param point point to query in map coordinates
      * \param searchRadius the radius of the search area in map unit
      * \returns interpolated value at the point. Returns NaN values for values
      * outside the mesh layer, nodata values and in case triangular mesh was not
      * previously used for rendering
      *
      * \note indexes are used to distinguish all the dataset groups handled by the layer (from dataprovider, extra dataset group,...)
      * In the layer scope, those indexes are different from the data provider indexes.
      *
      * \since QGIS 3.4
      */
    QgsMeshDatasetValue datasetValue( const QgsMeshDatasetIndex &index, const QgsPointXY &point, double searchRadius = 0 ) const;

    /**
      * Returns the 3d values of stacked 3d mesh defined by the given point
      *
      * \note It uses previously cached and indexed triangular mesh
      * and so if the layer has not been rendered previously
      * (e.g. when used in a script) it returns NaN value
      * \see updateTriangularMesh
      *
      * \param index dataset index specifying group and dataset to extract value from
      * \param point point to query in map coordinates
      * \returns all 3d stacked values that belong to face defined by given point. Returns invalid block
      * for point outside the mesh layer or in case triangular mesh was not
      * previously used for rendering or for datasets that do not have type DataOnVolumes
      *
      * \note indexes are used to distinguish all the dataset groups handled by the layer (from dataprovider, extra dataset group,...)
      * In the layer scope, those indexes are different from the data provider indexes.
      *
      * \since QGIS 3.12
      */
    QgsMesh3dDataBlock dataset3dValue( const QgsMeshDatasetIndex &index, const QgsPointXY &point ) const;

    /**
      * Returns the value of 1D mesh dataset defined on edge that are in the search area defined by point ans searchRadius
      *
      * \note It uses previously cached and indexed triangular mesh
      * and so if the layer has not been rendered previously
      * (e.g. when used in a script) it returns NaN value
      * \see updateTriangularMesh
      *
      * \param index dataset index specifying group and dataset to extract value from
      * \param point the center point of the search area
      * \param searchRadius the radius of the searc area in map unit
      * \returns interpolated value at the projected point. Returns NaN values for values
      * outside the mesh layer and in case triangular mesh was not previously used for rendering
      *
      * \note indexes are used to distinguish all the dataset groups handled by the layer (from dataprovider, extra dataset group,...)
      * In the layer scope, those indexes are different from the data provider indexes.
      *
      * \since QGIS 3.14
      */
    QgsMeshDatasetValue dataset1dValue( const QgsMeshDatasetIndex &index, const QgsPointXY &point, double searchRadius ) const;

    /**
      * Returns dataset index from datasets group depending on the time range.
      * If the temporal properties is not active, returns invalid dataset index. This method is used for rendering mesh layer.
      *
      * \param timeRange the time range
      * \param datasetGroupIndex the index of the dataset group
      * \returns dataset index
      *
      * \note the returned dataset index depends on the matching method, see setTemporalMatchingMethod()
      *
      * \note indexes are used to distinguish all the dataset groups handled by the layer (from dataprovider, extra dataset group,...)
      * In the layer scope, those indexes are different from the data provider indexes.
      *
      * \since QGIS 3.14
      */
    QgsMeshDatasetIndex datasetIndexAtTime( const QgsDateTimeRange &timeRange, int datasetGroupIndex ) const;

    /**
      * Returns dataset index from datasets group depending on the relative time from the layer reference time.
      * Dataset index is valid even the temporal properties is inactive. This method is used for calculation on mesh layer.
      *
      * \param relativeTime the relative from the mesh layer reference time
      * \param datasetGroupIndex the index of the dataset group
      * \returns dataset index
      *
      * \note the returned dataset index depends on the matching method, see setTemporalMatchingMethod()
      *
      * \note indexes are used to distinguish all the dataset groups handled by the layer (from dataprovider, extra dataset group,...)
      * In the layer scope, those indexes are different from the data provider indexes.
      *
      * \since QGIS 3.16
      */
    QgsMeshDatasetIndex datasetIndexAtRelativeTime( const QgsInterval &relativeTime, int datasetGroupIndex ) const;

    /**
      * Returns a list of dataset indexes from datasets group that are in a interval time from the layer reference time.
      * Dataset index is valid even the temporal properties is inactive. This method is used for calculation on mesh layer.
      *
      * \param startRelativeTime the start time of the relative interval from the reference time.
      * \param endRelativeTime the end time of the relative interval from the reference time.
      * \param datasetGroupIndex the index of the dataset group
      * \returns dataset index
      *
      * \note indexes are used to distinguish all the dataset groups handled by the layer (from dataprovider, extra dataset group,...)
      * In the layer scope, those indexes are different from the data provider indexes.
      *
      * \since QGIS 3.22
      */
    QList<QgsMeshDatasetIndex> datasetIndexInRelativeTimeInterval( const QgsInterval &startRelativeTime, const QgsInterval &endRelativeTime, int datasetGroupIndex ) const;

    /**
      * Returns dataset index from active scalar group depending on the time range.
      * If the temporal properties is not active, return the static dataset
      *
      * \param timeRange the time range
      * \returns dataset index
      *
      * \note the returned dataset index depends on the matching method, see setTemporalMatchingMethod()
      *
      * \since QGIS 3.14
      */
    QgsMeshDatasetIndex activeScalarDatasetAtTime( const QgsDateTimeRange &timeRange ) const;

    /**
      * Returns dataset index from active vector group depending on the time range
      * If the temporal properties is not active, return the static dataset
      *
      * \param timeRange the time range
      * \returns dataset index
      *
      * \note the returned dataset index depends on the matching method, see setTemporalMatchingMethod()
      *
      * \since QGIS 3.14
      */
    QgsMeshDatasetIndex activeVectorDatasetAtTime( const QgsDateTimeRange &timeRange ) const;

    /**
      * Sets the static scalar dataset index that is rendered if the temporal properties is not active
      *
      * \param staticScalarDatasetIndex the scalar data set index
      *
      * \since QGIS 3.14
      */
    void setStaticScalarDatasetIndex( const QgsMeshDatasetIndex &staticScalarDatasetIndex ) SIP_SKIP;

    /**
      * Sets the static vector dataset index that is rendered if the temporal properties is not active
      *
      * \param staticVectorDatasetIndex the vector data set index
      *
      * \since QGIS 3.14
      */
    void setStaticVectorDatasetIndex( const QgsMeshDatasetIndex &staticVectorDatasetIndex ) SIP_SKIP;

    /**
      * Returns the static scalar dataset index that is rendered if the temporal properties is not active
      *
      * \since QGIS 3.14
      */
    QgsMeshDatasetIndex staticScalarDatasetIndex() const;

    /**
      * Returns the static vector dataset index that is rendered if the temporal properties is not active
      *
      * \since QGIS 3.14
      */
    QgsMeshDatasetIndex staticVectorDatasetIndex() const;

    /**
      * Sets the reference time of the layer
      *
      * \param referenceTime the reference time
      *
      * \since QGIS 3.14
      */
    void setReferenceTime( const QDateTime &referenceTime );

    /**
      * Sets the method used to match the temporal dataset from a requested time, see activeVectorDatasetAtTime()
      *
      * \param matchingMethod the matching method
      *
      * \since QGIS 3.14
      */
    void setTemporalMatchingMethod( const QgsMeshDataProviderTemporalCapabilities::MatchingTemporalDatasetMethod &matchingMethod );

    /**
      * Returns the position of the snapped point on the mesh element closest to \a point intersecting with
      * the searching area defined by \a point and \a searchRadius
      *
      * For vertex, the snapped position is the vertex position
      * For edge, the snapped position is the projected point on the edge, extremity of edge if outside the edge
      * For face, the snapped position is the centroid of the face
      * The returned position is in map coordinates.
      *
      * \note It uses previously cached and indexed triangular mesh
      * and so if the layer has not been rendered previously
      * (e.g. when used in a script) it returns empty QgsPointXY
      * \see updateTriangularMesh
      *
      * \param elementType the type of element to snap
      * \param point the center of the search area in map coordinates
      * \param searchRadius the radius of the search area in map units
      * \return the position of the snapped point on the closest element, empty QgsPointXY if no element of type \a elementType
      *
      * \since QGIS 3.14
      */
    QgsPointXY snapOnElement( QgsMesh::ElementType elementType, const QgsPointXY &point, double searchRadius );

    /**
     * Returns a list of vertex indexes that meet the condition defined by \a expression with the context \a expressionContext
     *
     * To express the relation with a vertex, the expression can be defined with function returning value
     * linked to the current vertex, like " $vertex_z ", "$vertex_as_point"
     *
     * \since QGIS 3.22
     */
    QList<int> selectVerticesByExpression( QgsExpression expression );

    /**
     * Returns a list of faces indexes that meet the condition defined by \a expression with the context \a expressionContext
     *
     * To express the relation with a face, the expression can be defined with function returning value
     * linked to the current face, like " $face_area "
     *
     * \since QGIS 3.22
     */
    QList<int> selectFacesByExpression( QgsExpression expression );

    /**
      * Returns the root items of the dataset group tree item
      *
      * \return the root item
      *
      * \since QGIS 3.14
      */
    QgsMeshDatasetGroupTreeItem *datasetGroupTreeRootItem() const;

    /**
      * Sets the root items of the dataset group tree item.
      * Changes active dataset groups if those one are not enabled anymore :
      *
      * - new active scalar dataset group is the first root item enabled child
      * - new active vector dataset group is none
      *
      * Doesn't take ownership of the pointed item, the root item is cloned.
      *
      * \param rootItem the new root item
      *
      * \since QGIS 3.14
      */
    void setDatasetGroupTreeRootItem( QgsMeshDatasetGroupTreeItem *rootItem );

    /**
     * Reset the dataset group tree item to default from provider
     *
     * \since QGIS 3.14
     */
    void resetDatasetGroupTreeItem();

    /**
     * Returns the first valid time step of the dataset groups, invalid QgInterval if no time step is present
     *
     * \since QGIS 3.14
     */
    QgsInterval firstValidTimeStep() const;

    /**
     * Returns the relative time of the dataset from the reference time of its group
     *
     * \since QGIS 3.16
     */
    QgsInterval datasetRelativeTime( const QgsMeshDatasetIndex &index );

    /**
     * Returns the relative time (in milliseconds) of the dataset from the reference time of its group
     *
     * \since QGIS 3.16
     */
    qint64 datasetRelativeTimeInMilliseconds( const QgsMeshDatasetIndex &index );

    /**
    * Starts edition of the mesh frame. Coordinate \a transform used to initialize the triangular mesh if needed.
    * This operation will disconnect the mesh layer from the data provider anf removes all existing dataset group
    *
    * \since QGIS 3.22
    */
    bool startFrameEditing( const QgsCoordinateTransform &transform );

    /**
    * Commits edition of the mesh frame,
    * Rebuilds the triangular mesh and its spatial index with \a transform,
    * Continue editing with the same mesh editor if \a continueEditing is True
    *
    * \return TRUE if the commit succeeds
    * \since QGIS 3.22
    */
    bool commitFrameEditing( const QgsCoordinateTransform &transform, bool continueEditing = true );

    /**
    * Rolls Back edition of the mesh frame.
    * Reload mesh from file, rebuilds the triangular mesh and its spatial index with \a transform,
    * Continue editing with the same mesh editor if \a continueEditing is TRUE
    *
    * \return TRUE if the rollback succeeds
    * \since QGIS 3.22
    */
    bool rollBackFrameEditing( const QgsCoordinateTransform &transform, bool continueEditing = true );

    /**
    * Stops edition of the mesh, re-indexes the faces and vertices,
    * rebuilds the triangular mesh and its spatial index with \a transform,
    * clean the undostack
    *
    * \since QGIS 3.22
    */
    void stopFrameEditing( const QgsCoordinateTransform &transform );

    /**
    * Re-indexes the faces and vertices, and renumber the indexes if \a renumber is TRUE.
    * rebuilds the triangular mesh and its spatial index with \a transform,
    * clean the undostack
    *
    * Returns FALSE if the operation fails
    *
    * \since QGIS 3.22
    */
    bool reindex( const QgsCoordinateTransform &transform, bool renumber );

    /**
    * Returns a pointer to the mesh editor own by the mesh layer
    *
    * \since QGIS 3.22
    */
    QgsMeshEditor *meshEditor();

    /**
    * Returns whether the mesh frame has been modified since the last save
    *
    * \since QGIS 3.22
    */
    bool isModified() const override;

    /**
     *  Returns whether the mesh contains at mesh elements of given type
     *  \since QGIS 3.22
     */
    bool contains( const QgsMesh::ElementType &type ) const;

    /**
    * Returns the vertices count of the mesh frame
    *
    * \note during mesh editing, some vertices can be void and are not included in this returned value
    *
    *  \since QGIS 3.22
    */
    int meshVertexCount() const;

    /**
    * Returns the faces count of the mesh frame
    *
    * \note during mesh editing, some faces can be void and are not included in this returned value
    *
    * \since QGIS 3.22
    */
    int meshFaceCount() const;

    /**
    * Returns the edges count of the mesh frame
    *
    * \since QGIS 3.22
    */
    int meshEdgeCount() const;

  public slots:

    /**
     * Sets the coordinate transform context to \a transformContext.
     *
     * \since QGIS 3.8
     */
    void setTransformContext( const QgsCoordinateTransformContext &transformContext ) override;

  signals:

    /**
     * Emitted when active scalar group dataset is changed
     *
     * \since QGIS 3.14
     */
    void activeScalarDatasetGroupChanged( int index );

    /**
     * Emitted when active vector group dataset is changed
     *
     * \since QGIS 3.14
     */
    void activeVectorDatasetGroupChanged( int index );

    /**
     * Emitted when time format is changed
     *
     * \since QGIS 3.8
     */
    void timeSettingsChanged( );

  private: // Private methods

    /**
     * Returns TRUE if the provider is in read-only mode
     */
    bool isReadOnly() const override {return true;}

    /**
     * Binds layer to a specific data provider
     * \param provider provider key string, must match a valid QgsMeshDataProvider key. E.g. "mesh_memory", etc.
     * \param options generic provider options
     * \param flags provider flags since QGIS 3.16
     */
    bool setDataProvider( QString const &provider, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

#ifdef SIP_RUN
    QgsMeshLayer( const QgsMeshLayer &rhs );
#endif

    void fillNativeMesh();
    void assignDefaultStyleToDatasetGroup( int groupIndex );
    void setDefaultRendererSettings( const QList<int> &groupIndexes );
    void createSimplifiedMeshes();
    int levelsOfDetailsIndex( double partOfMeshInView ) const;

    bool hasSimplifiedMeshes() const;

    //! Changes scalar settings for classified scalar value (information about is in the metadata
    void applyClassificationOnScalarSettings( const QgsMeshDatasetGroupMetadata &meta, QgsMeshRendererScalarSettings &scalarSettings ) const;

  private slots:
    void onDatasetGroupsAdded( const QList<int> &datasetGroupIndexes );
    void onMeshEdited();

  private:
    //! Pointer to data provider derived from the abastract base class QgsMeshDataProvider
    QgsMeshDataProvider *mDataProvider = nullptr;

    //! List of extra dataset uri associated with this layer
    QStringList mExtraDatasetUri;

    std::unique_ptr<QgsMeshDatasetGroupStore> mDatasetGroupStore;

    //! Pointer to native mesh structure, used as cache for rendering
    std::unique_ptr<QgsMesh> mNativeMesh;

    //! Pointer to derived mesh structures (the first one is the base mesh, others are simplified meshes with decreasing level of detail)
    std::vector<std::unique_ptr<QgsTriangularMesh>> mTriangularMeshes;

    //! Pointer to the cache with data used for last rendering
    std::unique_ptr<QgsMeshLayerRendererCache> mRendererCache;

    //! Renderer configuration
    QgsMeshRendererSettings mRendererSettings;

    //! Time format configuration
    QgsMeshTimeSettings mTimeSettings;

    //! Simplify mesh configuration
    QgsMeshSimplificationSettings mSimplificationSettings;

    QgsMeshLayerTemporalProperties *mTemporalProperties = nullptr;
    QgsMeshLayerElevationProperties *mElevationProperties = nullptr;

    //! Temporal unit used by the provider
    QgsUnitTypes::TemporalUnit mTemporalUnit = QgsUnitTypes::TemporalHours;

    int mStaticScalarDatasetIndex = 0;
    int mStaticVectorDatasetIndex = 0;

    QgsMeshEditor *mMeshEditor = nullptr;

    int closestEdge( const QgsPointXY &point, double searchRadius, QgsPointXY &projectedPoint ) const;

    //! Returns the exact position in map coordinates of the closest vertex in the search area
    QgsPointXY snapOnVertex( const QgsPointXY &point, double searchRadius );

    //!Returns the position of the projected point on the closest edge in the search area
    QgsPointXY snapOnEdge( const QgsPointXY &point, double searchRadius );

    //!Returns the position of the centroid point on the closest face in the search area
    QgsPointXY snapOnFace( const QgsPointXY &point, double searchRadius );

    void updateActiveDatasetGroups();

    void setDataSourcePrivate( const QString &dataSource, const QString &baseName, const QString &provider,
                               const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags ) override;
};

#endif //QGSMESHLAYER_H
