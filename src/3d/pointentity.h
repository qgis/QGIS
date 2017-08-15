#ifndef POINTENTITY_H
#define POINTENTITY_H

#include <Qt3DCore/QEntity>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DCore/QTransform>

class Map3D;
class QgsPoint3DSymbol;

class QgsVectorLayer;
class QgsFeatureRequest;


//! Entity that handles rendering of points as 3D objects
class PointEntity : public Qt3DCore::QEntity
{
  public:
    PointEntity( const Map3D &map, QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol, Qt3DCore::QNode *parent = nullptr );
};

class InstancedPointEntityFactory
{
  public:
    static void addEntityForSelectedPoints( const Map3D &map, QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol, PointEntity *parent );
    static void addEntityForNotSelectedPoints( const Map3D &map, QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol, PointEntity *parent );

  private:
    static Qt3DRender::QMaterial *material( const QgsPoint3DSymbol &symbol );
};

class InstancedPointEntityNode : public Qt3DCore::QEntity
{
  public:
    InstancedPointEntityNode( const Map3D &map, QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol, const QgsFeatureRequest &req, Qt3DCore::QNode *parent = nullptr );

  private:
    Qt3DRender::QGeometryRenderer *renderer( const QgsPoint3DSymbol &symbol, const QList<QVector3D> &positions ) const;
};

class Model3DPointEntityFactory
{
  public:
    static void addEntitiesForSelectedPoints( const Map3D &map, QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol, PointEntity *parent );
    static void addEntitiesForNotSelectedPoints( const Map3D &map, QgsVectorLayer *layer, const QgsPoint3DSymbol &symbol, PointEntity *parent );

  private:
    static void addSceneEntities( const Map3D &map, QgsVectorLayer *layer, const QgsFeatureRequest &req, const QgsPoint3DSymbol &symbol, PointEntity *parent );
    static void addMeshEntities( const Map3D &map, QgsVectorLayer *layer, const QgsFeatureRequest &req, const QgsPoint3DSymbol &symbol, PointEntity *parent, bool are_selected );

    static Qt3DCore::QTransform *transform( const QVector3D &position, const QgsPoint3DSymbol &symbol );
};

#endif // POINTENTITY_H
