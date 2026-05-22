/***************************************************************************
  qgslayerstylewatcher.h
  --------------------------------------
  Date                 : May 2026
  Copyright            : (C) 2026 by Oslandia
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERSTYLEWATCHER_H
#define QGSLAYERSTYLEWATCHER_H

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include "qgs3dmapsceneentity.h"
#include "qgsraycasthit.h"

#include <QMatrix4x4>
#include <QTime>

#define SIP_NO_FILE

/**
 * \ingroup qgis_3d
 *
 * The main goal of this class is to update the terrain map tiles only when necessary.
 *
 * This class watches for layer renderer changes. When specific cases occur it calls the mCallBackToInvalidateMapImages callback.
 *
 * \see styleChanged() for more details.
 * \since QGIS 4.2
 */
class QgsLayerStyleWatcher : public QObject
{
    Q_OBJECT

  public:
    /**
     * Default constructor
     * \param mapSettings 3D settings to retrieve layers
     */
    QgsLayerStyleWatcher( Qgs3DMapSettings *mapSettings );

  signals:
    /**
     * Emitted when specific style change has been detected:
     *
     * - layer without a 3D renderer, when the 2D style changes
     * - layer without a 3D renderer, when a new 3D renderer is set
     * - layer with a 3D renderer, when the 3D renderer is removed
     */
    void styleChanged();

  private slots:
    void onLayersChanged();
    void onLayer3DRendererChanged();
    void onLayerRepaintRequested();
    void onLayerDestroyed();

  protected:
    Qgs3DMapSettings *mMapSettings = nullptr;
    //! Layers that are currently being used for map rendering (and thus being watched for renderer updates). Bool value tells if the layer has a 3D renderer or not.
    QHash<QgsMapLayer *, bool> mLayers;
};

/// @endcond

#endif // QGSLAYERSTYLEWATCHER_H
