/***************************************************************************
    qgsauthmasterpassresetdialog.h
    ---------------------
    begin                : September 10, 2015
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

#ifndef QGSAUTHMASTERPASSWORDRESETDIALOG_H
#define QGSAUTHMASTERPASSWORDRESETDIALOG_H

#include <QDialog>

#include "ui_qgsauthmasterpassresetdialog.h"

class QLabel;
class QVBoxLayout;
class QgsMessageBar;


/** \ingroup gui
 * \brief Dialog to verify current master password and initiate reset of
 * authentication database with a new password
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsMasterPasswordResetDialog : public QDialog, private Ui::QgsMasterPasswordResetDialog
{
    Q_OBJECT

  public:
    explicit QgsMasterPasswordResetDialog( QWidget *parent = nullptr );
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

#endif // QGSAUTHMASTERPASSWORDRESETDIALOG_H
