#ifndef CHUNKBOUNDSENTITY_H
#define CHUNKBOUNDSENTITY_H

#include <Qt3DCore/QEntity>

class AABB;
class AABBMesh;

//! Draws bounds of axis aligned bounding boxes
class ChunkBoundsEntity : public Qt3DCore::QEntity
{
  public:
    ChunkBoundsEntity( Qt3DCore::QNode *parent = nullptr );

    void setBoxes( const QList<AABB> &bboxes );

  private:
    AABBMesh *aabbMesh;
};

#endif // CHUNKBOUNDSENTITY_H
