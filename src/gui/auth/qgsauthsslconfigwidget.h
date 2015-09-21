/***************************************************************************
    qgsauthsslconfigwidget.h
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

#ifndef QGSAUTHSSLCONFIGWIDGET_H
#define QGSAUTHSSLCONFIGWIDGET_H

#include <QDialog>
#include <QWidget>
#include "ui_qgsauthsslconfigwidget.h"

#include <QSslCertificate>
#include <QSslConfiguration>

#include "qgsauthconfig.h"

class QComboBox;
class QGroupBox;
class QSpinBox;

/** \ingroup gui
 * Widget for editing an SSL server configuration
 */
class GUI_EXPORT QgsAuthSslConfigWidget : public QWidget, private Ui::QgsAuthSslConfigWidget
{
    Q_OBJECT

  public:
    explicit QgsAuthSslConfigWidget( QWidget *parent = 0,
                                     const QSslCertificate &cert = QSslCertificate(),
                                     const QString &hostport = QString(),
                                     const QList<QSslCertificate>& connectionCAs = QList<QSslCertificate>() );
    ~QgsAuthSslConfigWidget();

    QGroupBox *certificateGroupBox() { return grpbxCert; }
    QGroupBox *sslConfigGroupBox() { return grpbxSslConfig; }

    const QgsAuthConfigSslServer sslCustomConfig();

    const QSslCertificate sslCertificate() { return mCert; }

    const QString sslHost() { return leHost->text(); }

    QSsl::SslProtocol sslProtocol();

    const QList<QSslError::SslError> sslIgnoreErrorEnums();

    QSslSocket::PeerVerifyMode sslPeerVerifyMode();

    int sslPeerVerifyDepth();

  public slots:
    void enableSslCustomOptions( bool enable );

    // may also load existing config, if found
    void setSslCertificate( const QSslCertificate& cert, const QString &hostport = QString() );

    void loadSslCustomConfig( const QgsAuthConfigSslServer& config = QgsAuthConfigSslServer() );

    void saveSslCertConfig();

    void resetSslCertConfig();

    void setSslProtocol( QSsl::SslProtocol protocol );

    void resetSslProtocol();

    void appendSslIgnoreErrors( const QList<QSslError>& errors );

    void setSslIgnoreErrorEnums( const QList<QSslError::SslError>& errorenums );

    void setSslIgnoreErrors( const QList<QSslError>& errors );

    void resetSslIgnoreErrors();

    void setSslPeerVerify( QSslSocket::PeerVerifyMode mode, int modedepth );

    void resetSslPeerVerify();

    void setSslHost( const QString& host );

    void setConfigCheckable( bool checkable );

    void validateHostPortText( const QString &txt );

    bool readyToSave();

  signals:
    void configEnabledChanged( bool enabled );
    void certFoundInAuthDatabase( bool found );
    void hostPortValidityChanged( bool valid );
    void readyToSaveChanged( bool cansave );

  private slots:
    void on_btnCertInfo_clicked();

  private:
    enum ConfigType
    {
      ConfigParent = 1000,
      ConfigItem = 1001,
    };

    bool validateHostPort( const QString &txt );

    void setUpSslConfigTree();
    QTreeWidgetItem* addRootItem( const QString& label );

    QSslCertificate mCert;
    QList<QSslCertificate> mConnectionCAs;

    QTreeWidgetItem *mRootItem;
    QTreeWidgetItem *mProtocolItem;
    QComboBox *mProtocolCmbBx;
    QTreeWidgetItem *mIgnoreErrorsItem;
    QTreeWidgetItem *mVerifyModeItem;
    QComboBox *mVerifyPeerCmbBx;
    QTreeWidgetItem *mVerifyDepthItem;
    QSpinBox *mVerifyDepthSpnBx;

    bool mCanSave;
};

//////////////// Embed in dialog ///////////////////

class GUI_EXPORT QgsAuthSslConfigDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit QgsAuthSslConfigDialog( QWidget *parent = 0,
                                     const QSslCertificate& cert = QSslCertificate(),
                                     const QString &hostport = QString() );
    ~QgsAuthSslConfigDialog();

    QgsAuthSslConfigWidget *sslCustomConfigWidget() { return mSslConfigWdgt; }

  public slots:
    void accept();

  private slots:
    void checkCanSave( bool cansave );

  private:
    QgsAuthSslConfigWidget *mSslConfigWdgt;
    QPushButton *mSaveButton;
};

#endif // QGSAUTHSSLCONFIGWIDGET_H
