/***************************************************************************
    qgsabstractdatasourcewidget.h  -  base class for source selector widgets
                             -------------------
    begin                : 10 July 2017
    original             : (C) 2017 by Alessandro Pasotti
    email                : apasotti at boundlessgeo dot com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSABSTRACTDATASOURCEWIDGET_H
#define QGSABSTRACTDATASOURCEWIDGET_H

#include "qgis_sip.h"
#include "qgis_gui.h"

#include "qgsproviderguimetadata.h"
#include "qgsproviderregistry.h"
#include "qgsguiutils.h"
#include <QDialog>
#include <QDialogButtonBox>

class QgsMapCanvas;
class QgsBrowserModel;


/**
 * \ingroup gui
 * \brief  Abstract base Data Source Widget to create connections and add layers
 * This class provides common functionality and the interface for all
 * source select dialogs used by data providers to configure data sources
 * and add layers.
 *
 * The implementation is generic enough to handle other layer search and
 * selection widgets.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsAbstractDataSourceWidget : public QDialog
{
    Q_OBJECT

  public:

    /**
     * Sets a browser \a model to use with the widget.
     *
     * \see browserModel()
     * \since QGIS 3.18
     */
    virtual void setBrowserModel( QgsBrowserModel *model );

    /**
     * Returns the dialog map canvas
     * \see setMapCanvas()
     *
     */
    virtual QgsMapCanvas *mapCanvas() {return mMapCanvas; }

    /**
     * Sets the dialog map canvas
     * \see mapCanvas()
     */
    virtual void setMapCanvas( QgsMapCanvas *mapCanvas ) { mMapCanvas = mapCanvas; }

  public slots:

    /**
     * Triggered when the provider's connections need to be refreshed
     * The default implementation does nothing
     */
    virtual void refresh() {}

    /**
     * Triggered when the add button is clicked, the add layer signal is emitted
     * Concrete classes should implement the right behavior depending on the layer
     * being added.
     */
    virtual void addButtonClicked();

    /**
     * Called when this source select widget is being shown in a "new and clean" dialog.
     *
     * The data source manager recycles existing source select widgets but will call
     * this method on every reopening.
     * This should clear any selection that has previously been done.
     *
     * \since QGIS 3.10
     */
    virtual void reset();

  signals:

    /**
     * Emitted when the provider's connections have changed
     * This signal is normally forwarded the app and used to refresh browser items
     */
    void connectionsChanged();

    //! Emitted when a DB layer has been selected for addition
    void addDatabaseLayers( const QStringList &paths, const QString &providerKey );

    //! Emitted when a raster layer has been selected for addition
    void addRasterLayer( const QString &rasterLayerPath, const QString &baseName, const QString &providerKey );

    /**
     * Emitted when one or more GDAL supported layers are selected for addition
     * \param layersList list of layers protocol URIs
     * \since 3.20
     */
    void addRasterLayers( const QStringList &layersList );

    /**
     * Emitted when a vector layer has been selected for addition.
     *
     * If \a providerKey is not specified, the default provider key associated with the source
     * will be used.
     */
    void addVectorLayer( const QString &uri, const QString &layerName, const QString &providerKey = QString() );

    /**
     * Emitted when a mesh layer has been selected for addition.
     * \since QGIS 3.4
     */
    void addMeshLayer( const QString &url, const QString &baseName, const QString &providerKey );

    /**
     * Emitted when a vector tile layer has been selected for addition.
     * \since QGIS 3.14
     */
    void addVectorTileLayer( const QString &url, const QString &baseName );

    /**
     * Emitted when a point cloud layer has been selected for addition.
     * \since QGIS 3.18
     */
    void addPointCloudLayer( const QString &url, const QString &baseName, const QString &providerKey );

    /**
     * Emitted when one or more OGR supported layers are selected for addition
     * \param layerList list of layers protocol URIs
     * \param encoding encoding
     * \param dataSourceType string (can be "file" or "database")
     */
    void addVectorLayers( const QStringList &layerList, const QString &encoding, const QString &dataSourceType );

    /**
     * Emitted when a layer needs to be replaced
     * \param oldId old layer ID
     * \param source URI of the layer
     * \param name of the layer
     * \param provider key
     */
    void replaceVectorLayer( const QString &oldId, const QString &source, const QString &name, const QString &provider );

    /**
     * Emitted when a progress dialog is shown by the provider dialog.
     *
     * \deprecated Since QGIS 3.4 this signal is no longer used. Use QgsProxyProgressTask instead to show progress reports.
     */
    Q_DECL_DEPRECATED void progress( int, int ) SIP_DEPRECATED;

    //! Emitted when a progress dialog is shown by the provider dialog
    void progressMessage( QString message );

    //! Emitted when the ok/add buttons should be enabled/disabled
    void enableButtons( bool enable );

    /**
     * Emitted when a \a message with \a title and \a level must be shown to the user using the parent visible message bar
     * \since QGIS 3.14
     */
    void pushMessage( const QString &title, const QString &message, const Qgis::MessageLevel level = Qgis::MessageLevel::Info );

  protected:

    //! Constructor
    QgsAbstractDataSourceWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

    //! Returns the widget mode
    QgsProviderRegistry::WidgetMode widgetMode() const;

    /**
     * Returns the associated browser model (may be NULLPTR).
     *
     * \since QGIS 3.18
     */
    QgsBrowserModel *browserModel();

    //! Connect the ok and apply/add buttons to the slots
    void setupButtons( QDialogButtonBox *buttonBox );

    //! Returns the add Button
    QPushButton *addButton( ) const { return mAddButton; }

  private:
    QPushButton *mAddButton  = nullptr;
    QgsProviderRegistry::WidgetMode mWidgetMode;
    QgsBrowserModel *mBrowserModel = nullptr;
    QgsMapCanvas *mMapCanvas = nullptr;

};

#endif // QGSABSTRACTDATASOURCEWIDGET_H
