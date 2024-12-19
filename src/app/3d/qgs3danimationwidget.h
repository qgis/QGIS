/***************************************************************************
  qgs3danimationwidget.h
  --------------------------------------
  Date                 : July 2018
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

#ifndef QGS3DANIMATIONWIDGET_H
#define QGS3DANIMATIONWIDGET_H

#include <QWidget>
#include <memory>

#include "ui_animation3dwidget.h"


class Qgs3DAnimationSettings;
class QgsCameraController;
class Qgs3DMapSettings;

class Qgs3DAnimationWidget : public QWidget, private Ui::Animation3DWidget
{
    Q_OBJECT
  public:
    explicit Qgs3DAnimationWidget( QWidget *parent = nullptr );

    void setCameraController( QgsCameraController *cameraController );

    void setMap( Qgs3DMapSettings *map );

    void setAnimation( const Qgs3DAnimationSettings &animation );
    Qgs3DAnimationSettings animation() const;

    void setDefaultAnimation();

  signals:

  private slots:
    void onPlayPause();
    void onAnimationTimer();
    void onSliderValueChanged();
    void onCameraChanged();
    void onKeyframeChanged();
    void onAddKeyframe();
    void onRemoveKeyframe();
    void onEditKeyframe();
    void onDuplicateKeyframe();
    void onInterpolationChanged();
    void onExportAnimation();

  private:
    void initializeController( const Qgs3DAnimationSettings &animSettings );
    void setEditControlsEnabled( bool enabled );
    float askForKeyframeTime( float defaultTime, bool *ok );
    int findIndexForKeyframe( float time );

  private:
    std::unique_ptr<Qgs3DAnimationSettings> mAnimationSettings;
    QgsCameraController *mCameraController;
    Qgs3DMapSettings *mMap;
    QTimer *mAnimationTimer = nullptr;
};

#endif // QGS3DANIMATIONWIDGET_H
