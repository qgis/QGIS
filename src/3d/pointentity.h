#ifndef POINTENTITY_H
#define POINTENTITY_H

#include <Qt3DCore/QEntity>

class Map3D;
class Point3DSymbol;

class QgsVectorLayer;


//! Entity that handles rendering of points as 3D objects
class PointEntity : public Qt3DCore::QEntity
{
  public:
    PointEntity( const Map3D &map, QgsVectorLayer *layer, const Point3DSymbol &symbol, Qt3DCore::QNode *parent = nullptr );
};

#endif // POINTENTITY_H
