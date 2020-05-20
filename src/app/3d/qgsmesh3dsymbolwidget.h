/***************************************************************************
  qgsmesh3dsymbolwidget.h
  -----------------------
  Date                 : January 2019
  Copyright            : (C) 2019 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESH3DSYMBOLWIDGET_H
#define QGSMESH3DSYMBOLWIDGET_H

#include <QWidget>

<<<<<<< HEAD
#include "ui_mesh3dsymbolwidget.h"
=======
#include "qgsmesh3dsymbol.h"
#include "ui_qgsmesh3dpropswidget.h"
>>>>>>> e6fa6c8cb5... [BUG][MESH][3D] fix enable/disable mesh 3D rendering (#34999)

class QgsMesh3DSymbol;
class QgsMeshLayer;

//! A widget for configuration of 3D symbol for polygons
class QgsMesh3DSymbolWidget : public QWidget, private Ui::Mesh3DSymbolWidget
{
    Q_OBJECT
  public:
    explicit QgsMesh3DSymbolWidget( QWidget *parent = nullptr );

    void setSymbol( const QgsMesh3DSymbol &symbol, QgsMeshLayer *layer );
    QgsMesh3DSymbol symbol() const;

  signals:
    void changed();

<<<<<<< HEAD
  public slots:
=======
  private slots:

    void onColorRampShaderMinMaxChanged();
    void onColoringTypeChanged();

  private:
    double lineEditValue( const QLineEdit *lineEdit ) const;
    void setColorRampMinMax( double min, double max );
    QgsMeshLayer *mLayer = nullptr;
    QgsMeshDatasetGroupListModel *mDatasetGroupListModel = nullptr;
    QgsMesh3DSymbol mSymbol;

>>>>>>> e6fa6c8cb5... [BUG][MESH][3D] fix enable/disable mesh 3D rendering (#34999)
};

#endif // QGSMESH3DSYMBOLWIDGET_H
