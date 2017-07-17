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

    ~QgsAbstractDataSourceWidget( );
%Docstring
Destructor
%End

    QgsProviderRegistry::WidgetMode widgetMode( ) const;
%Docstring
Return the widget mode
 :rtype: QgsProviderRegistry.WidgetMode
%End

    void setCurrentCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
 Set the current CRS
 The CRS is normally the CRS of the map canvas, and it can be used
 by the provider dialog to transform the extent and constraint the service
%End

    void setCurrentExtent( const QgsRectangle &extent );
%Docstring
 Set the current extent
 The extent is normally the extent of the map canvas, and it can be used
 by the provider dialog to constraint the service
%End

    QgsRectangle currentExtent( ) const;
%Docstring
Return the current extent
 :rtype: QgsRectangle
%End

    QgsCoordinateReferenceSystem currentCrs( ) const;
%Docstring
Return the current CRS
 :rtype: QgsCoordinateReferenceSystem
%End

  public slots:

    virtual void refresh( );
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

  protected:

    QgsAbstractDataSourceWidget( QWidget *parent /TransferThis/ = 0, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );
%Docstring
Constructor
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsabstractdatasourcewidget.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
