#ifndef CHUNKBOUNDSENTITY_H
#define CHUNKBOUNDSENTITY_H

#include <Qt3DCore/QEntity>

class AABB;
class AABBMesh;

///@cond PRIVATE

/** \ingroup 3d
 * Draws bounds of axis aligned bounding boxes
 * \since QGIS 3.0
 */
class ChunkBoundsEntity : public Qt3DCore::QEntity
{
  public:
    //! Constructs the entity
    ChunkBoundsEntity( Qt3DCore::QNode *parent = nullptr );

    //! Sets a list of bounding boxes to be rendered by the entity
    void setBoxes( const QList<AABB> &bboxes );

  private:
    AABBMesh *aabbMesh;
};

/// @endcond

#endif // CHUNKBOUNDSENTITY_H
