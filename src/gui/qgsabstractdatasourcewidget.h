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
#include "qgis.h"
#include "qgis_gui.h"

#include "qgsproviderregistry.h"
#include "qgsguiutils.h"
#include <QDialog>
#include <QDialogButtonBox>

class QgsMapCanvas;

/** \ingroup gui
 * \brief  Abstract base Data Source Widget to create connections and add layers
 * This class provides common functionality and the interface for all
 * source select dialogs used by data providers to configure data sources
 * and add layers.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsAbstractDataSourceWidget : public QDialog
{
    Q_OBJECT

  public:

    //! Destructor
    ~QgsAbstractDataSourceWidget() = default;

    /** Store a pointer to the map canvas to retrieve extent and CRS
     * Used to select an appropriate CRS and possibly to retrieve data only in the current extent
     */
    void setMapCanvas( const QgsMapCanvas *mapCanvas );


  public slots:

    /** Triggered when the provider's connections need to be refreshed
     * The default implementation does nothing
     */
    virtual void refresh() {}

    /** Triggered when the add button is clicked, the add layer signal is emitted
     * Concrete classes should implement the right behavior depending on the layer
     * being added.
     */
    virtual void addButtonClicked() { }

    /** Triggered when the dialog is accepted, call addButtonClicked() and
     * emit the accepted() signal
     */
    virtual void okButtonClicked();

  signals:

    /** Emitted when the provider's connections have changed
     * This signal is normally forwarded the app and used to refresh browser items
     */
    void connectionsChanged();

    //! Emitted when a DB layer has been selected for addition
    void addDatabaseLayers( const QStringList &paths, const QString &providerKey );

    //! Emitted when a raster layer has been selected for addition
    void addRasterLayer( const QString &rasterLayerPath, const QString &baseName, const QString &providerKey );

    /**
     * Emitted when a vector layer has been selected for addition.
     *
     * If \a providerKey is not specified, the default provider key associated with the source
     * will be used.
     */
    void addVectorLayer( const QString &uri, const QString &layerName, const QString &providerKey = QString() );

    /** Emitted when one or more OGR supported layers are selected for addition
     * \param layerList list of layers protocol URIs
     * \param encoding encoding
     * \param dataSourceType string (can be "file" or "database")
     */
    void addVectorLayers( const QStringList &layerList, const QString &encoding, const QString &dataSourceType );

    /** Emitted when a layer needs to be replaced
     * \param oldId old layer ID
     * \param source URI of the layer
     * \param name of the layer
     * \param provider key
     */
    void replaceVectorLayer( const QString &oldId, const QString &source, const QString &name, const QString &provider );


    //! Emitted when a progress dialog is shown by the provider dialog
    void progress( int, int );

    //! Emitted when a progress dialog is shown by the provider dialog
    void progressMessage( QString message );

    //! Emitted when the ok/add buttons should be enabled/disabled
    void enableButtons( bool enable );


  protected:

    //! Constructor
    QgsAbstractDataSourceWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

    //! Return the widget mode
    QgsProviderRegistry::WidgetMode widgetMode() const;

    //! Return the map canvas (can be null)
    const QgsMapCanvas *mapCanvas() const;

    //! Connect the ok and apply/add buttons to the slots
    void setupButtons( QDialogButtonBox *buttonBox );

    //! Return the add Button
    QPushButton *addButton( ) const { return mAddButton; }

  private:
    QPushButton *mAddButton  = nullptr;
    QgsProviderRegistry::WidgetMode mWidgetMode;
    QgsMapCanvas const *mMapCanvas = nullptr;

};

#endif // QGSABSTRACTDATASOURCEWIDGET_H
