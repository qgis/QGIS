/***************************************************************************
  qgslayermetadatasearchwidget.h - QgsLayerMetadataSearchWidget

 ---------------------
 begin                : 1.9.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYERMETADATASEARCHWIDGET_H
#define QGSLAYERMETADATASEARCHWIDGET_H

#include "qgis_gui.h"
#include <QWidget>
#include "ui_qgslayermetadatasearchwidget.h"
#include "qgsfeedback.h"
#include "qgsabstractlayermetadataprovider.h"

class QgsMapCanvas;
class QgsLayerMetadataResultsProxyModel;

class GUI_EXPORT QgsLayerMetadataSearchWidget : public QWidget, private Ui::QgsLayerMetadataSearchWidget
{
    Q_OBJECT
  public:
    explicit QgsLayerMetadataSearchWidget( const QgsMapCanvas *mapCanvas = nullptr, QWidget *parent = nullptr );

  signals:

    void rejected();

    //! Emitted when layers have been selected for addition
    void addLayers( const QList< QgsLayerMetadataProviderResult > &metadataResults );

  public slots:

    void updateExtentFilter( int index );

  private:

    const QgsMapCanvas *mMapCanvas = nullptr;
    QgsLayerMetadataResultsProxyModel *mProxyModel = nullptr;
    bool mIsLoading = false;
    QPushButton *mAddButton = nullptr;


};

#endif // QGSLAYERMETADATASEARCHWIDGET_H
