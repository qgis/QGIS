/***************************************************************************
  qgspointcloud3dsymbolwidget.h
  ------------------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Nedjima Belgacem
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUD3DSYMBOLWIDGET_H
#define QGSPOINTCLOUD3DSYMBOLWIDGET_H

#include "ui_qgspointcloud3dsymbolwidget.h"

class QgsPointCloudLayer;
class QgsPointCloud3DSymbol;

class QgsPointCloud3DSymbolWidget : public QWidget, private Ui::QgsPointCloud3DSymbolWidget
{
    Q_OBJECT

  public:
    explicit QgsPointCloud3DSymbolWidget( QgsPointCloud3DSymbol *symbol, QWidget *parent = nullptr );

    void setSymbol( QgsPointCloud3DSymbol *symbol );

    QgsPointCloud3DSymbol *symbol() const;

  signals:
    void changed();
};

#endif // QGSPOINTCLOUD3DSYMBOLWIDGET_H
