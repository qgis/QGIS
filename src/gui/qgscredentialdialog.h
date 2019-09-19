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

#include "ui_qgscredentialdialog.h"
#include "qgsguiutils.h"
#include "qgscredentials.h"

#include <QString>
#include "qgis_sip.h"
#include "qgis_gui.h"

class QPushButton;

/**
 * \ingroup gui
 * A generic dialog for requesting credentials
 */
class GUI_EXPORT QgsCredentialDialog : public QDialog, public QgsCredentials, private Ui_QgsCredentialDialog
{
    Q_OBJECT
  public:
    //! QgsCredentialDialog constructor
    QgsCredentialDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

#ifndef SIP_RUN
  signals:

    //! \note not available in Python bindings
    void credentialsRequested( const QString &, QString *, QString *, const QString &, bool * );

    //! \note not available in Python bindings
    void credentialsRequestedMasterPassword( QString *, bool, bool * );
#endif

  private slots:
    void requestCredentials( const QString &, QString *, QString *, const QString &, bool * );

    void requestCredentialsMasterPassword( QString *password, bool stored, bool *ok );

    void leMasterPass_textChanged( const QString &pass );
    void leMasterPassVerify_textChanged( const QString &pass );
    void chkbxEraseAuthDb_toggled( bool checked );

  protected:
    bool request( const QString &realm, QString &username SIP_INOUT, QString &password SIP_INOUT, const QString &message = QString() ) override;

    bool requestMasterPassword( QString &password SIP_INOUT, bool stored = false ) override;

  private:
    QPushButton *mOkButton = nullptr;
};

#endif
