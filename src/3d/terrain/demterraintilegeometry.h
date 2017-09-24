
#ifndef TERRAINTILEGEOMETRY_H
#define TERRAINTILEGEOMETRY_H

#include <Qt3DExtras/qt3dextras_global.h>
#include <Qt3DRender/qgeometry.h>
#include <QSize>

#include <QImage>

namespace Qt3DRender
{

  class QAttribute;
  class QBuffer;

} // Qt3DRender


/** \ingroup 3d
 * Stores attributes and vertex/index buffers for one terrain tile based on DEM.
 * \since QGIS 3.0
 */
class DemTerrainTileGeometry : public Qt3DRender::QGeometry
{
    Q_OBJECT
    Q_PROPERTY( Qt3DRender::QAttribute *positionAttribute READ positionAttribute CONSTANT )
    Q_PROPERTY( Qt3DRender::QAttribute *normalAttribute READ normalAttribute CONSTANT )
    Q_PROPERTY( Qt3DRender::QAttribute *texCoordAttribute READ texCoordAttribute CONSTANT )
    Q_PROPERTY( Qt3DRender::QAttribute *indexAttribute READ indexAttribute CONSTANT )

  public:
    //! Constructs a terrain tile geometry. Resolution is the number of vertices on one side of the tile,
    //! heightMap is array of float values with one height value for each vertex
    explicit DemTerrainTileGeometry( int resolution, const QByteArray &heightMap, QNode *parent = nullptr );
    ~DemTerrainTileGeometry();

    //! Returns geometry attribute for vertex positions
    Qt3DRender::QAttribute *positionAttribute() const;
    //! Returns geometry attribute for vertex normals
    Qt3DRender::QAttribute *normalAttribute() const;
    //! Returns geometry attribute for texture coordinates for vertices
    Qt3DRender::QAttribute *texCoordAttribute() const;
    //! Returns attribute for indices of triangles
    Qt3DRender::QAttribute *indexAttribute() const;

  private:
    void init();

    int m_resolution;
    QByteArray m_heightMap;
    Qt3DRender::QAttribute *m_positionAttribute;
    Qt3DRender::QAttribute *m_normalAttribute;
    Qt3DRender::QAttribute *m_texCoordAttribute;
    Qt3DRender::QAttribute *m_indexAttribute;
    Qt3DRender::QBuffer *m_vertexBuffer;
    Qt3DRender::QBuffer *m_indexBuffer;
};


#endif
