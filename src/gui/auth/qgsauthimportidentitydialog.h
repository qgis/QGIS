/***************************************************************************
    qgsauthimportidentitydialog.cpp
    ---------------------
    begin                : May 9, 2015
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

#ifndef QGSAUTHIMPORTIDENTITYDIALOG_H
#define QGSAUTHIMPORTIDENTITYDIALOG_H

#include <QDialog>
#include "qgis_sip.h"
#include "ui_qgsauthimportidentitydialog.h"

#include <QSslCertificate>
#include <QSslKey>

#include "qgsauthconfig.h"
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \brief Widget for importing an identity certificate/key bundle into the authentication database
 */
class GUI_EXPORT QgsAuthImportIdentityDialog : public QDialog, private Ui::QgsAuthImportIdentityDialog
{
    Q_OBJECT

  public:
    //! Type of identity being imported
    enum IdentityType
    {
      CertIdentity = 0,
    };

    //! Type of bundles supported
    enum BundleTypes
    {
      PkiPaths = 0,
      PkiPkcs12 = 1,
    };

    //! Type of certificate/bundle validity output
    enum Validity
    {
      Valid,
      Invalid,
      Unknown
    };

    /**
     * Construct a dialog for importing identities
     * \param identitytype Type of the identity to import
     * \param parent Parent widget
     */
    explicit QgsAuthImportIdentityDialog( QgsAuthImportIdentityDialog::IdentityType identitytype, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! Gets identity type
    QgsAuthImportIdentityDialog::IdentityType identityType();

    /**
     * Gets certificate/key bundle to be imported.
     * \note not available in Python bindings
     */
    const QPair<QSslCertificate, QSslKey> certBundleToImport() SIP_SKIP;

    //! Gets certificate/key bundle to be imported as a PKI bundle object
    const QgsPkiBundle pkiBundleToImport() { return mPkiBundle; }

  private slots:
    void populateIdentityType();

    void validateIdentity();


    void clearValidation();
    void writeValidation( const QString &msg, QgsAuthImportIdentityDialog::Validity valid, bool append = false );

    // Cert Identity - PkiPaths
    void lePkiPathsKeyPass_textChanged( const QString &pass );
    void chkPkiPathsPassShow_stateChanged( int state );

    void btnPkiPathsCert_clicked();
    void btnPkiPathsKey_clicked();

    // Cert Identity - PkiPkcs#12
    void lePkiPkcs12KeyPass_textChanged( const QString &pass );
    void chkPkiPkcs12PassShow_stateChanged( int state );

    void btnPkiPkcs12Bundle_clicked();

  private:
    bool validateBundle();
    bool validatePkiPaths();
    bool validatePkiPkcs12();

    void fileFound( bool found, QWidget *widget );
    QString getOpenFileName( const QString &title, const QString &extfilter );

    QPushButton *okButton();

    QgsAuthImportIdentityDialog::IdentityType mIdentityType;
    QPair<QSslCertificate, QSslKey> mCertBundle;
    QgsPkiBundle mPkiBundle;

    bool mDisabled;
    QVBoxLayout *mAuthNotifyLayout = nullptr;
    QLabel *mAuthNotify = nullptr;
};

#endif // QGSAUTHIMPORTIDENTITYDIALOG_H
