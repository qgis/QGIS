/***************************************************************************
  qgs3dnavigationwidget.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
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
#include <QPushButton>
#include <QDial>
#include <QGridLayout>

#include "qgs3dmapcanvas.h"
#include "qgscameracontroller.h"

class Qgs3DNavigationWidget : public QWidget
{
    Q_OBJECT
public:
    Qgs3DNavigationWidget(Qgs3DMapCanvas *parent = nullptr);
    ~Qgs3DNavigationWidget();

signals:

public slots:

private:
    QPushButton *mZoomInButton = nullptr;
    QPushButton *mZoomOutButton = nullptr;
    QPushButton *mTiltUpButton = nullptr;
    QPushButton *mTiltDownButton = nullptr;
    QDial *mRotateSceneDial = nullptr;
};

#endif // QGS3DNAVIGATIONWIDGET_H
