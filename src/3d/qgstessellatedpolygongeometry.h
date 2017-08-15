#ifndef QGSTESSELLATEDPOLYGONGEOMETRY_H
#define QGSTESSELLATEDPOLYGONGEOMETRY_H

#include "qgspolygon.h"

#include <Qt3DRender/QGeometry>

namespace Qt3DRender
{
  class QBuffer;
}

//! Class that represents polygons tessellated into 3D geometry
class QgsTessellatedPolygonGeometry : public Qt3DRender::QGeometry
{
  public:
    QgsTessellatedPolygonGeometry( QNode *parent = nullptr );
    ~QgsTessellatedPolygonGeometry();

    // takes ownership of passed polygon geometries
    void setPolygons( const QList<QgsPolygonV2 *> &polygons, const QgsPointXY &origin, float extrusionHeight );

  private:
    QList<QgsPolygonV2 *> mPolygons;

    Qt3DRender::QAttribute *m_positionAttribute;
    Qt3DRender::QAttribute *m_normalAttribute;
    Qt3DRender::QBuffer *m_vertexBuffer;

    bool m_withNormals;
};

#endif // QGSTESSELLATEDPOLYGONGEOMETRY_H
