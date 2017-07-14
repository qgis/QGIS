/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsarcgisservicesourceselect.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsArcGisServiceSourceSelect : QgsSourceSelect, protected Ui::QgsArcGisServiceSourceSelectBase
{
%Docstring
 Base class for listing ArcGis layers available from a remote service.
%End

%TypeHeaderCode
#include "qgsarcgisservicesourceselect.h"
%End
  public:
    enum ServiceType { MapService, FeatureService };

    QgsArcGisServiceSourceSelect( const QString &serviceName, ServiceType serviceType, QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );
%Docstring
Constructor
%End

    ~QgsArcGisServiceSourceSelect();
    void setCurrentExtentAndCrs( const QgsRectangle &canvasExtent, const QgsCoordinateReferenceSystem &canvasCrs );
%Docstring
Sets the current extent and CRS. Used to select an appropriate CRS and possibly to retrieve data only in the current extent
%End

  signals:
    void addLayer( QString uri, QString typeName );
%Docstring
Emitted when a layer is added from the dialog
%End

  protected:

    virtual bool connectToService( const QgsOwsConnection &connection ) = 0;
%Docstring
To be implemented in the child class. Called when a new connection is initiated.
 :rtype: bool
%End
    virtual void buildQuery( const QgsOwsConnection &, const QModelIndex & );
%Docstring
May be implemented in child classes for services which support customized queries.
%End
    virtual QString getLayerURI( const QgsOwsConnection &connection,
                                 const QString &layerTitle,
                                 const QString &layerName,
                                 const QString &crs = QString(),
                                 const QString &filter = QString(),
                                 const QgsRectangle &bBox = QgsRectangle() ) const = 0;
%Docstring
To be implemented in the child class. Constructs an URI for the specified service layer.
 :rtype: str
%End
    void populateImageEncodings( const QStringList &availableEncodings );
%Docstring
Updates the UI for the list of available image encodings from the specified list.
%End
    QString getSelectedImageEncoding() const;
%Docstring
Returns the selected image encoding.
 :rtype: str
%End

  public slots:

    virtual void refresh( );

%Docstring
Triggered when the provider's connections need to be refreshed
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsarcgisservicesourceselect.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
