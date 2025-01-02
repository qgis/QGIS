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
#include "qgis_gui.h"

#include "qgis_sip.h"

class QgsMapLayer;
class QgsVectorLayer;
class QgsFields;

/**
 * \ingroup gui
 * \brief The QgsFieldComboBox is a combo box which displays the list of fields of a given layer.
 * It might be combined with a QgsMapLayerComboBox to automatically update fields according to a chosen layer.
 * If expression must be used, QgsFieldExpressionWidget shall be used instead.
 * \see QgsMapLayerComboBox
 */
class GUI_EXPORT QgsFieldComboBox : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY( QgsFieldProxyModel::Filters filters READ filters WRITE setFilters )
    Q_PROPERTY( bool allowEmptyFieldName READ allowEmptyFieldName WRITE setAllowEmptyFieldName )

  public:
    /**
     * \brief QgsFieldComboBox creates a combo box to display the fields of a layer.
     * The layer can be either manually given or dynamically set by connecting the signal QgsMapLayerComboBox::layerChanged to the slot setLayer.
     */
    explicit QgsFieldComboBox( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! setFilters allows filtering according to the type of field
    void setFilters( QgsFieldProxyModel::Filters filters );

    //! currently used filter on list of fields
    QgsFieldProxyModel::Filters filters() const { return mFieldProxyModel->filters(); }

    /**
     * Sets whether an optional empty field ("not set") option is shown in the combo box.
     * \see allowEmptyFieldName()
     */
    void setAllowEmptyFieldName( bool allowEmpty );

    /**
     * Returns TRUE if the combo box allows the empty field ("not set") choice.
     * \see setAllowEmptyFieldName()
     */
    bool allowEmptyFieldName() const;

    //! Returns the currently selected field
    QString currentField() const;

    /**
     * Returns the layer currently associated with the combobox.
     * \see setLayer()
     */
    QgsVectorLayer *layer() const;

    /**
     * Manually sets the \a fields to use for the combo box.
     *
     * This method should only be used when the combo box ISN'T associated with a layer()
     * and needs to show the fields from an arbitrary field collection instead. Calling
     * setFields() will automatically clear any existing layer().
     *
     * \see fields()
     * \since QGIS 3.14
     */
    void setFields( const QgsFields &fields );

    /**
     * Returns the fields currently shown in the combobox.
     *
     * This will either be fields from the associated layer() or the fields
     * manually set by a call to setFields().
     *
     * \since QGIS 3.14
     */
    QgsFields fields() const;

  signals:
    //! Emitted when the currently selected field changes.
    void fieldChanged( const QString &fieldName );

  public slots:

    /**
     * Sets the layer for which fields are listed in the combobox. If no layer is set
     * or a non-vector layer is set then the combobox will be empty.
     * \see layer()
     */
    void setLayer( QgsMapLayer *layer );

    //! setField sets the currently selected field
    void setField( const QString &fieldName );

  protected slots:
    void indexChanged( int i );

  private:
    QgsFieldProxyModel *mFieldProxyModel = nullptr;
};

#endif // QGSFIELDCOMBOBOX_H
