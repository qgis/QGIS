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

    void setDataProviderKey( const QString &key );

  signals:
     void selectedConfigIdChanged( const QString& authcfg );

     void selectedConfigIdRemoved( const QString& authcfg );

  public slots:
    void showMessage( const QString &msg );

    void clearMessage();
};


class QgsAuthConfigUriEdit : QDialog
{
%TypeHeaderCode
#include <qgsauthconfigselect.h>
%End

  public:
    explicit QgsAuthConfigUriEdit( QWidget *parent /TransferThis/ = 0,
                                   const QString &datauri = QString(),
                                   const QString &dataprovider = QString() );
    ~QgsAuthConfigUriEdit();

    void setDataSourceUri( const QString &datauri );

    QString dataSourceUri();

    static bool hasConfigID( const QString &txt );
};
