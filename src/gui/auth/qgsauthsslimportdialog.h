/***************************************************************************
    qgsauthsslimportdialog.h
    ---------------------
    begin                : May 17, 2015
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

/****************************************************************************
**
** Portions of this code were derived from the following...
**
** qt-everywhere-opensource-src-4.8.6/examples/network/
**   securesocketclient/certificateinfo.h (and .cpp)
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QGSAUTHSSLIMPORTDIALOG_H
#define QGSAUTHSSLIMPORTDIALOG_H

#include "ui_qgsauthsslimportdialog.h"

#include <QDialog>
#include <QAbstractSocket>
#include <QSslSocket>

class QPushButton;
class QSslSocket;
class QTimer;


/** \ingroup gui
 * Widget for importing an SSL server certificate exception into the authentication database
 */
class GUI_EXPORT QgsAuthSslImportDialog : public QDialog, private Ui::QgsAuthSslTestDialog
{
    Q_OBJECT
  public:
    /**
     * Construct dialog for importing certificates
     * @param parent
     */
    QgsAuthSslImportDialog( QWidget *parent = nullptr );
    ~QgsAuthSslImportDialog();

  public slots:
    /** Overridden slot of base dialog */
    void accept() override;

  private slots:
    void updateEnabledState();
    void secureConnect();
    void socketStateChanged( QAbstractSocket::SocketState state );
    void socketConnected();
    void socketDisconnected();
    void socketEncrypted();
    void socketError( QAbstractSocket::SocketError err );
    void socketReadyRead();
    void destroySocket();
    void sslErrors( const QList<QSslError> &errors );
    void showCertificateInfo();

    void widgetReadyToSaveChanged( bool cansave );
    void checkCanSave();

    void radioServerImportToggled( bool checked );
    void radioFileImportToggled( bool checked );

    void on_btnCertPath_clicked();
    void clearCertificateConfig();
    void clearStatusCertificateConfig();

  private:
    void loadCertFromFile();

    void appendString( const QString &line );

    QPushButton* saveButton();
    QPushButton* closeButton();
    QString getOpenFileName( const QString& title, const QString& extfilter );

    QSslSocket *mSocket;
    bool mExecErrorsDialog;
    QTimer *mTimer;
    QList<QSslError> mSslErrors;
    QList<QSslCertificate> mTrustedCAs;

    QVBoxLayout *mAuthNotifyLayout;
    QLabel *mAuthNotify;
};

#endif // QGSAUTHSSLIMPORTDIALOG_H
