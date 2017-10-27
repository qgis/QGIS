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

    virtual void addButtonClicked();
%Docstring
 Triggered when the add button is clicked, the add layer signal is emitted
 Concrete classes should implement the right behavior depending on the layer
 being added.
%End

    virtual void okButtonClicked();
%Docstring
 Triggered when the dialog is accepted, call addButtonClicked() and
 emit the accepted() signal
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

    void addVectorLayer( const QString &uri, const QString &layerName, const QString &providerKey = QString() );
%Docstring
 Emitted when a vector layer has been selected for addition.

 If ``providerKey`` is not specified, the default provider key associated with the source
 will be used.
%End

    void addVectorLayers( const QStringList &layerList, const QString &encoding, const QString &dataSourceType );
%Docstring
 Emitted when one or more OGR supported layers are selected for addition
 \param layerList list of layers protocol URIs
 \param encoding encoding
 \param dataSourceType string (can be "file" or "database")
%End

    void replaceVectorLayer( const QString &oldId, const QString &source, const QString &name, const QString &provider );
%Docstring
 Emitted when a layer needs to be replaced
 \param oldId old layer ID
 \param source URI of the layer
 \param name of the layer
 \param provider key
%End


    void progress( int, int );
%Docstring
Emitted when a progress dialog is shown by the provider dialog
%End

    void progressMessage( QString message );
%Docstring
Emitted when a progress dialog is shown by the provider dialog
%End

    void enableButtons( bool enable );
%Docstring
Emitted when the ok/add buttons should be enabled/disabled
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

    void setupButtons( QDialogButtonBox *buttonBox );
%Docstring
Connect the ok and apply/add buttons to the slots
%End

    QPushButton *addButton( ) const;
%Docstring
Return the add Button
 :rtype: QPushButton
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsabstractdatasourcewidget.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
