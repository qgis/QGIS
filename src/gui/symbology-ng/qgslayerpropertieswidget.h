/***************************************************************************
    qgslayerpropertieswidget.h
    ---------------------
    begin                : June 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : aruntheguy at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERPROPERTIESWIDGET_H
#define QGSLAYERPROPERTIESWIDGET_H

#include "ui_widget_layerproperties.h"

class QgsSymbolV2;
class QgsSymbolLayerV2;
class QgsSymbolLayerV2Widget;
class QgsVectorLayer;

class SymbolLayerItem;

#include <QMap>
#include <QStandardItemModel>


class GUI_EXPORT QgsLayerPropertiesWidget : public QWidget, private Ui::LayerPropertiesWidget
{
    Q_OBJECT

  public:
    QgsLayerPropertiesWidget( QgsSymbolLayerV2* layer, const QgsSymbolV2* symbol, const QgsVectorLayer* vl, QWidget* parent = NULL );


  public slots:
    void layerTypeChanged();
    void emitSignalChanged();

  signals:
    void changed();
    void changeLayer( QgsSymbolLayerV2* );

  protected:
    void populateLayerTypes();
    void updateSymbolLayerWidget( QgsSymbolLayerV2* layer );

  protected: // data
    QgsSymbolLayerV2* mLayer;

    const QgsSymbolV2* mSymbol;
    const QgsVectorLayer* mVectorLayer;
};

#endif //QGSLAYERPROPERTIESWIDGET_H
