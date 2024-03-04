/***************************************************************************
                         qgsmeshdataset.h
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

#ifndef QGSMESHDATASET_H
#define QGSMESHDATASET_H

#include <QVector>
#include <QString>
#include <QMap>
#include <QPair>

#include <limits>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgspoint.h"
#include "qgsdataprovider.h"

class QgsMeshLayer;
class QgsMeshDatasetGroup;
class QgsRectangle;
struct QgsMesh;

/**
 * \ingroup core
 *
 * \brief QgsMeshDatasetIndex is index that identifies the dataset group (e.g. wind speed)
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
    bool operator == ( QgsMeshDatasetIndex other ) const;
    //! Inequality operator
    bool operator != ( QgsMeshDatasetIndex other ) const;
  private:
    int mGroupIndex = -1;
    int mDatasetIndex = -1;
};

/**
 * \ingroup core
 *
 * \brief QgsMeshDatasetValue represents single dataset value.
 *
 * Values may be scalar or vector. Nodata values are represented by NaNs.
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

    bool operator==( QgsMeshDatasetValue other ) const;

  private:
    double mX = std::numeric_limits<double>::quiet_NaN();
    double mY = std::numeric_limits<double>::quiet_NaN();
};

/**
 * \ingroup core
 *
 * \brief QgsMeshDataBlock is a block of integers/doubles that can be used
 * to retrieve:
 * active flags (e.g. face's active integer flag)
 * scalars (e.g. scalar dataset double values)
 * vectors (e.g. vector dataset doubles x,y values)
 *
 * data are implicitly shared, so the class can be quickly copied
 * std::numeric_limits<double>::quiet_NaN() represents NODATA value
 *
 * Data can be accessed all at once with values() (faster) or
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
     * Sets active flag values.
     *
     * If the data provider/datasets does not have active
     * flag capability (== all values are valid), just
     * set block validity by setValid( TRUE )
     *
     * \param vals value vector with size count()
     *
     * For scalar and vector 2d the behavior is undefined
     *
     * \since QGIS 3.12
     */
    void setActive( const QVector<int> &vals );

    /**
     * Returns active flag array
     *
     * Even for active flag valid dataset, the returned array could be empty.
     * This means that the data provider/dataset does not support active flag
     * capability, so all faces are active by default.
     *
     * For scalar and vector 2d the behavior is undefined
     *
     * \since QGIS 3.12
     */
    QVector<int> active() const;

    /**
     * Returns buffer to the array with values
     * For vector it is pairs (x1, y1, x2, y2, ... )
     *
     * \since QGIS 3.12
     */
    QVector<double> values() const;

    /**
     * Sets values
     *
     * For scalar datasets, it must have size count()
     * For vector datasets, it must have size 2 * count()
     * For active flag the behavior is undefined
     *
     * \since QGIS 3.12
     */
    void setValues( const QVector<double> &vals );

    //! Sets block validity
    void setValid( bool valid );

  private:
    QVector<double> mDoubleBuffer;
    QVector<int> mIntegerBuffer;
    DataType mType;
    int mSize = 0;
    bool mIsValid = false;
};

