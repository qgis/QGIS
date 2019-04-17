/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/mesh/qgsmeshdataprovider.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsMeshDatasetIndex
{
%Docstring

QgsMeshDatasetIndex is index that identifies the dataset group (e.g. wind speed)
and a dataset in this group (e.g. magnitude of wind speed in particular time)

.. note::

   The API is considered EXPERIMENTAL and can be changed without a notice

.. versionadded:: 3.4
%End

%TypeHeaderCode
#include "qgsmeshdataprovider.h"
%End
  public:
    QgsMeshDatasetIndex( int group = -1, int dataset = -1 );
%Docstring
Creates an index. -1 represents invalid group/dataset
%End
    int group() const;
%Docstring
Returns a group index
%End
    int dataset() const;
%Docstring
Returns a dataset index within group()
%End
    bool isValid() const;
%Docstring
Returns whether index is valid, ie at least groups is set
%End
    bool operator == ( const QgsMeshDatasetIndex &other ) const;
    bool operator != ( const QgsMeshDatasetIndex &other ) const;
};

typedef QgsPoint QgsMeshVertex;

typedef QVector<int> QgsMeshFace;

struct QgsMesh
{
  int vertexCount() const;
%Docstring
Returns number of vertices
%End
  int faceCount() const;
%Docstring
Returns number of faces
%End

  QgsMeshVertex vertex( int index ) const;
%Docstring
Returns a vertex at the index
%End
  QgsMeshFace face( int index ) const;
%Docstring
Returns a face at the index
%End

};

class QgsMeshDatasetValue
{
%Docstring

QgsMeshDatasetValue represents single dataset value

could be scalar or vector. Nodata values are represented by NaNs.

.. note::

   The API is considered EXPERIMENTAL and can be changed without a notice

.. versionadded:: 3.2
%End

%TypeHeaderCode
#include "qgsmeshdataprovider.h"
%End
  public:
    QgsMeshDatasetValue( double x,
                         double y );
%Docstring
Constructor for vector value
%End

    QgsMeshDatasetValue( double scalar );
%Docstring
Constructor for scalar value
%End

    QgsMeshDatasetValue();
%Docstring
Default Ctor, initialize to NaN
%End

    ~QgsMeshDatasetValue();

    void set( double scalar );
%Docstring
Sets scalar value
%End

    void setX( double x );
%Docstring
Sets X value
%End

    void setY( double y );
%Docstring
Sets Y value
%End

    double scalar() const;
%Docstring
Returns magnitude of vector for vector data or scalar value for scalar data
%End

    double x() const;
%Docstring
Returns x value
%End

    double y() const;
%Docstring
Returns y value
%End

    bool operator==( const QgsMeshDatasetValue &other ) const;

};

class QgsMeshDataBlock
{
%Docstring

QgsMeshDataBlock is a block of integers/doubles that can be used
to retrieve:
active flags (e.g. face's active integer flag)
scalars (e.g. scalar dataset double values)
vectors (e.g. vector dataset doubles x,y values)

data are implicitly shared, so the class can be quickly copied
std.numeric_limits<double>.quiet_NaN() represents NODATA value

Data can be accessed all at once with buffer() (faster) or
value by value (slower) with active() or value()

.. versionadded:: 3.6
%End

%TypeHeaderCode
#include "qgsmeshdataprovider.h"
%End
  public:
    enum DataType
    {
      ActiveFlagInteger,
      ScalarDouble,
      Vector2DDouble,
    };

    QgsMeshDataBlock();
%Docstring
Constructs an invalid block
%End

    QgsMeshDataBlock( DataType type, int count );
%Docstring
Constructs a new block
%End

    DataType type() const;
%Docstring
Type of data stored in the block
%End

    int count() const;
%Docstring
Number of items stored in the block
%End

    bool isValid() const;
%Docstring
Whether the block is valid
%End

    QgsMeshDatasetValue value( int index ) const;
%Docstring
Returns a value represented by the index
For active flag the behavior is undefined
%End

    bool active( int index ) const;
%Docstring
Returns a value for active flag by the index
For scalar and vector 2d the behavior is undefined
%End



};

class QgsMeshDatasetGroupMetadata
{
%Docstring

QgsMeshDatasetGroupMetadata is a collection of dataset group metadata
such as whether the data is vector or scalar, name

.. note::

   The API is considered EXPERIMENTAL and can be changed without a notice

.. versionadded:: 3.4
%End

%TypeHeaderCode
#include "qgsmeshdataprovider.h"
%End
  public:

