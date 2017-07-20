#ifndef QUANTIZEDMESHGEOMETRY_H
#define QUANTIZEDMESHGEOMETRY_H

#include <QtGui/QTransform>

#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QAttribute>



struct QuantizedMeshHeader
{
  // The center of the tile in Earth-centered Fixed coordinates.
  double CenterX;
  double CenterY;
  double CenterZ;

  // The minimum and maximum heights in the area covered by this tile.
  // The minimum may be lower and the maximum may be higher than
  // the height of any vertex in this tile in the case that the min/max vertex
  // was removed during mesh simplification, but these are the appropriate
  // values to use for analysis or visualization.
  float MinimumHeight;
  float MaximumHeight;

  // The tile’s bounding sphere.  The X,Y,Z coordinates are again expressed
  // in Earth-centered Fixed coordinates, and the radius is in meters.
  double BoundingSphereCenterX;
  double BoundingSphereCenterY;
  double BoundingSphereCenterZ;
  double BoundingSphereRadius;

  // The horizon occlusion point, expressed in the ellipsoid-scaled Earth-centered Fixed frame.
  // If this point is below the horizon, the entire tile is below the horizon.
  // See http://cesiumjs.org/2013/04/25/Horizon-culling/ for more information.
  double HorizonOcclusionPointX;
  double HorizonOcclusionPointY;
  double HorizonOcclusionPointZ;
};

#include "qgsrectangle.h"

struct QuantizedMeshTile
{
  QgsRectangle extent;  // extent in WGS coordinates
  QuantizedMeshHeader header;
  QVector<qint16> uvh; // each coordinate 0-32767. u=lon, v=lat (within tile). not interleaved: first u, then v, then h
  QVector<quint16> indices;  // indices of triangles (length = 3*triangle count)
};

class QgsCoordinateTransform;
class QgsMapToPixel;

class Map3D;

class QuantizedMeshGeometry : public Qt3DRender::QGeometry
{
  public:
    QuantizedMeshGeometry( QuantizedMeshTile *t, const Map3D &map, const QgsMapToPixel &mapToPixel, const QgsCoordinateTransform &terrainToMap, QNode *parent = nullptr );

    static QuantizedMeshTile *readTile( int tx, int ty, int tz, const QgsRectangle &extent );
    static void downloadTileIfMissing( int tx, int ty, int tz );

  private:
    Qt3DRender::QBuffer *m_vertexBuffer;
    Qt3DRender::QBuffer *m_indexBuffer;
    Qt3DRender::QAttribute *m_positionAttribute;
    Qt3DRender::QAttribute *m_texCoordAttribute;
    Qt3DRender::QAttribute *m_indexAttribute;
};

#endif // QUANTIZEDMESHGEOMETRY_H