/**
 * \ingroup core
 *
 * \brief QgsMesh3DDataBlock is a block of 3d stacked mesh data related N
 * faces defined on base mesh frame.
 *
 * Data are implicitly shared, so the class can be quickly copied
 * std::numeric_limits<double>::quiet_NaN() represents NODATA value
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \note In QGIS 3.34 this class was renamed from QgsMesh3dDataBlock to QgsMesh3DDataBlock. The old QgsMesh3dDataBlock name
 * remains available in PyQGIS for compatibility.
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsMesh3DDataBlock
{
  public:
    //! Constructs an invalid block
    QgsMesh3DDataBlock();

    //! Dtor
    ~QgsMesh3DDataBlock();

    //! Constructs a new block for count faces
    QgsMesh3DDataBlock( int count, bool isVector );

    //! Sets block validity
    void setValid( bool valid );

    //! Whether the block is valid
    bool isValid() const;

    //! Whether we store vector values
    bool isVector() const;

    //! Number of 2d faces for which the volume data is stored in the block
    int count() const;

    //! Index of the first volume stored in the buffer (absolute)
    int firstVolumeIndex() const;

    //! Index of the last volume stored in the buffer (absolute)
    int lastVolumeIndex() const;

    //! Returns number of volumes stored in the buffer
    int volumesCount() const;

    /**
     * Returns number of vertical level above 2d faces
     */
    QVector<int> verticalLevelsCount() const;

    /**
     * Sets the vertical level counts
     */
    void setVerticalLevelsCount( const QVector<int> &verticalLevelsCount );

    /**
     * Returns the vertical levels height
     */
    QVector<double> verticalLevels() const;

    /**
     * Sets the vertical levels height
     */
    void setVerticalLevels( const QVector<double> &verticalLevels );

    /**
     * Returns the indexing between faces and volumes
     */
    QVector<int> faceToVolumeIndex() const;

    /**
     * Sets the indexing between faces and volumes
     */
    void setFaceToVolumeIndex( const QVector<int> &faceToVolumeIndex );

    /**
     * Returns the values at volume centers
     *
     * For vector datasets the number of values is doubled (x1, y1, x2, y2, ... )
     */
    QVector<double> values() const;

    /**
     * Returns the value at volume centers
     *
     * \param volumeIndex volume index relative to firstVolumeIndex()
     * \returns value (scalar or vector)
     */
    QgsMeshDatasetValue value( int volumeIndex ) const;

    /**
     * Sets the values at volume centers
     *
     * For vector datasets the number of values is doubled (x1, y1, x2, y2, ... )
     */
    void setValues( const QVector<double> &doubleBuffer );

  private:
    int mSize = 0;
    bool mIsValid = false;
    bool mIsVector = false;
    QVector<int> mVerticalLevelsCount;
    QVector<double> mVerticalLevels;
    QVector<int> mFaceToVolumeIndex;
    QVector<double> mDoubleBuffer; // for scalar/vector values
};

/**
 * \ingroup core
 *
 * \brief QgsMeshDatasetGroupMetadata is a collection of dataset group metadata
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
      DataOnFaces = 0, //!< Data is defined on faces
      DataOnVertices,  //!< Data is defined on vertices
      DataOnVolumes,   //!< Data is defined on volumes \since QGIS 3.12
      DataOnEdges      //!< Data is defined on edges \since QGIS 3.14
    };

    //! Constructs an empty metadata object
    QgsMeshDatasetGroupMetadata() = default;

    /**
     * Constructs a valid metadata object
     *
     * \param name name of the dataset group
     * \param isScalar dataset contains scalar data, specifically the y-value of QgsMeshDatasetValue is NaN
     * \param dataType where the data are defined on (vertices, faces or volumes)
     * \param minimum minimum value (magnitude for vectors) present among all group's dataset values
     * \param maximum maximum value (magnitude for vectors) present among all group's dataset values
     * \param maximumVerticalLevels maximum number of vertical levels for 3d stacked meshes, 0 for 2d meshes
     * \param referenceTime reference time of the dataset group
     * \param isTemporal weither the dataset group is temporal (contains time-related dataset)
     * \param extraOptions dataset's extra options stored by the provider. Usually contains the name, time value, time units, data file vendor, ...
     * \param uri The uri of the dataset
     */
    QgsMeshDatasetGroupMetadata( const QString &name,
                                 const QString uri,
                                 bool isScalar,
                                 DataType dataType,
                                 double minimum,
                                 double maximum,
                                 int maximumVerticalLevels,
                                 const QDateTime &referenceTime,
                                 bool isTemporal,
                                 const QMap<QString, QString> &extraOptions );

    /**
     * Returns name of the dataset group
     */
    QString name() const;

    /**
     * Returns the uri of the source
     *
     * \since QGIS 3.16
     */
    QString uri() const;

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
     * \brief Returns whether the dataset group is temporal (contains time-related dataset)
     */
    bool isTemporal() const;

    /**
     * Returns whether dataset group data is defined on vertices or faces or volumes
     *
     * \since QGIS 3.12
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

    /**
     * Returns maximum number of vertical levels for 3d stacked meshes
     *
     * \since QGIS 3.12
     */
    int maximumVerticalLevelsCount() const;

    /**
     * Returns the reference time
     *
     * \since QGIS 3.12
     */
    QDateTime referenceTime() const;

  private:
    QString mName;
    QString mUri;
    bool mIsScalar = false;
    DataType mDataType = DataType::DataOnFaces;
    double mMinimumValue = std::numeric_limits<double>::quiet_NaN();
    double mMaximumValue = std::numeric_limits<double>::quiet_NaN();
    QMap<QString, QString> mExtraOptions;
    int mMaximumVerticalLevelsCount = 0; // for 3d stacked meshes
    QDateTime mReferenceTime;
    bool mIsTemporal = false;
};

