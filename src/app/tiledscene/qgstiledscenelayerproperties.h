/***************************************************************************
  qgstiledscenelayerproperties.h
  --------------------------------------
  Date                 : June 2023
  Copyright            : (C) 2023 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTILEDSCENELAYERPROPERTIES_H
#define QGSTILEDSCENELAYERPROPERTIES_H

#include "qgslayerpropertiesdialog.h"
#include "ui_qgstiledscenelayerpropertiesbase.h"
#include "qgis_app.h"

class QgsTiledSceneLayer;
class QgsMessageBar;
class QgsMetadataWidget;

class APP_EXPORT QgsTiledSceneLayerProperties : public QgsLayerPropertiesDialog, private Ui::QgsTiledSceneLayerPropertiesBase
{
    Q_OBJECT
  public:
    QgsTiledSceneLayerProperties( QgsTiledSceneLayer *layer, QgsMapCanvas *canvas, QgsMessageBar *messageBar, QWidget *parent = nullptr, Qt::WindowFlags = QgsGuiUtils::ModalDialogFlags );

  protected slots:
    void syncToLayer() FINAL;
    void apply() FINAL;
    void rollback() FINAL;

  private slots:
    void aboutToShowStyleMenu();
    void showHelp();
    void crsChanged( const QgsCoordinateReferenceSystem &crs );

  private:
    QgsTiledSceneLayer *mLayer = nullptr;

    QAction *mActionLoadMetadata = nullptr;
    QAction *mActionSaveMetadataAs = nullptr;

    QgsMetadataWidget *mMetadataWidget = nullptr;

    QgsCoordinateReferenceSystem mBackupCrs;
};

#endif // QGSTILEDSCENELAYERPROPERTIES_H
