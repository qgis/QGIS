#ifndef POINTENTITY_H
#define POINTENTITY_H

#include <Qt3DCore/QEntity>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QGeometryRenderer>

class Map3D;
class Point3DSymbol;

class QgsVectorLayer;
class QgsFeatureRequest;


//! Entity that handles rendering of points as 3D objects
class PointEntity : public Qt3DCore::QEntity
{
  public:
    PointEntity( const Map3D &map, QgsVectorLayer *layer, const Point3DSymbol &symbol, Qt3DCore::QNode *parent = nullptr );

    PointEntity( const Map3D &map, QgsVectorLayer *layer, const Point3DSymbol &symbol, bool sel, Qt3DCore::QNode *parent = nullptr );

  private:
    Qt3DRender::QGeometryRenderer *renderer( const Point3DSymbol &symbol, const QList<QVector3D> &positions ) const;
    Qt3DRender::QMaterial *material( const Map3D &map, const Point3DSymbol &symbol, bool sel = false ) const;
    QList<QVector3D> positions( const Map3D &map, const QgsVectorLayer *layer, const QgsFeatureRequest &req ) const;
};

#endif // POINTENTITY_H
