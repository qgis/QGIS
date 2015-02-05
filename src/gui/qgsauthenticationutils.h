/***************************************************************************
    qgsauthenticationutils.h
    ---------------------
    begin                : October 24, 2014
    copyright            : (C) 2014 by Boundless Spatial, Inc. USA
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

#ifndef QGSAUTHENTICATIONUTILS_H
#define QGSAUTHENTICATIONUTILS_H

#include <QDialog>

#include "ui_qgsmasterpasswordresetdialog.h"

class QgsMessageBar;

/** \ingroup gui
 * \brief Dialog to verify current master password and initiate reset of
 * authentication database with a new password
 * \since 2.8
 */
class GUI_EXPORT QgsMasterPasswordResetDialog : public QDialog, private Ui::QgsMasterPasswordResetDialog
{
    Q_OBJECT

  public:
    explicit QgsMasterPasswordResetDialog( QWidget *parent = 0 );
    ~QgsMasterPasswordResetDialog();

    bool requestMasterPasswordReset( QString *newpass, QString *oldpass, bool *keepbackup );

  private slots:
    void on_leMasterPassCurrent_textChanged( const QString& pass );
    void on_leMasterPassNew_textChanged( const QString& pass );

    void on_chkPassShowCurrent_stateChanged( int state );
    void on_chkPassShowNew_stateChanged( int state );

  private:
    void validatePasswords();

    bool mPassCurOk;
    bool mPassNewOk;
    QVBoxLayout *mAuthNotifyLayout;
    QLabel *mAuthNotify;
};

///////////////////////////////////////////////

/** \ingroup gui
 * \brief Interface to authentication manager utility functions for use by GUI apps,
 * which have a message bar to notify the user
 * \since 2.8
 */
class GUI_EXPORT QgsAuthUtils
{
  public:

    /** Verify the authentication system is active, else notify user */
    static bool isDisabled( QgsMessageBar *msgbar, int timeout = 0 );

    /** Sets the cached master password (and verifies it if its hash is in authentication database) */
    static void setMasterPassword( QgsMessageBar *msgbar, int timeout = 0 );

    /** Clear the currently cached master password (not its hash in database) */
    static void clearCachedMasterPassword( QgsMessageBar *msgbar, int timeout = 0 );

    /** Reset the cached master password, updating its hash in authentication database and reseting all existing configs to use it */
    static void resetMasterPassword( QgsMessageBar *msgbar, int timeout = 0, QWidget *parent = 0 );

    /** Clear all cached authentication configs for session */
    static void clearCachedAuthenticationConfigs( QgsMessageBar *msgbar, int timeout = 0 );

    /** Remove all authentication configs */
    static void removeAuthenticationConfigs( QgsMessageBar *msgbar, int timeout = 0, QWidget *parent = 0 );

    /** Completely clear out the authentication database (configs and master password) */
    static void eraseAuthenticationDatabase( QgsMessageBar *msgbar, int timeout = 0, QWidget *parent = 0 );

};

#endif // QGSAUTHENTICATIONUTILS_H
