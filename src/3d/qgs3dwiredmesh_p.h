/***************************************************************************
  qgs3dwiredmesh_p.h
  --------------------------------------
  Date                 : March 2022
  Copyright            : (C) 2022 by Jean Felder
  Email                : jean dot felder at oslandia dot com
  ***************************************************************************
  *                                                                         *
  *   This program is free software; you can redistribute it and/or modify  *
  *   it under the terms of the GNU General Public License as published by  *
  *   the Free Software Foundation; either version 2 of the License, or     *
  *   (at your option) any later version.                                   *
  *                                                                         *
 ***************************************************************************/

#ifndef QGS3DWIREDMESH_P_H
#define QGS3DWIREDMESH_P_H

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include <QVector3D>

#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
#include <Qt3DRender/QBuffer>
#else
#include <Qt3DCore/QBuffer>
#endif
#include <Qt3DRender/QGeometryRenderer>

class QgsAABB;

#define SIP_NO_FILE

/**
 * \ingroup 3d
 * \brief Geometry renderer for lines, draws a wired mesh
 *
 * \since QGIS 3.26
 */
class Qgs3DWiredMesh : public Qt3DRender::QGeometryRenderer
{
    Q_OBJECT

  public:
    /**
     * \brief Default Qgs3DWiredMesh constructor
     */
    Qgs3DWiredMesh( Qt3DCore::QNode *parent = nullptr );
    ~Qgs3DWiredMesh() override;

    /**
     * \brief add or replace mesh vertices coordinates
     */
    void setVertices( const QList<QVector3D> &vertices );

    /**
     * \brief add or replace mesh vertices coordinates from QgsAABB coordinates
     */
    void setVertices( const QList<QgsAABB> &bboxes );

  private:
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
    Qt3DRender::QGeometry *mGeom = nullptr;
    Qt3DRender::QAttribute *mPositionAttribute = nullptr;
    Qt3DRender::QBuffer *mVertexBuffer = nullptr;
#else
    Qt3DCore::QGeometry *mGeom = nullptr;
    Qt3DCore::QAttribute *mPositionAttribute = nullptr;
    Qt3DCore::QBuffer *mVertexBuffer = nullptr;
#endif
};

/// @endcond

#endif // QGS3DWIREDMESH_P_H
