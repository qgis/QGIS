/***************************************************************************
   qgsfieldcombobox.h
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

#ifndef QGSFIELDCOMBOBOX_H
#define QGSFIELDCOMBOBOX_H

#include <QComboBox>

#include "qgsfieldproxymodel.h"

class QgsMapLayer;
class QgsVectorLayer;

/**
 * @brief The QgsFieldComboBox is a combo box which displays the list of fields of a given layer.
 * It might be combined with a QgsMapLayerComboBox to automatically update fields according to a chosen layer.
 * If expression must be used, QgsFieldExpressionWidget shall be used instead.
 * @see QgsMapLayerComboBox
 * @note added in 2.3
 */
class GUI_EXPORT QgsFieldComboBox : public QComboBox
{
    Q_OBJECT
    Q_FLAGS( QgsFieldProxyModel::Filters )
    Q_PROPERTY( QgsFieldProxyModel::Filters filters READ filters WRITE setFilters )

  public:
    /**
     * @brief QgsFieldComboBox creates a combo box to display the fields of a layer.
     * The layer can be either manually given or dynamically set by connecting the signal QgsMapLayerComboBox::layerChanged to the slot setLayer.
     */
    explicit QgsFieldComboBox( QWidget *parent = 0 );

    //! setFilters allows fitering according to the type of field
    void setFilters( QgsFieldProxyModel::Filters filters );

    //! currently used filter on list of fields
    QgsFieldProxyModel::Filters filters() { return mFieldProxyModel->filters(); }

    //! return the currently selected field
    QString currentField();

    //! Returns the currently used layer
    QgsVectorLayer* layer();

  signals:
    //! the signal is emitted when the currently selected field changes
    void fieldChanged( QString fieldName );

  public slots:
    //! set the layer of which the fields are listed
    void setLayer( QgsVectorLayer* layer );

    //! convenience slot to connect QgsMapLayerComboBox layer signal
    void setLayer( QgsMapLayer* layer );

    //! setField sets the currently selected field
    void setField( QString fieldName );

  protected slots:
    void indexChanged( int i );

  private:
    QgsFieldProxyModel* mFieldProxyModel;
};

#endif // QGSFIELDCOMBOBOX_H
