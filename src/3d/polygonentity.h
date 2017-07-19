#ifndef POLYGONENTITY_H
#define POLYGONENTITY_H

#include <Qt3DCore/QEntity>

class Map3D;
class PolygonGeometry;
class PolygonRenderer;

class QgsPointXY;
class QgsVectorLayer;


//! Entity that handles rendering of polygons
class PolygonEntity : public Qt3DCore::QEntity
{
  public:
    PolygonEntity( const Map3D &map, const PolygonRenderer &settings, Qt3DCore::QNode *parent = nullptr );

    PolygonGeometry *geometry;
};

#endif // POLYGONENTITY_H
