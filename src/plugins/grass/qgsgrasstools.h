/***************************************************************************
                              qgsgrasstools.h
                             -------------------
    begin                : March, 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGRASSTOOLS_H
#define QGSGRASSTOOLS_H

#include <QDockWidget>

#include "ui_qgsgrasstoolsbase.h"

class QDomElement;
class QSortFilterProxyModel;
class QStandardItem;
class QStandardItemModel;

class QgisInterface;
class QgsMapCanvas;

class QgsGrassRegion;
class QgsGrassToolsTreeFilterProxyModel;

/** \class QgsGrassTools
 *  \brief Interface to GRASS modules.
 *
 */
class QgsGrassTools: public QDockWidget, private Ui::QgsGrassToolsBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsGrassTools( QgisInterface *iface,
                   QWidget * parent = 0, const char * name = 0, Qt::WindowFlags f = 0 );

    //! Destructor
    ~QgsGrassTools();

    //! Append item to model or parent
    void appendItem( QStandardItemModel *treeModel, QStandardItem *parent, QStandardItem *item );

    //! Recursively add sections and modules to the list view
    //  If parent is 0, the modules are added to mModulesListView root
    void addModules( QStandardItem *parent, QDomElement &element, QStandardItemModel *treeModel, QStandardItemModel * modulesListModel, bool direct );

    //! Returns application directory
    QString appDir();

  public slots:
    bool loadConfig();

    //! Load configuration from file
    bool loadConfig( QString filePath, QStandardItemModel *treeModel, QStandardItemModel * modulesListModel, bool direct );

    void debugChanged();

    //! Close
    void close( void );

    //! Close event
    void closeEvent( QCloseEvent *e ) override;

    //! Restore window position
    void restorePosition();

    //! Save window position
    void saveWindowLocation();

    //! Close mapset and save it to project
    void closeMapset();

    //! Current mapset changed
    void mapsetChanged();

    // Emits regionChanged
    void emitRegionChanged();

    //! Close open tabs with tools
    void closeTools();

    //! Update the regex used to filter the modules list (autoconnect to ui)
    void on_mFilterInput_textChanged( QString theText );
    //! Run a module when its entry is clicked in the list view
    void itemClicked( const QModelIndex &theIndex );
    //! Run a module given its module name e.g. r.in.gdal
    void runModule( QString name, bool direct );
    void on_mDebugButton_clicked();
    void on_mCloseDebugButton_clicked();
    void on_mViewModeButton_clicked();

  signals:
    void regionChanged();

  private:
    // data offset to Qt::UserRole for items data
    enum DataOffset
    {
      Label, // original label
      Name, // module name
      Search // search text
    };

    // debug item recursively, return number of errors
    int debug( QStandardItem *item );

    //! Pointer to the QGIS interface object
    QgisInterface *mIface;

    //! Pointer to canvas
    QgsMapCanvas *mCanvas;

    QStandardItemModel * mTreeModel;
    QgsGrassToolsTreeFilterProxyModel * mTreeModelProxy;

    // For model & filtered model by Tim
    QStandardItemModel * mModulesListModel;
    QSortFilterProxyModel * mModelProxy;

    // Region widget
    QgsGrassRegion *mRegion;

    // this was used for direct
    void removeEmptyItems( QStandardItemModel *treeModel );
    void removeEmptyItems( QStandardItem *item );

    // Show (fill) / hide tabs according to direct/indirect mode
    void showTabs();
};

#endif // QGSGRASSTOOLS_H
