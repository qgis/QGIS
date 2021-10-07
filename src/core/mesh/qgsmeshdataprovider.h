/***************************************************************************
                         qgsmeshdataprovider.h
                         ---------------------
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

#ifndef QGSMESHDATAPROVIDER_H
#define QGSMESHDATAPROVIDER_H

#include <QVector>
#include <QString>
#include <QMap>
#include <QPair>

#include <limits>

#include "qgis_core.h"
#include "qgspoint.h"
#include "qgsdataprovider.h"
#include "qgsprovidermetadata.h"
#include "qgsmeshdataset.h"
#include "qgsmeshdataprovidertemporalcapabilities.h"


class QgsRectangle;

//! xyz coords of vertex
typedef QgsPoint QgsMeshVertex;

//! List of vertex indexes
typedef QVector<int> QgsMeshFace;

/**
 * Edge is a straight line seqment between 2 points.
 * Stores the pair of vertex indexes
 * \since QGIS 3.14
 */
typedef QPair<int, int> QgsMeshEdge;

/**
 * \ingroup core
 *
 * \brief Mesh - vertices, edges and faces
 *
 * \since QGIS 3.6
 */
struct CORE_EXPORT QgsMesh
{

  /**
   * Defines type of mesh elements
   *  \since QGIS 3.14
   */
  enum ElementType
  {
    Vertex = 1,
    Edge   = 2,
    Face   = 4
  };

  /**
   * Returns whether the mesh contains at mesh elements of given type
   *  \since QGIS 3.14
   */
  bool contains( const ElementType &type ) const;

  //! Returns number of vertices
  int vertexCount() const;
  //! Returns number of faces
  int faceCount() const;

  /**
   * Returns number of edge
   * \since QGIS 3.14
   */
  int edgeCount() const;

  //! Returns a vertex at the index
  QgsMeshVertex vertex( int index ) const;
  //! Returns a face at the index
  QgsMeshFace face( int index ) const;

  /**
   * Returns an edge at the index
   * \since QGIS 3.14
   */
  QgsMeshEdge edge( int index ) const;

  /**
    * Remove all vertices, edges and faces
    * \since QGIS 3.14
    */
  void clear();

  /**
   * Compare two faces, return TRUE if they are equivalent : same indexes and same clock wise
    * \since QGIS 3.16
   */
  static bool compareFaces( const QgsMeshFace &face1, const QgsMeshFace &face2 );

  QVector<QgsMeshVertex> vertices SIP_SKIP;
  QVector<QgsMeshEdge> edges SIP_SKIP;
  QVector<QgsMeshFace> faces SIP_SKIP;
};

/**
 * \ingroup core
 *
 * \brief Interface for mesh data sources
 *
 * Mesh is a collection of vertices, edges and faces in 2D or 3D space
 *
 * - vertex - XY(Z) point (in the mesh's coordinate reference system)
 * - edge   - two XY(Z) points (in the mesh's coordinate reference system) representing straight seqment
 * - faces  - sets of vertices forming a closed shape - typically triangles or quadrilaterals
 *
 * Base on the underlying data provider/format, whole mesh is either stored in memory or
 * read on demand
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshDataSourceInterface SIP_ABSTRACT
{
  public:
    //! Dtor
    virtual ~QgsMeshDataSourceInterface() = default;

    /**
     * Returns whether the mesh contains at mesh elements of given type
     *  \since QGIS 3.14
     */
    bool contains( const QgsMesh::ElementType &type ) const;

    /**
     * \brief Returns number of vertices in the native mesh
     * \returns Number of vertices in the mesh
     */
    virtual int vertexCount() const = 0;

    /**
     * \brief Returns number of faces in the native mesh
     * \returns Number of faces in the mesh
     */
    virtual int faceCount() const = 0;

    /**
     * \brief Returns number of edges in the native mesh
     * \returns Number of edges in the mesh
     *
     * \since QGIS 3.14
     */
    virtual int edgeCount() const = 0;

    /**
     * \brief Returns the maximum number of vertices per face supported by the current mesh,
     * if returns 0, the number of vertices is unlimited
     *
     * \returns Maximum number of vertices per face
     *
     * \since QGIS 3.22
     */
    virtual int maximumVerticesCountPerFace() const {return 0;};

    /**
     * Populates the mesh vertices, edges and faces
     * \since QGIS 3.6
     */
    virtual void populateMesh( QgsMesh *mesh ) const = 0;

    /**
     * Saves the \a mesh frame to the source.
     *
     * \param mesh the mesh to save
     *
     * \returns TRUE on success
     *
     * \since QGIS 3.22
     */
    virtual bool saveMeshFrame( const QgsMesh &mesh ) = 0;
};

