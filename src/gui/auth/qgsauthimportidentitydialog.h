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
#include "ui_qgsauthimportidentitydialog.h"

#include <QSslCertificate>
#include <QSslKey>

#include "qgsauthconfig.h"

/** \ingroup gui
 * Widget for importing an identity certificate/key bundle into the authentication database
 */
class GUI_EXPORT QgsAuthImportIdentityDialog : public QDialog, private Ui::QgsAuthImportIdentityDialog
{
    Q_OBJECT

  public:
    enum IdentityType
    {
      CertIdentity = 0,
    };

    enum BundleTypes
    {
      PkiPaths = 0,
      PkiPkcs12 = 1,
    };

    enum Validity
    {
      Valid,
      Invalid,
      Unknown
    };

    explicit QgsAuthImportIdentityDialog( QgsAuthImportIdentityDialog::IdentityType identitytype,
                                          QWidget *parent = 0 );
    ~QgsAuthImportIdentityDialog();

    QgsAuthImportIdentityDialog::IdentityType identityType() { return mIdentityType; }

    const QPair<QSslCertificate, QSslKey> certBundleToImport() { return mCertBundle; }

    const QgsPkiBundle pkiBundleToImport() { return mPkiBundle; }

  private slots:
    void populateIdentityType();

    void validateIdentity();



    void clearValidation();
    void writeValidation( const QString &msg,
                          QgsAuthImportIdentityDialog::Validity valid,
                          bool append = false );

    // Cert Identity - PkiPaths
    void on_lePkiPathsKeyPass_textChanged( const QString &pass );
    void on_chkPkiPathsPassShow_stateChanged( int state );

    void on_btnPkiPathsCert_clicked();
    void on_btnPkiPathsKey_clicked();

    // Cert Identity - PkiPkcs#12
    void on_lePkiPkcs12KeyPass_textChanged( const QString &pass );
    void on_chkPkiPkcs12PassShow_stateChanged( int state );

    void on_btnPkiPkcs12Bundle_clicked();

  private:
    bool validateBundle();
    bool validatePkiPaths();
    bool validatePkiPkcs12();

    void fileFound( bool found, QWidget *widget );
    QString getOpenFileName( const QString& title, const QString& extfilter );

    QPushButton* okButton();

    QgsAuthImportIdentityDialog::IdentityType  mIdentityType;
    QPair<QSslCertificate, QSslKey> mCertBundle;
    QgsPkiBundle mPkiBundle;
};

#endif // QGSAUTHIMPORTIDENTITYDIALOG_H
