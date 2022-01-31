/***************************************************************************
  qgsogrdbsourceselect.h - QgsOgrDbSourceSelect

 ---------------------
 begin                : 5.9.2017
 copyright            : (C) 2017 by Alessandro Pasotti
 based on work by     : (C) 2008 by Sandro Furieri for spatialite source sel.
 email                : a.furieri@lqt.it
 email                : apasotti at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGOGRDBSOURCESELECT_H
#define QGSGOGRDBSOURCESELECT_H

#include "qgsguiutils.h"
#include "qgshelp.h"
#include "qgsabstractdbsourceselect.h"
#include "qgsproviderregistry.h"
#include "qgis_sip.h"

class QPushButton;
class QgsOgrDbTableModel;

///@cond PRIVATE
#define SIP_NO_FILE

/**
 * The QgsOgrDbSourceSelect class is a generic class for DB based OGR
 * source selects.
 *
 */
class QgsOgrDbSourceSelect: public QgsAbstractDbSourceSelect
{
    Q_OBJECT

  public:

    /**
     * Construct a DB Source Select with \a theOgrDriverName specified (i.e. "GPKG", "SQLite" etc.)
     * and \a theName as string for describing the layers managed by the source select (e.g. : "GeoPackage" etc.)
     * The \a extensions is a string dscribing the accepted file extensions (e.g. : "GeoPackage Database (*.gpkg *.GPKG)")
     */
    QgsOgrDbSourceSelect( const QString &theOgrDriverName, const QString &theName, const QString &theExtensions, QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode theWidgetMode = QgsProviderRegistry::WidgetMode::None );

    ~QgsOgrDbSourceSelect() override;

    QString layerURI( const QModelIndex &index );

    //! Populate the connection list combo box
    void populateConnectionList();

    // Store the selected database
    void dbChanged();

    //! Returns the QSettings key name
    const QString ogrDriverName( ) { return mOgrDriverName; }

    //! Returns the name of the managed layers, needs to be translatable
    const QString name( ) { return mName; }

    //! Returns the extensions of the managed layers, needs to be translatable
    const QString extension() { return mExtension; }

    //! Open file selector to add new connection
    static bool newConnection( QWidget *parent );


  public slots:

    //! Triggered when the provider's connections need to be refreshed
    void refresh() override;
    void addButtonClicked() override;

    /**
     * Connects to the database using the stored connection parameters.
     * Once connected, available layers are displayed.
     */
    void btnConnect_clicked();
    //! Opens the create connection dialog to build a new connection
    void btnNew_clicked();
    //! Deletes the selected connection
    void btnDelete_clicked();
    void cbxAllowGeometrylessTables_stateChanged( int );
    void cmbConnections_activated( int );
    void treeWidgetSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
    //!Sets a new regular expression to the model
    void setSearchExpression( const QString &regexp );

    void showHelp();

  protected slots:
    void treeviewClicked( const QModelIndex &index ) override;
    void setSql( const QModelIndex &index ) override;
    void treeviewDoubleClicked( const QModelIndex &index ) override;

  private:
    void setConnectionListPosition();
    //! Model that acts as datasource for mTableTreeWidget
    QgsOgrDbTableModel *mTableModel = nullptr;
    QString mPath;
    QString mOgrDriverName;
    QString mName;
    QString mExtension;
};

///@endcond
#endif // QGSGOGRDBSOURCESELECT_H
