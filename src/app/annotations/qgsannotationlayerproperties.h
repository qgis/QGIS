/***************************************************************************
  qgsannotationlayerproperties.h
  --------------------------------------
  Date                 : September 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSANNOTATIONLAYERPROPERTIES_H
#define QGSANNOTATIONLAYERPROPERTIES_H

#include "qgslayerpropertiesdialog.h"

#include "ui_qgsannotationlayerpropertiesbase.h"

#include "qgsannotationlayer.h"

class QgsMapLayer;
class QgsMapCanvas;
class QgsMessageBar;
class QgsAnnotationLayer;
class QgsMetadataWidget;
class QgsMapLayerConfigWidgetFactory;
class QgsMapLayerConfigWidget;

class QgsAnnotationLayerProperties : public QgsLayerPropertiesDialog, private Ui::QgsAnnotationLayerPropertiesBase
{
    Q_OBJECT
  public:
    QgsAnnotationLayerProperties( QgsAnnotationLayer *layer, QgsMapCanvas *canvas, QgsMessageBar *messageBar, QWidget *parent = nullptr, Qt::WindowFlags = QgsGuiUtils::ModalDialogFlags );
    ~QgsAnnotationLayerProperties() override;

    void addPropertiesPageFactory( const QgsMapLayerConfigWidgetFactory *factory );

  private slots:
    void apply() FINAL;
    void onCancel();

    void aboutToShowStyleMenu();
    void showHelp();
    void urlClicked( const QUrl &url );
    void crsChanged( const QgsCoordinateReferenceSystem &crs );

  private:
    void syncToLayer() FINAL;

  private:
    QgsAnnotationLayer *mLayer = nullptr;

    QPushButton *mBtnStyle = nullptr;

    QgsMapCanvas *mMapCanvas = nullptr;

    std::unique_ptr< QgsPaintEffect > mPaintEffect;

    QList<QgsMapLayerConfigWidget *> mConfigWidgets;

    QgsCoordinateReferenceSystem mBackupCrs;

};

#endif // QGSANNOTATIONLAYERPROPERTIES_H
