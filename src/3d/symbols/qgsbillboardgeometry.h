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

class QgsBillboardGeometry : public Qt3DRender::QGeometry
{
    Q_OBJECT

    Q_PROPERTY( int count READ count NOTIFY countChanged )
  public:
    QgsBillboardGeometry( Qt3DCore::QNode *parent = nullptr );

    void setPoints( const QVector<QVector3D> &vertices );

    int count();

  signals:
    void countChanged( int count );

  private:
    Qt3DRender::QAttribute *mPositionAttribute = nullptr;
    Qt3DRender::QBuffer *mVertexBuffer = nullptr;
    int mVertexCount = 0;
};

#endif // QGSBILLBOARDGEOMETRY_H
