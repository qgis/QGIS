#ifndef POLYGONGEOMETRY_H
#define POLYGONGEOMETRY_H

#include "qgspolygon.h"

#include <Qt3DRender/QGeometry>

namespace Qt3DRender
{
  class QBuffer;
}

class PolygonGeometry : public Qt3DRender::QGeometry
{
  public:
    PolygonGeometry( QNode *parent = nullptr );
    ~PolygonGeometry();

    // takes ownership of passed polygon geometries
    void setPolygons( const QList<QgsPolygonV2 *> &polygons, const QgsPointXY &origin, float extrusionHeight );

  private:
    QList<QgsPolygonV2 *> mPolygons;

    Qt3DRender::QAttribute *m_positionAttribute;
    Qt3DRender::QAttribute *m_normalAttribute;
    Qt3DRender::QBuffer *m_vertexBuffer;

    bool m_withNormals;
};

#endif // POLYGONGEOMETRY_H
