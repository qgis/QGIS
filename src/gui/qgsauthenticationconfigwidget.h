/***************************************************************************
    qgsauthenticationconfigwidget.h
    ---------------------
    begin                : October 5, 2014
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

#ifndef QGSAUTHENTICATIONCINFIGWIDGET_H
#define QGSAUTHENTICATIONCINFIGWIDGET_H

#include <QDialog>

#include "ui_qgsauthenticationconfigwidget.h"
#include "qgsauthenticationconfig.h"

/** \ingroup gui
 * Widget for editing an authentication configuration
 * \since 2.8
 */
class GUI_EXPORT QgsAuthConfigWidget : public QDialog, private Ui::QgsAuthConfigWidget
{
    Q_OBJECT

  public:
    enum Validity
    {
      Valid,
      Invalid,
      Unknown
    };

    /**
     * Create a dialog for editing an authentication configuration
     *
     * @param authid  Authentication config id for a existing config in auth database
     */
    explicit QgsAuthConfigWidget( QWidget *parent = 0, const QString& authid = QString() );
    ~QgsAuthConfigWidget();

    /** Authentication config id, updated with generated id when a new config is saved to auth database */
    const QString configId() const { return mAuthId; }

  signals:
    /** Emit generated id when a new config is saved to auth database */
    void authenticationConfigStored( const QString& authid );

    /** Emit current id when an existing config is updated in auth database */
    void authenticationConfigUpdated( const QString& authid );

  private slots:
    void loadConfig();
    void resetConfig();
    void saveConfig();

    void on_btnClear_clicked();
    void clearAll();

    void validateAuth();

    void on_leName_textChanged( const QString& txt );

    // Auth Basic
    void clearAuthBasic();
    void on_leBasicUsername_textChanged( const QString& txt );
    void on_chkBasicPasswordShow_stateChanged( int state );

#ifndef QT_NO_OPENSSL
    void clearPkiMessage( QLineEdit *lineedit );
    void writePkiMessage( QLineEdit *lineedit, const QString& msg, Validity valid = Unknown );

    // Auth PkiPaths
    void clearPkiPathsCert();

    void clearPkiPathsCertId();
    void clearPkiPathsKeyId();
    void clearPkiPathsKeyPassphrase();
    void clearPkiPathsIssuerId();
    void clearPkiPathsIssuerSelfSigned();

    void on_chkPkiPathsPassShow_stateChanged( int state );

    void on_btnPkiPathsCert_clicked();

    void on_btnPkiPathsKey_clicked();

    void on_btnPkiPathsIssuer_clicked();

    // Auth PkiPkcs#12
    void clearPkiPkcs12Bundle();

    void clearPkiPkcs12BundlePath();
    void clearPkiPkcs12KeyPassphrase();
    void clearPkiPkcs12IssuerPath();
    void clearPkiPkcs12IssuerSelfSigned();

    void on_lePkiPkcs12KeyPass_textChanged( const QString &pass );
    void on_chkPkiPkcs12PassShow_stateChanged( int state );

    void on_btnPkiPkcs12Bundle_clicked();

    void on_btnPkiPkcs12Issuer_clicked();
#endif

  private:
    bool validateBasic();

#ifndef QT_NO_OPENSSL
    bool validatePkiPaths();

    bool validatePkiPkcs12();
#endif

    int providerIndexByType( QgsAuthType::ProviderType ptype );

    void fileFound( bool found, QWidget * widget );
    QString getOpenFileName( const QString& title, const QString& extfilter );

    QString mAuthId;
    QVBoxLayout *mAuthNotifyLayout;
    QLabel *mAuthNotify;
};

#endif // QGSAUTHENTICATIONCINFIGWIDGET_H