/**
 * \ingroup core
 * \brief Interface for mesh datasets and dataset groups
 *
 *  Dataset is a  collection of vector or scalar values on vertices or faces of the mesh.
 *  Based on the underlying data provider/format, whole dataset is either stored in memory
 *  or read on demand
 *
 *  Datasets are grouped in the dataset groups. A dataset group represents a measured quantity
 *  (e.g. depth or wind speed), dataset represents values of the quantity in a particular time.
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshDatasetSourceInterface SIP_ABSTRACT
{
  public:
    QgsMeshDatasetSourceInterface();
    //! Dtor
    virtual ~QgsMeshDatasetSourceInterface() = default;

    /**
     * \brief Associate dataset with the mesh
     *
     * emits dataChanged when successful
     */
    virtual bool addDataset( const QString &uri ) = 0;

    /**
     * Returns list of additional dataset file URIs added using addDataset() calls.
     */
    virtual QStringList extraDatasets() const = 0;

    /**
     * \brief Returns number of datasets groups loaded
     */
    virtual int datasetGroupCount( ) const = 0;

    /**
     * \brief Returns number of datasets loaded in the group
     */
    virtual int datasetCount( int groupIndex ) const = 0;

    /**
     * \brief Returns number of datasets loaded in the group
     */
    int datasetCount( QgsMeshDatasetIndex index ) const;

    /**
     * \brief Returns dataset group metadata
     */
    virtual QgsMeshDatasetGroupMetadata datasetGroupMetadata( int groupIndex ) const = 0;

    /**
     * \brief Returns dataset group metadata
     */
    QgsMeshDatasetGroupMetadata datasetGroupMetadata( QgsMeshDatasetIndex index ) const;

    /**
     * \brief Returns dataset metadata
     */
    virtual QgsMeshDatasetMetadata datasetMetadata( QgsMeshDatasetIndex index ) const = 0;

    /**
     * \brief Returns vector/scalar value associated with the index from the dataset
     * To read multiple continuous values, use datasetValues()
     *
     * See QgsMeshDatasetMetadata::isVector() or QgsMeshDataBlock::type()
     * to check if the returned value is vector or scalar
     *
     * Returns invalid value for DataOnVolumes
     *
     * \see datasetValues
     */
    virtual QgsMeshDatasetValue datasetValue( QgsMeshDatasetIndex index, int valueIndex ) const = 0;

    /**
     * \brief Returns N vector/scalar values from the index from the dataset
     *
     * See QgsMeshDatasetMetadata::isVector() or QgsMeshDataBlock::type()
     * to check if the returned value is vector or scalar
     *
     * Returns invalid block for DataOnVolumes. Use QgsMeshLayerUtils::datasetValues() if you
     * need block for any type of data type
     *
     * \since QGIS 3.6
     */
    virtual QgsMeshDataBlock datasetValues( QgsMeshDatasetIndex index, int valueIndex, int count ) const = 0;

    /**
     * \brief Returns N vector/scalar values from the face index from the dataset for 3d stacked meshes
     *
     * See QgsMeshDatasetMetadata::isVector() to check if the returned value is vector or scalar
     *
     * returns invalid block for DataOnFaces and DataOnVertices.
     *
     * \see datasetValues
     *
     * \since QGIS 3.12
     */
    virtual QgsMesh3dDataBlock dataset3dValues( QgsMeshDatasetIndex index, int faceIndex, int count ) const = 0;

    /**
     * \brief Returns whether the face is active for particular dataset
     *
     * For example to represent the situation when F1 and F3 are flooded, but F2 is dry,
     * some solvers store water depth on vertices V1-V8 (all non-zero values) and
     * set active flag for F2 to FALSE.
     *  V1 ---- V2 ---- V5-----V7
     *  |   F1  |   F2   | F3  |
     *  V3 ---- V4 ---- V6-----V8
     */
    virtual bool isFaceActive( QgsMeshDatasetIndex index, int faceIndex ) const = 0;

    /**
     * \brief Returns whether the faces are active for particular dataset
     *
     * \since QGIS 3.6
     */
    virtual QgsMeshDataBlock areFacesActive( QgsMeshDatasetIndex index, int faceIndex, int count ) const = 0;

    /**
     * Creates a new dataset group from a data and
     * persists it into a destination path
     *
     * On success, the mesh's dataset group count is changed
     *
     * \param path destination path of the stored file in form DRIVER_NAME:path
     * \param meta new group's metadata
     * \param datasetValues scalar/vector values for all datasets and all faces/vertices in the group
     * \param datasetActive active flag values for all datasets in the group. Empty array represents can be used
     *                      when all faces are active
     * \param times times in hours for all datasets in the group
     * \returns TRUE on failure, FALSE on success
     *
     * \note Doesn't work if there is ":" in the path (e.g. Windows system)
     *
     * \since QGIS 3.6
     * \deprecated QGIS 3.12.3
     */
    Q_DECL_DEPRECATED virtual bool persistDatasetGroup( const QString &path,
        const QgsMeshDatasetGroupMetadata &meta,
        const QVector<QgsMeshDataBlock> &datasetValues,
        const QVector<QgsMeshDataBlock> &datasetActive,
        const QVector<double> &times
                                                      ) SIP_DEPRECATED;

    /**
     * Creates a new dataset group from a data and
     * persists it into a destination path
     *
     * On success, the mesh's dataset group count is changed
     *
     * \param outputFilePath destination path of the stored file
     * \param outputDriver output driver name
     * \param meta new group's metadata
     * \param datasetValues scalar/vector values for all datasets and all faces/vertices in the group
     * \param datasetActive active flag values for all datasets in the group. Empty array represents can be used
     *                      when all faces are active
     * \param times times in hours for all datasets in the group
     * \returns TRUE on failure, FALSE on success
     *
     * \since QGIS 3.12.3
     */
    virtual bool persistDatasetGroup( const QString &outputFilePath,
                                      const QString &outputDriver,
                                      const QgsMeshDatasetGroupMetadata &meta,
                                      const QVector<QgsMeshDataBlock> &datasetValues,
                                      const QVector<QgsMeshDataBlock> &datasetActive,
                                      const QVector<double> &times
                                    ) = 0;


    /**
     * Saves a an existing dataset group provided by \a source to a file with a specified driver
     *
     * On success, the mesh's dataset group count is changed
     *
     * \param outputFilePath destination path of the stored file
     * \param outputDriver output driver name
     * \param source source of the dataset group
     * \param datasetGroupIndex index of the dataset group in the \a source
     *
     * \returns TRUE on failure, FALSE on success
     *
     * \since QGIS 3.16
     */
    virtual bool persistDatasetGroup( const QString &outputFilePath,
                                      const QString &outputDriver,
                                      QgsMeshDatasetSourceInterface *source,
                                      int datasetGroupIndex
                                    ) = 0;

    /**
     * Returns the dataset index of the dataset in a specific dataset group at \a time from the \a reference time
     *
     * \param referenceTime the reference time from where to find the dataset
     * \param groupIndex the index of the dataset group
     * \param time the relative time from reference time
     * \param method the method used to check the time
     *
     * \return the dataset index
     */
    QgsMeshDatasetIndex datasetIndexAtTime( const QDateTime &referenceTime,
                                            int groupIndex,
                                            qint64 time,
                                            QgsMeshDataProviderTemporalCapabilities::MatchingTemporalDatasetMethod method ) const;

    /**
     * Returns a list of dataset indexes of the dataset in a specific dataset group that are between \a time1 and \a time2 from the \a reference time
     *
     * \param referenceTime the reference time from where to find the dataset
     * \param groupIndex the index of the dataset group
     * \param time1 the first relative time of the time intervale from reference time
     * \param time2 the second relative time of the time intervale from reference time
     *
     * \return the dataset index
     *
     * \since QGIS 3.22
     */
    QList<QgsMeshDatasetIndex> datasetIndexInTimeInterval( const QDateTime &referenceTime,
        int groupIndex,
        qint64 time1,
        qint64 time2 ) const;

  protected:
    std::unique_ptr<QgsMeshDataProviderTemporalCapabilities> mTemporalCapabilities;
};


