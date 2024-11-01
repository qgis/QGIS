/***************************************************************************
    qgsautheditorwidgets.h
    ---------------------
    begin                : April 26, 2015
    copyright            : (C) 2015 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAUTHEDITORWIDGETS_H
#define QGSAUTHEDITORWIDGETS_H

#include <QWidget>
#include "qgis_sip.h"
#include "ui_qgsautheditorwidgets.h"
#include "ui_qgsauthmethodplugins.h"
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \brief Dialog for viewing available authentication method plugins
 */
class GUI_EXPORT QgsAuthMethodPlugins : public QDialog, private Ui::QgsAuthMethodPlugins
{
    Q_OBJECT

  public:
    /**
     * Construct a dialog for viewing available authentication method plugins
     * \param parent Parent widget
     */
    explicit QgsAuthMethodPlugins( QWidget *parent SIP_TRANSFERTHIS = nullptr );

  private slots:
    void populateTable();

  private:
    void setupTable();

    QVBoxLayout *mAuthNotifyLayout = nullptr;
    QLabel *mAuthNotify = nullptr;
};


/**
 * \ingroup gui
 * \brief Wrapper widget for available authentication editors
 */
class GUI_EXPORT QgsAuthEditorWidgets : public QWidget, private Ui::QgsAuthEditors
{
    Q_OBJECT

  public:
    /**
     * Construct a widget to contain various authentication editors
     * \param parent Parent widget
     */
    explicit QgsAuthEditorWidgets( QWidget *parent SIP_TRANSFERTHIS = nullptr );

  private slots:
    void btnCertManager_clicked();
    void btnAuthPlugins_clicked();

    //! Import authentication configurations from a XML file
    void importAuthenticationConfigs();

    //! Exports selected authentication configurations to a XML file
    void exportSelectedAuthenticationConfigs();

    //! Sets the cached master password (and verifies it if its hash is in authentication database)
    void setMasterPassword();

    //! Clear the currently cached master password (not its hash in database)
    void clearCachedMasterPassword();

    //! Reset the cached master password, updating its hash in authentication database and resetting all existing configs to use it
    void resetMasterPassword();

    //! Clear all cached authentication configs for session
    void clearCachedAuthenticationConfigs();

    //! Remove all authentication configs
    void removeAuthenticationConfigs();

    //! Completely clear out the authentication database (configs and master password)
    void eraseAuthenticationDatabase();

    //! Relay messages to widget's messagebar
    void authMessageLog( const QString &message, const QString &authtag, Qgis::MessageLevel level );

    //! Remove master password from wallet
    void passwordHelperDelete();

    //! Store master password into the wallet
    void passwordHelperSync();

    //! Toggle password helper (enable/disable)
    void passwordHelperEnableTriggered();

    //! Toggle password helper logging (enable/disable)
    void passwordHelperLoggingEnableTriggered();

  private:
    void setupUtilitiesMenu();

    QgsMessageBar *messageBar();

    QMenu *mAuthUtilitiesMenu = nullptr;
    QAction *mActionExportSelectedAuthenticationConfigs = nullptr;
    QAction *mActionImportAuthenticationConfigs = nullptr;
    QAction *mActionSetMasterPassword = nullptr;
    QAction *mActionClearCachedMasterPassword = nullptr;
    QAction *mActionResetMasterPassword = nullptr;
    QAction *mActionClearCachedAuthConfigs = nullptr;
    QAction *mActionRemoveAuthConfigs = nullptr;
    QAction *mActionEraseAuthDatabase = nullptr;
    QAction *mActionPasswordHelperDelete = nullptr;
    QAction *mActionPasswordHelperSync = nullptr;
    QAction *mActionPasswordHelperEnable = nullptr;
    QAction *mActionPasswordHelperLoggingEnable = nullptr;
    QAction *mActionClearAccessCacheNow = nullptr;
    QAction *mActionAutoClearAccessCache = nullptr;
};

#endif // QGSAUTHEDITORWIDGETS_H
