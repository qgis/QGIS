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
#include "qgis_app.h"

class QgsMapLayer;
class QgsMapCanvas;
class QgsMessageBar;
class QgsAnnotationLayer;
class QgsMetadataWidget;
class QgsMapLayerConfigWidgetFactory;
class QgsMapLayerConfigWidget;

class APP_EXPORT QgsAnnotationLayerProperties : public QgsLayerPropertiesDialog, private Ui::QgsAnnotationLayerPropertiesBase
{
    Q_OBJECT
  public:
    QgsAnnotationLayerProperties( QgsAnnotationLayer *layer, QgsMapCanvas *canvas, QgsMessageBar *messageBar, QWidget *parent = nullptr, Qt::WindowFlags = QgsGuiUtils::ModalDialogFlags );
    ~QgsAnnotationLayerProperties() override;

  private slots:
    void apply() FINAL;
    void rollback() FINAL;

    void aboutToShowStyleMenu();
    void showHelp();
    void crsChanged( const QgsCoordinateReferenceSystem &crs );

  private:
    void syncToLayer() FINAL;

  private:
    QgsAnnotationLayer *mLayer = nullptr;

    QPushButton *mBtnStyle = nullptr;

    std::unique_ptr<QgsPaintEffect> mPaintEffect;

    QgsCoordinateReferenceSystem mBackupCrs;
};

#endif // QGSANNOTATIONLAYERPROPERTIES_H
