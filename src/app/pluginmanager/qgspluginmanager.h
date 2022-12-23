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
#include "qgsguiutils.h"
#include "qgshelp.h"
#include "qgis.h"

class QgsPluginSortFilterProxyModel;
class QgsPythonUtils;
class QgsMessageBar;

const int PLUGMAN_TAB_ALL = 0;
const int PLUGMAN_TAB_INSTALLED = 1;
const int PLUGMAN_TAB_NOT_INSTALLED = 2;
const int PLUGMAN_TAB_UPGRADEABLE = 3;
const int PLUGMAN_TAB_NEW = 4;
const int PLUGMAN_TAB_INVALID = 5;
const int PLUGMAN_TAB_INSTALL_FROM_ZIP = 6;
const int PLUGMAN_TAB_SETTINGS = 7;

/**
 * \brief Plugin manager for browsing, (un)installing and (un)loading plugins
*/
class QgsPluginManager : public QgsOptionsDialogBase, private Ui::QgsPluginManagerBase
{
    Q_OBJECT
  public:
    //! Constructor; set pluginsAreEnabled to false in --noplugins mode
    QgsPluginManager( QWidget *parent = nullptr, bool pluginsAreEnabled = true, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    ~QgsPluginManager() override;

    //! Save pointer to Python utils and enable Python support
    void setPythonUtils( QgsPythonUtils *pythonUtils );

    //! Load selected plugin
    void loadPlugin( const QString &id );

    //! Unload deselected plugin
    void unloadPlugin( const QString &id );

    //! Save plugin enabled/disabled state to QgsSettings
    void savePluginState( QString id, bool state );

    //! Gets metadata of C++ plugins
    void getCppPluginsMetadata();

    //! Create new spacer item for sorting by status in the plugin list view
    QStandardItem *createSpacerItem( const QString &text, const QString &value );

    //! Repopulate the plugin list model
    void reloadModelData();

    //! Fill the html browser widget with plugin details
    void showPluginDetails( QStandardItem *item );

    //! Remove Python plugins from the metadata registry (c++ plugins stay)
    void clearPythonPluginMetadata();

    //! Add a single plugin to the metadata registry
    void addPluginMetadata( const QString &key, const QMap<QString, QString> &metadata );

    //! Returns the metadata of given plugin
    const QMap<QString, QString> *pluginMetadata( const QString &key ) const;

    //! Select one of the vertical tabs programmatically
    void selectTabItem( int idx );

    //! Clear the repository listWidget
    void clearRepositoryList();

    //! Add repository to the repository listWidget
    void addToRepositoryList( const QMap<QString, QString> &repository );

  public slots:
    //! Close the dialog window (called when the "Close" button clicked)
    void reject() override;

    //! Sets tab of the stacked widget (called from the vertical list item)
    void setCurrentTab( int idx );

    //! Update the window title according to the current filters
    void updateWindowTitle() override;

    //! Handle plugin selection
    void currentPluginChanged( const QModelIndex &index );

    //! Load/unload plugin when checkbox state changed
    void pluginItemChanged( QStandardItem *item );

    //! Load/unload plugin by double-click
    void vwPlugins_doubleClicked( const QModelIndex &index );

    //! Handle click in the web view
    void wvDetails_linkClicked( const QUrl &url );

    //! Update the filter when user changes the filter expression
    void leFilter_textChanged( QString text );

    //! Upgrade all upgradeable plugins
    void buttonUpgradeAll_clicked();

    //! Install selected plugin
    void buttonInstall_clicked();

    //! Install selected plugin
    void buttonInstallExperimental_clicked();

    //! Uninstall selected plugin
    void buttonUninstall_clicked();

    /**
     * Enable the Install button if selected path is valid
     * \since QGIS 3.0
     */
    void mZipFileWidget_fileChanged( const QString &filePath );

    /**
     * Install plugin from ZIP file
     * \since QGIS 3.0
     */
    void buttonInstallFromZip_clicked();

    //! Enable/disable buttons according to selected repository
    void treeRepositories_itemSelectionChanged();

    //! Edit selected repository
    void treeRepositories_doubleClicked( const QModelIndex & );

    //! Define new repository connection
    void buttonAddRep_clicked();

    //! Edit selected repository connection
    void buttonEditRep_clicked();

    //! Delete selected repository connection
    void buttonDeleteRep_clicked();

    //! Reload all repositories
    void buttonRefreshRepos_clicked();

    //! Reload plugin metadata registry after allowing/disallowing experimental plugins
    void ckbExperimental_toggled( bool state );

    //! Reload plugin metadata registry after allowing/disallowing deprecated plugins
    void ckbDeprecated_toggled( bool state );

    //! Open help browser
    void showHelp();

    //! Reimplement QgsOptionsDialogBase method to prevent modifying the tab list by signals from the stacked widget
    void optionsStackedWidget_CurrentChanged( int index ) override { Q_UNUSED( index ) };

    //! Only show plugins from selected repository (e.g. for inspection)
    void setRepositoryFilter();

    //! Enable all repositories disabled by "Enable selected repository only"
    void clearRepositoryFilter();

    //! show the given message in the Plugin Manager internal message bar
    void pushMessage( const QString &text, Qgis::MessageLevel level, int duration = -1 );

#ifndef WITH_QTWEBKIT
    //! vote button was clicked
    void submitVote();
#endif

  protected:
    //! Reimplement QgsOptionsDialogBase method as we have a custom window title what would be overwritten by this method
    void showEvent( QShowEvent *e ) override;

  private:
    //! Load translated descriptions. Source strings implemented in external qgspluginmanager_texts.cpp
    void initTabDescriptions();

    //! Returns true if given plugin is enabled in QgsSettings
    bool isPluginEnabled( QString key );

    //! Returns true if there are plugins available for download in the metadata registry
    bool hasAvailablePlugins();

    //! Returns true if there are installed plugins also available for download in the metadata registry
    bool hasReinstallablePlugins();

    //! Returns true if there are upgradeable plugins in metadata the registry
    bool hasUpgradeablePlugins();

    //! Returns true if there are new plugins in the metadata registry
    bool hasNewPlugins();

    //! Returns true if there are plugins in the metadata registry that are newer installed than available
    bool hasNewerPlugins();

    //! Returns true if there are invalid plugins in the metadata registry
    bool hasInvalidPlugins();

    //! send vote
    void sendVote( int pluginId, int vote );

    QStandardItemModel *mModelPlugins = nullptr;

    QgsPluginSortFilterProxyModel *mModelProxy = nullptr;

    QgsPythonUtils *mPythonUtils = nullptr;

    //! true by default; false in --noplugins mode
    bool mPluginsAreEnabled;

    QMap<QString, QString> mTabDescriptions;

    QMap< QString, QMap< QString, QString > > mPlugins;

    QString mCurrentlyDisplayedPlugin;

    QgsMessageBar *msgBar = nullptr;

#ifndef WITH_QTWEBKIT
    int mCurrentPluginId;
#endif
};

#endif
