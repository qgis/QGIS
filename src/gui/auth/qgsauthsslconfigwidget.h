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
    /**
     * Construct a widget for editing an SSL server certificate configuration
     * @param parent Parent widget
     * @param cert SSL server certificate object
     * @param hostport Unique host:port to associate with the server certificate
     * @param connectionCAs List of trusted Certificate Authorities objects
     */
    explicit QgsAuthSslConfigWidget( QWidget *parent = nullptr,
                                     const QSslCertificate &cert = QSslCertificate(),
                                     const QString &hostport = QString(),
                                     const QList<QSslCertificate>& connectionCAs = QList<QSslCertificate>() );
    ~QgsAuthSslConfigWidget();

    /** Access to the certificate's group box widget */
    QGroupBox *certificateGroupBox();
    /** Access to the SSL configuration's group box widget */
    QGroupBox *sslConfigGroupBox();

    /** Get the SSL configuration */
    const QgsAuthConfigSslServer sslCustomConfig();

    /** Get the SSL server certificate */
    const QSslCertificate sslCertificate();

    /** Get the host:port to associate with the server certificate */
    const QString sslHost();

    /** Get the SSL protocl used for connections */
    QSsl::SslProtocol sslProtocol();

    /** Get list of the SSL errors (as enums) to be ignored for connections */
    const QList<QSslError::SslError> sslIgnoreErrorEnums();

    /** Get the client's peer verify mode for connections */
    QSslSocket::PeerVerifyMode sslPeerVerifyMode();

    /** Get the client's peer verify depth for connections
     * @note Value of 0 = unlimited
     */
    int sslPeerVerifyDepth();

  public slots:
    /** Enable or disable the custom options widget */
    void enableSslCustomOptions( bool enable );

    // may also load existing config, if found
    /** Set SSl certificate and any associated host:port */
    void setSslCertificate( const QSslCertificate& cert, const QString &hostport = QString() );

    /** Load an existing SSL server configuration */
    void loadSslCustomConfig( const QgsAuthConfigSslServer& config = QgsAuthConfigSslServer() );

    /** Save the current SSL server configuration to the authentication database */
    void saveSslCertConfig();

    /** Clear the current SSL server configuration and disabled it */
    void resetSslCertConfig();

    /** Set the SSL protocol to use in connections */
    void setSslProtocol( QSsl::SslProtocol protocol );

    /** Reset the SSL protocol to use in connections to the default */
    void resetSslProtocol();

    /** Add to SSL errors to ignore for the connection */
    void appendSslIgnoreErrors( const QList<QSslError>& errors );

    /** Set the SSL errors (as enums) to ignore for the connection */
    void setSslIgnoreErrorEnums( const QList<QSslError::SslError>& errorenums );

    /** Set the SSL errors to ignore for the connection */
    void setSslIgnoreErrors( const QList<QSslError>& errors );

    /** Clear the SSL errors to ignore for the connection */
    void resetSslIgnoreErrors();

    /** Set the client's peer verify mode for connections */
    void setSslPeerVerify( QSslSocket::PeerVerifyMode mode, int modedepth );

    /** Reset the client's peer verify mode for connections to default */
    void resetSslPeerVerify();

    /** Set the host of the server */
    void setSslHost( const QString& host );

    /** Set whether the config group box is checkable */
    void setConfigCheckable( bool checkable );

    /** Parse string for host:port */
    void validateHostPortText( const QString &txt );

    /** Verify if the configuration if ready to save */
    bool readyToSave();

  signals:
    /** Emitted when the enabled state of the configuration changes */
    void configEnabledChanged( bool enabled );

    /** Emitted when an certificate of same SHA hash is found in authentication database */
    void certFoundInAuthDatabase( bool found );

    /** Emitted when the validity of the host:port changes */
    void hostPortValidityChanged( bool valid );

    /** Emitted when the configuration can be saved changes */
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

    QTreeWidgetItem *mProtocolItem;
    QComboBox *mProtocolCmbBx;
    QTreeWidgetItem *mIgnoreErrorsItem;
    QTreeWidgetItem *mVerifyModeItem;
    QComboBox *mVerifyPeerCmbBx;
    QTreeWidgetItem *mVerifyDepthItem;
    QSpinBox *mVerifyDepthSpnBx;

    bool mCanSave;

    bool mDisabled;
    QVBoxLayout *mAuthNotifyLayout;
    QLabel *mAuthNotify;
};

//////////////// Embed in dialog ///////////////////

/** \ingroup gui
 * Dialog wrapper of widget for editing an SSL server configuration
 */
class GUI_EXPORT QgsAuthSslConfigDialog : public QDialog
{
    Q_OBJECT

  public:
    /**
     * Construct wrapper dialog for the SSL config widget
     * @param parent Parent widget
     * @param cert SSL server certificate object
     * @param hostport Unique host:port to associate with the server certificate
     */
    explicit QgsAuthSslConfigDialog( QWidget *parent = nullptr,
                                     const QSslCertificate& cert = QSslCertificate(),
                                     const QString &hostport = QString() );
    ~QgsAuthSslConfigDialog();

    /** Access the embedded SSL server configuration widget */
    QgsAuthSslConfigWidget *sslCustomConfigWidget() { return mSslConfigWdgt; }

  public slots:
    /** Overridden base dialog accept slot */
    void accept() override;

  private slots:
    void checkCanSave( bool cansave );

  private:
    QgsAuthSslConfigWidget *mSslConfigWdgt;
    QPushButton *mSaveButton;
};

#endif // QGSAUTHSSLCONFIGWIDGET_H
