/***************************************************************************
                             qgsquantizedmeshtiles.h
                             ----------------------------
    begin                : May 2024
    copyright            : (C) 2024 by David Koňařík
    email                : dvdkon at konarici dot cz

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQUANTIZEDMESHTILED_H
#define QGSQUANTIZEDMESHTILED_H

#include "qgis_core.h"
#include "qgsexception.h"
#include "qgsmeshdataprovider.h"
#include "qgsrectangle.h"
#include <qbytearray.h>

#define TINYGLTF_NO_STB_IMAGE         // we use QImage-based reading of images
#define TINYGLTF_NO_STB_IMAGE_WRITE   // we don't need writing of images
#include "tiny_gltf.h"

#define SIP_NO_FILE

// Definition copied from format spec: https://github.com/CesiumGS/quantized-mesh
#pragma pack (push, 1)
struct QgsQuantizedMeshHeader
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

  uint32_t vertexCount;
};
#pragma pack (pop)

/**
 * \ingroup core
 * \brief Exception thrown on failure to parse Quantized Mesh tile (malformed data)
 * \since QGIS 3.35
 */
class CORE_EXPORT QgsQuantizedMeshParsingException : public QgsException
{
  public:
    using QgsException::QgsException;
};

struct CORE_EXPORT QgsQuantizedMeshTile
{
  QgsQuantizedMeshHeader mHeader;
  std::vector<uint16_t> mVertexCoords;
  std::vector<float> mNormalCoords;
  std::vector<uint32_t> mTriangleIndices;
  std::vector<uint32_t> mWestVertices;
  std::vector<uint32_t> mSouthVertices;
  std::vector<uint32_t> mEastVertices;
  std::vector<uint32_t> mNorthVertices;
  std::map<uint8_t, std::vector<char>> mExtensions;

  QgsQuantizedMeshTile( const QByteArray &data );
  // For some reason, commonly available QM tiles often have a very high (as
  // much as 50%) percentage of degenerate triangles. They don't harm our
  // rendering, but removing them could improve performance and makes working
  // with the data easier.
  void removeDegenerateTriangles();
  void generateNormals();
  tinygltf::Model toGltf( bool addSkirt = false, double skirtDepth = 0, bool withTextureCoords = false );
  // Make sure to call removeDegenerateTriangles() beforehand!
  QgsMesh toMesh( QgsRectangle tileBounds );
};

#endif // QGSQUANTIZEDMESHTILED_H
