#ifndef QGSLINE3DSYMBOL_P_H
#define QGSLINE3DSYMBOL_P_H

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include <Qt3DCore/QEntity>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QGeometryRenderer>

class Map3D;
class PolygonGeometry;
class QgsLine3DSymbol;

class QgsVectorLayer;
class QgsFeatureRequest;


//! Entity that handles rendering of linestrings
class QgsLine3DSymbolEntity : public Qt3DCore::QEntity
{
  public:
    QgsLine3DSymbolEntity( const Map3D &map, QgsVectorLayer *layer, const QgsLine3DSymbol &symbol, Qt3DCore::QNode *parent = nullptr );

  private:
    void addEntityForSelectedLines( const Map3D &map, QgsVectorLayer *layer, const QgsLine3DSymbol &symbol );
    void addEntityForNotSelectedLines( const Map3D &map, QgsVectorLayer *layer, const QgsLine3DSymbol &symbol );

    Qt3DExtras::QPhongMaterial *material( const QgsLine3DSymbol &symbol ) const;
};

class QgsLine3DSymbolEntityNode : public Qt3DCore::QEntity
{
  public:
    QgsLine3DSymbolEntityNode( const Map3D &map, QgsVectorLayer *layer, const QgsLine3DSymbol &symbol, const QgsFeatureRequest &req, Qt3DCore::QNode *parent = nullptr );

  private:
    Qt3DRender::QGeometryRenderer *renderer( const Map3D &map, const QgsLine3DSymbol &symbol, const QgsVectorLayer *layer, const QgsFeatureRequest &req );

    PolygonGeometry *mGeometry;
};

/// @endcond

#endif // QGSLINE3DSYMBOL_P_H
