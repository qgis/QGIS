/***************************************************************************
  qgspointcloudlayerproperties.h
  --------------------------------------
  Date                 : October 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDLAYERPROPERTIES_H
#define QGSPOINTCLOUDLAYERPROPERTIES_H

#include "qgslayerpropertiesdialog.h"

#include "ui_qgspointcloudlayerpropertiesbase.h"

#include <QAbstractTableModel>

#include "qgis_app.h"
#include "qgspointcloudlayer.h"

class QgsMapLayer;
class QgsMapCanvas;
class QgsMessageBar;
class QgsPointCloudLayer;
class QgsMetadataWidget;
class QgsMapLayerConfigWidgetFactory;
class QgsMapLayerConfigWidget;
class QgsLayerPropertiesGuiUtils;

class QgsPointCloudAttributeStatisticsModel : public QAbstractTableModel
{
    Q_OBJECT

  public:
    enum Columns
    {
      Name,
      Min,
      Max,
      Mean,
      StDev
    };

    QgsPointCloudAttributeStatisticsModel( QgsPointCloudLayer *layer, QObject *parent );
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

  private:
    QgsPointCloudLayer *mLayer = nullptr;
    QgsPointCloudAttributeCollection mAttributes;
};

class QgsPointCloudClassificationStatisticsModel : public QAbstractTableModel
{
    Q_OBJECT

  public:
    enum Columns
    {
      Value,
      Classification,
      Count,
      Percent
    };

    QgsPointCloudClassificationStatisticsModel( QgsPointCloudLayer *layer, const QString &attribute, QObject *parent );
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

  private:
    QgsPointCloudLayer *mLayer = nullptr;
    QString mAttribute;
    QList<int> mClassifications;
};

class APP_EXPORT QgsPointCloudLayerProperties : public QgsLayerPropertiesDialog, private Ui::QgsPointCloudLayerPropertiesBase
{
    Q_OBJECT
  public:
    QgsPointCloudLayerProperties( QgsPointCloudLayer *lyr, QgsMapCanvas *canvas, QgsMessageBar *messageBar, QWidget *parent = nullptr, Qt::WindowFlags = QgsGuiUtils::ModalDialogFlags );

  private slots:
    void apply() FINAL;
    void rollback() FINAL;

    void aboutToShowStyleMenu();
    void showHelp();
    void pbnQueryBuilder_clicked();
    void crsChanged( const QgsCoordinateReferenceSystem &crs );

  private:
    void syncToLayer() FINAL;

  private:
    QgsPointCloudLayer *mLayer = nullptr;

    QAction *mActionLoadMetadata = nullptr;
    QAction *mActionSaveMetadataAs = nullptr;

    QgsMetadataWidget *mMetadataWidget = nullptr;

    QgsCoordinateReferenceSystem mBackupCrs;
};

#endif // QGSPOINTCLOUDLAYERPROPERTIES_H
