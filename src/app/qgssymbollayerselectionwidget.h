/***************************************************************************
    qgssymbollayerselectionwidget.h
    ---------------------
    begin                : July 2019
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
#ifndef QGSSYMBOLLAYERSELECTIONWIDGET_H
#define QGSSYMBOLLAYERSELECTIONWIDGET_H

#include <QWidget>
#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgssymbollayerreference.h"

class QTreeWidget;
class QTreeWidgetItem;
class QgsVectorLayer;

/**
 * A widget that allows the selection of a list of symbol layers from a layer.
 * A tree shows a list of selectable symbol layers.
 *
 * \since QGIS 3.12
 */
class QgsSymbolLayerSelectionWidget : public QWidget
{
    Q_OBJECT
  public:
    //! Default constructor
    explicit QgsSymbolLayerSelectionWidget( QWidget *parent = nullptr );

    //! Populate the tree with selectable symbol layers from a given layer
    void setLayer( const QgsVectorLayer *layer );

    //! Returns current symbol layer selection
    QSet<QgsSymbolLayerId> selection() const;

    //! Sets the symbol layer selection
    void setSelection( const QSet<QgsSymbolLayerId> &sel );

  signals:
    //! Signal emitted when something the configuration is changed
    void changed();

  private:
    //! The tree object
    QTreeWidget *mTree;
    //! The current vector layer
    const QgsVectorLayer *mLayer = nullptr;

    // Mapping between symbol layer id and tree elements
    QHash<QgsSymbolLayerId, QTreeWidgetItem *> mItems;
};

#endif
