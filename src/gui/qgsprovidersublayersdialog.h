/***************************************************************************
    qgsprovidersublayersdialog.h
    ---------------------
    begin                : July 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROVIDERSUBLAYERSDIALOG_H
#define QGSPROVIDERSUBLAYERSDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QPointer>

#include "qgsgui.h"
#include "ui_qgsprovidersublayersdialogbase.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsprovidersublayermodel.h"

class QgsProviderSublayerModel;
class QgsProviderSublayerProxyModel;
class QgsProviderSublayerTask;

/**
 * \ingroup gui
 *
 * \brief A model for representing the sublayers present in a URI for the QgsProviderSublayersDialog.
 *
 * \since QGIS 3.30
 */
class GUI_EXPORT QgsProviderSublayerDialogModel : public QgsProviderSublayerModel
{
    Q_OBJECT

  public:

    /**
     * Constructor.
     */
    QgsProviderSublayerDialogModel( QObject *parent = nullptr );

    QVariant data( const QModelIndex &index, int role ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;

    /**
     * Sets whether geometry types are resolved.
     */
    void setGeometryTypesResolved( bool resolved );

  private:

    bool mGeometryTypesResolved = false;


};

/**
 * \ingroup gui
 *
 * \brief Dialog for selecting provider sublayers.
 *
 * \since QGIS 3.30
 */
class GUI_EXPORT QgsProviderSublayersDialog : public QDialog, private Ui::QgsProviderSublayersDialogBase
{
    Q_OBJECT
  public:

    /**
     * Constructor.
     */
    QgsProviderSublayersDialog( const QString &uri,
                                const QString &providerKey,
                                const QString &filePath,
                                const QList< QgsProviderSublayerDetails> initialDetails = QList< QgsProviderSublayerDetails>(),
                                const QList< QgsMapLayerType > &acceptableTypes = QList< QgsMapLayerType >(),
                                QWidget *parent SIP_TRANSFERTHIS = nullptr,
                                Qt::WindowFlags fl = Qt::WindowFlags() );

    /**
     * Set list of non-layer items (e.g. embedded QGIS project items).
     */
    void setNonLayerItems( const QList< QgsProviderSublayerModel::NonLayerItem > &items );

    ~QgsProviderSublayersDialog() override;

    /**
     * Returns the list of selected layers.
     */
    QList< QgsProviderSublayerDetails > selectedLayers() const;

    /**
     * Returns the list of selected non-layer items (e.g. embedded QGIS project items).
     */
    QList< QgsProviderSublayerModel::NonLayerItem > selectedNonLayerItems() const;

    /**
     * Returns an appropriate name for the layer group.
     */
    QString groupName() const;

    /**
     * Sets an appropriate name for the layer group.
     */
    void setGroupName( const QString &groupNameIn );

  signals:

    /**
     * Emitted when sublayers selected from the dialog should be added to the project.
     */
    void layersAdded( const QList< QgsProviderSublayerDetails > &layers );

  private slots:
    void treeSelectionChanged( const QItemSelection &, const QItemSelection & );
    void selectAll();

  private:

    QgsProviderSublayerDialogModel *mModel = nullptr;
    QgsProviderSublayerProxyModel *mProxyModel = nullptr;
    QPointer< QgsProviderSublayerTask > mTask;
    QString mGroupName;
    bool mBlockSelectionChanges = false;

};

#endif // QGSPROVIDERSUBLAYERSDIALOG_H
