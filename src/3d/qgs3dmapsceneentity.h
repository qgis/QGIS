/***************************************************************************
  qgs3dmapsceneentity.h
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

#ifndef QGS3DMAPSCENEENTITY_H
#define QGS3DMAPSCENEENTITY_H

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
#include "qgssettings.h"

class Qgs3DMapSettings;

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
    Qgs3DMapSceneEntity( Qgs3DMapSettings *mapSettings, Qt3DCore::QNode *parent = nullptr )
      : Qt3DCore::QEntity( parent )
      , mMapSettings( mapSettings )
    {
      const QgsSettings settings;
      mGpuMemoryLimit = settings.value( QStringLiteral( "map3d/gpuMemoryLimit" ), 500.0, QgsSettings::App ).toDouble();
    }

    //! Records some bits about the scene (context for handleSceneUpdate() method)
    struct SceneContext
    {
        QVector3D cameraPos;             //!< Camera position
        float cameraFov;                 //!< Field of view (in degrees)
        int screenSizePx;                //!< Size of the viewport in pixels
        QMatrix4x4 viewProjectionMatrix; //!< For frustum culling
    };

    //! Called when e.g. camera changes and entity may need updated
    virtual void handleSceneUpdate( const SceneContext &sceneContext ) { Q_UNUSED( sceneContext ) }

    //! Returns number of jobs pending for this entity until it is fully loaded/updated in the current view
    virtual int pendingJobsCount() const { return 0; }

    //! Returns whether the entity needs update of active nodes
    virtual bool needsUpdate() const { return false; }

    //! Returns the near to far plane range for the entity using the specified \a viewMatrix
    virtual QgsRange<float> getNearFarPlaneRange( const QMatrix4x4 &viewMatrix ) const
    {
      Q_UNUSED( viewMatrix )
      return QgsRange<float>( 1e9, 0 );
    }

    /**
     * Returns the associated 3D mapSettings settings.
     *
     * \since QGIS 3.40
     */
    Qgs3DMapSettings *mapSettings() { return mMapSettings; }

    //! Sets the limit of the GPU memory used to render the entity
    void setGpuMemoryLimit( double gpuMemoryLimit ) { mGpuMemoryLimit = gpuMemoryLimit; }

    //! Returns the limit of the GPU memory used to render the entity in megabytes
    double gpuMemoryLimit() const { return mGpuMemoryLimit; }

    //! Returns whether the entity has reached GPU memory limit
    bool hasReachedGpuMemoryLimit() const { return mHasReachedGpuMemoryLimit; }

  protected:
    //! Sets whether the GPU memory limit has been reached
    void setHasReachedGpuMemoryLimit( bool reached ) { mHasReachedGpuMemoryLimit = reached; }

  signals:
    //! Emitted when the number of pending jobs changes (some jobs have finished or some jobs have been just created)
    void pendingJobsCountChanged();

    //! Emitted when a new 3D entity has been created. Other components can use that to do extra work
    void newEntityCreated( Qt3DCore::QEntity *entity );

  protected:
    Qgs3DMapSettings *mMapSettings = nullptr;

    //! Limit how much GPU memory this entity can use
    double mGpuMemoryLimit = 500.0; // in megabytes
    //! Whether the entity is currently over the GPU memory limit (used to report a warning to the user)
    bool mHasReachedGpuMemoryLimit = false;
};

/// @endcond

#endif // QGS3DMAPSCENEENTITY_H
