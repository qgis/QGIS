#ifndef LINEENTITY_H
#define LINEENTITY_H

#include <Qt3DCore/QEntity>

class Map3D;
class PolygonGeometry;
class Line3DSymbol;

class QgsVectorLayer;


//! Entity that handles rendering of linestrings
class LineEntity : public Qt3DCore::QEntity
{
  public:
    LineEntity( const Map3D &map, QgsVectorLayer *layer, const Line3DSymbol &symbol, Qt3DCore::QNode *parent = nullptr );

    PolygonGeometry *geometry;
};

#endif // LINEENTITY_H
