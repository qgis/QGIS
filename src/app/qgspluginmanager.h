/***************************************************************************
                          qgspluginmanager.h
               Plugin manager for loading/unloading QGIS plugins
                             -------------------
    begin                : 2004-02-12
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPLUGINMANAGER_H
#define QGSPLUGINMANAGER_H
#include <vector>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QHeaderView>
#include "ui_qgspluginmanagerbase.h"
#include "qgisgui.h"

class QgsPluginItem;
class QgsPythonUtils;
class QTableView;

/*!
 * \brief Plugin manager for loading/unloading plugins
@author Gary Sherman
*/
class QgsPluginManager : public QDialog, private Ui::QgsPluginManagerBase
{
    Q_OBJECT
  public:
    //! Constructor
    QgsPluginManager( QgsPythonUtils* pythonUtils, QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );
    //! Destructor
    ~QgsPluginManager();
    //! Get description of plugins (name, etc)
    void getPluginDescriptions();
    //! Get description of plugins in python (does nothing when python is disabled)
    void getPythonPluginDescriptions();
    //! Unload the selected plugins
    void unload();
    //! Gets the selected plugins
    std::vector<QgsPluginItem> getSelectedPlugins();
    //! Set lstPlugins table GUI
    void setTable();
    //! Resize columns to contents
    void resizeColumnsToContents();
    //! Sort model by column ascending
    void sortModel( int );
    //! Check whether plugin installer is available (and tries to load it if it's disabled)
    bool checkForPluginInstaller();
  public slots:
    //! Enable disable checkbox
    void on_vwPlugins_clicked( const QModelIndex & );
    //! Load selected plugins and close the dialog
    void accept();
    //! Select all plugins by setting their checkbox on
    void selectAll();
    //! Clear all selections by clearing the plugins checkbox
    void clearAll();
    //! Update the filter when user changes the filter expression
    void on_leFilter_textChanged( QString theText );
    //! Show the plugin installer
    void showPluginInstaller();
  private:
    QStandardItemModel *mModelPlugins;
    QSortFilterProxyModel * mModelProxy;

    QgsPythonUtils* mPythonUtils;
};

#endif
