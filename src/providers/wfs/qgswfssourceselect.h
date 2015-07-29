/***************************************************************************
                              qgswfssourceselect.h
                              -------------------
  begin                : August 25, 2006
  copyright            : (C) 2006 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWFSSOURCESELECT_H
#define QGSWFSSOURCESELECT_H

#include "ui_qgswfssourceselectbase.h"
#include "qgscontexthelp.h"

#include <QItemDelegate>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

class QgsGenericProjectionSelector;
class QgsWFSCapabilities;

class QgsWFSItemDelegate : public QItemDelegate
{
    Q_OBJECT

  public:

    QgsWFSItemDelegate( QObject *parent = 0 ) : QItemDelegate( parent ) { }

    virtual QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

};

class QgsWFSSourceSelect: public QDialog, private Ui::QgsWFSSourceSelectBase
{
    Q_OBJECT

  public:

    QgsWFSSourceSelect( QWidget* parent, Qt::WindowFlags fl, bool embeddedMode = false );
    ~QgsWFSSourceSelect();

  signals:
    void addWfsLayer( QString uri, QString typeName );
    void connectionsChanged();

  private:
    QgsWFSSourceSelect(); //default constructor is forbidden
    QgsGenericProjectionSelector* mProjectionSelector;
    /** Stores the available CRS for a server connections.
     The first string is the typename, the corresponding list
    stores the CRS for the typename in the form 'EPSG:XXXX'*/
    std::map<QString, std::list<QString> > mAvailableCRS;
    QgsWFSCapabilities* mCapabilities;
    QString mUri;            // data source URI
    QgsWFSItemDelegate* mItemDelegate;
    QStandardItemModel* mModel;
    QSortFilterProxyModel* mModelProxy;
    QPushButton *mBuildQueryButton;
    QPushButton *mAddButton;

    void populateConnectionList();

    /** Returns the best suited CRS from a set of authority ids
       1. project CRS if contained in the set
       2. WGS84 if contained in the set
       3. the first entry in the set else
    @return the authority id of the crs or an empty string in case of error*/
    QString getPreferredCrs( const QSet<QString>& crsSet ) const;

  private slots:
    void addEntryToServerList();
    void modifyEntryOfServerList();
    void deleteEntryOfServerList();
    void connectToServer();
    void addLayer();
    void buildQuery( const QModelIndex& index );
    void changeCRS();
    void changeCRSFilter();
    void on_cmbConnections_activated( int index );
    void capabilitiesReplyFinished();
    void on_btnSave_clicked();
    void on_btnLoad_clicked();
    void treeWidgetItemDoubleClicked( const QModelIndex & index );
    void treeWidgetCurrentRowChanged( const QModelIndex & current, const QModelIndex & previous );
    void buildQueryButtonClicked();
    void filterChanged( QString text );

    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

};


#endif
