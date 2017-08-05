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

  private:
    void addEntityForSelectedPoints( const Map3D &map, QgsVectorLayer *layer, const Point3DSymbol &symbol );
    void addEntityForNotSelectedPoints( const Map3D &map, QgsVectorLayer *layer, const Point3DSymbol &symbol );

    Qt3DRender::QMaterial *material( const Point3DSymbol &symbol ) const;
};

class PointEntityNode : public Qt3DCore::QEntity
{
  public:
    PointEntityNode( const Map3D &map, QgsVectorLayer *layer, const Point3DSymbol &symbol, const QgsFeatureRequest &req, Qt3DCore::QNode *parent = nullptr );

  private:
    Qt3DRender::QGeometryRenderer *renderer( const Point3DSymbol &symbol, const QList<QVector3D> &positions ) const;
    QList<QVector3D> positions( const Map3D &map, const QgsVectorLayer *layer, const QgsFeatureRequest &req ) const;
};

#endif // POINTENTITY_H
