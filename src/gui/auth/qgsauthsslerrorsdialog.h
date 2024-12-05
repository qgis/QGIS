/***************************************************************************
    qgsauthsslerrorsdialog.h
    ---------------------
    begin                : May 22, 2015
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


#ifndef QGSAUTHSSLERRORSDIALOG_H
#define QGSAUTHSSLERRORSDIALOG_H

#include <QDialog>
#include <QSslError>
#include "ui_qgsauthsslerrorsdialog.h"
#include "qgis_gui.h"
#include "qgis_sip.h"

class QNetworkReply;
class QPushButton;

/**
 * \ingroup gui
 * \brief Widget for reporting SSL errors and offering an option to store an SSL server exception into the authentication database
 */
class GUI_EXPORT QgsAuthSslErrorsDialog : public QDialog, private Ui::QgsAuthSslErrorsDialog
{
    Q_OBJECT
  public:
    /**
     * Construct a dialog to handle SSL errors and saving SSL server certificate exceptions
     * \param reply Network reply that hand error(s)
     * \param sslErrors SSL errors that occurred
     * \param parent Parent widget
     * \param digest SHA digest of server certificate
     * \param hostport Unique host:port to associate with the server certificate
     */
    QgsAuthSslErrorsDialog( QNetworkReply *reply, const QList<QSslError> &sslErrors, QWidget *parent SIP_TRANSFERTHIS = nullptr, const QString &digest = QString(), const QString &hostport = QString() );

  private slots:
    void loadUnloadCertificate( bool load );

    void showCertificateChainInfo();

    void showCertificateChainCAsInfo();

    void widgetReadyToSaveChanged( bool cansave );
    void checkCanSave();

    void clearCertificateConfig();

    void buttonBox_clicked( QAbstractButton *button );

    void btnChainInfo_clicked();

    void btnChainCAs_clicked();

    void grpbxSslErrors_collapsedStateChanged( bool collapsed );

  private:
    void populateErrorsList();

    QPushButton *ignoreButton();
    QPushButton *abortButton();
    QPushButton *saveButton();

    QSslConfiguration mSslConfiguration;
    QList<QSslError> mSslErrors;
    QString mDigest;
    QString mHostPort;
};

#endif // QGSAUTHSSLERRORSDIALOG_H
