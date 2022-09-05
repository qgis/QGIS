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

/**
 * \ingroup gui
 * \brief The QgsLayerMetadataSearchWidget class offers layer metadata search and filtering.
 * It is designed to be embedded in the data source manager dialog.
 * \since QGIS 3.28
 */
class GUI_EXPORT QgsLayerMetadataSearchWidget : public QWidget, private Ui::QgsLayerMetadataSearchWidget
{
    Q_OBJECT
  public:

    /**
     * Created a new QgsLayerMetadataSearchWidget.
     * \param mapCanvas optional map canvas
     * \param parent optional parent
     */
    explicit QgsLayerMetadataSearchWidget( const QgsMapCanvas *mapCanvas = nullptr, QWidget *parent = nullptr );

  signals:

    //! Emitted when the close button is clicked.
    void rejected();

    //! Emitted when layers have been selected for addition
    void addLayers( const QList< QgsLayerMetadataProviderResult > &metadataResults );

  public slots:

    //! Updates the extent filter based on the combo box current item \a index.
    void updateExtentFilter( int index );

  private:

    const QgsMapCanvas *mMapCanvas = nullptr;
    QgsLayerMetadataResultsProxyModel *mProxyModel = nullptr;
    bool mIsLoading = false;
    QPushButton *mAddButton = nullptr;


};

#endif // QGSLAYERMETADATASEARCHWIDGET_H
