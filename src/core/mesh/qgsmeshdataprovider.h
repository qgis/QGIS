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

//! Dataset's metadata key:value map
typedef QMap<QString, QString> QgsMeshDatasetMetadata;

/**
 * \ingroup core
 *
 * QgsMeshDatasetValue is a vector or a scalar value on vertex or face of the mesh with
 * support of nodata values
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshDatasetValue
{
    Q_GADGET

  public:
    QgsMeshDatasetValue( double x,
                         double y );
    QgsMeshDatasetValue( double scalar );
    QgsMeshDatasetValue( ) = default;

    ~QgsMeshDatasetValue() = default;
    void setNodata( bool nodata = true );
    bool isNodata() const;
    bool isScalar() const;
    double scalar() const; //length for vectors, value for scalars
    void set( double scalar );
    void setX( double x );
    void setY( double y ) ;
    double x() const;
    double y() const;
    bool operator==( const QgsMeshDatasetValue &other ) const;

  private:
    double mX  = std::numeric_limits<double>::quiet_NaN();
    double mY  = std::numeric_limits<double>::quiet_NaN();
    bool mIsNodata = true;
    bool mIsScalar = true;
};

/**
 * \ingroup core
 * Mesh is a  collection of vertices and faces in 2D or 3D space
 *  - vertex - XY(Z) point (in the mesh's coordinate reference system)
 *  - faces - sets of vertices forming a closed shape - typically triangles or quadrilaterals
 *
 * Base on the underlying data provider/format, whole mesh is either stored in memory or
 * read on demand
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshSource SIP_ABSTRACT
{
  public:
    //! Dtor
    virtual ~QgsMeshSource() = default;

    /**
     * \brief Return number of vertices in the native mesh
     * \returns Number of vertices in the mesh
     */
    virtual int vertexCount() const = 0;

    /**
     * \brief Return number of faces in the native mesh
     * \returns Number of faces in the mesh
     */
    virtual int faceCount() const = 0;

    /**
     * \brief Factory for mesh vertex with index
     * \returns new mesh vertex on index
     */
    virtual QgsMeshVertex vertex( int index ) const = 0;

    /**
     * \brief Factory for mesh face with index
     * \returns new mesh face on index
     */
    virtual QgsMeshFace face( int index ) const = 0;
};

/**
 * \ingroup core
 * Dataset is a  collection of vector or scalar values on vertices or faces of the mesh
 *
 * Base on the underlying data provider/format, whole dataset is either stored in memory or
 * read on demand
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshDatasetSource SIP_ABSTRACT
{
  public:
    //! Dtor
    virtual ~QgsMeshDatasetSource() = default;

    /**
     * \brief Associate dataset with the mesh
     */
    virtual bool addDataset( const QString &uri ) = 0;

    /**
     * \brief Return number of datasets loaded
     */
    virtual int datasetCount() const = 0;

    /**
     * \brief Whether dataset has scalar data associated
     */
    virtual bool datasetHasScalarData( int index ) const = 0;

    /**
     * \brief Whether dataset is on vertices
     */
    virtual bool datasetIsOnVertices( int index ) const = 0;

    /**
     * \brief Return dataset metadata
     */
    virtual QgsMeshDatasetMetadata datasetMetadata( int index ) const = 0;

    /**
     * \brief Return value associated with the index from the dataset
     */
    virtual QgsMeshDatasetValue datasetValue( int datasetIndex, int valueIndex ) const = 0;

    /**
     * \brief Return whether dataset is valid
     */
    virtual bool datasetIsValid( int index ) const = 0;
};


/**
 * \ingroup core
 * Base class for providing data for QgsMeshLayer
 *
 * Responsible for reading native mesh data
 *
 * \see QgsMeshSource
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMeshDataProvider: public QgsDataProvider, public QgsMeshSource, public QgsMeshDatasetSource
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
