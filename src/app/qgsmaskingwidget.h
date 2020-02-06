/***************************************************************************
    qgsmaskingwidget.h
    ---------------------
    begin                : September 2019
    copyright            : (C) 2019 by Hugo Mercier
    email                : hugo dot mercier at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMASKINGWIDGET_H
#define QGSMASKINGWIDGET_H

#include "qgspanelwidget.h"
#include "ui_qgsmaskingwidgetbase.h"
#include "qgis_sip.h"
#include "qgis_gui.h"

/**
 * Main widget for the configuration of mask sources and targets.
 */
class QgsMaskingWidget: public QgsPanelWidget, private Ui::QgsMaskingWidgetBase
{
    Q_OBJECT
  public:
    QgsMaskingWidget( QWidget *parent = nullptr );

    void setLayer( QgsVectorLayer *layer );

    void apply();
  signals:
    void widgetChanged();
  private:
    QgsVectorLayer *mLayer;
};

#endif