/**
 * \ingroup core
 *
 * \brief QgsMeshDatasetMetadata is a collection of mesh dataset metadata such
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
     * \param maximumVerticalLevels maximum number of vertical levels for 3d stacked meshes, 0 for 2d meshes
     */
    QgsMeshDatasetMetadata( double time,
                            bool isValid,
                            double minimum,
                            double maximum,
                            int maximumVerticalLevels
                          );

    /**
     * Returns the time value for this dataset
     */
    double time() const;

    /**
     * Returns whether dataset is valid
     */
    bool isValid() const;

    /**
     * Returns minimum scalar value/vector magnitude present for the dataset
     */
    double minimum() const;

    /**
     * Returns maximum scalar value/vector magnitude present for the dataset
     */
    double maximum() const;

    /**
     * Returns maximum number of vertical levels for 3d stacked meshes
     *
     * \since QGIS 3.12
     */
    int maximumVerticalLevelsCount() const;

  private:
    double mTime = std::numeric_limits<double>::quiet_NaN();
    bool mIsValid = false;
    double mMinimumValue = std::numeric_limits<double>::quiet_NaN();
    double mMaximumValue = std::numeric_limits<double>::quiet_NaN();
    int mMaximumVerticalLevelsCount = 0; // for 3d stacked meshes
};


/**
 * \ingroup core
 *
 * \brief Abstract class that represents a dataset
 *
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsMeshDataset
{
  public:
    //! Constructor
    QgsMeshDataset() = default;

    //! Destructor
    virtual ~QgsMeshDataset() = default;

    //! Returns the value with index \a valueIndex
    virtual QgsMeshDatasetValue datasetValue( int valueIndex ) const = 0;

    //! Returns \a count values from \a valueIndex
    virtual QgsMeshDataBlock datasetValues( bool isScalar, int valueIndex, int count ) const = 0;

    //! Returns whether faces are active
    virtual QgsMeshDataBlock areFacesActive( int faceIndex, int count ) const = 0;

    //! Returns whether the face is active
    virtual bool isActive( int faceIndex ) const = 0;

    //! Returns the metadata of the dataset
    virtual QgsMeshDatasetMetadata metadata() const = 0;

    //! Returns the values count
    virtual int valuesCount() const = 0;
};

/**
 * \ingroup core
 *
 * \brief Abstract class that represents a dataset group
 *
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsMeshDatasetGroup
{
  public:

    /**
     * Type of the dataset group
     *
     * \since QGIS 3.16
     */
    enum Type
    {
      None, //! Generic type used for non typed dataset group
      Persistent, //! Dataset group store in a file
      Memory, //! Temporary dataset group in memory
      Virtual, //! Virtual Dataset group defined by a formula
    };

    //! Default constructor
    QgsMeshDatasetGroup() = default;
    virtual ~QgsMeshDatasetGroup();

    //! Constructor with the \a name of the dataset group
    QgsMeshDatasetGroup( const QString &name );

    //! Constructor with the \a name of the dataset group and the \a dataTYpe
    QgsMeshDatasetGroup( const QString &name, QgsMeshDatasetGroupMetadata::DataType dataType );

    //! Initialize the dataset group
    virtual void initialize() = 0;

    //! Returns the metadata of the dataset group
    QgsMeshDatasetGroupMetadata groupMetadata() const;

    //! Returns the metadata of the dataset with index \a datasetIndex
    virtual QgsMeshDatasetMetadata datasetMetadata( int datasetIndex ) const = 0 ;

    //! Returns the count of datasets in the group
    virtual int datasetCount() const = 0;

    //! Returns the dataset with \a index
    virtual QgsMeshDataset *dataset( int index ) const = 0;

    //! Returns the type of dataset group
    virtual QgsMeshDatasetGroup::Type type() const = 0;

    //! Returns the minimum value of the whole dataset group
    double minimum() const;

    //! Returns the maximum value of the whole dataset group
    double maximum() const;

    //! Overrides the minimum and the maximum value of the whole dataset group
    void setMinimumMaximum( double min, double max ) const;

    //! Returns the name of the dataset group
    QString name() const;

    //! Sets the name of the dataset group
    void setName( const QString &name );

    //! Returns the data type of the dataset group
    QgsMeshDatasetGroupMetadata::DataType dataType() const;

    //! Sets the data type of the dataset group
    void setDataType( const QgsMeshDatasetGroupMetadata::DataType &dataType );

    //! Adds extra metadata to the group
    void addExtraMetadata( QString key, QString value );
    //! Returns all the extra metadata of the group
    QMap<QString, QString> extraMetadata() const;

    //! Returns whether the group contain scalar values
    bool isScalar() const;

    //! Sets whether the group contain scalar values
    void setIsScalar( bool isScalar );

    //! Returns whether all the datasets contain \a count values
    bool checkValueCountPerDataset( int count ) const;

    //! Calculates the statistics (minimum and maximum)
    void calculateStatistic() const;

    //! Sets statistic obsolete, that means statistic will be recalculated when requested
    void setStatisticObsolete() const;

    //! Returns the dataset group variable name which this dataset group depends on
    virtual QStringList datasetGroupNamesDependentOn() const;

    //! Write dataset group information in a DOM element
    virtual QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const = 0;

    //! Returns some information about the dataset group
    virtual QString description() const;

    //! Sets the reference time of the dataset group
    void setReferenceTime( const QDateTime &referenceTime );

  protected:
    QString mName;

    QgsMeshDatasetGroupMetadata::DataType mDataType = QgsMeshDatasetGroupMetadata::DataOnVertices;
    QMap<QString, QString> mMetadata;
    bool mIsScalar = true;

  private:
    mutable double mMinimum = std::numeric_limits<double>::quiet_NaN();
    mutable double mMaximum = std::numeric_limits<double>::quiet_NaN();
    mutable bool mIsStatisticObsolete = true;

    void updateStatistic() const;

    QDateTime mReferenceTime;
};

