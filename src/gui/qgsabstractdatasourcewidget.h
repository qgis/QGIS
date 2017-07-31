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

  signals:

    /** Emitted when the provider's connections have changed
     * This signal is normally forwarded the app and used to refresh browser items
     */
    void connectionsChanged();

    //! Emitted when a DB layer has been selected for addition
    void addDatabaseLayers( const QStringList &paths, const QString &providerKey );

    //! Emitted when a raster layer has been selected for addition
    void addRasterLayer( const QString &rasterLayerPath, const QString &baseName, const QString &providerKey );

    //! Emitted when a vector layer has been selected for addition
    void addVectorLayer( const QString &uri, const QString &layerName );

    //! Emitted when a progress dialog is shown by the provider dialog
    void progress( int, int );

    //! Emitted when a progress dialog is shown by the provider dialog
    void progressMessage( QString message );

  protected:

    //! Constructor
    QgsAbstractDataSourceWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

    //! Return the widget mode
    QgsProviderRegistry::WidgetMode widgetMode() const;

    /** Return the map canvas (can be null)
     */
    const QgsMapCanvas *mapCanvas() const;

  private:

    QgsProviderRegistry::WidgetMode mWidgetMode;
    QgsMapCanvas const *mMapCanvas = nullptr;

};

#endif // QGSABSTRACTDATASOURCEWIDGET_H