    enum DataType
    {
      DataOnFaces,
      DataOnVertices
    };

    QgsMeshDatasetGroupMetadata();
%Docstring
Constructs an empty metadata object
%End

    QgsMeshDatasetGroupMetadata( const QString &name,
                                 bool isScalar,
                                 bool isOnVertices,
                                 double minimum,
                                 double maximum,
                                 const QMap<QString, QString> &extraOptions );
%Docstring
Constructs a valid metadata object

:param name: name of the dataset group
:param isScalar: dataset contains scalar data, specifically the y-value of QgsMeshDatasetValue is NaN
:param isOnVertices: dataset values are defined on mesh's vertices. If ``False``, values are defined on faces.
:param minimum: minimum value (magnitude for vectors) present among all group's dataset values
:param maximum: maximum value (magnitude for vectors) present among all group's dataset values
:param extraOptions: dataset's extra options stored by the provider. Usually contains the name, time value, time units, data file vendor, ...
%End

    QString name() const;
%Docstring
Returns name of the dataset group
%End

    QMap<QString, QString> extraOptions() const;
%Docstring
Returns extra metadata options, for example description
%End

    bool isVector() const;
%Docstring
Returns whether dataset group has vector data
%End

    bool isScalar() const;
%Docstring
Returns whether dataset group has scalar data
%End

    DataType dataType() const;
%Docstring
Returns whether dataset group data is defined on vertices or faces
%End

    double minimum() const;
%Docstring
Returns minimum scalar value/vector magnitude present for whole dataset group
%End

    double maximum() const;
%Docstring
Returns maximum scalar value/vector magnitude present for whole dataset group
%End

};

class QgsMeshDatasetMetadata
{
%Docstring

QgsMeshDatasetMetadata is a collection of mesh dataset metadata such
as whether the data is valid or associated time for the dataset

.. note::

   The API is considered EXPERIMENTAL and can be changed without a notice

.. versionadded:: 3.2
%End

%TypeHeaderCode
#include "qgsmeshdataprovider.h"
%End
  public:
    QgsMeshDatasetMetadata();
%Docstring
Constructs an empty metadata object
%End

    QgsMeshDatasetMetadata( double time,
                            bool isValid,
                            double minimum,
                            double maximum
                          );
%Docstring
Constructs a valid metadata object

:param time: a time which this dataset represents in the dataset group
:param isValid: dataset is loadad and valid for fetching the data
:param minimum: minimum value (magnitude for vectors) present among dataset values
:param maximum: maximum value (magnitude for vectors) present among dataset values
%End

    double time() const;
%Docstring
Returns the time value for this dataset
%End

    bool isValid() const;
%Docstring
Returns whether dataset is valid
%End

    double minimum() const;
%Docstring
Returns minimum scalar value/vector magnitude present for the dataset
%End

    double maximum() const;
%Docstring
Returns maximum scalar value/vector magnitude present for the dataset
%End

};

class QgsMeshDataSourceInterface /Abstract/
{
%Docstring

Interface for mesh data sources

Mesh is a collection of vertices and faces in 2D or 3D space
- vertex - XY(Z) point (in the mesh's coordinate reference system)
- faces - sets of vertices forming a closed shape - typically triangles or quadrilaterals

Base on the underlying data provider/format, whole mesh is either stored in memory or
read on demand

.. note::

   The API is considered EXPERIMENTAL and can be changed without a notice

.. versionadded:: 3.2
%End

%TypeHeaderCode
#include "qgsmeshdataprovider.h"
%End
  public:
    virtual ~QgsMeshDataSourceInterface();

    virtual int vertexCount() const = 0;
%Docstring
Returns number of vertices in the native mesh

:return: Number of vertices in the mesh
%End

    virtual int faceCount() const = 0;
%Docstring
Returns number of faces in the native mesh

:return: Number of faces in the mesh
%End

    virtual void populateMesh( QgsMesh *mesh ) const = 0;
%Docstring
Populates the mesh vertices and faces

.. versionadded:: 3.6
%End
};

class QgsMeshDatasetSourceInterface /Abstract/
{
%Docstring
Interface for mesh datasets and dataset groups

Dataset is a  collection of vector or scalar values on vertices or faces of the mesh.
Based on the underlying data provider/format, whole dataset is either stored in memory
or read on demand

Datasets are grouped in the dataset groups. A dataset group represents a measured quantity
(e.g. depth or wind speed), dataset represents values of the quantity in a particular time.

.. note::

