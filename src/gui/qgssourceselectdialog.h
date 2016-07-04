/***************************************************************************
    qgssourceselectdialog.h
    ---------------------
    begin                : Nov 26, 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : smani at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
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

/** \ingroup gui
 * Generic class listing layers available from a remote service.
 */
class GUI_EXPORT QgsSourceSelectDialog : public QDialog, protected Ui::QgsSourceSelectBase
{
    Q_OBJECT

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

    /** To be implemented in the child class. Called when a new connection is initiated. */
    virtual bool connectToService( const QgsOWSConnection& connection ) = 0;
    /** May be implemented in child classes for services which support customized queries. */
    virtual void buildQuery( const QgsOWSConnection&, const QModelIndex& ) {}
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