/**
 * \ingroup core
 * \brief Base class for providing data for QgsMeshLayer
 *
 * Responsible for reading native mesh data
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshDataProvider: public QgsDataProvider, public QgsMeshDataSourceInterface, public QgsMeshDatasetSourceInterface
{
    Q_OBJECT
  public:
    //! Ctor
    QgsMeshDataProvider( const QString &uri,
                         const QgsDataProvider::ProviderOptions &providerOptions,
                         QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    QgsMeshDataProviderTemporalCapabilities *temporalCapabilities() override;
    const QgsMeshDataProviderTemporalCapabilities *temporalCapabilities() const override SIP_SKIP;

    /**
     * Sets the temporal unit of the provider and reload data if it changes.
     *
     * \param unit the temporal unit
     *
     * \since QGIS 3.14
     */
    void setTemporalUnit( QgsUnitTypes::TemporalUnit unit );


    /**
     * Returns the mesh driver metadata of the provider
     *
     * \return the mesh driver metadata of the provider
     *
     * \since QGIS 3.22
     */
    virtual QgsMeshDriverMetadata driverMetadata()  const;


    /**
     * Closes the data provider and free every resources used
     *
     * \since QGIS 3.22
     */
    virtual void close() = 0;

  signals:
    //! Emitted when some new dataset groups have been added
    void datasetGroupsAdded( int count );

};

#endif // QGSMESHDATAPROVIDER_H
