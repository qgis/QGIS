/***************************************************************************
    qgs3deditutils.h
    ---------------------
    begin                : February 2025
    copyright            : (C) 2025 by Matej Bagar
    email                : matej dot bagar at lutraconsulting dot co dot uk
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DEDITUTILS_H
#define QGS3DEDITUTILS_H

#define SIP_NO_FILE
#include "qgsvector3d.h"

#include <QMatrix4x4>

class QgsBox3D;
class QgsGeos;
class QgsPointCloudLayer;
class Qgs3DMapCanvas;
class QgsGeometry;
class QgsPointCloudNodeId;
typedef QHash<QgsPointCloudNodeId, QVector<int> > SelectedPoints;

struct MapToPixel3D
{
    QMatrix4x4 VP;      // combined view-projection matrix
    QgsVector3D origin; // shift of world coordinates
    QSize canvasSize;

    QPointF transform( double x, double y, double z, bool *isInView = nullptr ) const
    {
      const QVector4D cClip = VP * QVector4D( static_cast<float>( x - origin.x() ), static_cast<float>( y - origin.y() ), static_cast<float>( z - origin.z() ), 1 );
      // normalized device coordinates (-1 to +1)
      // z == -1 is near plane, z == +1 is far plane
      const float xNdc = cClip.x() / cClip.w();
      const float yNdc = cClip.y() / cClip.w();
      const float zNdc = cClip.z() / cClip.w();
      if ( isInView )
        *isInView = !( zNdc < -1 || zNdc > 1 || yNdc < -1 || yNdc > 1 || zNdc < -1 || xNdc > 1 );

      // window / screen space coordinates
      const float xScreen = ( xNdc + 1 ) * 0.5f * static_cast<float>( canvasSize.width() );
      const float yScreen = ( -yNdc + 1 ) * 0.5f * static_cast<float>( canvasSize.height() );

      return QPointF( xScreen, yScreen );
    }
};

/**
 * \ingroup 3d
 * \brief Utility functions used by 3D editing tools.
 * \note Not available in Python bindings
 * \since QGIS 3.44
 */
class Qgs3DEditUtils
{
  public:
    /**
     * Changes the value of \a attributeName to \a newValue for points inside \a geometry
     * \note \a geometry is expected in screen coordinates
     */
    static void changeAttributeValue( const QgsGeometry &geometry, const QString &attributeName, double newValue, Qgs3DMapCanvas &canvas );

  private:
    static SelectedPoints searchPoints( QgsPointCloudLayer *layer, const QgsGeos &searchPolygon, Qgs3DMapCanvas &canvas );

    static QVector<int> selectedPointsInNode( const QgsGeos &searchPolygon, const QgsPointCloudNodeId &n, const MapToPixel3D &mapToPixel3D, QgsPointCloudLayer *layer, Qgs3DMapCanvas &canvas );

    static QgsGeometry box3DToPolygonInScreenSpace( const QgsBox3D &box, const MapToPixel3D &mapToPixel3D );
};


#endif //QGS3DEDITUTILS_H
