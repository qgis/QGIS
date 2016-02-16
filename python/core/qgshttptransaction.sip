class QgsHttpTransaction : QObject
{
%TypeHeaderCode
#include <qgshttptransaction.h>
%End
  public:
    /**
     * Constructor.
     */
    QgsHttpTransaction( const QString& uri,
                        const QString& proxyHost = QString(),
                        int     proxyPort = 80,
                        const QString& proxyUser = QString(),
                        const QString& proxyPass = QString(),
                        QNetworkProxy::ProxyType proxyType = QNetworkProxy::NoProxy,
                        const QString& userName = QString(),
                        const QString& password = QString() );

    //! Destructor
    virtual ~QgsHttpTransaction();

    void getAsynchronously();

    //! Gets the response synchronously.  Note that signals will still be emitted
    //! while in this function.

    /*!
        The function returns false if there is an error while getting the response.
        @param[out] respondedContent is replaced with the new content.

        @param[in]  redirections     is used to measure how many http redirections we've been through.
        Clients typically don't need to set this.

        @param postData data to send with the http message. This is only used for HTTP POST. If
        0 then the request is done with HTTP GET.

        @return true in case of success
     */
    bool getSynchronously( QByteArray &respondedContent, int redirections = 0, const QByteArray* postData = 0 );

    QString responseContentType();

    /**
     * If an operation returns 0 (e.g. getSynchronously()), this function
     * returns the text of the error associated with the failure.
     * Interactive users of this provider can then, for example,
     * call a QMessageBox to display the contents.
     */
    QString errorString();

    /** Apply proxy settings from QSettings to a http object
    @return true if proxy settings was applied, false else*/
    static bool applyProxySettings( QHttp& http, const QString& url );

    /** Set the credentials (username and password) */
    void setCredentials( const QString& username, const QString &password );

    /** Returns the network timeout in msec*/
    int networkTimeout() const;
    /** Sets the network timeout in milliseconds*/
    void setNetworkTimeout( int msec );


  public slots:

    void dataStarted( int id );

    void dataHeaderReceived( const QHttpResponseHeader& resp );

    void dataReceived( const QHttpResponseHeader& resp );

    void dataProgress( int done, int total );

    void dataFinished( int id, bool error );

    void transactionFinished( bool error );

    void dataStateChanged( int state );

    void networkTimedOut();

    /** Aborts the current transaction*/
    void abort();

  signals:

    /** Legacy code. This signal is currently not emitted and only kept for API compatibility*/
    void setProgress( int done, int total );

    /** Signal for progress update */
    void dataReadProgress( int theProgress );
    /** Signal for adjusted number of steps*/
    void totalSteps( int theTotalSteps );

    /** \brief emit a signal to be caught by qgisapp and display a msg on status bar */
    void statusChanged( const QString& theStatusQString );
};
