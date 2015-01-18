class QgsAuthConfigSelect : QWidget
{
%TypeHeaderCode
#include <qgsauthenticationconfigselect.h>
%End

  public:
    explicit QgsAuthConfigSelect( QWidget *parent = 0, bool keypasssupported = true );
    ~QgsAuthConfigSelect();

    void setKeyPassSupported( bool supported );
    bool keyPassSupported() const;

    void setConfigId( const QString& authid );
    const QString configId() const;
};
