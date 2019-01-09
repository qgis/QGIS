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

#include "ui_mesh3dsymbolwidget.h"

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

  public slots:
};

#endif // QGSMESH3DSYMBOLWIDGET_H
