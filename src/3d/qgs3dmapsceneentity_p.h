/***************************************************************************
  qgs3dmapsceneentity_p.h
  --------------------------------------
  Date                 : May 2023
  Copyright            : (C) 2023 by Stefanos Natsis
  Email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DMAPSCENEENTITY_P_H
#define QGS3DMAPSCENEENTITY_P_H

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include <Qt3DCore/QEntity>
#include <QVector3D>
#include <QMatrix4x4>

#include "qgsrange.h"

#define SIP_NO_FILE


/**
 * \ingroup 3d
 * \brief Abstract entity that all entities rendered in a Qgs3DMapScene inherit
 * \since QGIS 3.32
 */
class Qgs3DMapSceneEntity : public Qt3DCore::QEntity
{
    Q_OBJECT
  public:
    //! Constructs a chunked entity
    Qgs3DMapSceneEntity( Qt3DCore::QNode *parent = nullptr )
      : Qt3DCore::QEntity( parent )
    {}

    //! Records some bits about the scene (context for handleSceneUpdate() method)
    struct SceneState
    {
      QVector3D cameraPos;   //!< Camera position
      float cameraFov;       //!< Field of view (in degrees)
      int screenSizePx;      //!< Size of the viewport in pixels
      QMatrix4x4 viewProjectionMatrix; //!< For frustum culling
    };

    //! Called when e.g. camera changes and entity may need updated
    virtual void handleSceneUpdate( const SceneState &state ) { Q_UNUSED( state ) }

    //! Returns number of jobs pending for this entity until it is fully loaded/updated in the current view
    virtual int pendingJobsCount() const { return 0; }

    //! Returns whether the entity needs update of active nodes
    virtual bool needsUpdate() const { return false; }

    //! Returns the near to far plane range for the entity using the specified \a viewMatrix
    virtual QgsRange<float> getNearFarPlaneRange( const QMatrix4x4 &viewMatrix ) const { Q_UNUSED( viewMatrix ) return QgsRange<float>( 1e9, 0 ); }

  signals:
    //! Emitted when the number of pending jobs changes (some jobs have finished or some jobs have been just created)
    void pendingJobsCountChanged();

    //! Emitted when a new 3D entity has been created. Other components can use that to do extra work
    void newEntityCreated( Qt3DCore::QEntity *entity );
};

/// @endcond

#endif // QGS3DMAPSCENEENTITY_P_H
