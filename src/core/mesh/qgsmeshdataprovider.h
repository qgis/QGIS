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
#include <limits>

#include "qgis_core.h"
#include "qgspoint.h"
#include "qgsdataprovider.h"

class QgsRectangle;

/**
 * \ingroup core
 *
 * QgsMeshDatasetIndex is index that identifies the dataset group (e.g. wind speed)
 * and a dataset in this group (e.g. magnitude of wind speed in particular time)
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.4
 */
class CORE_EXPORT QgsMeshDatasetIndex
{
  public:
    //! Creates an index. -1 represents invalid group/dataset
    QgsMeshDatasetIndex( int group = -1, int dataset = -1 );
    //! Returns a group index
    int group() const;
    //! Returns a dataset index within group()
    int dataset() const;
    //! Returns whether index is valid, ie at least groups is set
    bool isValid() const;
    //! Equality operator
    bool operator == ( const QgsMeshDatasetIndex &other ) const;
    //! Inequality operator
    bool operator != ( const QgsMeshDatasetIndex &other ) const;
  private:
    int mGroupIndex = -1;
    int mDatasetIndex = -1;
};

//! xyz coords of vertex
typedef QgsPoint QgsMeshVertex;

//! List of vertex indexes
typedef QVector<int> QgsMeshFace;

/**
 * \ingroup core
 *
 *  Mesh - vertices and faces
 *
 * \since QGIS 3.6
 */
struct CORE_EXPORT QgsMesh
{
  //! Returns number of vertices
  int vertexCount() const;
  //! Returns number of faces
  int faceCount() const;

  //! Returns a vertex at the index
  QgsMeshVertex vertex( int index ) const;
  //! Returns a face at the index
  QgsMeshFace face( int index ) const;

  //! vertices
  QVector<QgsMeshVertex> vertices SIP_SKIP;
  //! faces
  QVector<QgsMeshFace> faces SIP_SKIP;
};

/**
 * \ingroup core
 *
 * QgsMeshDatasetValue represents single dataset value
 *
 * could be scalar or vector. Nodata values are represented by NaNs.
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshDatasetValue
{
  public:
    //! Constructor for vector value
    QgsMeshDatasetValue( double x,
                         double y );

    //! Constructor for scalar value
    QgsMeshDatasetValue( double scalar );

    //! Default Ctor, initialize to NaN
    QgsMeshDatasetValue() = default;

    //! Dtor
    ~QgsMeshDatasetValue() = default;

    //! Sets scalar value
    void set( double scalar );

    //! Sets X value
    void setX( double x );

    //! Sets Y value
    void setY( double y ) ;

    //! Returns magnitude of vector for vector data or scalar value for scalar data
    double scalar() const;

    //! Returns x value
    double x() const;

    //! Returns y value
    double y() const;

    bool operator==( const QgsMeshDatasetValue &other ) const;

  private:
    double mX  = std::numeric_limits<double>::quiet_NaN();
    double mY  = std::numeric_limits<double>::quiet_NaN();
};

/**
 * \ingroup core
 *
 * QgsMeshDataBlock is a block of integers/doubles that can be used
 * to retrieve:
 * active flags (e.g. face's active integer flag)
 * scalars (e.g. scalar dataset double values)
 * vectors (e.g. vector dataset doubles x,y values)
 *
 * data are implicitly shared, so the class can be quickly copied
 * std::numeric_limits<double>::quiet_NaN() represents NODATA value
 *
 * Data can be accessed all at once with buffer() (faster) or
 * value by value (slower) with active() or value()
 *
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsMeshDataBlock
{
  public:
    //! Type of data stored in the block
    enum DataType
    {
      ActiveFlagInteger, //!< Integer boolean flag whether face is active
      ScalarDouble, //!< Scalar double values
      Vector2DDouble, //!< Vector double pairs (x1, y1, x2, y2, ... )
    };

    //! Constructs an invalid block
    QgsMeshDataBlock();

    //! Constructs a new block
    QgsMeshDataBlock( DataType type, int count );

    //! Type of data stored in the block
    DataType type() const;

    //! Number of items stored in the block
    int count() const;

    //! Whether the block is valid
    bool isValid() const;

    /**
     * Returns a value represented by the index
     * For active flag the behavior is undefined
     */
    QgsMeshDatasetValue value( int index ) const;

    /**
     * Returns a value for active flag by the index
     * For scalar and vector 2d the behavior is undefined
     */
    bool active( int index ) const;

    /**
     * Returns internal buffer to the array
     *
     * The buffer is already allocated with size:
     * count() * sizeof(int) for ActiveFlagInteger
     * count() * sizeof(double) for ScalarDouble
     * count() * 2 * sizeof(double) for Vector2DDouble
     *
     * Primary usage of the function is to write/populate
     * data to the block by data provider.
     */
    void *buffer() SIP_SKIP;

    /**
     * Returns internal buffer to the array for fast
     * values reading
     *
     * The buffer is allocated with size:
     * count() * sizeof(int) for ActiveFlagInteger
     * count() * sizeof(double) for ScalarDouble
     * count() * 2 * sizeof(double) for Vector2DDouble
     */
    const void *constBuffer() const SIP_SKIP;

  private:
    QVector<double> mDoubleBuffer;
    QVector<int> mIntegerBuffer;
    DataType mType;
};

/**
 * \ingroup core
 *
 * QgsMeshDatasetGroupMetadata is a collection of dataset group metadata
 * such as whether the data is vector or scalar, name
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.4
 */
class CORE_EXPORT QgsMeshDatasetGroupMetadata
{
  public:

    //! Location of where data is specified for datasets in the dataset group
    enum DataType
    {
      DataOnFaces, //!< Data is defined on faces
      DataOnVertices //!< Data is defined on vertices
    };

