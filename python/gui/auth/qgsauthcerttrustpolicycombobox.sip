/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/auth/qgsauthcerttrustpolicycombobox.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsAuthCertTrustPolicyComboBox : QComboBox
{
%Docstring
 Widget for editing the trust policy associated with a Certificate (Intermediate) Authority
%End

%TypeHeaderCode
#include "qgsauthcerttrustpolicycombobox.h"
%End
  public:

    explicit QgsAuthCertTrustPolicyComboBox(
      QWidget *parent /TransferThis/ = 0,
      QgsAuthCertUtils::CertTrustPolicy policy = QgsAuthCertUtils::DefaultTrust,
      QgsAuthCertUtils::CertTrustPolicy defaultpolicy = QgsAuthCertUtils::DefaultTrust );
%Docstring
 Construct a combo box for defining certificate trust policy
 \param parent Parent widget
 \param policy Defined trust policy
 \param defaultpolicy Default trust policy
%End

    QgsAuthCertUtils::CertTrustPolicy trustPolicy();
%Docstring
Get currently set trust policy
 :rtype: QgsAuthCertUtils.CertTrustPolicy
%End

    QgsAuthCertUtils::CertTrustPolicy trustPolicyForIndex( int indx );
%Docstring
Get trust policy for a given index of combobox
 :rtype: QgsAuthCertUtils.CertTrustPolicy
%End

  public slots:
    void setTrustPolicy( QgsAuthCertUtils::CertTrustPolicy policy );
%Docstring
Set current trust policy
%End

    void setDefaultTrustPolicy( QgsAuthCertUtils::CertTrustPolicy defaultpolicy );
%Docstring
Set default trust policy
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/auth/qgsauthcerttrustpolicycombobox.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
