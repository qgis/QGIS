#ifndef POLYGONENTITY_H
#define POLYGONENTITY_H

#include <Qt3DCore/QEntity>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QGeometryRenderer>

class Map3D;
class PolygonGeometry;
class QgsPolygon3DSymbol;

class QgsPointXY;
class QgsVectorLayer;
class QgsFeatureRequest;


//! Entity that handles rendering of polygons
class PolygonEntity : public Qt3DCore::QEntity
{
  public:
    PolygonEntity( const Map3D &map, QgsVectorLayer *layer, const QgsPolygon3DSymbol &symbol, Qt3DCore::QNode *parent = nullptr );

  private:
    void addEntityForSelectedPolygons( const Map3D &map, QgsVectorLayer *layer, const QgsPolygon3DSymbol &symbol );
    void addEntityForNotSelectedPolygons( const Map3D &map, QgsVectorLayer *layer, const QgsPolygon3DSymbol &symbol );

    Qt3DExtras::QPhongMaterial *material( const QgsPolygon3DSymbol &symbol ) const;
};

class PolygonEntityNode : public Qt3DCore::QEntity
{
  public:
    PolygonEntityNode( const Map3D &map, QgsVectorLayer *layer, const QgsPolygon3DSymbol &symbol, const QgsFeatureRequest &req, Qt3DCore::QNode *parent = nullptr );

  private:
    Qt3DRender::QGeometryRenderer *renderer( const Map3D &map, const QgsPolygon3DSymbol &symbol, const QgsVectorLayer *layer, const QgsFeatureRequest &request );

    PolygonGeometry *mGeometry;
};

#endif // POLYGONENTITY_H
