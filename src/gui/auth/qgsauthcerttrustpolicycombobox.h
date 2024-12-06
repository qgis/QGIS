/***************************************************************************
    qgsauthcerttrustpolicycombobox.h
    ---------------------
    begin                : May 02, 2015
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

#ifndef QGSAUTHCERTTRUSTPOLICYCOMBOBOX_H
#define QGSAUTHCERTTRUSTPOLICYCOMBOBOX_H

#include <QComboBox>
#include "qgis_sip.h"
#include "qgsauthcertutils.h"
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \brief Widget for editing the trust policy associated with a Certificate (Intermediate) Authority
 */
class GUI_EXPORT QgsAuthCertTrustPolicyComboBox : public QComboBox
{
    Q_OBJECT

  public:
    /**
     * Construct a combo box for defining certificate trust policy
     * \param parent Parent widget
     * \param policy Defined trust policy
     * \param defaultpolicy Default trust policy
     */
    explicit QgsAuthCertTrustPolicyComboBox(
      QWidget *parent SIP_TRANSFERTHIS = nullptr,
      QgsAuthCertUtils::CertTrustPolicy policy = QgsAuthCertUtils::DefaultTrust,
      QgsAuthCertUtils::CertTrustPolicy defaultpolicy = QgsAuthCertUtils::DefaultTrust
    );

    //! Gets currently set trust policy
    QgsAuthCertUtils::CertTrustPolicy trustPolicy();

    //! Gets trust policy for a given index of combobox
    QgsAuthCertUtils::CertTrustPolicy trustPolicyForIndex( int indx );

  public slots:
    //! Sets current trust policy
    void setTrustPolicy( QgsAuthCertUtils::CertTrustPolicy policy );

    //! Sets default trust policy
    void setDefaultTrustPolicy( QgsAuthCertUtils::CertTrustPolicy defaultpolicy );

  private slots:
    void highlightCurrentIndex( int indx );

  private:
    const QString defaultTrustText(
      QgsAuthCertUtils::CertTrustPolicy defaultpolicy = QgsAuthCertUtils::DefaultTrust
    );
};

#endif // QGSAUTHCERTTRUSTPOLICYCOMBOBOX_H