#ifndef SIP_RUN

/**
 * \ingroup core
 *
 * \brief Class to store memory dataset.
 *
 * The QgsMeshDatasetValue objects and whether the faces are active are stored in QVector containers that are exposed for efficiency
 *
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsMeshMemoryDataset: public QgsMeshDataset
{
  public:
    //! Constructor
    QgsMeshMemoryDataset() = default;

    QgsMeshDatasetValue datasetValue( int valueIndex ) const override;
    QgsMeshDataBlock datasetValues( bool isScalar, int valueIndex, int count ) const override;
    QgsMeshDataBlock areFacesActive( int faceIndex, int count ) const override;
    QgsMeshDatasetMetadata metadata() const override;
    bool isActive( int faceIndex ) const override;
    int valuesCount() const override;

    //! Calculates the minimum and the maximum of this group
    void calculateMinMax();

    QVector<QgsMeshDatasetValue> values;
    QVector<int> active;
    double time = -1;
    bool valid = false;
    double minimum = std::numeric_limits<double>::quiet_NaN();
    double maximum = std::numeric_limits<double>::quiet_NaN();
};

/**
 * \ingroup core
 *
 * \brief Class that represents a dataset group stored in memory.
 *
 * The QgsMeshMemoryDataset objects stores in a QVector container that are exposed for efficiency
 *
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsMeshMemoryDatasetGroup: public QgsMeshDatasetGroup
{
  public:
    //! Constructor
    QgsMeshMemoryDatasetGroup() = default;
    //! Constructor with the \a name of the group
    QgsMeshMemoryDatasetGroup( const QString &name );
    //! Constructor with the \a name of the group and the type of data \a dataType
    QgsMeshMemoryDatasetGroup( const QString &name, QgsMeshDatasetGroupMetadata::DataType dataType );

    void initialize() override;
    int datasetCount() const override;
    QgsMeshDatasetMetadata datasetMetadata( int datasetIndex ) const override;
    QgsMeshDataset *dataset( int index ) const override;
    virtual QgsMeshDatasetGroup::Type type() const override {return QgsMeshDatasetGroup::Memory;}

    //! Returns a invalid DOM element
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context )  const override;

    //! Adds a memory dataset to the group
    void addDataset( std::shared_ptr<QgsMeshMemoryDataset> dataset );

    //! Removes all the datasets from the group
    void clearDatasets();

    //! Returns the dataset with \a index
    std::shared_ptr<const QgsMeshMemoryDataset> constDataset( int index ) const;

    //! Contains all the memory datasets
    QVector<std::shared_ptr<QgsMeshMemoryDataset>> memoryDatasets;
};

/**
 * \ingroup core
 *
 * \brief Class that represents a dataset with elevation value of the vertices of a existing mesh that can be edited
 *
 * \since QGIS 3.22
 */
