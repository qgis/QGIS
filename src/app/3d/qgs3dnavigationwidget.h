/***************************************************************************
  qgs3dnavigationwidget.h
  --------------------------------------
  Date                 : June 2019
  Copyright            : (C) 2019 by Ismail Sunni
  Email                : imajimatika at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DNAVIGATIONWIDGET_H
#define QGS3DNAVIGATIONWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QToolButton>

#include "qwt_compass.h"

#include "qgs3dmapcanvas.h"
#include "qgscameracontroller.h"

class Qgs3DNavigationWidget : public QWidget
{
    Q_OBJECT
  public:
    Qgs3DNavigationWidget( Qgs3DMapCanvas *parent = nullptr );

    /**
     * Update the state of navigation widget from camera's state
     */
    void updateFromCamera();

  signals:

  public slots:

  private:
    Qgs3DMapCanvas *mParent3DMapCanvas = nullptr;
    QToolButton *mZoomInButton = nullptr;
    QToolButton *mZoomOutButton = nullptr;
    QToolButton *mTiltUpButton = nullptr;
    QToolButton *mTiltDownButton = nullptr;
    QToolButton *mMoveUpButton = nullptr;
    QToolButton *mMoveRightButton = nullptr;
    QToolButton *mMoveDownButton = nullptr;
    QToolButton *mMoveLeftButton = nullptr;
    QwtCompass *mCompas = nullptr;
};

#endif // QGS3DNAVIGATIONWIDGET_H
