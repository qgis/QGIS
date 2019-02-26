/***************************************************************************
  qgsdatasourceselectdialog.h - QgsDataSourceSelectDialog

 ---------------------
 begin                : 1.11.2018
 copyright            : (C) 2018 by Alessandro Pasotti
 email                : elpaso@itopen.it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDATASOURCESELECTDIALOG_H
#define QGSDATASOURCESELECTDIALOG_H

#include <QObject>
#include "ui_qgsdatasourceselectdialog.h"

#include "qgis_gui.h"
#include "qgsmaplayer.h"
#include "qgsmimedatautils.h"
#include "qgsbrowsermodel.h"
#include "qgsbrowserproxymodel.h"


/**
 * \ingroup gui
 * The QgsDataSourceSelectDialog class embeds the browser view to
 * select an existing data source.
 *
 * By default any layer type can be chosen, the valid layer
 * type can be restricted by setting a layer type filter with
 * setLayerTypeFilter(layerType) or by activating the filter
 * directly from the constructor.
 *
 * To retrieve the selected data source, uri() can be called and it
 * will return a (possibly invalid) QgsMimeDataUtils::Uri.
 *
 * \since QGIS 3.6
 */
class GUI_EXPORT QgsDataSourceSelectDialog: public QDialog, private Ui::QgsDataSourceSelectDialog
{
    Q_OBJECT

  public:

    /**
     * Constructs a QgsDataSourceSelectDialog, optionally filtering by layer type
     *
     * \param browserModel an existing browser model (typically from app), if NULLPTR an instance will be created
     * \param setFilterByLayerType activates filtering by layer type
     * \param layerType sets the layer type filter, this is in effect only if filtering by layer type is also active
     * \param parent the object
     */
    QgsDataSourceSelectDialog( QgsBrowserModel *browserModel = nullptr,
                               bool setFilterByLayerType = false,
                               const QgsMapLayer::LayerType &layerType = QgsMapLayer::LayerType::VectorLayer,
                               QWidget *parent = nullptr );


    ~QgsDataSourceSelectDialog() override;

    /**
     * Sets layer type filter to \a layerType and activates the filtering
     */
    void setLayerTypeFilter( QgsMapLayer::LayerType layerType );

    /**
     * Returns the (possibly invalid) uri of the selected data source
     */
    QgsMimeDataUtils::Uri uri() const;

    //! Show/hide filter widget
    void showFilterWidget( bool visible );
    //! Sets filter syntax
    void setFilterSyntax( QAction * );
    //! Sets filter case sensitivity
    void setCaseSensitive( bool caseSensitive );
    //! Apply filter to the model
    void setFilter();
    //! Scroll to last selected index and expand it's children
    void showEvent( QShowEvent *e ) override;

  private slots:

    //! Triggered when a layer is selected in the browser
    void onLayerSelected( const QModelIndex &index );

  private:

    //! Refresh the model
    void refreshModel( const QModelIndex &index );

    QgsBrowserProxyModel mBrowserProxyModel;
    std::unique_ptr<QgsBrowserModel> mBrowserModel;
    bool mOwnModel = true;
    QgsMimeDataUtils::Uri mUri;

};

#endif // QGSDATASOURCESELECTDIALOG_H
