/***************************************************************************
    qgsauthimportcertdialog.h
    ---------------------
    begin                : April 30, 2015
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

#ifndef QGSAUTHIMPORTCERTDIALOG_H
#define QGSAUTHIMPORTCERTDIALOG_H

#include <QDialog>
#include "qgis_sip.h"
#include "ui_qgsauthimportcertdialog.h"

#include <QSslCertificate>
#include "qgis_gui.h"

class QPushButton;

/**
 * \ingroup gui
 * Widget for importing a certificate into the authentication database
 */
class GUI_EXPORT QgsAuthImportCertDialog : public QDialog, private Ui::QgsAuthImportCertDialog
{
    Q_OBJECT

  public:
    //! Type of filter to apply to dialog
    enum CertFilter
    {
      NoFilter = 1,
      CaFilter = 2,
    };

    //! Type of inputs for certificates
    enum CertInput
    {
      AllInputs = 1,
      FileInput = 2,
      TextInput = 3,
    };

    /**
     * Construct a dialog for importing certificates
     * \param parent Parent widget
     * \param filter Certificate type filter to apply to dialog
     * \param input Type of input(s) for certificates
     */
    explicit QgsAuthImportCertDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr,
                                      QgsAuthImportCertDialog::CertFilter filter = NoFilter,
                                      QgsAuthImportCertDialog::CertInput input = AllInputs );

    //! Gets list of certificate objects to import
    const QList<QSslCertificate> certificatesToImport();

    //! Gets the file path to a certificate to import
    const QString certFileToImport();

    //! Gets certificate text to import
    const QString certTextToImport();

    //! Whether to allow importation of invalid certificates (so trust policy can be overridden)
    bool allowInvalidCerts();

    //! Defined trust policy for imported certificates
    QgsAuthCertUtils::CertTrustPolicy certTrustPolicy();

  private slots:
    void updateGui();

    void validateCertificates();

    void btnImportFile_clicked();

    void chkAllowInvalid_toggled( bool checked );

  private:
    QString getOpenFileName( const QString &title, const QString &extfilter );

    QPushButton *okButton();

    QList<QSslCertificate> mCerts;
    QgsAuthImportCertDialog::CertFilter mFilter;
    QgsAuthImportCertDialog::CertInput mInput;

    bool mDisabled = false;
    QVBoxLayout *mAuthNotifyLayout = nullptr;
    QLabel *mAuthNotify = nullptr;
};

#endif // QGSAUTHIMPORTCERTDIALOG_H
