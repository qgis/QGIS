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

#include "qgs3dsymbolwidget.h"
#include "ui_polygon3dsymbolwidget.h"

class QgsPolygon3DSymbol;

//! A widget for configuration of 3D symbol for polygons
class QgsPolygon3DSymbolWidget : public Qgs3DSymbolWidget, private Ui::Polygon3DSymbolWidget
{
    Q_OBJECT
  public:
    explicit QgsPolygon3DSymbolWidget( QWidget *parent = nullptr );

    static Qgs3DSymbolWidget *create( QgsVectorLayer *layer );

    void setSymbol( const QgsAbstract3DSymbol *symbol, QgsVectorLayer *layer ) override;
    QgsAbstract3DSymbol *symbol() override;
    QString symbolType() const override;

  private slots:
    void updateGuiState();
};

#endif // QGSPOLYGON3DSYMBOLWIDGET_H
