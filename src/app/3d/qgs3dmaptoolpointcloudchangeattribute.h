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
#include "qgschunknode.h"

#include <QMatrix4x4>
#include <QVector>

class QgsRubberBand3D;
class QgsPointCloudLayer;


struct MapToPixel3D
{
    QMatrix4x4 VP;      // combined view-projection matrix
    QgsVector3D origin; // shift of world coordinates
    QSize canvasSize;

    QPointF transform( double x, double y, double z ) const
    {
      QVector4D cClip = VP * QVector4D( x - origin.x(), y - origin.y(), z - origin.z(), 1 );
      float xNdc = cClip.x() / cClip.w();
      float yNdc = cClip.y() / cClip.w();
      float xScreen = ( xNdc + 1 ) * 0.5 * canvasSize.width();
      float yScreen = ( -yNdc + 1 ) * 0.5 * canvasSize.height();
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
    SelectedPoints searchPoints( QgsPointCloudLayer *layer, const QgsGeometry &searchPolygon ) const;
    static QVector<int> selectedPointsInNode( const QgsGeometry &searchPolygon, const QgsChunkNode *ch, const MapToPixel3D &mapToPixel3D, QgsPointCloudIndex &pcIndex );
    static QgsGeometry box3DToPolygonInScreenSpace( QgsBox3D box, const MapToPixel3D &mapToPixel3D );

    QVector<QgsPointXY> mScreenPoints;
    QgsRubberBand3D *mPolygonRubberBand = nullptr;
    QString mAttributeName;
    double mNewValue = 0;
    QPoint mClickPoint;
};

#endif // QGS3DMAPTOOLPOINTCLOUDCHANGEATTRIBUTE_H
