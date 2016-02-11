/***************************************************************************
                          qgscredentialdialog.h  -  description
                             -------------------
    begin                : February 2010
    copyright            : (C) 2010 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCREDENTIALDIALOG_H
#define QGSCREDENTIALDIALOG_H

#include <ui_qgscredentialdialog.h>
#include <qgisgui.h>
#include "qgscredentials.h"

#include <QString>

class QPushButton;

/** \ingroup gui
 * A generic dialog for requesting credentials
 */
class GUI_EXPORT QgsCredentialDialog : public QDialog, public QgsCredentials, private Ui_QgsCredentialDialog
{
    Q_OBJECT
  public:
    QgsCredentialDialog( QWidget *parent = nullptr, const Qt::WindowFlags& fl = QgisGui::ModalDialogFlags );
    ~QgsCredentialDialog();

  signals:

    //! @note not available in Python bindings
    void credentialsRequested( const QString&, QString *, QString *, const QString&, bool * );

    //! @note not available in Python bindings
    void credentialsRequestedMasterPassword( QString *, bool, bool * );

  private slots:
    void requestCredentials( const QString&, QString *, QString *, const QString&, bool * );

    void requestCredentialsMasterPassword( QString *password, bool stored, bool *ok );

    void on_chkMasterPassShow_stateChanged( int state );
    void on_leMasterPass_textChanged( const QString& pass );
    void on_leMasterPassVerify_textChanged( const QString& pass );
    void on_chkbxEraseAuthDb_toggled( bool checked );

  protected:
    virtual bool request( const QString& realm, QString &username, QString &password, const QString& message = QString::null ) override;

    virtual bool requestMasterPassword( QString &password, bool stored = false ) override;

  private:
    QPushButton *mOkButton;
};

#endif
