/***************************************************************************
  qgsvectorlayer3dpropertieswidget.h
  --------------------------------------
  Date                 : January 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYER3DPROPERTIESWIDGET_H
#define QGSVECTORLAYER3DPROPERTIESWIDGET_H

#include <QWidget>

#include "ui_qgsvectorlayer3dpropertieswidget.h"

class QgsAbstractVectorLayer3DRenderer;


class QgsVectorLayer3DPropertiesWidget : public QWidget, private Ui::QgsVectorLayer3DPropertiesWidget
{
    Q_OBJECT
  public:
    QgsVectorLayer3DPropertiesWidget( QWidget *parent = nullptr );

    //! Initializes GUI from the given renderer
    void load( QgsAbstractVectorLayer3DRenderer *renderer );
    //! Applies configuration from GUI to the given renderer
    void apply( QgsAbstractVectorLayer3DRenderer *renderer );

  signals:
    //! Emitted whenever any parameter gets changed
    void changed();
};

#endif // QGSVECTORLAYER3DPROPERTIESWIDGET_H
