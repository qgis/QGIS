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

#include "qgsoptionsdialogbase.h"

#include "ui_qgspointcloudlayerpropertiesbase.h"

#include "qgsmaplayerstylemanager.h"
#include <QAbstractTableModel>

#include "qgspointcloudlayer.h"

class QgsMapLayer;
class QgsMapCanvas;
class QgsMessageBar;
class QgsPointCloudLayer;
class QgsMetadataWidget;
class QgsMapLayerConfigWidgetFactory;
class QgsMapLayerConfigWidget;


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
    QVariant headerData( int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole ) const override;
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
    QVariant headerData( int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole ) const override;
  private:

    QgsPointCloudLayer *mLayer = nullptr;
    QString mAttribute;
    QList<int> mClassifications;
};

class QgsPointCloudLayerProperties : public QgsOptionsDialogBase, private Ui::QgsPointCloudLayerPropertiesBase
{
    Q_OBJECT
  public:
    QgsPointCloudLayerProperties( QgsPointCloudLayer *lyr, QgsMapCanvas *canvas, QgsMessageBar *messageBar, QWidget *parent = nullptr, Qt::WindowFlags = QgsGuiUtils::ModalDialogFlags );


    void addPropertiesPageFactory( const QgsMapLayerConfigWidgetFactory *factory );

  private slots:
    void apply();
    void onCancel();

    void loadDefaultStyle();
    void saveDefaultStyle();
    void loadStyle();
    void saveStyleAs();
    void aboutToShowStyleMenu();
    void loadMetadata();
    void saveMetadataAs();
    void saveDefaultMetadata();
    void loadDefaultMetadata();
    void showHelp();
    void urlClicked( const QUrl &url );
    void pbnQueryBuilder_clicked();
    void crsChanged( const QgsCoordinateReferenceSystem &crs );

  protected slots:
    void optionsStackedWidget_CurrentChanged( int index ) override SIP_SKIP ;

  private:
    void syncToLayer();

  private:
    QgsPointCloudLayer *mLayer = nullptr;

    QPushButton *mBtnStyle = nullptr;
    QPushButton *mBtnMetadata = nullptr;
    QAction *mActionLoadMetadata = nullptr;
    QAction *mActionSaveMetadataAs = nullptr;

    QgsMapCanvas *mMapCanvas = nullptr;
    QgsMetadataWidget *mMetadataWidget = nullptr;

    /**
     * Previous layer style. Used to reset style to previous state if new style
     * was loaded but dialog is canceled.
    */
    QgsMapLayerStyle mOldStyle;

    QList<QgsMapLayerConfigWidget *> mConfigWidgets;

    QgsCoordinateReferenceSystem mBackupCrs;

};

#endif // QGSPOINTCLOUDLAYERPROPERTIES_H
