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

    /**
     * Lock the instance against access from multiple threads. This does not really lock access to get/put methds,
     * it will just prevent other threads to lock the instance and continue the execution. When the class is used
     * from non-GUI threads, they should call lock() before the get/put calls to avoid race conditions.
     * @note added in 2.4
     */
    void lock();

    /**
     * Unlock the instance after being locked.
     * @note added in 2.4
     */
    void unlock();

    /**
     * Return pointer to mutex
     * @note added in 2.4
     */
    QMutex *mutex();

  protected:
    QgsCredentials();

    //! request a password
    virtual bool request( QString realm, QString &username /In,Out/, QString &password /In,Out/, QString message = QString::null ) = 0;

    //! register instance
    void setInstance( QgsCredentials *theInstance );

  private:
    QgsCredentials( const QgsCredentials& );
};


/**
\brief Default implementation of credentials interface

This class outputs message to the standard output and retrieves input from
standard input. Therefore it won't be the right choice for apps without
GUI.
*/
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
