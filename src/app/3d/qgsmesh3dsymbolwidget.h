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

//! A widget for configuration of 3D symbol for meshes
class QgsMesh3DSymbolWidget : public QWidget, private Ui::Mesh3DSymbolWidget
{
    Q_OBJECT
  public:
    explicit QgsMesh3DSymbolWidget( QgsMeshLayer *meshLayer, QWidget *parent = nullptr );

    QgsMesh3DSymbol symbol() const;

    void setLayer( QgsMeshLayer *meshLayer );
    int rendererTypeComboBoxIndex() const;
    void setRendererTypeComboBoxIndex( int index );

  signals:
    void changed();

  private slots:
    void reloadColorRampShaderMinMax();
    void onSymbologyTypeChanged();
    void onColorRampShaderMinMaxChanged();
    void onColoringTypeChanged();

  private:
    void setSymbol( const QgsMesh3DSymbol &symbol );
    double lineEditValue( const QLineEdit *lineEdit ) const;

    QgsMeshLayer *mLayer = nullptr;
};

#endif // QGSMESH3DSYMBOLWIDGET_H
