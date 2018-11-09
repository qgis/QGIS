/***************************************************************************
  qgs3dmapscenepickhandler.h
  --------------------------------------
  Date                 : Sep 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DMAPSCENEPICKHANDLER_H
#define QGS3DMAPSCENEPICKHANDLER_H

#include "qgsfeatureid.h"

class QVector3D;
class QgsVectorLayer;

/**
 * \ingroup 3d
 * Abstract base class for handlers that process pick events from a 3D map scene.
 * 3D entities in map scene get QObjectPicker components assigned and mouse press events trigger virtual methods
 * or pick handlers.
 *
 * This is used for identify tool to be able to identify entities coming from 3D renderers assigned to map layers.
 *
 * \since QGIS 3.4
 */
class Qgs3DMapScenePickHandler
{
  public:
    virtual ~Qgs3DMapScenePickHandler() = default;

    //! Called when user clicked a 3D entity belonging to a feature of a vector layer
    virtual void handlePickOnVectorLayer( QgsVectorLayer *vlayer, QgsFeatureId id, const QVector3D &worldIntersection ) = 0;
};

#endif // QGS3DMAPSCENEPICKHANDLER_H
