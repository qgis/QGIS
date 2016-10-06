class QgsCredentials
{
%TypeHeaderCode
#include <qgscredentials.h>
%End

  public:
    //! virtual destructor
    virtual ~QgsCredentials();

    bool get( const QString& realm, QString &username /In,Out/, QString &password /In,Out/, const QString& message = QString::null );
    void put( const QString& realm, const QString& username, const QString& password );

    bool getMasterPassword( QString &password /In,Out/, bool stored = false );

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
    virtual bool request( const QString& realm, QString &username /In,Out/, QString &password /In,Out/, const QString& message = QString::null ) = 0;

    //! request a master password
    virtual bool requestMasterPassword( QString &password /In,Out/, bool stored = false ) = 0;

    //! register instance
    void setInstance( QgsCredentials *theInstance );

  private:
    QgsCredentials( const QgsCredentials& );
};


/**
\brief Default implementation of credentials interface

This class doesn't prompt or return credentials
*/
class QgsCredentialsNone : QObject, QgsCredentials
{
%TypeHeaderCode
#include <qgscredentials.h>
%End

  public:
    QgsCredentialsNone();

  signals:
    //! signals that object will be destroyed and shouldn't be used anymore
    void destroyed();

  protected:
    virtual bool request( const QString& realm, QString &username /In,Out/, QString &password /In,Out/, const QString& message = QString::null );
    virtual bool requestMasterPassword( QString &password /In,Out/, bool stored = false );
};


/**
\brief Implementation of credentials interface for the console

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
    virtual bool request( const QString& realm, QString &username /In,Out/, QString &password /In,Out/, const QString& message = QString::null );
    virtual bool requestMasterPassword( QString &password /In,Out/, bool stored = false );
};
