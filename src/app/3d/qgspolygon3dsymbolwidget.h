/***************************************************************************
  qgspolygon3dsymbolwidget.h
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

#ifndef QGSPOLYGON3DSYMBOLWIDGET_H
#define QGSPOLYGON3DSYMBOLWIDGET_H

#include <QWidget>

#include "ui_polygon3dsymbolwidget.h"

class QgsPolygon3DSymbol;

//! A widget for configuration of 3D symbol for polygons
class QgsPolygon3DSymbolWidget : public QWidget, private Ui::Polygon3DSymbolWidget
{
    Q_OBJECT
  public:
    explicit QgsPolygon3DSymbolWidget( QWidget *parent = nullptr );

    void setSymbol( const QgsPolygon3DSymbol &symbol, QgsVectorLayer *layer );
    QgsPolygon3DSymbol symbol() const;

  signals:
    void changed();

  public slots:
};

#endif // QGSPOLYGON3DSYMBOLWIDGET_H
