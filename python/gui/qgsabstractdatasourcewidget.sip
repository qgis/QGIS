/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsabstractdatasourcewidget.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsAbstractDataSourceWidget : QDialog
{
%Docstring
  Abstract base Data Source Widget to create connections and add layers
 This class provides common functionality and the interface for all
 source select dialogs used by data providers to configure data sources
 and add layers.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsabstractdatasourcewidget.h"
%End
  public:

    ~QgsAbstractDataSourceWidget();
%Docstring
Destructor
%End

    void setMapCanvas( const QgsMapCanvas *mapCanvas );
%Docstring
 Store a pointer to the map canvas to retrieve extent and CRS
 Used to select an appropriate CRS and possibly to retrieve data only in the current extent
%End


  public slots:

    virtual void refresh();
%Docstring
 Triggered when the provider's connections need to be refreshed
 The default implementation does nothing
%End

  signals:

    void connectionsChanged();
%Docstring
 Emitted when the provider's connections have changed
 This signal is normally forwarded the app and used to refresh browser items
%End

    void addDatabaseLayers( const QStringList &paths, const QString &providerKey );
%Docstring
Emitted when a DB layer has been selected for addition
%End

    void addRasterLayer( const QString &rasterLayerPath, const QString &baseName, const QString &providerKey );
%Docstring
Emitted when a raster layer has been selected for addition
%End

    void addVectorLayer( const QString &uri, const QString &layerName );
%Docstring
Emitted when a vector layer has been selected for addition
%End

    void progress( int, int );
%Docstring
Emitted when a progress dialog is shown by the provider dialog
%End

    void progressMessage( QString message );
%Docstring
Emitted when a progress dialog is shown by the provider dialog
%End

  protected:

    QgsAbstractDataSourceWidget( QWidget *parent /TransferThis/ = 0, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );
%Docstring
Constructor
%End

    QgsProviderRegistry::WidgetMode widgetMode() const;
%Docstring
Return the widget mode
 :rtype: QgsProviderRegistry.WidgetMode
%End

    const QgsMapCanvas *mapCanvas() const;
%Docstring
 Return the map canvas (can be null)
 :rtype: QgsMapCanvas
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsabstractdatasourcewidget.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
