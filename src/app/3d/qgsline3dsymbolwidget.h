/***************************************************************************
  qgsline3dsymbolwidget.h
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

#ifndef QGSLINE3DSYMBOLWIDGET_H
#define QGSLINE3DSYMBOLWIDGET_H

#include <QWidget>

#include "ui_line3dsymbolwidget.h"

class QgsLine3DSymbol;

//! A widget for configuration of 3D symbol for polygons
class QgsLine3DSymbolWidget : public QWidget, private Ui::Line3DSymbolWidget
{
    Q_OBJECT
  public:
    explicit QgsLine3DSymbolWidget( QWidget *parent = nullptr );

    void setSymbol( const QgsLine3DSymbol &symbol );
    QgsLine3DSymbol symbol() const;

  private slots:
    void updateGuiState();

  signals:
    void changed();

  public slots:
};

#endif // QGSLINE3DSYMBOLWIDGET_H
