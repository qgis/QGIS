/***************************************************************************
   qgsmaplayercombobox.h
    --------------------------------------
   Date                 : 01.04.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSMAPLAYERCOMBOBOX_H
#define QGSMAPLAYERCOMBOBOX_H

#include <QComboBox>

#include "qgsmaplayerproxymodel.h"

class QgsMapLayer;
class QgsVectorLayer;

/**
 * @brief The QgsMapLayerComboBox class is a combo box which displays the list of layers
 * @note added in 2.3
 */
class GUI_EXPORT QgsMapLayerComboBox : public QComboBox
{
    Q_OBJECT
  public:
    /**
     * @brief QgsMapLayerComboBox creates a combo box to dislpay the list of layers (currently in the registry).
     * The layers can be filtered and/or ordered.
     */
    explicit QgsMapLayerComboBox( QWidget *parent = 0 );

    /**
     * @brief setFilters allows fitering according to layer type and/or geometry type.
     */
    void setFilters( QgsMapLayerProxyModel::Filters filters );

    /**
     * @brief currentLayer returns the current layer selected in the combo box
     */
    QgsMapLayer* currentLayer();

  public slots:
    /**
     * @brief setLayer set the current layer selected in the combo
     */
    void setLayer( QgsMapLayer* layer );

  signals:
    /**
     * @brief layerChanged this signal is emitted whenever the currently selected layer changes
     */
    void layerChanged( QgsMapLayer* layer );

  protected slots:
    void indexChanged( int i );

  private:
    QgsMapLayerProxyModel* mProxyModel;

};

#endif // QGSMAPLAYERCOMBOBOX_H
