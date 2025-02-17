/***************************************************************************
    qgs3dmaptoolpointcloudchangeattribute.h
    ---------------------
    begin                : January 2025
    copyright            : (C) 2025 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DMAPTOOLPOINTCLOUDCHANGEATTRIBUTE_H
#define QGS3DMAPTOOLPOINTCLOUDCHANGEATTRIBUTE_H

#include "qgs3dmaptool.h"
#include "qgspointxy.h"
#include "qgsgeometry.h"
#include "qgspointcloudindex.h"

#include <QMatrix4x4>
#include <QVector>

class QgsRubberBand3D;
class QgsPointCloudLayer;
class QgsGeos;


struct MapToPixel3D
{
    QMatrix4x4 VP;      // combined view-projection matrix
    QgsVector3D origin; // shift of world coordinates
    QSize canvasSize;

    QPointF transform( double x, double y, double z, bool *isInView = nullptr ) const
    {
      QVector4D cClip = VP * QVector4D( static_cast<float>( x - origin.x() ), static_cast<float>( y - origin.y() ), static_cast<float>( z - origin.z() ), 1 );
      // normalized device coordinates (-1 to +1)
      // z == -1 is near plane, z == +1 is far plane
      const float xNdc = cClip.x() / cClip.w();
      const float yNdc = cClip.y() / cClip.w();
      const float zNdc = cClip.z() / cClip.w();
      if ( isInView )
        *isInView = !( zNdc < -1 || zNdc > 1 || yNdc < -1 || yNdc > 1 || zNdc < -1 || xNdc > 1 );

      // window / sceen space coordinates
      const float xScreen = ( xNdc + 1 ) * 0.5f * static_cast<float>( canvasSize.width() );
      const float yScreen = ( -yNdc + 1 ) * 0.5f * static_cast<float>( canvasSize.height() );

      return QPointF( xScreen, yScreen );
    }
};


typedef QHash<QgsPointCloudNodeId, QVector<int> > SelectedPoints;


class Qgs3DMapToolPointCloudChangeAttribute : public Qgs3DMapTool
{
    Q_OBJECT

  public:
    Qgs3DMapToolPointCloudChangeAttribute( Qgs3DMapCanvas *canvas );
    ~Qgs3DMapToolPointCloudChangeAttribute() override;

    void mousePressEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;

    void activate() override;
    void deactivate() override;

    bool allowsCameraControls() const override { return false; }

    void setAttribute( const QString &attribute );
    void setNewValue( double value );

  private:
    void run();
    void restart();
    QgsPoint screenPointToMap( const QPoint &pos ) const;
    SelectedPoints searchPoints( QgsPointCloudLayer *layer, const QgsGeos &searchPolygon ) const;
    QVector<int> selectedPointsInNode( const QgsGeos &searchPolygon, const QgsPointCloudNodeId &n, const MapToPixel3D &mapToPixel3D, QgsPointCloudLayer *layer ) const;
    static QgsGeometry box3DToPolygonInScreenSpace( const QgsBox3D &box, const MapToPixel3D &mapToPixel3D );

    QVector<QgsPointXY> mScreenPoints;
    QgsRubberBand3D *mPolygonRubberBand = nullptr;
    QString mAttributeName;
    double mNewValue = 0;
    QPoint mClickPoint;
};

#endif // QGS3DMAPTOOLPOINTCLOUDCHANGEATTRIBUTE_H
