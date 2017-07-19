#ifndef POINTENTITY_H
#define POINTENTITY_H

#include <Qt3DCore/QEntity>

class Map3D;
class PointRenderer;

class PointEntity : public Qt3DCore::QEntity
{
  public:
    PointEntity( const Map3D &map, const PointRenderer &settings, Qt3DCore::QNode *parent = nullptr );
};

#endif // POINTENTITY_H
