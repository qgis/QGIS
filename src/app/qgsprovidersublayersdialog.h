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

#include "ui_qgsprovidersublayersdialogbase.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsprovidersublayermodel.h"

class QgsProviderSublayerModel;
class QgsProviderSublayerProxyModel;
class QgsProviderSublayerTask;

class QgsProviderSublayerDialogModel : public QgsProviderSublayerModel
{
    Q_OBJECT

  public:

    QgsProviderSublayerDialogModel( QObject *parent = nullptr );

    QVariant data( const QModelIndex &index, int role ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;

    void setGeometryTypesResolved( bool resolved );

  private:

    bool mGeometryTypesResolved = false;


};

class QgsProviderSublayersDialog : public QDialog, private Ui::QgsProviderSublayersDialogBase
{
    Q_OBJECT
  public:

    QgsProviderSublayersDialog( const QString &uri,
                                const QString &filePath,
                                const QList< QgsProviderSublayerDetails> initialDetails = QList< QgsProviderSublayerDetails>(),
                                const QList< QgsMapLayerType > &acceptableTypes = QList< QgsMapLayerType >(),
                                QWidget *parent SIP_TRANSFERTHIS = nullptr,
                                Qt::WindowFlags fl = Qt::WindowFlags() );

    void setNonLayerItems( const QList< QgsProviderSublayerModel::NonLayerItem > &items );

    ~QgsProviderSublayersDialog() override;

    QList< QgsProviderSublayerDetails > selectedLayers() const;
    QList< QgsProviderSublayerModel::NonLayerItem > selectedNonLayerItems() const;
    QString groupName() const;

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
    QString mFilePath;
    bool mBlockSelectionChanges = false;

};

#endif // QGSPROVIDERSUBLAYERSDIALOG_H
