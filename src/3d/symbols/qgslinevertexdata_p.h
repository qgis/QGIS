/***************************************************************************
  qgslinevertexdata_p.h
  --------------------------------------
  Date                 : Apr 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLINEVERTEXDATA_P_H
#define QGSLINEVERTEXDATA_P_H

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include <QVector>
#include <QVector3D>

#define SIP_NO_FILE

#include "qgs3dtypes.h"
#include "qgis.h"

namespace Qt3DCore
{
  class QNode;
}
namespace Qt3DRender
{
  class QGeometry;
}

class QgsLineString;
class Qgs3DMapSettings;


/**
 * \ingroup 3d
 * \brief Helper class to store vertex buffer and index buffer data that will be used to render
 * lines (either using "line strip" or "line strip with adjacency" primitive.
 *
 * Index zero is used for primitive restart (to separate two linestrings).
 *
 * It is expected that client code:
 *
 * # calls init()
 * # calls addLineString() many times
 * # calls createGeometry()
 */
struct QgsLineVertexData
{
  QVector<QVector3D> vertices;
  QVector<unsigned int> indexes;

  bool withAdjacency = false;  //!< Whether line strip with adjacency primitive will be used

  // extra info to calculate elevation
  Qgis::AltitudeClamping altClamping = Qgis::AltitudeClamping::Relative;
  Qgis::AltitudeBinding altBinding = Qgis::AltitudeBinding::Vertex;
  float baseHeight = 0;
  const Qgs3DMapSettings *mapSettings = nullptr;

  QgsLineVertexData();

  void init( Qgis::AltitudeClamping clamping, Qgis::AltitudeBinding binding, float height, const Qgs3DMapSettings *map );

  QByteArray createVertexBuffer();
  QByteArray createIndexBuffer();
  Qt3DRender::QGeometry *createGeometry( Qt3DCore::QNode *parent );

  void addLineString( const QgsLineString &lineString, float extraHeightOffset = 0 );
  void addVerticalLines( const QgsLineString &lineString, float verticalLength, float extraHeightOffset = 0 );
};

/// @endcond

#endif // QGSLINEVERTEXDATA_P_H
