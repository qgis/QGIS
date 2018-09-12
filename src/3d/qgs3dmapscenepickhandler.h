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
    virtual ~Qgs3DMapScenePickHandler() {}

    //! Called when user clicked a 3D entity belonging to a feature of a vector layer
    virtual void handlePickOnVectorLayer( QgsVectorLayer *vlayer, QgsFeatureId id, const QVector3D &worldIntersection ) = 0;
};

#endif // QGS3DMAPSCENEPICKHANDLER_H
