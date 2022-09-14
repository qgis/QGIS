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
#include "qgsabstractdatasourcewidget.h"

class QgsMapCanvas;
class QgsLayerMetadataResultsProxyModel;
class QgsLayerMetadataResultsModel;

/**
 * \ingroup gui
 * \brief The QgsLayerMetadataSearchWidget class offers layer metadata search and filtering.
 * It is designed to be embedded in the data source manager dialog.
 * \since QGIS 3.28
 */
class GUI_EXPORT QgsLayerMetadataSearchWidget : public QgsAbstractDataSourceWidget, private Ui::QgsLayerMetadataSearchWidget
{
    Q_OBJECT
  public:

    /**
     * Creates a new QgsLayerMetadataSearchWidget.
     */
    explicit QgsLayerMetadataSearchWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::WindowFlags(), QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

    void setMapCanvas( QgsMapCanvas *mapCanvas ) override;

  public slots:

    //! Updates the extent filter based on the combo box current item \a index.
    void updateExtentFilter( int index );

    void refresh() override;
    void addButtonClicked() override;
    void reset() override;

  private:

    QgsLayerMetadataResultsProxyModel *mProxyModel = nullptr;
    bool mIsLoading = false;
    QgsLayerMetadataResultsModel *mSourceModel = nullptr;
    unsigned long int mRowCount = 0;

    // QWidget interface
  protected:
    void showEvent( QShowEvent *event ) override;
};

#endif // QGSLAYERMETADATASEARCHWIDGET_H
