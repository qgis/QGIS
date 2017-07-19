#ifndef LINEENTITY_H
#define LINEENTITY_H

#include <Qt3DCore/QEntity>

class Map3D;
class PolygonGeometry;
class LineRenderer;

//! Entity that handles rendering of linestrings
class LineEntity : public Qt3DCore::QEntity
{
  public:
    LineEntity( const Map3D &map, const LineRenderer &settings, Qt3DCore::QNode *parent = nullptr );

    PolygonGeometry *geometry;
};

#endif // LINEENTITY_H
