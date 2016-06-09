/**
 * Generic class listing layers available from a remote service.
 */
class QgsSourceSelectDialog : public QDialog //, protected Ui::QgsSourceSelectBase
{
%TypeHeaderCode
#include "qgssourceselectdialog.h"
%End

  public:
    /** Whether the dialog is for a map service or a feature service */
    enum ServiceType { MapService, FeatureService };

    /** Constructor */
    QgsSourceSelectDialog( const QString& serviceName, ServiceType serviceType, QWidget* parent, Qt::WindowFlags fl );
    /** Destructor */
    ~QgsSourceSelectDialog();
    /** Sets the current extent and CRS. Used to select an appropriate CRS and possibly to retrieve data only in the current extent */
    void setCurrentExtentAndCrs( const QgsRectangle& canvasExtent, const QgsCoordinateReferenceSystem& canvasCrs );

  signals:
    /** Emitted when a layer is added from the dialog */
    void addLayer( QString uri, QString typeName );
    /** Emitted when the connections for the service were changed */
    void connectionsChanged();

  protected:
    /** To be implemented in the child class. Called when a new connection is initiated. */
    virtual bool connectToService( const QgsOWSConnection& connection ) = 0;
    /** May be implemented in child classes for services which support customized queries. */
    virtual void buildQuery( const QgsOWSConnection&, const QModelIndex& );
    /** To be implemented in the child class. Constructs an URI for the specified service layer. */
    virtual QString getLayerURI( const QgsOWSConnection& connection,
                                 const QString& layerTitle,
                                 const QString& layerName,
                                 const QString& crs = QString(),
                                 const QString& filter = QString(),
                                 const QgsRectangle& bBox = QgsRectangle() ) const = 0;
    /** Updates the UI for the list of available image encodings from the specified list. */
    void populateImageEncodings( const QStringList& availableEncodings );
    /** Returns the selected image encoding. */
    QString getSelectedImageEncoding() const;
};
