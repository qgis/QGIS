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

#include "ui_animation3dwidget.h"

namespace Qt3DRender
{
  class QCamera;
}
namespace Qt3DAnimation
{
  class QAnimationController;
}

class Qgs3DAnimationSettings;

class Qgs3DAnimationWidget : public QWidget, private Ui::Animation3DWidget
{
    Q_OBJECT
  public:
    explicit Qgs3DAnimationWidget( QWidget *parent = nullptr );

    void setCamera( Qt3DRender::QCamera *camera );

    void setAnimation( const Qgs3DAnimationSettings &animation );
    Qgs3DAnimationSettings animation() const;

    void setDefaultAnimation();

  signals:

  private slots:
    void onPlayPause();
    void onAnimationTimer();
    void onSliderValueChanged();
    void onCameraViewMatrixChanged();
    void onKeyframeChanged();

  private:
    void initializeController( const Qgs3DAnimationSettings &animSettings );

  private:
    QTimer *mAnimationTimer = nullptr;
    Qt3DAnimation::QAnimationController *mAnimationController = nullptr;
    Qt3DRender::QCamera *mCamera = nullptr;   //!< Camera (not owned)
};

#endif // QGS3DANIMATIONWIDGET_H
