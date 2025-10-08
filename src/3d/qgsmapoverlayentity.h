/***************************************************************************
  qgsmapoverlayentity.h
  --------------------------------------
  Date                 : July 2025
  Copyright            : (C) 2025 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPOVERLAYENTITY_H
#define QGSMAPOVERLAYENTITY_H

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#define SIP_NO_FILE

#include "qgsrectangle.h"
#include "qgsdebugtextureentity.h"


class Qgs3DMapSettings;
class QgsDebugTextureRenderView;
class QgsMapLayer;
class QgsMapOverlayTextureGenerator;
class QgsWindow3DEngine;


/**
 * \ingroup qgis_3d
 * \brief An entity that displays a texture over the current 3D scene to show the current 3D
 * location on the 2D canvas.
 *
 * \since QGIS 4.0
 */
class QgsMapOverlayEntity : public QgsDebugTextureEntity
{
    Q_OBJECT

  public:
    explicit QgsMapOverlayEntity( QgsWindow3DEngine *engine, QgsDebugTextureRenderView *debugTextureRenderView, Qgs3DMapSettings *mapSettings, Qt3DCore::QNode *parent = nullptr );

    ~QgsMapOverlayEntity();

    void update( const QgsRectangle &extent, double rotationDegrees );

    static const int SIZE;

  private slots:

    void onLayersChanged();
    void onTextureReady( const QImage &image );
    void onSizeChanged();

  private:
    void invalidateMapImage();
    void connectToLayersRepaintRequest();

  private:
    QgsWindow3DEngine *mEngine = nullptr;
    Qgs3DMapSettings *mMapSettings = nullptr;
    QgsMapOverlayTextureGenerator *mTextureGenerator = nullptr;

    Qt3DRender::QTextureImageDataPtr mImageDataPtr;

    //! layers that are currently being used for map rendering (and thus being watched for renderer updates)
    QList<QgsMapLayer *> mLayers;

    QgsRectangle mExtent;
    double mRotation;
};

/// @endcond

#endif // QGSMAPOVERLAYENTITY_H
