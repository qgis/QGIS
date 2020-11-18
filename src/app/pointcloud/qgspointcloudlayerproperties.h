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

class QgsMapLayer;
class QgsMapCanvas;
class QgsMessageBar;
class QgsPointCloudLayer;
class QgsMetadataWidget;
class QgsMapLayerConfigWidgetFactory;
class QgsMapLayerConfigWidget;

class QgsPointCloudLayerProperties : public QgsOptionsDialogBase, private Ui::QgsPointCloudLayerPropertiesBase
{
    Q_OBJECT
  public:
    QgsPointCloudLayerProperties( QgsPointCloudLayer *lyr, QgsMapCanvas *canvas, QgsMessageBar *messageBar, QWidget *parent = nullptr, Qt::WindowFlags = QgsGuiUtils::ModalDialogFlags );


    void addPropertiesPageFactory( QgsMapLayerConfigWidgetFactory *factory );

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

};

#endif // QGSPOINTCLOUDLAYERPROPERTIES_H
