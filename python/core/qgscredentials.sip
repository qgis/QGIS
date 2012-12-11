class QgsCredentials
{
%TypeHeaderCode
#include <qgscredentials.h>
%End

  public:
    //! virtual destructor
    virtual ~QgsCredentials();

    bool get( QString realm, QString &username /In,Out/, QString &password /In,Out/, QString message = QString::null );
    void put( QString realm, QString username, QString password );

    //! retrieves instance
    static QgsCredentials *instance();

  protected:
    //! request a password
    virtual bool request( QString realm, QString &username /In,Out/, QString &password /In,Out/, QString message = QString::null ) = 0;

    //! register instance
    void setInstance( QgsCredentials *theInstance );
};


class QgsCredentialsConsole : QObject, QgsCredentials
{
%TypeHeaderCode
#include <qgscredentials.h>
%End

  public:
    QgsCredentialsConsole();

  signals:
    //! signals that object will be destroyed and shouldn't be used anymore
    void destroyed();

  protected:
    virtual bool request( QString realm, QString &username /In,Out/, QString &password /In,Out/, QString message = QString::null );
};
