class QgsAuthMethodEdit : QWidget
{
%TypeHeaderCode
#include <qgsauthmethodedit.h>
%End

  public:
    virtual bool validateConfig() = 0;

    virtual QgsStringMap configMap() const = 0;

  signals:
    void validityChanged( bool valid );

  public slots:
    virtual void loadConfig( const QgsStringMap &configmap ) = 0;

    virtual void resetConfig() = 0;

    virtual void clearConfig() = 0;

  protected:
    explicit QgsAuthMethodEdit( QWidget *parent = 0 );

    virtual ~QgsAuthMethodEdit();
};