class QgsMeshVerticesElevationDataset: public QgsMeshDataset
{
  public:
    //! Constructor
    QgsMeshVerticesElevationDataset( QgsMesh *mesh );

    QgsMeshDatasetValue datasetValue( int valueIndex ) const override;
    QgsMeshDataBlock datasetValues( bool isScalar, int valueIndex, int count ) const override;;
    QgsMeshDataBlock areFacesActive( int faceIndex, int count ) const override;;
    bool isActive( int ) const override {return true;};
    QgsMeshDatasetMetadata metadata() const override;;
    int valuesCount() const override;
  private:
    QgsMesh *mMesh;
};

/**
 * \ingroup core
 *
 * \brief Class that represents a dataset group with elevation value of the vertices of a existing mesh that can be edited
 *        This dataset group contains only one dataset.
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsMeshVerticesElevationDatasetGroup : public QgsMeshDatasetGroup
{
  public:
    //! Constructor with a \a name and linked to \a mesh
    QgsMeshVerticesElevationDatasetGroup( QString name, QgsMesh *mesh );

    void initialize() override;
    QgsMeshDatasetMetadata datasetMetadata( int datasetIndex ) const override;;
    int datasetCount() const override;;
    QgsMeshDataset *dataset( int index ) const override;;
    QgsMeshDatasetGroup::Type type() const override;
    QDomElement writeXml( QDomDocument &, const QgsReadWriteContext & ) const override {return QDomElement();};

  private:
    std::unique_ptr<QgsMeshVerticesElevationDataset> mDataset;
};

#endif //SIP_RUN

/**
 * \ingroup core
 *
 * \brief Tree item for display of the mesh dataset groups.
 * Dataset group is set of datasets with the same name,
 * but different control variable (e.g. time)
 *
 * Support for multiple levels, because groups can have
 * subgroups, for example
 *
 * Groups:
 *   Depth
 *     Minimum
 *     Maximum
 *   Velocity
 *   Wind speed
 *     Minimum
 *     Maximum
 *
 * Tree items handle also the dependencies between dataset groups represented by these items
 *
 * \since QGIS 3.14 in core API
 */

class CORE_EXPORT QgsMeshDatasetGroupTreeItem
{
  public:

    /**
     * Constructor for an empty dataset group tree item
     */
    QgsMeshDatasetGroupTreeItem();

    /**
     * Constructor
     *
     * \param defaultName the name that will be used to display the item if iot not overrides (\see setName())
     * \param sourceName the name used by the source (provider, dataset group store,...)
     * \param isVector whether the dataset group is a vector dataset group
     * \param index index of the dataset group
     */
    QgsMeshDatasetGroupTreeItem( const QString &defaultName,
                                 const QString &sourceName,
                                 bool isVector,
                                 int index );

    /**
     * Constructor from a DOM element, constructs also the children
     *
     * \param itemElement the DOM element
     * \param context writing context (e.g. for conversion between relative and absolute paths)
     */
    QgsMeshDatasetGroupTreeItem( const QDomElement &itemElement, const QgsReadWriteContext &context );

    /**
     * Destructor, destructs also the children
     *
    */
    ~QgsMeshDatasetGroupTreeItem();

    /**
     * Clones the item
     *
     * \return the cloned item
     */
    QgsMeshDatasetGroupTreeItem *clone() const SIP_FACTORY;

    /**
     * Appends a child \a item.
     *
     * \note takes ownership of item
     */
    void appendChild( QgsMeshDatasetGroupTreeItem *item SIP_TRANSFER );

    /**
     * Removes and destroy a item child if exists
     * \param item the item to append
     *
     * \since QGIS 3.16
     */
    void removeChild( QgsMeshDatasetGroupTreeItem *item SIP_TRANSFER );

