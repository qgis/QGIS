class QgsAuthCertTrustPolicyComboBox : QComboBox
{
%TypeHeaderCode
#include <qgsauthcerttrustpolicycombobox.h>
%End

  public:
    explicit QgsAuthCertTrustPolicyComboBox(
        QWidget *parent /TransferThis/ = 0,
        QgsAuthCertUtils::CertTrustPolicy policy = QgsAuthCertUtils::DefaultTrust,
        QgsAuthCertUtils::CertTrustPolicy defaultpolicy =  QgsAuthCertUtils::DefaultTrust );
    ~QgsAuthCertTrustPolicyComboBox();

    QgsAuthCertUtils::CertTrustPolicy trustPolicy();
    QgsAuthCertUtils::CertTrustPolicy trustPolicyForIndex( int indx );

  public slots:
    void setTrustPolicy( QgsAuthCertUtils::CertTrustPolicy policy );

    void setDefaultTrustPolicy( QgsAuthCertUtils::CertTrustPolicy defaultpolicy );
};
