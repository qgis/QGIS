/***************************************************************************
    qgssslcertificatewidget.h
                             -------------------
    begin                : 2014-09-12
    copyright            : (C) 2014 by Boundless Spatial, Inc.
    web                  : http://boundlessgeo.com
    author               : Larry Shaffer
    email                : larrys at dakotacarto dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSSLCERTIFICATEWIDGET_H
#define QGSSSLCERTIFICATEWIDGET_H

#include <QWidget>

#include "qgssslutils.h"

#include "ui_qgssslcertificatewidget.h"


class GUI_EXPORT QgsSslCertificateWidget : public QWidget, private Ui::QgsSslCertificateWidget
{
    Q_OBJECT

  public:
    enum Validity
    {
      Valid,
      Invalid,
      Unknown
    };

    /** Widget for choosing/inspecting SSL certificate, using different store methods, and testing validity
     */
    explicit QgsSslCertificateWidget( QWidget *parent = 0, const QString& accessUrl = QString( "" ) );
    ~QgsSslCertificateWidget();

    /**
     * @brief Load settings at once, like from QSettings
     * @see QgsSslPkiSettings
     * @note The resource's access url is missing, because it may not be known yet.
     */
    void loadSettings( QgsSslPkiSettings::SslStoreType storeType = QgsSslPkiSettings::QGISStore,
                       const QString& certId = "",
                       const QString& keyId = "",
                       bool needsKeyPath = false,
                       bool needsKeyPhrase = false,
                       const QString& issuerId = "",
                       bool issuerSelf = false );

    /**
     * @brief Load settings at once from existing QgsSslPkiSettings
     * @see QgsSslPkiSettings
     * @note Includes the resource's access url, though it may be empty.
     */
    void loadSslCertSettings( const QgsSslPkiSettings& pki );

    /**
     * @brief Get settings as QgsSslPkiSettings
     * @note Includes the resource's access url, though it may be empty.
     */
    QgsSslPkiSettings toSslCertSettings();

    /**
     * @brief Whether the cert is valid at the time function is accessed.
     * @note This could easily change later on (e.g. expired cert),
     * so it doesn't really help to save it for later
     */
    bool certIsValid();

    /**
     * @see QgsSslPkiSettings
     */
    QgsSslPkiSettings::SslStoreType ssLStoreType();
    /**
     * @see QgsSslPkiSettings
     */
    QString certId() const;
    /**
     * @see QgsSslPkiSettings
     */
    QString keyId() const;
    /**
     * @see QgsSslPkiSettings
     */
    bool needsKeyPath();
    /**
     * @see QgsSslPkiSettings
     */
    bool needsKeyPassphrase();
    /**
     * @see QgsSslPkiSettings
     */
    QString issuerId() const;
    /**
     * @see QgsSslPkiSettings
     */
    bool issuerSelfSigned();
    /**
     * @see QgsSslPkiSettings
     */
    QString accessUrl() const { return mAccessUrl; }

  public slots:
    void setSslStoreType( QgsSslPkiSettings::SslStoreType storeType );
    void setCertId( const QString& id );
    void setKeyId( const QString& id );
    void setNeedsKeyPath( bool needsPath );
    void setNeedsKeyPassphrase( bool needsPass );
    void setIssuerId( const QString& id );
    void setIssuerSelfSigned( bool selfSigned );
    void setAccessUrl( const QString& url ) { mAccessUrl = url; }

    /**
     * @brief Validate certificate and private key combination.
     * @note This is just lightweight validation, and does not traverse the issuer certificate chain
     * @see QSslCertificate.isValid()
     */
    void validateCert();

  private slots:
    void clearMessage();
    void writeMessage( const QString& msg, Validity valid = Unknown );

    void clearCert();

    void clearCertId();
    void clearKeyId();
    void clearKeyHasPassPhrase();
    void clearIssuerId();
    void clearIssuerSelfSigned();

    // QGIS user-local store
    void openQgisStoreDir();
    void populateQgisCertsMenu();
    void populateQgisKeysMenu();
    void populateQgisIssuersMenu();
    void qgisCertsMenuTriggered( QAction * act );
    void qgisKeysMenuTriggered( QAction * act );
    void qgisIssuersMenuTriggered( QAction * act );

  private:
    void fileFound( bool found, QWidget * widget );
    QString getOpenFileName( const QString& path );

    QAction * clearAction();
    QAction * selectAction();

    QgsSslPkiSettings::SslStoreType mStoreType;
    bool mCertValid;
    QString mAccessUrl;

    // QGIS user-local store
    QMenu * mQgisCertsMenu;
    QMenu * mQgisKeysMenu;
    QMenu * mQgisIssuersMenu;
};

#endif // QGSSSLCERTIFICATEWIDGET_H