    /**
     * Returns a child
     * \param row the position of the child
     * \return the item at the position \a row
     */
    QgsMeshDatasetGroupTreeItem *child( int row ) const;

    /**
     * Returns the child with dataset group \a index
     * Searches as depper as needed on the child hierarchy
     *
     * \param index the index of the dataset group index
     * \return the item with index as dataset group index, nullptr if no item is found
     */
    QgsMeshDatasetGroupTreeItem *childFromDatasetGroupIndex( int index );

    /**
     * Returns the count of children
     * \return the children's count
     */
    int childCount() const;

    /**
    * Returns the total count of children, that is included deeper children and disabled items
    * \return the total children's count
    */
    int totalChildCount() const;

    /**
     * Returns a list of enabled dataset group indexes, included deeper children
     * \return the list of dataset group indexes
     *
     * \since QGIS 3.16.3
     */
    QList<int> enabledDatasetGroupIndexes() const;

    /**
     * Returns the parent item, nullptr if it is root item
     * \return the parent item
     */
    QgsMeshDatasetGroupTreeItem *parentItem() const;

    /**
     * Returns the position of the item in the parent
     * \return tow position of the item
     */
    int row() const;

    /**
     * Returns the name of the item
     * This name is the default name if the name has not been overridden (\see setName())
     * \return the name to display
     */
    QString name() const;

    /**
     * Overrides the default name with the name to display.
     * The default name is still stored in the item
     * but will not be displayed anymore except if the empty string is set.
     * \param name to display
     */
    void setName( const QString &name );

    /**
     * Returns the name used by the provider to identify the dataset
     *
     * \return the provider name
     *
     * \since QGIS 3.16
     */
    QString providerName() const;

    /**
     * \return whether the dataset group is vector
     */
    bool isVector() const;

    /**
     * \return the dataset group index
     */
    int datasetGroupIndex() const;

    /**
     * \return whether the item is enabled, that is if it is displayed in view
     */
    bool isEnabled() const;

    /**
     * Sets whether the item is enabled, that is if it is displayed in view
     * \param isEnabled whether the item is enabled
     */
    void setIsEnabled( bool isEnabled );

    /**
     * \return the default name
     */
    QString defaultName() const;

    /**
     * \return the dataset group type
     *
     * \since QGIS 3.16
     */
    QgsMeshDatasetGroup::Type datasetGroupType() const;

    /**
     * Returns a list of group index corresponding to dataset group that depends on the dataset group represented by this item
     *
     * \return list of group index
     *
     */
    QList<int> groupIndexDependencies() const;

    /**
     * Returns description about the dataset group (URI, formula,...)
     *
     * \since QGIS 3.16
     */
    QString description() const;

    /**
     * Set parameters of the item in accordance with the dataset group
     *
     * \param datasetGroup pointer to the dataset group to accord with
     *
     * \since QGIS 3.16
     */
    void setDatasetGroup( QgsMeshDatasetGroup *datasetGroup );

    /**
     * Set parameters of the item in accordance with the persistent dataset group with \a uri
     *
     * \param uri uri of the persistent dataset group
     *
     * \since QGIS 3.16
     */
    void setPersistentDatasetGroup( const QString &uri );

    /**
     * Writes the item and its children in a DOM document
     * \param doc the DOM document
     * \param context writing context (e.g. for conversion between relative and absolute paths)
     * \return the dom element where the item is written
     */
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context );

  private:
    QgsMeshDatasetGroupTreeItem *mParent = nullptr;
    QList< QgsMeshDatasetGroupTreeItem * > mChildren;
    QMap<int, QgsMeshDatasetGroupTreeItem *> mDatasetGroupIndexToChild;

    // Data
    QString mUserName;
    QString mOriginalName;
    QString mSourceName;
    QgsMeshDatasetGroup::Type mDatasetGroupType = QgsMeshDatasetGroup::None;
    QString mDescription;

    bool mIsVector = false;
    int mDatasetGroupIndex = -1;
    bool mIsEnabled = true;

    QList<int> mDatasetGroupDependencies;
    QList<int> mDatasetGroupDependentOn;

    QgsMeshDatasetGroupTreeItem *searchItemBySourceName( const QString &sourceName ) const;
    QgsMeshDatasetGroupTreeItem *rootItem() const;
    void freeAsDependency();
    void freeFromDependencies();
};

#endif // QGSMESHDATASET_H
