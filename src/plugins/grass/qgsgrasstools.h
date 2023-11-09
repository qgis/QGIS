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

#include "qgsdockwidget.h"

#include "ui_qgsgrasstoolsbase.h"

#include <QSortFilterProxyModel>
#include <QStandardItem>

class QDomElement;
class QSortFilterProxyModel;
class QStandardItem;
class QStandardItemModel;

class QgisInterface;
class QgsMapCanvas;

class QgsGrassRegion;
class QgsGrassToolsTreeFilterProxyModel;

/**
 * \class QgsGrassTools
 *  \brief Interface to GRASS modules.
 *
 */
class QgsGrassTools: public QgsDockWidget, public Ui::QgsGrassToolsBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsGrassTools( QgisInterface *iface,
                   QWidget *parent = nullptr, const char *name = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );


    //! Append item to model or parent
    void appendItem( QStandardItemModel *treeModel, QStandardItem *parent, QStandardItem *item );

    //! Recursively add sections and modules to the list view
    //  If parent is 0, the modules are added to mModulesListView root
    void addModules( QStandardItem *parent, QDomElement &element, QStandardItemModel *treeModel, QStandardItemModel *modulesListModel, bool direct );

    //! Returns application directory
    QString appDir();

    void resetTitle();

  public slots:
    bool loadConfig();

    //! Load configuration from file
    bool loadConfig( QString filePath, QStandardItemModel *treeModel, QStandardItemModel *modulesListModel, bool direct );

    void debugChanged();

    //! Close
    void close( void );

    //! Close event
    void closeEvent( QCloseEvent *e ) override;

    //! Close mapset and save it to project
    void closeMapset();

    //! Current mapset changed
    void mapsetChanged();

    // Emits regionChanged
    void emitRegionChanged();

    //! Close open tabs with tools
    void closeTools();

    //! Update the regex used to filter the modules list (autoconnect to ui)
    void mFilterInput_textChanged( QString text );
    //! Run a module when its entry is clicked in the list view
    void itemClicked( const QModelIndex &index );
    //! Run a module given its module name e.g. r.in.gdal
    void runModule( QString name, bool direct );
    void mDebugButton_clicked();
    void mCloseDebugButton_clicked();
    void mViewModeButton_clicked();

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
    QgisInterface *mIface = nullptr;

    //! Pointer to canvas
    QgsMapCanvas *mCanvas = nullptr;

    QStandardItemModel *mTreeModel = nullptr;
    QgsGrassToolsTreeFilterProxyModel *mTreeModelProxy = nullptr;

    // For model & filtered model by Tim
    QStandardItemModel *mModulesListModel = nullptr;
    QSortFilterProxyModel *mModelProxy = nullptr;

    // Region widget
    QgsGrassRegion *mRegion = nullptr;

    // this was used for direct
    void removeEmptyItems( QStandardItemModel *treeModel );
    void removeEmptyItems( QStandardItem *item );

    // Show (fill) / hide tabs according to direct/indirect mode
    void showTabs();
};


// TODO: searching across the tree is taken from QgsDockBrowserTreeView -> create common base class
class QgsGrassToolsTreeFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:
    explicit QgsGrassToolsTreeFilterProxyModel( QObject *parent );

    void setSourceModel( QAbstractItemModel *sourceModel ) override;

    void setFilter( const QString &filter );

  protected:

    QAbstractItemModel *mModel = nullptr;
    QString mFilter; // filter string provided
    QRegExp mRegExp; // regular expression constructed from filter string

    bool filterAcceptsString( const QString &value ) const;

    // It would be better to apply the filer only to expanded (visible) items, but using mapFromSource() + view here was causing strange errors
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;

    // returns true if at least one ancestor is accepted by filter
    bool filterAcceptsAncestor( const QModelIndex &sourceIndex ) const;

    // returns true if at least one descendant s accepted by filter
    bool filterAcceptsDescendant( const QModelIndex &sourceIndex ) const;

    // filter accepts item name
    bool filterAcceptsItem( const QModelIndex &sourceIndex ) const;
};

#endif // QGSGRASSTOOLS_H