   The API is considered EXPERIMENTAL and can be changed without a notice

.. versionadded:: 3.2
%End

%TypeHeaderCode
#include "qgsmeshdataprovider.h"
%End
  public:
    virtual ~QgsMeshDatasetSourceInterface();

    virtual bool addDataset( const QString &uri ) = 0;
%Docstring
Associate dataset with the mesh

emits dataChanged when successful
%End

    virtual QStringList extraDatasets() const = 0;
%Docstring
Returns list of additional dataset file URIs added using addDataset() calls.
%End

    virtual int datasetGroupCount( ) const = 0;
%Docstring
Returns number of datasets groups loaded
%End

    virtual int datasetCount( int groupIndex ) const = 0;
%Docstring
Returns number of datasets loaded in the group
%End

    int datasetCount( QgsMeshDatasetIndex index ) const;
%Docstring
Returns number of datasets loaded in the group
%End

    virtual QgsMeshDatasetGroupMetadata datasetGroupMetadata( int groupIndex ) const = 0;
%Docstring
Returns dataset group metadata
%End

    QgsMeshDatasetGroupMetadata datasetGroupMetadata( QgsMeshDatasetIndex index ) const;
%Docstring
Returns dataset group metadata
%End

    virtual QgsMeshDatasetMetadata datasetMetadata( QgsMeshDatasetIndex index ) const = 0;
%Docstring
Returns dataset metadata
%End

    virtual QgsMeshDatasetValue datasetValue( QgsMeshDatasetIndex index, int valueIndex ) const = 0;
%Docstring
Returns vector/scalar value associated with the index from the dataset
To read multiple continuous values, use :py:func:`QgsMeshDatasetSourceInterface.datasetValues()`

See QgsMeshDatasetMetadata.isVector() or :py:func:`QgsMeshDataBlock.type()`
to check if the returned value is vector or scalar
%End

    virtual QgsMeshDataBlock datasetValues( QgsMeshDatasetIndex index, int valueIndex, int count ) const = 0;
%Docstring
Returns N vector/scalar values from the index from the dataset

See QgsMeshDatasetMetadata.isVector() to check if the returned value is vector or scalar

.. versionadded:: 3.6
%End

    virtual bool isFaceActive( QgsMeshDatasetIndex index, int faceIndex ) const = 0;
%Docstring
Returns whether the face is active for particular dataset

For example to represent the situation when F1 and F3 are flooded, but F2 is dry,
some solvers store water depth on vertices V1-V8 (all non-zero values) and
set active flag for F2 to ``False``.
V1 ---- V2 ---- V5-----V7
|   F1  |   F2   | F3  |
V3 ---- V4 ---- V6-----V8
%End

    virtual QgsMeshDataBlock areFacesActive( QgsMeshDatasetIndex index, int faceIndex, int count ) const = 0;
%Docstring
Returns whether the faces are active for particular dataset

.. versionadded:: 3.6
%End

    virtual bool persistDatasetGroup( const QString &path,
                                      const QgsMeshDatasetGroupMetadata &meta,
                                      const QVector<QgsMeshDataBlock> &datasetValues,
                                      const QVector<QgsMeshDataBlock> &datasetActive,
                                      const QVector<double> &times
                                    ) = 0;
%Docstring
Creates a new dataset group from a data and
persists it into a destination path

On success, the mesh's dataset group count is changed

:param path: destination path of the stored file
:param meta: new group's metadata
:param datasetValues: scalar/vector values for all datasets and all faces/vertices in the group
:param datasetActive: active flag values for all datasets in the group. Empty array represents can be used
                      when all faces are active
:param times: times in hours for all datasets in the group

:return: ``True`` on failure, ``False`` on success

.. versionadded:: 3.6
%End
};


class QgsMeshDataProvider: QgsDataProvider, QgsMeshDataSourceInterface, QgsMeshDatasetSourceInterface
{
%Docstring
Base class for providing data for :py:class:`QgsMeshLayer`

Responsible for reading native mesh data

.. note::

   The API is considered EXPERIMENTAL and can be changed without a notice

.. versionadded:: 3.2
%End

%TypeHeaderCode
#include "qgsmeshdataprovider.h"
%End
  public:
    QgsMeshDataProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions );
%Docstring
Ctor
%End

  signals:
    void datasetGroupsAdded( int count );
%Docstring
Emitted when some new dataset groups have been added
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/mesh/qgsmeshdataprovider.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
