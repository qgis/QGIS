/***************************************************************************
  qgsvectortilelayerproperties.h
  --------------------------------------
  Date                 : May 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORTILELAYERPROPERTIES_H
#define QGSVECTORTILELAYERPROPERTIES_H

#include "ui_qgsvectortilelayerpropertiesbase.h"
#include "qgslayerpropertiesdialog.h"

class QgsMapLayer;
class QgsMapCanvas;
class QgsMessageBar;
class QgsVectorTileBasicLabelingWidget;
class QgsVectorTileBasicRendererWidget;
class QgsVectorTileLayer;
class QgsMetadataWidget;
class QgsProviderSourceWidget;


/**
 * \ingroup gui
 * \class QgsVectorTileLayerProperties
 * \brief Vectortile layer properties dialog
 * \since QGIS 3.28
 */
class GUI_EXPORT QgsVectorTileLayerProperties : public QgsLayerPropertiesDialog, private Ui::QgsVectorTileLayerPropertiesBase
{
    Q_OBJECT
  public:
    //! Constructor
    QgsVectorTileLayerProperties( QgsVectorTileLayer *lyr, QgsMapCanvas *canvas, QgsMessageBar *messageBar, QWidget *parent = nullptr, Qt::WindowFlags = QgsGuiUtils::ModalDialogFlags );

    /**
     * Saves the default style when appropriate button is pressed
     *
     * \deprecated QGIS 3.40. Use saveStyleAsDefault() instead.
     */
    Q_DECL_DEPRECATED void saveDefaultStyle() SIP_DEPRECATED;

    /**
     * Loads a saved style when appropriate button is pressed
     *
     * \since QGIS 3.30
     */
    void loadStyle();

    /**
     * Saves a style when appriate button is pressed
     *
     * \deprecated QGIS 3.40. Use saveStyleToFile() instead.
     */
    Q_DECL_DEPRECATED void saveStyleAs() SIP_DEPRECATED;

  private slots:
    void apply() FINAL;

    void aboutToShowStyleMenu();
    void showHelp();
    void crsChanged( const QgsCoordinateReferenceSystem &crs );

  private:
    void syncToLayer() FINAL;

  private:
    QgsVectorTileLayer *mLayer = nullptr;

    QgsVectorTileBasicRendererWidget *mRendererWidget = nullptr;
    QgsVectorTileBasicLabelingWidget *mLabelingWidget = nullptr;

    QPushButton *mBtnStyle = nullptr;
    QPushButton *mBtnMetadata = nullptr;
    QAction *mActionLoadMetadata = nullptr;
    QAction *mActionSaveMetadataAs = nullptr;

    QgsMetadataWidget *mMetadataWidget = nullptr;

    QgsProviderSourceWidget *mSourceWidget = nullptr;
};

#endif // QGSVECTORTILELAYERPROPERTIES_H
