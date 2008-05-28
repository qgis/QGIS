//Added by qt3to4:
#include <QCloseEvent>
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

class QCloseEvent;
class QString;
class QTreeWidget;
class QTreeWidgetItem;
class QDomNode;
class QDomElement;
class QSize;

class QgisInterface;
class QgsGrassProvider;
class QgsGrassBrowser;
class QgsMapCanvas;

#include <QDialog>
#include <QTabWidget>

//
// For experimental filterable list model by Tim
//
#include <QListView>
#include <QLineEdit>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

class QgsGrassToolsTabWidget: public QTabWidget
{
    Q_OBJECT;

public:
    //! Constructor
    QgsGrassToolsTabWidget ( QWidget * parent = 0 );

    //! Destructor
    ~QgsGrassToolsTabWidget();

    QSize iconSize();
};

/*! \class QgsGrassTools
 *  \brief Interface to GRASS modules.
 *
 */
class QgsGrassTools: public QDialog
{
    Q_OBJECT;

public:
    //! Constructor
    QgsGrassTools ( QgisInterface *iface, 
	           QWidget * parent = 0, const char * name = 0, Qt::WFlags f = 0 );

    //! Destructor
    ~QgsGrassTools();

    //! Recursively add sections and modules to the list view
    //  If parent is 0, the modules are added to mModulesListView root
    void addModules ( QTreeWidgetItem *parent, QDomElement &element );

    //! Returns application directory
    QString appDir();

public slots:
    //! Load configuration from file
    bool loadConfig(QString filePath);
    
    //! Close
    void close ( void);

    //! Close event
    void closeEvent(QCloseEvent *e);

    //! Restore window position 
    void restorePosition();

    //! Save window position 
    void saveWindowLocation();

    //! Module in list clicked
    void moduleClicked ( QTreeWidgetItem * item, int column );

    //! Current mapset changed
    void mapsetChanged();

    // Emits regionChanged
    void emitRegionChanged();

    //! Close open tabs with tools
    void closeTools();

    //! Update the regex used to filter the modules list 
    void filterChanged(QString theText);
    //! Run a module when its entry is clicked in the list view
    void listItemClicked(const QModelIndex &theIndex );
    //! Run a module given its module name e.g. r.in.gdal
    void runModule(QString name);
signals:
    void regionChanged();

private:
    //! Pointer to the QGIS interface object
    QgisInterface *mIface;

    //! Pointer to canvas
    QgsMapCanvas *mCanvas;

    //! Browser
    QgsGrassBrowser *mBrowser;

    QgsGrassToolsTabWidget *mTabWidget;
    QTreeWidget *mModulesListView;


    //
    // For experimental model & filtered model by Tim
    //
    QListView * mListView;
    QStandardItemModel * mModelTools;
    QSortFilterProxyModel * mModelProxy;
    QLineEdit * mFilterInput;

};

#endif // QGSGRASSTOOLS_H
