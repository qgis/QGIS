/***************************************************************************
  qgsphongmaterialwidget.h
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

#ifndef QGSPHONGMATERIALWIDGET_H
#define QGSPHONGMATERIALWIDGET_H

#include <QWidget>

#include <ui_phongmaterialwidget.h>

class QgsPhongMaterialSettings;


//! Widget for configuration of Phong material settings
class QgsPhongMaterialWidget : public QWidget, private Ui::PhongMaterialWidget
{
    Q_OBJECT
  public:
    explicit QgsPhongMaterialWidget( QWidget *parent = nullptr );

    void setDiffuseVisible( bool visible );
    bool isDiffuseVisible() const;

    void setMaterial( const QgsPhongMaterialSettings &material );
    QgsPhongMaterialSettings material() const;

  signals:
    void changed();

  public slots:
};

#endif // QGSPHONGMATERIALWIDGET_H
