/***************************************************************************
   qgspointcloudattributecombobox.h
    --------------------------------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSPOINTCLOUDATTRIBUTECOMBOBOX_H
#define QGSPOINTCLOUDATTRIBUTECOMBOBOX_H

#include <QComboBox>

#include "qgspointcloudattributemodel.h"
#include "qgis_gui.h"

#include "qgis_sip.h"

class QgsMapLayer;
class QgsPointCloudLayer;

/**
 * \ingroup gui
 * \brief The QgsPointCloudAttributeComboBox is a combo box which displays the list of attributes of a given point cloud layer.
 * It might be combined with a QgsMapLayerComboBox to automatically update attributes according to a chosen layer.
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsPointCloudAttributeComboBox : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY( QgsPointCloudAttributeProxyModel::Filters filters READ filters WRITE setFilters )
    Q_PROPERTY( bool allowEmptyAttributeName READ allowEmptyAttributeName WRITE setAllowEmptyAttributeName )

  public:
    /**
     * \brief QgsPointCloudAttributeComboBox creates a combo box to display the fields of a layer.
     * The layer can be either manually given or dynamically set by connecting the signal QgsMapLayerComboBox::layerChanged to the slot setLayer.
     */
    explicit QgsPointCloudAttributeComboBox( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets \a filters to allow filtering available attributes according to the attribute properties.
     * \see filters()
     */
    void setFilters( QgsPointCloudAttributeProxyModel::Filters filters );

    /**
     * Returns the current filters used for filtering available attributes.
     * \see setFilters()
     */
    QgsPointCloudAttributeProxyModel::Filters filters() const { return mProxyModel->filters(); }

    /**
     * Sets whether an optional empty attribute ("not set") option is shown in the combo box.
     * \see allowEmptyAttributeName()
     */
    void setAllowEmptyAttributeName( bool allowEmpty );

    /**
     * Returns TRUE if the combo box allows the empty field ("not set") choice.
     * \see setAllowEmptyAttributeName()
     */
    bool allowEmptyAttributeName() const;

    //! Returns the currently selected attribute
    QString currentAttribute() const;

    /**
     * Returns the layer currently associated with the combobox.
     * \see setLayer()
     */
    QgsPointCloudLayer *layer() const;

    /**
     * Manually sets the \a attributes to use for the combo box.
     *
     * This method should only be used when the combo box ISN'T associated with a layer()
     * and needs to show the fields from an arbitrary attribute collection instead. Calling
     * setAttributes() will automatically clear any existing layer().
     *
     * \see attributes()
     */
    void setAttributes( const QgsPointCloudAttributeCollection &attributes );

    /**
     * Returns the attributes currently shown in the combobox.
     *
     * This will either be attributes from the associated layer() or the attributes
     * manually set by a call to setAttributes().
     */
    QgsPointCloudAttributeCollection attributes() const;

  signals:
    //! Emitted when the currently selected attribute changes.
    void attributeChanged( const QString &name );

  public slots:

    /**
     * Sets the layer for which fields are listed in the combobox. If no layer is set
     * or a non-point cloud layer is set then the combobox will be empty.
     * \see layer()
     */
    void setLayer( QgsMapLayer *layer );

    //! Sets the currently selected attribute by \a name
    void setAttribute( const QString &name );

  private slots:
    void indexChanged( int i );

  private:
    QgsPointCloudAttributeModel *mAttributeModel = nullptr;
    QgsPointCloudAttributeProxyModel *mProxyModel = nullptr;
};

#endif // QGSPOINTCLOUDATTRIBUTECOMBOBOX_H
