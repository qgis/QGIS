/***************************************************************************
  qgsbillboardgeometry.h
  --------------------------------------
  Date                 : Jul 2019
  Copyright            : (C) 2019 by Ismail Sunni
  Email                : imajimatika at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSBILLBOARDGEOMETRY_H
#define QGSBILLBOARDGEOMETRY_H

#include <QObject>
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QAttribute>


/**
 * \ingroup 3d
 * Geometry of the billboard rendering for points in 3D map view.
 *
 * \since QGIS 3.10
 */
class QgsBillboardGeometry : public Qt3DRender::QGeometry
{
    Q_OBJECT

    Q_PROPERTY( int count READ count NOTIFY countChanged )
  public:
    //! Constructor of QgsBillboardGeometry.
    QgsBillboardGeometry( Qt3DCore::QNode *parent = nullptr );

    //! Set the points for the billboard with \a vertices.
    void setPoints( const QVector<QVector3D> &vertices );

    //! Returns the number of points.
    int count() const;

  signals:
    //! Signal when the number of points changed.
    void countChanged( int count );

  private:
    Qt3DRender::QAttribute *mPositionAttribute = nullptr;
    Qt3DRender::QBuffer *mVertexBuffer = nullptr;
    int mVertexCount = 0;
};

#endif // QGSBILLBOARDGEOMETRY_H
