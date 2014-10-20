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
#include <QMap>
#include <QString>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QHeaderView>
#include "ui_qgspluginmanagerbase.h"
#include "qgsoptionsdialogbase.h"
#include "qgisgui.h"
#include "qgscontexthelp.h"
#include "qgspythonutils.h"
#include "qgspluginsortfilterproxymodel.h"
#include "qgsmessagebar.h"

const int PLUGMAN_TAB_ALL = 0;
const int PLUGMAN_TAB_INSTALLED = 1;
const int PLUGMAN_TAB_NOT_INSTALLED = 2;
const int PLUGMAN_TAB_UPGRADEABLE = 3;
const int PLUGMAN_TAB_NEW = 4;
const int PLUGMAN_TAB_INVALID = 5;

/*!
 * \brief Plugin manager for browsing, (un)installing and (un)loading plugins
@author Gary Sherman
*/
class QgsPluginManager : public QgsOptionsDialogBase, private Ui::QgsPluginManagerBase
{
    Q_OBJECT
  public:
    //! Constructor; set pluginsAreEnabled to false in --noplugins mode
    QgsPluginManager( QWidget *parent = 0, bool pluginsAreEnabled = true, Qt::WindowFlags fl = QgisGui::ModalDialogFlags );

    //! Destructor
    ~QgsPluginManager();

    //! Save pointer to python utils and enable Python support
    void setPythonUtils( QgsPythonUtils* pythonUtils );

    //! Load selected plugin
    void loadPlugin( QString id );

    //! Unload unselected plugin
    void unloadPlugin( QString id );

    //! Save plugin enabled/disabled state to QSettings
    void savePluginState( QString id, bool state );

    //! Get metadata of C++ plugins
    void getCppPluginsMetadata();

    //! Create new spacer item for sorting by status in the plugin list view
    QStandardItem * createSpacerItem( QString text, QString value );

    //! Repopulate the plugin list model
    void reloadModelData();

    //! Fill the html browser widget with plugin details
    void showPluginDetails( QStandardItem * item );

    //! Remove python plugins from the metadata registry (c++ plugins stay)
    void clearPythonPluginMetadata();

    //! Add a single plugin to the metadata registry
    void addPluginMetadata( QString key, QMap<QString, QString> metadata );

    //! Return metadata of given plugin
    const QMap<QString, QString> * pluginMetadata( QString key ) const;

    //! Select one of the vertical tabs programatically
    void selectTabItem( int idx );

    //! Clear the repository listWidget
    void clearRepositoryList();

    //! Add repository to the repository listWidget
    void addToRepositoryList( QMap<QString, QString> repository );

  public slots:
    //! Close the dialog window (called when the "Close" button clicked)
    void reject();

    //! Set tab of the stacked widget (called from the vertical list item)
    void setCurrentTab( int idx );

    //! Update the window title according to the current filters
    void updateWindowTitle();

    //! Handle plugin selection
    void currentPluginChanged( const QModelIndex & theIndex );

    //! Load/unload plugin when checkbox state changed
    void pluginItemChanged( QStandardItem * item );

    //! Display details of inactive item too
    void on_vwPlugins_clicked( const QModelIndex & index );

    //! Load/unload plugin by double click
    void on_vwPlugins_doubleClicked( const QModelIndex & index );

    //! Handle click in the web wiew
    void on_wvDetails_linkClicked( const QUrl & url );

    //! Update the filter when user changes the filter expression
    void on_leFilter_textChanged( QString theText );

    //! Upgrade all upgradeable plugins
    void on_buttonUpgradeAll_clicked();

    //! Install selected plugin
    void on_buttonInstall_clicked();

    //! Uninstall selected plugin
    void on_buttonUninstall_clicked();

    //! Enable/disable buttons according to selected repository
    void on_treeRepositories_itemSelectionChanged();

    //! Edit selected repository
    void on_treeRepositories_doubleClicked( QModelIndex );

    //! Define new repository connection
    void on_buttonAddRep_clicked();

    //! Edit selected repository connection
    void on_buttonEditRep_clicked();

    //! Delete selected repository connection
    void on_buttonDeleteRep_clicked();

    //! Reload all repositories
    void on_buttonRefreshRepos_clicked();

    //! Reload plugin metadata registry after allowing/disallowing experimental plugins
    void on_ckbExperimental_toggled( bool state );

    //! Reload plugin metadata registry after allowing/disallowing deprecated plugins
    void on_ckbDeprecated_toggled( bool state );

    //! Open help browser
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

    //! Reimplement QgsOptionsDialogBase method to prevent modifying the tab list by signals from the stacked widget
    void optionsStackedWidget_CurrentChanged( int indx ) { Q_UNUSED( indx ) }

    //! Only show plugins from selected repository (e.g. for inspection)
    void setRepositoryFilter();

    //! Enable all repositories disabled by "Enable selected repository only"
    void clearRepositoryFilter();

    //! show the given message in the Plugin Manager internal message bar
    void pushMessage( const QString &text, QgsMessageBar::MessageLevel level, int duration = -1 );

  protected:
    //! Reimplement QgsOptionsDialogBase method as we have a custom window title what would be overwritten by this method
    void showEvent( QShowEvent* e );

  private:
    //! Load translated descriptions. Source strings implemented in external qgspluginmanager_texts.cpp
    void initTabDescriptions();

    //! Return true if given plugin is enabled in QSettings
    bool isPluginEnabled( QString key );

    //! Return true if there are plugins available for download in the metadata registry
    bool hasAvailablePlugins();

    //! Return true if there are installed plugins also available for download in the metadata registry
    bool hasReinstallablePlugins();

    //! Return true if there are upgradeable plugins in metadata the registry
    bool hasUpgradeablePlugins();

    //! Return true if there are new plugins in the metadata registry
    bool hasNewPlugins();

    //! Return true if there are plugins in the metadata registry that are newer installed than available
    bool hasNewerPlugins();

    //! Return true if there are invalid plugins in the metadata registry
    bool hasInvalidPlugins();

    QStandardItemModel *mModelPlugins;

    QgsPluginSortFilterProxyModel * mModelProxy;

    QgsPythonUtils* mPythonUtils;

    //! true by default; false in --noplugins mode
    bool mPluginsAreEnabled;

    QMap<QString, QString> mTabDescriptions;

    QMap< QString, QMap< QString, QString > > mPlugins;

    QString mCurrentlyDisplayedPlugin;

    QList<int> mCheckingOnStartIntervals;

    QgsMessageBar *msgBar;
};

#endif
