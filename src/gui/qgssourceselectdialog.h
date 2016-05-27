/***************************************************************************
    qgssourceselectdialog.h
    -----------------------
  begin                : Nov 26, 2015
  copyright            : (C) 2015 by Sandro Mani
  email                : smani@sourcepole.ch
 ***************************************************************************/

#ifndef QGSSOURCESELECTDIALOG_H
#define QGSSOURCESELECTDIALOG_H

#include "ui_qgssourceselectdialogbase.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"

class QStandardItemModel;
class QSortFilterProxyModel;
class QgsGenericProjectionSelector;
class QgsOWSConnection;


class GUI_EXPORT QgsSourceSelectDialog : public QDialog, protected Ui::QgsSourceSelectBase
{
    Q_OBJECT

  public:
    enum ServiceType { MapService, FeatureService };

    QgsSourceSelectDialog( const QString& serviceName, ServiceType serviceType, QWidget* parent, Qt::WindowFlags fl );
    ~QgsSourceSelectDialog();
    void setCurrentExtentAndCrs( const QgsRectangle& canvasExtent, const QgsCoordinateReferenceSystem& canvasCrs );

  signals:
    void addLayer( QString uri, QString typeName );
    void connectionsChanged();

  protected:
    QString mServiceName;
    ServiceType mServiceType;
    QgsGenericProjectionSelector* mProjectionSelector;
    //  Available CRS for a server connection, key=typename, value=list("EPSG:XXXX")
    QMap<QString, QStringList> mAvailableCRS;
    QStandardItemModel* mModel;
    QSortFilterProxyModel* mModelProxy;
    QPushButton *mBuildQueryButton;
    QPushButton *mAddButton;
    QButtonGroup* mImageEncodingGroup;
    QgsRectangle mCanvasExtent;
    QgsCoordinateReferenceSystem mCanvasCrs;

    virtual bool connectToService( const QgsOWSConnection& connection ) = 0;
    virtual void buildQuery( const QgsOWSConnection&, const QModelIndex& ) {}
    virtual QString getLayerURI( const QgsOWSConnection& connection,
                                 const QString& layerTitle,
                                 const QString& layerName,
                                 const QString& crs = QString(),
                                 const QString& filter = QString(),
                                 const QgsRectangle& bBox = QgsRectangle() ) const = 0;
    void populateImageEncodings( const QStringList& availableEncodings );
    QString getSelectedImageEncoding() const;


  private:
    void populateConnectionList();

    /** Returns the best suited CRS from a set of authority ids
       1. project CRS if contained in the set
       2. WGS84 if contained in the set
       3. the first entry in the set else
    @return the authority id of the crs or an empty string in case of error*/
    QString getPreferredCrs( const QSet<QString>& crsSet ) const;

  private slots:
    void addEntryToServerList();
    void deleteEntryOfServerList();
    void modifyEntryOfServerList();
    void addButtonClicked();
    void buildQueryButtonClicked();
    void changeCRS();
    void changeCRSFilter();
    void connectToServer();
    void filterChanged( QString text );
    void on_btnLoad_clicked();
    void on_btnSave_clicked();
    void on_cmbConnections_activated( int index );
    void on_buttonBox_helpRequested() const;
    void treeWidgetItemDoubleClicked( const QModelIndex & index );
    void treeWidgetCurrentRowChanged( const QModelIndex & current, const QModelIndex & previous );
};


#endif // QGSSOURCESELECTDIALOG_H
