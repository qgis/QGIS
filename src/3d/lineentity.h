#ifndef LINEENTITY_H
#define LINEENTITY_H

#include <Qt3DCore/QEntity>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QGeometryRenderer>

class Map3D;
class PolygonGeometry;
class QgsLine3DSymbol;

class QgsVectorLayer;
class QgsFeatureRequest;


//! Entity that handles rendering of linestrings
class LineEntity : public Qt3DCore::QEntity
{
  public:
    LineEntity( const Map3D &map, QgsVectorLayer *layer, const QgsLine3DSymbol &symbol, Qt3DCore::QNode *parent = nullptr );

  private:
    void addEntityForSelectedLines( const Map3D &map, QgsVectorLayer *layer, const QgsLine3DSymbol &symbol );
    void addEntityForNotSelectedLines( const Map3D &map, QgsVectorLayer *layer, const QgsLine3DSymbol &symbol );

    Qt3DExtras::QPhongMaterial *material( const QgsLine3DSymbol &symbol ) const;
};

class LineEntityNode : public Qt3DCore::QEntity
{
  public:
    LineEntityNode( const Map3D &map, QgsVectorLayer *layer, const QgsLine3DSymbol &symbol, const QgsFeatureRequest &req, Qt3DCore::QNode *parent = nullptr );

  private:
    Qt3DRender::QGeometryRenderer *renderer( const Map3D &map, const QgsLine3DSymbol &symbol, const QgsVectorLayer *layer, const QgsFeatureRequest &req );

    PolygonGeometry *mGeometry;
};

#endif // LINEENTITY_H
