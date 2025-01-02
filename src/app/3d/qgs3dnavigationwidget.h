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

class QStandardItemModel;

class Qgs3DMapCanvas;
class QwtCompass;

#include <ui_3dnavigationwidget.h>

class Qgs3DNavigationWidget : public QWidget, private Ui::Q3DNavigationWidget
{
    Q_OBJECT
  public:
    Qgs3DNavigationWidget( Qgs3DMapCanvas *canvas, QWidget *parent = nullptr );

  private:
    Qgs3DMapCanvas *m3DMapCanvas = nullptr;
};

#endif // QGS3DNAVIGATIONWIDGET_H
