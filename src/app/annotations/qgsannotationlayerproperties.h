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

#include "qgsoptionsdialogbase.h"

#include "ui_qgsannotationlayerpropertiesbase.h"

#include "qgsmaplayerstylemanager.h"

#include "qgsannotationlayer.h"

class QgsMapLayer;
class QgsMapCanvas;
class QgsMessageBar;
class QgsAnnotationLayer;
class QgsMetadataWidget;
class QgsMapLayerConfigWidgetFactory;
class QgsMapLayerConfigWidget;


class QgsAnnotationLayerProperties : public QgsOptionsDialogBase, private Ui::QgsAnnotationLayerPropertiesBase
{
    Q_OBJECT
  public:
    QgsAnnotationLayerProperties( QgsAnnotationLayer *layer, QgsMapCanvas *canvas, QgsMessageBar *messageBar, QWidget *parent = nullptr, Qt::WindowFlags = QgsGuiUtils::ModalDialogFlags );
    ~QgsAnnotationLayerProperties() override;

    void addPropertiesPageFactory( const QgsMapLayerConfigWidgetFactory *factory );

  private slots:
    void apply();
    void onCancel();

    void loadDefaultStyle();
    void saveDefaultStyle();
    void loadStyle();
    void saveStyleAs();
    void aboutToShowStyleMenu();
    void showHelp();
    void urlClicked( const QUrl &url );
    void crsChanged( const QgsCoordinateReferenceSystem &crs );

  protected slots:
    void optionsStackedWidget_CurrentChanged( int index ) override SIP_SKIP ;

  private:
    void syncToLayer();

  private:
    QgsAnnotationLayer *mLayer = nullptr;

    QPushButton *mBtnStyle = nullptr;

    QgsMapCanvas *mMapCanvas = nullptr;

    /**
     * Previous layer style. Used to reset style to previous state if new style
     * was loaded but dialog is canceled.
    */
    QgsMapLayerStyle mOldStyle;

    std::unique_ptr< QgsPaintEffect > mPaintEffect;

    QList<QgsMapLayerConfigWidget *> mConfigWidgets;

    QgsCoordinateReferenceSystem mBackupCrs;

};

#endif // QGSANNOTATIONLAYERPROPERTIES_H
