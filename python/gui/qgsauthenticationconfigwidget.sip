class QgsAuthConfigWidget : QDialog
{
%TypeHeaderCode
#include <qgsauthenticationconfigwidget.h>
%End

  public:
    enum Validity
    {
      Valid,
      Invalid,
      Unknown
    };

    explicit QgsAuthConfigWidget( QWidget *parent = 0, const QString& authid = QString() );
    ~QgsAuthConfigWidget();

    const QString configId() const;

  signals:
    void authenticationConfigStored( const QString& authid );
    void authenticationConfigUpdated( const QString& authid );

};
