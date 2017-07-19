#ifndef TERRAINBOUNDSENTITY_H
#define TERRAINBOUNDSENTITY_H

#include <Qt3DCore/QEntity>

class AABB;
class AABBMesh;

//! Draws bounds of axis aligned bounding boxes
class TerrainBoundsEntity : public Qt3DCore::QEntity
{
  public:
    TerrainBoundsEntity( Qt3DCore::QNode *parent = nullptr );

    void setBoxes( const QList<AABB> &bboxes );

  private:
    AABBMesh *aabbMesh;
};

#endif // TERRAINBOUNDSENTITY_H
