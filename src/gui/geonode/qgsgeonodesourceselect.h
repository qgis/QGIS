/***************************************************************************
                              qgsgeonodesourceselect.h
                              -------------------
    begin                : Feb 2017
    copyright            : (C) 2017 by Muhammad Yarjuna Rohmat, Ismail Sunni
    email                : rohmat at kartoza dot com, ismail at kartoza dot com
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
#include "qgsabstractdatasourcewidget.h"
#include "ui_qgsgeonodesourceselectbase.h"
#include "qgis_gui.h"

class GUI_EXPORT QgsGeonodeItemDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    explicit QgsGeonodeItemDelegate( QObject *parent = nullptr ) : QItemDelegate( parent ) { }
};

class GUI_EXPORT QgsGeoNodeSourceSelect: public QgsAbstractDataSourceWidget, private Ui::QgsGeonodeSourceSelectBase
{
    Q_OBJECT

  public:

    QgsGeoNodeSourceSelect( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

  public slots:

    void addButtonClicked() override;

  private:

    /** Stores the available CRS for a server connections.
     The first string is the typename, the corresponding list
    stores the CRS for the typename in the form 'EPSG:XXXX'*/
    QMap<QString, QStringList > mAvailableCRS;
    QString mUri;            // data source URI
    QgsGeonodeItemDelegate *mItemDelegate = nullptr;
    QStandardItemModel *mModel = nullptr;
    QSortFilterProxyModel *mModelProxy = nullptr;
    QPushButton *mBuildQueryButton = nullptr;
    QModelIndex mSQLIndex;

  private slots:
    void addConnectionsEntryList();
    void modifyConnectionsEntryList();
    void deleteConnectionsEntryList();
    void connectToGeonodeConnection();
    void saveGeonodeConnection();
    void loadGeonodeConnection();
    void filterChanged( const QString &text );
    void treeViewSelectionChanged();
    void populateConnectionList();
    void setConnectionListPosition();
    void showHelp();

};


#endif
