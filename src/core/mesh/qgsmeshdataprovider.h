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

#include "qgis_core.h"
#include "qgspoint.h"
#include "qgsrectangle.h"
#include "qgsdataprovider.h"

#include <QVector>
#include <QString>
#include <QMap>
#include <limits>

//! xyz coords of vertex
typedef QgsPoint QgsMeshVertex;

//! List of vertex indexes
typedef QVector<int> QgsMeshFace;

/**
 * \ingroup core
 *
 * QgsMeshDatasetValue represents single mesh dataset value
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
 * QgsMeshDatasetMetadata is a collection of mesh dataset metadata such
 * as whether the data is vector or scalar, etc.
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
     * \param isScalar dataset contains scalar data, particulary the y-value of QgsMeshDatasetValue is NaN
     * \param isValid dataset is loadad and valid for fetching the data
     * \param isOnVertices dataset values are defined on mesh's vertices. If false, values are defined on faces.
     * \param extraOptions dataset's extra options stored by the provider. Usually contains the name, time value, time units, data file vendor, ...
     */
    QgsMeshDatasetMetadata( bool isScalar,
                            bool isValid,
                            bool isOnVertices,
                            const QMap<QString, QString> &extraOptions );

    /**
     * Returns extra metadata options
     * Usually including name, description or time variable
     */
    QMap<QString, QString> extraOptions() const;

    /**
     * \brief Returns whether dataset has vector data
     */
    bool isVector() const;

    /**
     * \brief Returns whether dataset has scalar data
     */
    bool isScalar() const;

    /**
     * \brief Returns whether dataset data is defined on vertices
     */
    bool isOnVertices() const;

    /**
     * \brief Returns whether dataset is valid
     */
    bool isValid() const;

  private:
    bool mIsScalar = false;
    bool mIsValid = false;
    bool mIsOnVertices = false;
    QMap<QString, QString> mExtraOptions;
};

/**
 * \ingroup core
 *
 * Interface for mesh data sources
 *
 * Mesh is a  collection of vertices and faces in 2D or 3D space
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
     * Returns the mesh vertex at index
     */
    virtual QgsMeshVertex vertex( int index ) const = 0;

    /**
     * Returns the mesh face at index
     */
    virtual QgsMeshFace face( int index ) const = 0;
};

/**
 * \ingroup core
 * Interface for mesh datasets
 *
 *  Dataset is a  collection of vector or scalar values on vertices or faces of the mesh.
 *  Based on the underlying data provider/format, whole dataset is either stored in memory
 *  or read on demand
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
     */
    virtual bool addDataset( const QString &uri ) = 0;

    /**
     * \brief Returns number of datasets loaded
     */
    virtual int datasetCount() const = 0;

    /**
     * \brief Returns dataset metadata
     */
    virtual QgsMeshDatasetMetadata datasetMetadata( int datasetIndex ) const = 0;

    /**
     * \brief Returns vector/scalar value associated with the index from the dataset
     *
     * See QgsMeshDatasetMetadata::isVector() to check if the returned value is vector or scalar
     */
    virtual QgsMeshDatasetValue datasetValue( int datasetIndex, int valueIndex ) const = 0;
};


/**
 * \ingroup core
 * Base class for providing data for QgsMeshLayer
 *
 * Responsible for reading native mesh data
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \see QgsMeshSource
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshDataProvider: public QgsDataProvider, public QgsMeshDataSourceInterface, public QgsMeshDatasetSourceInterface
{
    Q_OBJECT

  public:
    //! Ctor
    QgsMeshDataProvider( const QString &uri = QString() );

    /**
     * Returns the extent of the layer
     * \returns QgsRectangle containing the extent of the layer
     */
    virtual QgsRectangle extent() const;
};

#endif // QGSMESHDATAPROVIDER_H