    //! Constructs an empty metadata object
    QgsMeshDatasetGroupMetadata() = default;

    /**
     * Constructs a valid metadata object
     *
     * \param name name of the dataset group
     * \param isScalar dataset contains scalar data, specifically the y-value of QgsMeshDatasetValue is NaN
     * \param isOnVertices dataset values are defined on mesh's vertices. If FALSE, values are defined on faces.
     * \param minimum minimum value (magnitude for vectors) present among all group's dataset values
     * \param maximum maximum value (magnitude for vectors) present among all group's dataset values
     * \param extraOptions dataset's extra options stored by the provider. Usually contains the name, time value, time units, data file vendor, ...
     */
    QgsMeshDatasetGroupMetadata( const QString &name,
                                 bool isScalar,
                                 bool isOnVertices,
                                 double minimum,
                                 double maximum,
                                 const QMap<QString, QString> &extraOptions );

    /**
     * Returns name of the dataset group
     */
    QString name() const;

    /**
     * Returns extra metadata options, for example description
     */
    QMap<QString, QString> extraOptions() const;

    /**
     * \brief Returns whether dataset group has vector data
     */
    bool isVector() const;

    /**
     * \brief Returns whether dataset group has scalar data
     */
    bool isScalar() const;

    /**
     * \brief Returns whether dataset group data is defined on vertices or faces
     */
    DataType dataType() const;

    /**
     * \brief Returns minimum scalar value/vector magnitude present for whole dataset group
     */
    double minimum() const;

    /**
     * \brief Returns maximum scalar value/vector magnitude present for whole dataset group
     */
    double maximum() const;

  private:
    QString mName;
    bool mIsScalar = false;
    bool mIsOnVertices = false;
    double mMinimumValue = std::numeric_limits<double>::quiet_NaN();
    double mMaximumValue = std::numeric_limits<double>::quiet_NaN();
    QMap<QString, QString> mExtraOptions;
};

/**
 * \ingroup core
 *
 * QgsMeshDatasetMetadata is a collection of mesh dataset metadata such
 * as whether the data is valid or associated time for the dataset
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshDatasetMetadata
{
  public:
    //! Constructs an empty metadata object
    QgsMeshDatasetMetadata() = default;

    /**
     * Constructs a valid metadata object
     *
     * \param time a time which this dataset represents in the dataset group
     * \param isValid dataset is loadad and valid for fetching the data
     * \param minimum minimum value (magnitude for vectors) present among dataset values
     * \param maximum maximum value (magnitude for vectors) present among dataset values
     */
    QgsMeshDatasetMetadata( double time,
                            bool isValid,
                            double minimum,
                            double maximum
                          );

    /**
     * \brief Returns the time value for this dataset
     */
    double time() const;

    /**
     * \brief Returns whether dataset is valid
     */
    bool isValid() const;

    /**
     * \brief Returns minimum scalar value/vector magnitude present for the dataset
     */
    double minimum() const;

    /**
     * \brief Returns maximum scalar value/vector magnitude present for the dataset
     */
    double maximum() const;

  private:
    double mTime = std::numeric_limits<double>::quiet_NaN();
    bool mIsValid = false;
    double mMinimumValue = std::numeric_limits<double>::quiet_NaN();
    double mMaximumValue = std::numeric_limits<double>::quiet_NaN();
};

/**
 * \ingroup core
 *
 * Interface for mesh data sources
 *
 * Mesh is a collection of vertices and faces in 2D or 3D space
 *  - vertex - XY(Z) point (in the mesh's coordinate reference system)
 *  - faces - sets of vertices forming a closed shape - typically triangles or quadrilaterals
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
     * Populates the mesh vertices and faces
     * \since QGIS 3.6
     */
    virtual void populateMesh( QgsMesh *mesh ) const = 0;
};

/**
 * \ingroup core
 * Interface for mesh datasets and dataset groups
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
     * To read multiple continuous values, use QgsMeshDatasetSourceInterface::datasetValues()
     *
     * See QgsMeshDatasetMetadata::isVector() or QgsMeshDataBlock::type()
     * to check if the returned value is vector or scalar
     */
    virtual QgsMeshDatasetValue datasetValue( QgsMeshDatasetIndex index, int valueIndex ) const = 0;

    /**
     * \brief Returns N vector/scalar values from the index from the dataset
     *
     * See QgsMeshDatasetMetadata::isVector() to check if the returned value is vector or scalar
     *
     * \since QGIS 3.6
     */
    virtual QgsMeshDataBlock datasetValues( QgsMeshDatasetIndex index, int valueIndex, int count ) const = 0;

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
     * \param path destination path of the stored file
     * \param meta new group's metadata
     * \param datasetValues scalar/vector values for all datasets and all faces/vertices in the group
     * \param datasetActive active flag values for all datasets in the group. Empty array represents can be used
     *                      when all faces are active
     * \param times times in hours for all datasets in the group
     * \returns TRUE on failure, FALSE on success
     *
     * \since QGIS 3.6
     */
    virtual bool persistDatasetGroup( const QString &path,
                                      const QgsMeshDatasetGroupMetadata &meta,
                                      const QVector<QgsMeshDataBlock> &datasetValues,
                                      const QVector<QgsMeshDataBlock> &datasetActive,
                                      const QVector<double> &times
                                    ) = 0;
};


/**
 * \ingroup core
 * Base class for providing data for QgsMeshLayer
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
    QgsMeshDataProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions );

  signals:
    //! Emitted when some new dataset groups have been added
    void datasetGroupsAdded( int count );
};

#endif // QGSMESHDATAPROVIDER_H
