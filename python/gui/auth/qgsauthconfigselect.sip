class QgsAuthConfigSelect : QWidget
{
%TypeHeaderCode
#include <qgsauthconfigselect.h>
%End

  public:
    explicit QgsAuthConfigSelect( QWidget *parent /TransferThis/ = 0, const QString &dataprovider = QString() );
    ~QgsAuthConfigSelect();

    void setConfigId( const QString& authcfg );
    const QString configId() const;
};
