/***************************************************************************
                              qgsgeonodesourceselect.h
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

#ifndef QGSGEONODESOURCESELECT_H
#define QGSGEONODESOURCESELECT_H

#include <QItemDelegate>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include "ui_qgsgeonodesourceselectbase.h"

class QgsGeonodeItemDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    explicit QgsGeonodeItemDelegate( QObject *parent = nullptr ) : QItemDelegate( parent ) { }
};

class QgsGeoNodeSourceSelect: public QDialog, private Ui::QgsGeonodeSourceSelectBase
{
    Q_OBJECT

  public:

    QgsGeoNodeSourceSelect( QWidget *parent, Qt::WindowFlags fl, bool embeddedMode = false );
    ~QgsGeoNodeSourceSelect();

  signals:
    void connectionsChanged();

  private:
    QgsGeoNodeSourceSelect(); //default constructor is forbidden

    /** Stores the available CRS for a server connections.
     The first string is the typename, the corresponding list
    stores the CRS for the typename in the form 'EPSG:XXXX'*/
    QMap<QString, QStringList > mAvailableCRS;
    QString mUri;            // data source URI
    QgsGeonodeItemDelegate *mItemDelegate = nullptr;
    QStandardItemModel *mModel = nullptr;
    QSortFilterProxyModel *mModelProxy = nullptr;
    QPushButton *mBuildQueryButton = nullptr;
    QPushButton *mAddButton = nullptr;
    QModelIndex mSQLIndex;

  private slots:
    void addConnectionsEntryList();
    void modifyConnectionsEntryList();
    void deleteConnectionsEntryList();
    void connectToGeonodeConnection();
    void saveGeonodeConnection();
    void loadGeonodeConnection();
    void filterChanged( const QString &text );

    void populateConnectionList();
    void setConnectionListPosition();

};


#endif
