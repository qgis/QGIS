
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


class DemTerrainTileGeometry : public Qt3DRender::QGeometry
{
    Q_OBJECT
    Q_PROPERTY( Qt3DRender::QAttribute *positionAttribute READ positionAttribute CONSTANT )
    Q_PROPERTY( Qt3DRender::QAttribute *normalAttribute READ normalAttribute CONSTANT )
    Q_PROPERTY( Qt3DRender::QAttribute *texCoordAttribute READ texCoordAttribute CONSTANT )
    Q_PROPERTY( Qt3DRender::QAttribute *indexAttribute READ indexAttribute CONSTANT )

  public:
    explicit DemTerrainTileGeometry( int resolution, const QByteArray &heightMap, QNode *parent = nullptr );
    ~DemTerrainTileGeometry();

    //void updateVertices();
    //void updateIndices();

    void setHeightMap( const QByteArray &heightMap );

    Qt3DRender::QAttribute *positionAttribute() const;
    Qt3DRender::QAttribute *normalAttribute() const;
    Qt3DRender::QAttribute *texCoordAttribute() const;
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
