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

typedef QgsPoint QgsMeshVertex; //xyz coords of vertex
typedef QVector<int> QgsMeshFace; //list of vertex indexes

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
  * Base class for providing data for QgsMeshLayer
  *
  * Responsible for reading native mesh data
  *
  * \see QgsMeshSource
  *
  */
class CORE_EXPORT QgsMeshDataProvider: public QgsDataProvider, public QgsMeshSource
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
